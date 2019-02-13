#pragma once

#include "database.h"
#include <iostream>
#include <memory>
#include <sqlite3.h>

namespace sqlite_orm {

class sqlite_error_category : public std::error_category {
public:
    const char* name() const noexcept override final
    {
        return "SQLite error";
    }

    std::string message(int c) const override final
    {
        return sqlite3_errstr(c);
    }
};

using sqlite_shared_statement = std::shared_ptr<sqlite3_stmt>;
static sqlite_shared_statement make_sqlite_shared_statement(sqlite3_stmt* ptr = nullptr)
{
    return std::shared_ptr<sqlite3_stmt>(ptr, sqlite3_finalize);
}

static void throw_if_not_sqlite_ok(database* db, int result)
{
    if (result != SQLITE_OK) {
        throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
    }
}
class sqlite3_query : public query {

    sqlite3_stmt* stmt = nullptr;
    // query interface
public:
    sqlite3_query(const std::string& statement)
        : query(statement)
    {
    }
    ~sqlite3_query()
    {
        sqlite3_finalize(stmt);
    };
    std::vector<char> columnBlob(int index) override
    {
        std::vector<char> result;
        auto bytes = static_cast<const char*>(sqlite3_column_blob(stmt, index));
        auto len = sqlite3_column_bytes(stmt, index);
        result.reserve(len);
        result.assign(bytes, bytes + len);
        return result;
    }
    bool isColumnValid(int index) override
    {
        auto type = sqlite3_column_type(stmt, index);
        return type != SQLITE_NULL;
    }

    double columnDouble(int index) override
    {
        return sqlite3_column_double(stmt, index);
    }

    int columnInt(int index) override
    {
        return sqlite3_column_int(stmt, index);
    }
    int64_t columnBigInt(int index) override
    {
        return sqlite3_column_int64(stmt, index);
    }
    std::string columnString(int index) override
    {
        auto result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, index));
        if (result) {
            return { result };
        }
        return {};
    }
    void bindString(database* db, int index, const char* data, int len) override
    {
        throw_if_not_sqlite_ok(db, sqlite3_bind_text(stmt, index + 1, data, static_cast<int>(len), SQLITE_TRANSIENT));
    }
    void bindBlob(database* db, int index, const void* data, size_t len) override
    {
        throw_if_not_sqlite_ok(db, sqlite3_bind_blob(stmt, index + 1, data, static_cast<int>(len), SQLITE_TRANSIENT));
    }
    void bindDouble(database* db, int index, double data) override
    {
        throw_if_not_sqlite_ok(db, sqlite3_bind_double(stmt, index + 1, data));
    }
    void bindInt(database* db, int index, int data) override
    {
        throw_if_not_sqlite_ok(db, sqlite3_bind_int(stmt, index + 1, data));
    }
    void bindBigInt(database* db, int index, int64_t data) override
    {
        throw_if_not_sqlite_ok(db, sqlite3_bind_int64(stmt, index + 1, data));
    }
    void bindNull(database* db, int index) override
    {
        throw_if_not_sqlite_ok(db, sqlite3_bind_null(stmt, index + 1));
    }

    bool prepare(database* db) override
    {
        return db->prepare(this);
    }
    step next(database* db) override
    {
        auto ret = sqlite3_step(stmt);
        switch (ret) {
        case SQLITE_DONE:
            return query::step::done;
        case SQLITE_ROW:
            return query::step::row;
        default:
            return query::step::error;
        }
    }
    void** statement_ptr() override
    {
        return reinterpret_cast<void**>(&stmt);
    }
};

struct sqlite3_database : public database {

    inline const sqlite_error_category& sqlite_error_category_instance() const
    {
        static sqlite_error_category res;
        return res;
    }

public:
    static std::unique_ptr<database> create(const std::string& path)
    {
        return std::make_unique<sqlite3_database>(path);
    }
    static std::string driver_name() { return "sqlite"; }

private:
    static bool g_registered;

private:
    std::string _filename;
    sqlite3* db = nullptr;

public:
    sqlite3_database(const std::string& filename)
        : _filename(filename)
    {
    }
    // database interface
public:
    bool open() override
    {
        auto rc = sqlite3_open(_filename.c_str(), &db);
        return rc == SQLITE_OK;
    }
    void close() override
    {
        sqlite3_close(db);
        db = nullptr;
    }
    std::string last_error_description() const override
    {
        return sqlite3_errstr(last_error_code());
    }

    int last_error_code() const override
    {
        return sqlite3_errcode(db);
    }
    const std::error_category& error_category() const override
    {
        const static auto& category = sqlite_error_category_instance();
        return category;
    }
    bool prepare(query* query) override
    {
        auto stmt_ptr = query->statement_ptr();
        std::cout << query->statement() << std::endl;
        auto stmt_ptr_ptr = reinterpret_cast<sqlite3_stmt**>(stmt_ptr);
        auto prepareResult = sqlite3_prepare_v2(db, query->statement().c_str(), -1, stmt_ptr_ptr, nullptr);
        if (prepareResult == SQLITE_OK) {
            return true;
        }
        return false;
    }

    query::step exec(query* query) override
    {
        std::cout << query->statement() << std::endl;
        auto ret = sqlite3_exec(db, query->statement().c_str(), nullptr, nullptr, nullptr);
        if (ret) {
            return query::step::error;
        }
        return query::step::done;
    }
    static const char* SQLITE_TABLE_NAMES_QUERY;

    std::vector<table_info> get_table_info(const std::string& tableName) override
    {
        std::vector<table_info> res;
        auto query = "PRAGMA table_info('" + tableName + "')";
        auto rc = sqlite3_exec(db,
            query.c_str(),
            [](void* data, int argc, char** argv, char**) -> int {
                auto& res = *(std::vector<table_info>*)data;
                if (argc) {
                    auto index = 0;
                    auto cid = std::atoi(argv[index++]);
                    std::string name = argv[index++];
                    std::string type = argv[index++];
                    bool notnull = !!std::atoi(argv[index++]);
                    std::string dflt_value = argv[index] ? argv[index] : "";
                    index++;
                    auto pk = std::atoi(argv[index++]);
                    res.push_back(table_info { cid, name, type, notnull, dflt_value, pk });
                }
                return 0;
            },
            &res, nullptr);
        if (rc != SQLITE_OK) {
            throw std::system_error(std::error_code(last_error_code(), error_category()));
        }
        return res;
    }

    std::vector<std::string> table_names() override
    {
        std::vector<std::string> tableNames;

        int res = sqlite3_exec(db, SQLITE_TABLE_NAMES_QUERY,
            [](void* data, int argc, char** argv, char* * /*columnName*/) -> int {
                auto& tableNames = *(std::vector<std::string>*)data;
                for (int i = 0; i < argc; i++) {
                    if (argv[i]) {
                        tableNames.push_back(argv[i]);
                    }
                }
                return 0;
            },
            &tableNames, nullptr);

        if (res != SQLITE_OK) {
            throw std::system_error(std::error_code(last_error_code(), error_category()));
        }
        return tableNames;
    }
    bool table_exists(const std::string& table_name) override
    {
        auto res = false;
        std::stringstream ss;
        ss << "SELECT COUNT(*) FROM sqlite_master WHERE type = '"
           << "table"
           << "' AND name = '" << table_name << "'";
        auto query = ss.str();
        auto rc = sqlite3_exec(db,
            query.c_str(),
            [](void* data, int argc, char** argv, char* * /*azColName*/) -> int {
                auto& res = *(bool*)data;
                if (argc) {
                    res = !!std::atoi(argv[0]);
                }
                return 0;
            },
            &res, nullptr);
        if (rc != SQLITE_OK) {
            throw std::system_error(std::error_code(last_error_code(), error_category()));
        }
        return res;
    }

    // database interface
public:
    void begin_transaction() override
    {
        std::stringstream ss;
        ss << "BEGIN TRANSACTION";
        auto query = sqlite3_query(ss.str());
        auto stmt_ptr = query.statement_ptr();
        auto stmt_ptr_ptr = reinterpret_cast<sqlite3_stmt**>(stmt_ptr);
        if (sqlite3_prepare_v2(db, query.statement().c_str(), -1, stmt_ptr_ptr, nullptr) == SQLITE_OK) {
            auto ret = query.next(this);
            if (ret == query::step::done) {
                //  done..
            } else {
                throw std::system_error(std::error_code(last_error_code(), error_category()));
            }
        } else {
            throw std::system_error(std::error_code(last_error_code(), error_category()));
        }
    }
    void commit_transaction() override
    {
        std::stringstream ss;
        ss << "COMMIT";
        auto query = sqlite3_query(ss.str());
        auto stmt_ptr = query.statement_ptr();
        auto stmt_ptr_ptr = reinterpret_cast<sqlite3_stmt**>(stmt_ptr);
        if (sqlite3_prepare_v2(db, query.statement().c_str(), -1, stmt_ptr_ptr, nullptr) == SQLITE_OK) {
            auto ret = query.next(this);
            if (ret == query::step::done) {
                //  done..
            } else {
                throw std::system_error(std::error_code(last_error_code(), error_category()));
            }
        } else {
            throw std::system_error(std::error_code(last_error_code(), error_category()));
        }
    }
    void rollback_transaction() override
    {
        std::stringstream ss;
        ss << "ROLLBACK";
        auto query = sqlite3_query(ss.str());
        auto stmt_ptr = query.statement_ptr();
        auto stmt_ptr_ptr = reinterpret_cast<sqlite3_stmt**>(stmt_ptr);
        if (sqlite3_prepare_v2(db, query.statement().c_str(), -1, stmt_ptr_ptr, nullptr) == SQLITE_OK) {
            auto ret = query.next(this);
            if (ret == query::step::done) {
                //  done..
            } else {
                throw std::system_error(std::error_code(last_error_code(), error_category()));
            }
        } else {
            throw std::system_error(std::error_code(last_error_code(), error_category()));
        }
    }

    void rename_table(const std::string& old_name, const std::string& new_name) override
    {
        std::stringstream ss;
        ss << "ALTER TABLE " << old_name << " RENAME TO " << new_name;
        auto query = sqlite3_query(ss.str());
        auto stmt_ptr = query.statement_ptr();
        auto stmt_ptr_ptr = reinterpret_cast<sqlite3_stmt**>(stmt_ptr);
        if (sqlite3_prepare_v2(db, query.statement().c_str(), -1, stmt_ptr_ptr, nullptr) == SQLITE_OK) {
            auto ret = query.next(this);
            if (ret == query::step::done) {
                //  done..
            } else {
                throw std::system_error(std::error_code(last_error_code(), error_category()));
            }
        } else {
            throw std::system_error(std::error_code(last_error_code(), error_category()));
        }
    }

    std::string current_timestamp() override
    {
        std::string res;
        std::stringstream ss;
        ss << "SELECT CURRENT_TIMESTAMP";
        auto query = ss.str();
        auto rc = sqlite3_exec(db,
            query.c_str(),
            [](void* data, int argc, char** argv, char**) -> int {
                auto& res = *(std::string*)data;
                if (argc) {
                    if (argv[0]) {
                        res = row_extractor<std::string>().extract(argv[0]);
                    }
                }
                return 0;
            },
            &res, nullptr);
        if (rc != SQLITE_OK) {
            throw std::system_error(std::error_code(last_error_code(), error_category()));
        }
        return res;
    }

    bool threadsafe() override
    {
        return true;
    }

    // database interface
public:
    void add_column(const std::string& tableName, const table_info& ti) override
    {
        std::stringstream ss;
        ss << "ALTER TABLE " << tableName << " ADD COLUMN " << ti.name << " ";
        ss << ti.type << " ";
        if (ti.pk) {
            ss << "PRIMARY KEY ";
        }
        if (ti.notnull) {
            ss << "NOT NULL ";
        }
        if (ti.dflt_value.length()) {
            ss << "DEFAULT " << ti.dflt_value << " ";
        }
        auto query = make_query(ss.str());

        auto prepareResult = prepare(query.get());
        if (prepareResult) {
            if (query::step::done != exec(query.get())) {
                throw std::system_error(std::error_code(last_error_code(), error_category()));
            }
        } else {
            throw std::system_error(std::error_code(last_error_code(), error_category()));
        }
    }
    query_ptr make_query(const std::string& statement) override
    {
        return std::make_shared<sqlite3_query>(statement);
    }

    // database interface
public:
    int limit_length() override { return sqlite3_limit(db, SQLITE_LIMIT_LENGTH, -1); }
    void limit_set_length(int value) override { sqlite3_limit(db, SQLITE_LIMIT_LENGTH, value); }
    int limit_sql_length() override { return sqlite3_limit(db, SQLITE_LIMIT_SQL_LENGTH, -1); }
    void limit_set_sql_length(int value) override { sqlite3_limit(db, SQLITE_LIMIT_SQL_LENGTH, value); }
    int limit_columns() override { return sqlite3_limit(db, SQLITE_LIMIT_COLUMN, -1); }
    void limit_set_columns(int value) override { sqlite3_limit(db, SQLITE_LIMIT_COLUMN, value); }
    int limit_expr_depth() override { return sqlite3_limit(db, SQLITE_LIMIT_EXPR_DEPTH, -1); }
    void limit_set_expr_depth(int value) override { sqlite3_limit(db, SQLITE_LIMIT_EXPR_DEPTH, value); }
    int limit_compound_select() override { return sqlite3_limit(db, SQLITE_LIMIT_COMPOUND_SELECT, -1); }
    void limit_set_compound_select(int value) override { sqlite3_limit(db, SQLITE_LIMIT_COMPOUND_SELECT, value); }
    int limit_vdbe_op() override { return sqlite3_limit(db, SQLITE_LIMIT_VDBE_OP, -1); }
    void limit_set_vdbe_op(int value) override { sqlite3_limit(db, SQLITE_LIMIT_VDBE_OP, value); }
    int limit_function_arg() override { return sqlite3_limit(db, SQLITE_LIMIT_FUNCTION_ARG, -1); }
    void limit_set_function_arg(int value) override { sqlite3_limit(db, SQLITE_LIMIT_FUNCTION_ARG, value); }
    int limit_attached() override { return sqlite3_limit(db, SQLITE_LIMIT_ATTACHED, -1); }
    void limit_set_attached(int value) override { sqlite3_limit(db, SQLITE_LIMIT_ATTACHED, value); }
    int limit_like_pattern_length() override { return sqlite3_limit(db, SQLITE_LIMIT_LIKE_PATTERN_LENGTH, -1); }
    void limit_set_like_pattern_length(int value) override { sqlite3_limit(db, SQLITE_LIMIT_LIKE_PATTERN_LENGTH, value); }
    int limit_variable_number() override { return sqlite3_limit(db, SQLITE_LIMIT_VARIABLE_NUMBER, -1); }
    void limit_set_variable_number(int value) override { sqlite3_limit(db, SQLITE_LIMIT_VARIABLE_NUMBER, value); }
    int limit_trigger_depth() override { return sqlite3_limit(db, SQLITE_LIMIT_TRIGGER_DEPTH, -1); }
    void limit_set_trigger_depth(int value) override { sqlite3_limit(db, SQLITE_LIMIT_TRIGGER_DEPTH, value); }
    int limit_worker_threads() override { return sqlite3_limit(db, SQLITE_LIMIT_WORKER_THREADS, -1); }
    void limit_set_worker_threads(int value) override { sqlite3_limit(db, SQLITE_LIMIT_WORKER_THREADS, value); }
    int last_changed_rows() override
    {
        return sqlite3_changes(db);
    }
    int total_changed_rows() override
    {
        return sqlite3_total_changes(db);
    }
    int64_t last_insert_rowid() override
    {
        return sqlite3_last_insert_rowid(db);
    }
    int busy_timeout(int ms) override
    {
        return sqlite3_busy_timeout(db, ms);
    }
    std::string version() override
    {
        return sqlite3_libversion();
    }
};

const char* sqlite3_database::SQLITE_TABLE_NAMES_QUERY = "SELECT name FROM sqlite_master WHERE type='table'";
bool sqlite3_database::g_registered = db_driver_factory::register_driver(sqlite3_database::driver_name(),
    sqlite3_database::create);
}
