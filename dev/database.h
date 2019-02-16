#pragma once

#include "query.h"
#include "table_info.h"
#include <algorithm> // std::find
#include <functional>
#include <map>
#include <memory>
#include <string> //  std::string
#include <system_error> //  std::error_code, std::system_error
#include <vector>

namespace sqlite_orm {

class query;
using query_ptr = std::shared_ptr<query>;

class db_driver_factory {
public:
    using create_db_t = std::unique_ptr<database> (*)(const std::string& path);

public:
    db_driver_factory() = delete;

    static bool register_driver(std::string name, create_db_t funcCreate)
    {
        auto it = g_methods.find(name);
        if (it == g_methods.end()) {
            g_methods[name] = funcCreate;
            return true;
        }
        return false;
    }

    static std::unique_ptr<database> create(const std::string& name, const std::string& path)
    {
        auto it = g_methods.find(name);
        if (it != g_methods.end()) {
            return it->second(path);
        }
        return {};
    }

private:
    static std::map<std::string, create_db_t> g_methods;
};
std::map<std::string, db_driver_factory::create_db_t> db_driver_factory::g_methods;

struct database {
public:
    virtual bool open() = 0;
    virtual void close() = 0;
    virtual ~database() = default;
    virtual std::string last_error_description() const = 0;
    virtual int last_error_code() const = 0;
    virtual const std::error_category& error_category() const = 0;
    virtual bool prepare(query* query) = 0;
    virtual query::step exec(query* query) = 0;
    virtual std::vector<std::string> table_names() = 0;
    virtual std::vector<table_info> get_table_info(const std::string& tableName) = 0;
    virtual void add_column(const std::string& tableName, const table_info& ti) = 0;
    virtual bool table_exists(const std::string& table_name)
    {
        const auto& tables { table_names() };
        auto begin { std::begin(tables) };
        auto end { std::end(tables) };
        return std::find(begin, end, table_name) != end;
    }
    virtual void begin_transaction() = 0;
    virtual void commit_transaction() = 0;
    virtual void rollback_transaction() = 0;
    virtual void rename_table(const std::string& old_name, const std::string& new_name) = 0;
    virtual std::string current_timestamp() = 0;
    virtual bool threadsafe() = 0;
    virtual query_ptr make_query(const std::string& statement) = 0;
    using collating_function = std::function<int(int, const void*, int, const void*)>;

    virtual void add_collation(std::string name, collating_function* f) = 0;
    /*
     * bind methods
     */
    // if (len < 0) - strlen must be used by database or derived
    virtual void bindString(query* query, int index, const char* data, int len)
    {
        query->bindString(this, index, data, len);
    }
    virtual void bindString(query* query, int index, const std::string& data)
    {
        bindString(query, index, data.c_str(), data.size());
    }
    virtual void bindBlob(query* query, int index, const void* data, size_t len)
    {
        query->bindBlob(this, index, data, len);
    }
    virtual void bindDouble(query* query, int index, double data)
    {
        query->bindDouble(this, index, data);
    }

    virtual void bindInt(query* query, int index, int data)
    {
        query->bindInt(this, index, data);
    }

    virtual void bindBigInt(query* query, int index, int64_t data)
    {
        query->bindBigInt(this, index, data);
    }
    virtual void bindNull(query* query, int index)
    {
        query->bindNull(this, index);
    }
    enum class limit_type {
        length,
        sql_length,
        columns,
        expr_depth,
        compound_select,
        vdbe_op,
        function_arg,
        attached,
        like_pattern_length,
        trigger_depth,
        variable_number,
        worker_threads,
    };
    // limits
    virtual int limit_length() = 0;
    virtual void limit_set_length(int value) = 0;
    virtual int limit_sql_length() = 0;
    virtual void limit_set_sql_length(int value) = 0;
    virtual int limit_columns() = 0;
    virtual void limit_set_columns(int value) = 0;
    virtual int limit_expr_depth() = 0;
    virtual void limit_set_expr_depth(int value) = 0;
    virtual int limit_compound_select() = 0;
    virtual void limit_set_compound_select(int value) = 0;
    virtual int limit_vdbe_op() = 0;
    virtual void limit_set_vdbe_op(int value) = 0;
    virtual int limit_function_arg() = 0;
    virtual void limit_set_function_arg(int value) = 0;
    virtual int limit_attached() = 0;
    virtual void limit_set_attached(int value) = 0;
    virtual int limit_like_pattern_length() = 0;
    virtual void limit_set_like_pattern_length(int value) = 0;
    virtual int limit_variable_number() = 0;
    virtual void limit_set_variable_number(int value) = 0;
    virtual int limit_trigger_depth() = 0;
    virtual void limit_set_trigger_depth(int value) = 0;
    virtual int limit_worker_threads() = 0;
    virtual void limit_set_worker_threads(int value) = 0;
    virtual int last_changed_rows() = 0;
    virtual int total_changed_rows() = 0;
    virtual int64_t last_insert_rowid() = 0;
    virtual int busy_timeout(int ms) = 0;
    virtual std::string version() = 0;
};
}
