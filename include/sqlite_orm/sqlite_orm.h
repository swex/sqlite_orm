#pragma once

#include <string> //  std::string

namespace sqlite_orm {

struct table_info {
    int cid;
    std::string name;
    std::string type;
    bool notnull;
    std::string dflt_value;
    int pk;
};

}
#pragma once

#include <string> //  std::string
#include <vector>

namespace sqlite_orm {

struct database;

class query {
    std::string m_query;

public:
    virtual void** statement_ptr() = 0;
    query() = default;
    virtual ~query() = default;
    query(const std::string& query)
        : m_query(query)
    {
    }
    enum class step {
        done,
        row,
        error
    };
    const std::string& statement() const
    {
        return m_query;
    }
    /*
     * column value methods
     */
    virtual bool isColumnValid(int index) = 0;
    virtual std::vector<char> columnBlob(int index) = 0;
    virtual double columnDouble(int index) = 0;
    virtual int columnInt(int index) = 0;
    virtual int64_t columnBigInt(int index) = 0;
    virtual bool prepare(database* db) = 0;
    virtual step next(database* db) = 0;
    virtual std::string columnString(int index) = 0;
    /*
     * bind helpers
     */
    virtual void bindString(database* db, int index, const char* data, int len) = 0;
    virtual void bindString(database* db, int index, const std::string& data)
    {
        return bindString(db, index, data.c_str(), data.size());
    }
    virtual void bindBlob(database* db, int index, const void* data, size_t len) = 0;
    virtual void bindDouble(database* db, int index, double data) = 0;
    virtual void bindInt(database* db, int index, int data) = 0;
    virtual void bindBigInt(database* db, int index, int64_t data) = 0;
    virtual void bindNull(database* db, int index) = 0;
};
}
#pragma once

// #include "query.h"

// #include "table_info.h"

#include <algorithm> // std::find
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
#pragma once

#if defined(_MSC_VER)
#if defined(min)
__pragma(push_macro("min"))
#undef min
#define __RESTORE_MIN__
#endif
#if defined(max)
    __pragma(push_macro("max"))
#undef max
#define __RESTORE_MAX__
#endif
#endif // defined(_MSC_VER)

#include <ciso646> //  due to #166

#pragma once

#include <stdexcept>
#include <string> //  std::string
#include <system_error> // std::error_code, std::system_error

        namespace sqlite_orm
{

    enum class orm_error_code {
        not_found = 1,
        type_is_not_mapped_to_storage,
        trying_to_dereference_null_iterator,
        too_many_tables_specified,
        incorrect_set_fields_specified,
        column_not_found,
        table_has_no_primary_key_column,
        cannot_start_a_transaction_within_a_transaction,
        no_active_transaction,
        incorrect_journal_mode_string,
    };
}

namespace sqlite_orm {

class orm_error_category : public std::error_category {
public:
    const char* name() const noexcept override final
    {
        return "ORM error";
    }

    std::string message(int c) const override final
    {
        switch (static_cast<orm_error_code>(c)) {
        case orm_error_code::not_found:
            return "Not found";
        case orm_error_code::type_is_not_mapped_to_storage:
            return "Type is not mapped to storage";
        case orm_error_code::trying_to_dereference_null_iterator:
            return "Trying to dereference null iterator";
        case orm_error_code::too_many_tables_specified:
            return "Too many tables specified";
        case orm_error_code::incorrect_set_fields_specified:
            return "Incorrect set fields specified";
        case orm_error_code::column_not_found:
            return "Column not found";
        case orm_error_code::table_has_no_primary_key_column:
            return "Table has no primary key column";
        case orm_error_code::cannot_start_a_transaction_within_a_transaction:
            return "Cannot start a transaction within a transaction";
        case orm_error_code::no_active_transaction:
            return "No active transaction";
        default:
            return "unknown error";
        }
    }
};

inline const orm_error_category& get_orm_error_category()
{
    static orm_error_category res;
    return res;
}

}

namespace std {
template <>
struct is_error_code_enum<sqlite_orm::orm_error_code> : std::true_type {
};

inline std::error_code make_error_code(sqlite_orm::orm_error_code errorCode)
{
    return std::error_code(static_cast<int>(errorCode), sqlite_orm::get_orm_error_category());
}
}
#pragma once

#include <cctype> //  std::toupper
#include <cstdint>
#include <map> //  std::map
#include <memory> //  std::make_shared, std::shared_ptr
#include <regex> //  std::regex, std::regex_match
#include <string> //  std::string
#include <vector> //  std::vector

namespace sqlite_orm {
using int64 = int64_t;
using uint64 = uint64_t;

//  numeric and real are the same for c++
enum class sqlite_type {
    INTEGER,
    TEXT,
    BLOB,
    REAL,
};

/**
     *  @param str case doesn't matter - it is uppercased before comparing.
     */
inline std::shared_ptr<sqlite_type> to_sqlite_type(const std::string& str)
{
    auto asciiStringToUpper = [](std::string& s) {
        std::transform(s.begin(),
            s.end(),
            s.begin(),
            [](char c) {
                return std::toupper(c);
            });
    };
    auto upperStr = str;
    asciiStringToUpper(upperStr);

    static std::map<sqlite_type, std::vector<std::regex>> typeMap = {
        { sqlite_type::INTEGER, {
                                    std::regex("INT"),
                                    std::regex("INT.*"),
                                    std::regex("TINYINT"),
                                    std::regex("SMALLINT"),
                                    std::regex("MEDIUMINT"),
                                    std::regex("BIGINT"),
                                    std::regex("UNSIGNED BIG INT"),
                                    std::regex("INT2"),
                                    std::regex("INT8"),
                                } },
        { sqlite_type::TEXT, {
                                 std::regex("CHARACTER\\([[:digit:]]+\\)"),
                                 std::regex("VARCHAR\\([[:digit:]]+\\)"),
                                 std::regex("VARYING CHARACTER\\([[:digit:]]+\\)"),
                                 std::regex("NCHAR\\([[:digit:]]+\\)"),
                                 std::regex("NATIVE CHARACTER\\([[:digit:]]+\\)"),
                                 std::regex("NVARCHAR\\([[:digit:]]+\\)"),
                                 std::regex("CLOB"),
                                 std::regex("TEXT"),
                             } },
        { sqlite_type::BLOB, {
                                 std::regex("BLOB"),
                             } },
        { sqlite_type::REAL, {
                                 std::regex("REAL"),
                                 std::regex("DOUBLE"),
                                 std::regex("DOUBLE PRECISION"),
                                 std::regex("FLOAT"),
                                 std::regex("NUMERIC"),
                                 std::regex("DECIMAL\\([[:digit:]]+,[[:digit:]]+\\)"),
                                 std::regex("BOOLEAN"),
                                 std::regex("DATE"),
                                 std::regex("DATETIME"),
                             } },
    };
    for (auto& p : typeMap) {
        for (auto& r : p.second) {
            if (std::regex_match(upperStr, r)) {
                return std::make_shared<sqlite_type>(p.first);
            }
        }
    }

    return {};
}
}
#pragma once

#include <tuple> //  std::tuple
#include <type_traits> //  std::false_type, std::true_type
#include <utility> //  std::index_sequence, std::index_sequence_for

namespace sqlite_orm {

//  got from here http://stackoverflow.com/questions/25958259/how-do-i-find-out-if-a-tuple-contains-a-type
namespace tuple_helper {

    template <typename T, typename Tuple>
    struct has_type;

    template <typename T>
    struct has_type<T, std::tuple<>> : std::false_type {
    };

    template <typename T, typename U, typename... Ts>
    struct has_type<T, std::tuple<U, Ts...>> : has_type<T, std::tuple<Ts...>> {
    };

    template <typename T, typename... Ts>
    struct has_type<T, std::tuple<T, Ts...>> : std::true_type {
    };

    template <typename T, typename Tuple>
    using tuple_contains_type = typename has_type<T, Tuple>::type;

    template <size_t N, class... Args>
    struct iterator {

        template <class L>
        void operator()(const std::tuple<Args...>& t, L l, bool reverse = true)
        {
            if (reverse) {
                l(std::get<N>(t));
                iterator<N - 1, Args...>()(t, l, reverse);
            } else {
                iterator<N - 1, Args...>()(t, l, reverse);
                l(std::get<N>(t));
            }
        }
    };

    template <class... Args>
    struct iterator<0, Args...> {

        template <class L>
        void operator()(const std::tuple<Args...>& t, L l, bool /*reverse*/ = true)
        {
            l(std::get<0>(t));
        }
    };

    template <size_t N>
    struct iterator<N> {

        template <class L>
        void operator()(const std::tuple<>&, L, bool /*reverse*/ = true)
        {
            //..
        }
    };

    template <class F, typename T, std::size_t... I>
    void tuple_for_each_impl(F&& f, const T& t, std::index_sequence<I...>)
    {
        int _[] = { (f(std::get<I>(t)), int {})... };
        (void)_;
    }

    template <typename F, typename... Args>
    void tuple_for_each(const std::tuple<Args...>& t, F&& f)
    {
        tuple_for_each_impl(std::forward<F>(f), t, std::index_sequence_for<Args...> {});
    }
}
}
#pragma once

#include <type_traits> //  std::false_type, std::true_type, std::integral_constant

namespace sqlite_orm {

//  got from here https://stackoverflow.com/questions/37617677/implementing-a-compile-time-static-if-logic-for-different-string-types-in-a-co
namespace static_magic {

    template <typename T, typename F>
    auto static_if(std::true_type, T t, F f) { return t; }

    template <typename T, typename F>
    auto static_if(std::false_type, T t, F f) { return f; }

    template <bool B, typename T, typename F>
    auto static_if(T t, F f) { return static_if(std::integral_constant<bool, B> {}, t, f); }

    template <bool B, typename T>
    auto static_if(T t)
    {
        return static_if(std::integral_constant<bool, B> {}, t, [](auto&&...) {});
    }
}

}
#pragma once

#include <memory> //  std::shared_ptr, std::unique_ptr
#include <string> //  std::string
#include <vector> //  std::vector

namespace sqlite_orm {

/**
     *  This class accepts c++ type and transfers it to sqlite name (int -> INTEGER, std::string -> TEXT)
     */
template <class T>
struct type_printer;

struct integer_printer {
    inline const std::string& print()
    {
        static const std::string res = "INTEGER";
        return res;
    }
};

struct text_printer {
    inline const std::string& print()
    {
        static const std::string res = "TEXT";
        return res;
    }
};

struct real_printer {
    inline const std::string& print()
    {
        static const std::string res = "REAL";
        return res;
    }
};

struct blob_printer {
    inline const std::string& print()
    {
        static const std::string res = "BLOB";
        return res;
    }
};

//Note unsigned/signed char and simple char used for storing integer values, not char values.
template <>
struct type_printer<unsigned char> : public integer_printer {
};

template <>
struct type_printer<signed char> : public integer_printer {
};

template <>
struct type_printer<char> : public integer_printer {
};

template <>
struct type_printer<unsigned short int> : public integer_printer {
};

template <>
struct type_printer<short> : public integer_printer {
};

template <>
struct type_printer<unsigned int> : public integer_printer {
};

template <>
struct type_printer<int> : public integer_printer {
};

template <>
struct type_printer<unsigned long> : public integer_printer {
};

template <>
struct type_printer<long> : public integer_printer {
};

template <>
struct type_printer<unsigned long long> : public integer_printer {
};

template <>
struct type_printer<long long> : public integer_printer {
};

template <>
struct type_printer<bool> : public integer_printer {
};

template <>
struct type_printer<std::string> : public text_printer {
};

template <>
struct type_printer<std::wstring> : public text_printer {
};

template <>
struct type_printer<const char*> : public text_printer {
};

template <>
struct type_printer<float> : public real_printer {
};

template <>
struct type_printer<double> : public real_printer {
};

template <class T>
struct type_printer<std::shared_ptr<T>> : public type_printer<T> {
};

template <class T>
struct type_printer<std::unique_ptr<T>> : public type_printer<T> {
};

template <>
struct type_printer<std::vector<char>> : public blob_printer {
};
}
#pragma once

namespace sqlite_orm {

namespace internal {

    enum class collate_argument {
        binary,
        nocase,
        rtrim,
    };
}

}
#pragma once

#include <ostream> //  std::ostream
#include <sstream> //  std::stringstream
#include <string> //  std::string
#include <tuple> //  std::tuple, std::make_tuple
#include <type_traits> //  std::is_base_of, std::false_type, std::true_type

namespace sqlite_orm {

namespace constraints {

    /**
         *  AUTOINCREMENT constraint class.
         */
    struct autoincrement_t {

        operator std::string() const
        {
            return "AUTOINCREMENT";
        }
    };

    /**
         *  PRIMARY KEY constraint class.
         *  Cs is parameter pack which contains columns (member pointer and/or function pointers). Can be empty when used withen `make_column` function.
         */
    template <class... Cs>
    struct primary_key_t {
        std::tuple<Cs...> columns;
        enum class order_by {
            unspecified,
            ascending,
            descending,
        };
        order_by asc_option = order_by::unspecified;

        primary_key_t(decltype(columns) c)
            : columns(std::move(c))
        {
        }

        using field_type = void; //  for column iteration. Better be deleted
        using constraints_type = std::tuple<>;

        operator std::string() const
        {
            std::string res = "PRIMARY KEY";
            switch (this->asc_option) {
            case order_by::ascending:
                res += " ASC";
                break;
            case order_by::descending:
                res += " DESC";
                break;
            default:
                break;
            }
            return res;
        }

        primary_key_t<Cs...> asc() const
        {
            auto res = *this;
            res.asc_option = order_by::ascending;
            return res;
        }

        primary_key_t<Cs...> desc() const
        {
            auto res = *this;
            res.asc_option = order_by::descending;
            return res;
        }
    };

    /**
         *  UNIQUE constraint class.
         */
    struct unique_t {

        operator std::string() const
        {
            return "UNIQUE";
        }
    };

    /**
         *  DEFAULT constraint class.
         *  T is a value type.
         */
    template <class T>
    struct default_t {
        using value_type = T;

        value_type value;

        operator std::string() const
        {
            std::stringstream ss;
            ss << "DEFAULT ";
            auto needQuotes = std::is_base_of<text_printer, type_printer<T>>::value;
            if (needQuotes) {
                ss << "'";
            }
            ss << this->value;
            if (needQuotes) {
                ss << "'";
            }
            return ss.str();
        }
    };

    /**
         *  FOREIGN KEY constraint class.
         *  Cs are columns which has foreign key
         *  Rs are column which C references to
         *  Available in SQLite 3.6.19 or higher
         */

    template <class A, class B>
    struct foreign_key_t;

    enum class foreign_key_action {
        none, //  not specified
        no_action,
        restrict_,
        set_null,
        set_default,
        cascade,
    };

    inline std::ostream& operator<<(std::ostream& os, foreign_key_action action)
    {
        switch (action) {
        case decltype(action)::no_action:
            os << "NO ACTION";
            break;
        case decltype(action)::restrict_:
            os << "RESTRICT";
            break;
        case decltype(action)::set_null:
            os << "SET NULL";
            break;
        case decltype(action)::set_default:
            os << "SET DEFAULT";
            break;
        case decltype(action)::cascade:
            os << "CASCADE";
            break;
        case decltype(action)::none:
            break;
        }
        return os;
    }

    /**
         *  F - foreign key class
         */
    template <class F>
    struct on_update_delete_t {
        using foreign_key_type = F;

        const foreign_key_type& fk;
        const bool update; //  true if update and false if delete

        on_update_delete_t(decltype(fk) fk_, decltype(update) update_, foreign_key_action action_)
            : fk(fk_)
            , update(update_)
            , _action(action_)
        {
        }

        foreign_key_action _action = foreign_key_action::none;

        foreign_key_type no_action() const
        {
            auto res = this->fk;
            if (update) {
                res.on_update._action = foreign_key_action::no_action;
            } else {
                res.on_delete._action = foreign_key_action::no_action;
            }
            return res;
        }

        foreign_key_type restrict_() const
        {
            auto res = this->fk;
            if (update) {
                res.on_update._action = foreign_key_action::restrict_;
            } else {
                res.on_delete._action = foreign_key_action::restrict_;
            }
            return res;
        }

        foreign_key_type set_null() const
        {
            auto res = this->fk;
            if (update) {
                res.on_update._action = foreign_key_action::set_null;
            } else {
                res.on_delete._action = foreign_key_action::set_null;
            }
            return res;
        }

        foreign_key_type set_default() const
        {
            auto res = this->fk;
            if (update) {
                res.on_update._action = foreign_key_action::set_default;
            } else {
                res.on_delete._action = foreign_key_action::set_default;
            }
            return res;
        }

        foreign_key_type cascade() const
        {
            auto res = this->fk;
            if (update) {
                res.on_update._action = foreign_key_action::cascade;
            } else {
                res.on_delete._action = foreign_key_action::cascade;
            }
            return res;
        }

        operator bool() const
        {
            return this->_action != decltype(this->_action)::none;
        }

        operator std::string() const
        {
            if (this->update) {
                return "ON UPDATE";
            } else {
                return "ON DELETE";
            }
        }
    };

    template <class... Cs, class... Rs>
    struct foreign_key_t<std::tuple<Cs...>, std::tuple<Rs...>> {
        using columns_type = std::tuple<Cs...>;
        using references_type = std::tuple<Rs...>;
        using self = foreign_key_t<columns_type, references_type>;

        columns_type columns;
        references_type references;

        on_update_delete_t<self> on_update;
        on_update_delete_t<self> on_delete;

        static_assert(std::tuple_size<columns_type>::value == std::tuple_size<references_type>::value, "Columns size must be equal to references tuple");

        foreign_key_t(columns_type columns_, references_type references_)
            : columns(std::move(columns_))
            , references(std::move(references_))
            , on_update(*this, true, foreign_key_action::none)
            , on_delete(*this, false, foreign_key_action::none)
        {
        }

        foreign_key_t(const self& other)
            : columns(other.columns)
            , references(other.references)
            , on_update(*this, true, other.on_update._action)
            , on_delete(*this, false, other.on_delete._action)
        {
        }

        self& operator=(const self& other)
        {
            this->columns = other.columns;
            this->references = other.references;
            this->on_update = { *this, true, other.on_update._action };
            this->on_delete = { *this, false, other.on_delete._action };
            return *this;
        }

        using field_type = void; //  for column iteration. Better be deleted
        using constraints_type = std::tuple<>;

        template <class L>
        void for_each_column(L) {}

        template <class... Opts>
        constexpr bool has_every() const
        {
            return false;
        }
    };

    /**
         *  Cs can be a class member pointer, a getter function member pointer or setter
         *  func member pointer
         *  Available in SQLite 3.6.19 or higher
         */
    template <class... Cs>
    struct foreign_key_intermediate_t {
        using tuple_type = std::tuple<Cs...>;

        tuple_type columns;

        foreign_key_intermediate_t(tuple_type columns_)
            : columns(std::move(columns_))
        {
        }

        template <class... Rs>
        foreign_key_t<std::tuple<Cs...>, std::tuple<Rs...>> references(Rs... references)
        {
            using ret_type = foreign_key_t<std::tuple<Cs...>, std::tuple<Rs...>>;
            return ret_type(std::move(this->columns), std::make_tuple(std::forward<Rs>(references)...));
        }
    };

    struct collate_t {
        internal::collate_argument argument;

        collate_t(internal::collate_argument argument_)
            : argument(argument_)
        {
        }

        operator std::string() const
        {
            std::string res = "COLLATE " + string_from_collate_argument(this->argument);
            return res;
        }

        static std::string string_from_collate_argument(internal::collate_argument argument)
        {
            switch (argument) {
            case decltype(argument)::binary:
                return "BINARY";
            case decltype(argument)::nocase:
                return "NOCASE";
            case decltype(argument)::rtrim:
                return "RTRIM";
            }
            return {};
        }
    };

    template <class T>
    struct is_constraint : std::false_type {
    };

    template <>
    struct is_constraint<autoincrement_t> : std::true_type {
    };

    template <class... Cs>
    struct is_constraint<primary_key_t<Cs...>> : std::true_type {
    };

    template <>
    struct is_constraint<unique_t> : std::true_type {
    };

    template <class T>
    struct is_constraint<default_t<T>> : std::true_type {
    };

    template <class C, class R>
    struct is_constraint<foreign_key_t<C, R>> : std::true_type {
    };

    template <>
    struct is_constraint<collate_t> : std::true_type {
    };

    template <class... Args>
    struct constraints_size;

    template <>
    struct constraints_size<> {
        static constexpr const int value = 0;
    };

    template <class H, class... Args>
    struct constraints_size<H, Args...> {
        static constexpr const int value = is_constraint<H>::value + constraints_size<Args...>::value;
    };
}

/**
     *  FOREIGN KEY constraint construction function that takes member pointer as argument
     *  Available in SQLite 3.6.19 or higher
     */
template <class... Cs>
constraints::foreign_key_intermediate_t<Cs...> foreign_key(Cs... columns)
{
    return { std::make_tuple(std::forward<Cs>(columns)...) };
}

/**
     *  UNIQUE constraint builder function.
     */
inline constraints::unique_t unique()
{
    return {};
}

inline constraints::autoincrement_t autoincrement()
{
    return {};
}

template <class... Cs>
inline constraints::primary_key_t<Cs...> primary_key(Cs... cs)
{
    using ret_type = constraints::primary_key_t<Cs...>;
    return ret_type(std::make_tuple(cs...));
}

template <class T>
constraints::default_t<T> default_value(T t)
{
    return { t };
}

inline constraints::collate_t collate_nocase()
{
    return { internal::collate_argument::nocase };
}

inline constraints::collate_t collate_binary()
{
    return { internal::collate_argument::binary };
}

inline constraints::collate_t collate_rtrim()
{
    return { internal::collate_argument::rtrim };
}

namespace internal {

    /**
         *  FOREIGN KEY traits. Common case
         */
    template <class T>
    struct is_foreign_key : std::false_type {
    };

    /**
         *  FOREIGN KEY traits. Specialized case
         */
    template <class C, class R>
    struct is_foreign_key<constraints::foreign_key_t<C, R>> : std::true_type {
    };

    /**
         *  PRIMARY KEY traits. Common case
         */
    template <class T>
    struct is_primary_key : public std::false_type {
    };

    /**
         *  PRIMARY KEY traits. Specialized case
         */
    template <class... Cs>
    struct is_primary_key<constraints::primary_key_t<Cs...>> : public std::true_type {
    };
}

}
#pragma once

#include <memory> //  std::shared_ptr, std::unique_ptr
#include <type_traits> //  std::false_type, std::true_type

namespace sqlite_orm {

/**
     *  This is class that tells `sqlite_orm` that type is nullable. Nullable types
     *  are mapped to sqlite database as `NULL` and not-nullable are mapped as `NOT NULL`.
     *  Default nullability status for all types is `NOT NULL`. So if you want to map
     *  custom type as `NULL` (for example: boost::optional) you have to create a specialiation
     *  of type_is_nullable for your type and derive from `std::true_type`.
     */
template <class T>
struct type_is_nullable : public std::false_type {
    bool operator()(const T&) const
    {
        return true;
    }
};

/**
     *  This is a specialization for std::shared_ptr. std::shared_ptr is nullable in sqlite_orm.
     */
template <class T>
struct type_is_nullable<std::shared_ptr<T>> : public std::true_type {
    bool operator()(const std::shared_ptr<T>& t) const
    {
        return static_cast<bool>(t);
    }
};

/**
     *  This is a specialization for std::unique_ptr. std::unique_ptr is nullable too.
     */
template <class T>
struct type_is_nullable<std::unique_ptr<T>> : public std::true_type {
    bool operator()(const std::unique_ptr<T>& t) const
    {
        return static_cast<bool>(t);
    }
};

}
#pragma once

#include <memory> //  std::shared_ptr
#include <sstream> //  std::stringstream
#include <string> //  std::string

// #include "constraints.h"

namespace sqlite_orm {

namespace internal {

    /**
         *  This class is used in tuple interation to know whether tuple constains `default_value_t`
         *  constraint class and what it's value if it is
         */
    struct default_value_extractor {

        template <class A>
        std::shared_ptr<std::string> operator()(const A&)
        {
            return {};
        }

        template <class T>
        std::shared_ptr<std::string> operator()(const constraints::default_t<T>& t)
        {
            std::stringstream ss;
            ss << t.value;
            return std::make_shared<std::string>(ss.str());
        }
    };

}

}
#pragma once

#include <type_traits> //  std::false_type, std::true_type

namespace sqlite_orm {

namespace internal {

    /**
         *  Inherit this class to support arithmetic types overloading
         */
    struct arithmetic_t {
    };

    /**
         *  Result of concatenation || operator
         */
    template <class L, class R>
    struct conc_t {
        L l;
        R r;
    };

    /**
         *  Result of addition + operator
         */
    template <class L, class R>
    struct add_t : arithmetic_t {
        using left_type = L;
        using right_type = R;

        left_type l;
        right_type r;

        add_t() = default;

        add_t(left_type l_, right_type r_)
            : l(std::move(l_))
            , r(std::move(r_))
        {
        }
    };

    /**
         *  Result of subscribe - operator
         */
    template <class L, class R>
    struct sub_t : arithmetic_t {
        using left_type = L;
        using right_type = R;

        left_type l;
        right_type r;

        sub_t() = default;

        sub_t(left_type l_, right_type r_)
            : l(std::move(l_))
            , r(std::move(r_))
        {
        }
    };

    /**
         *  Result of multiply * operator
         */
    template <class L, class R>
    struct mul_t : arithmetic_t {
        using left_type = L;
        using right_type = R;

        left_type l;
        right_type r;

        mul_t() = default;

        mul_t(left_type l_, right_type r_)
            : l(std::move(l_))
            , r(std::move(r_))
        {
        }
    };

    /**
         *  Result of divide / operator
         */
    template <class L, class R>
    struct div_t : arithmetic_t {
        using left_type = L;
        using right_type = R;

        left_type l;
        right_type r;

        div_t() = default;

        div_t(left_type l_, right_type r_)
            : l(std::move(l_))
            , r(std::move(r_))
        {
        }
    };

    /**
         *  Result of mod % operator
         */
    template <class L, class R>
    struct mod_t : arithmetic_t {
        using left_type = L;
        using right_type = R;

        left_type l;
        right_type r;

        mod_t() = default;

        mod_t(left_type l_, right_type r_)
            : l(std::move(l_))
            , r(std::move(r_))
        {
        }
    };

    /**
         *  Result of assign = operator
         */
    template <class L, class R>
    struct assign_t {
        L l;
        R r;

        assign_t() {}

        assign_t(L l_, R r_)
            : l(l_)
            , r(r_)
        {
        }
    };

    /**
         *  Assign operator traits. Common case
         */
    template <class T>
    struct is_assign_t : public std::false_type {
    };

    /**
         *  Assign operator traits. Specialized case
         */
    template <class L, class R>
    struct is_assign_t<assign_t<L, R>> : public std::true_type {
    };

    /**
         *  Is not an operator but a result of c(...) function. Has operator= overloaded which returns assign_t
         */
    template <class T>
    struct expression_t {
        T t;

        expression_t(T t_)
            : t(t_)
        {
        }

        template <class R>
        assign_t<T, R> operator=(R r) const
        {
            return { this->t, r };
        }
    };

}

/**
     *  Public interface for syntax sugar for columns. Example: `where(c(&User::id) == 5)` or `storage.update(set(c(&User::name) = "Dua Lipa"));
     */
template <class T>
internal::expression_t<T> c(T t)
{
    using result_type = internal::expression_t<T>;
    return result_type(t);
}

/**
     *  Public interface for || concatenation operator. Example: `select(conc(&User::name, "@gmail.com"));` => SELECT name + '@gmail.com' FROM users
     */
template <class L, class R>
internal::conc_t<L, R> conc(L l, R r)
{
    return { l, r };
}

/**
     *  Public interface for + operator. Example: `select(add(&User::age, 100));` => SELECT age + 100 FROM users
     */
template <class L, class R>
internal::add_t<L, R> add(L l, R r)
{
    return { l, r };
}

/**
     *  Public interface for - operator. Example: `select(add(&User::age, 1));` => SELECT age - 1 FROM users
     */
template <class L, class R>
internal::sub_t<L, R> sub(L l, R r)
{
    return { l, r };
}

template <class L, class R>
internal::mul_t<L, R> mul(L l, R r)
{
    return { l, r };
}

template <class L, class R>
internal::div_t<L, R> div(L l, R r)
{
    return { l, r };
}

template <class L, class R>
internal::mod_t<L, R> mod(L l, R r)
{
    return { l, r };
}

template <class L, class R>
internal::assign_t<L, R> assign(L l, R r)
{
    return { std::move(l), std::move(r) };
}

}
#pragma once

#include <memory> //  std::shared_ptr
#include <string> //  std::string
#include <tuple> //  std::tuple
#include <type_traits> //  std::true_type, std::false_type, std::is_same, std::enable_if

// #include "type_is_nullable.h"

// #include "tuple_helper.h"

// #include "default_value_extractor.h"

// #include "constraints.h"

namespace sqlite_orm {

namespace internal {

    /**
         *  This class stores single column info. column_t is a pair of [column_name:member_pointer] mapped to a storage
         *  O is a mapped class, e.g. User
         *  T is a mapped class'es field type, e.g. &User::name
         *  Op... is a constraints pack, e.g. primary_key_t, autoincrement_t etc
         */
    template <class O, class T, class G /* = const T& (O::*)() const*/, class S /* = void (O::*)(T)*/, class... Op>
    struct column_t {
        using object_type = O;
        using field_type = T;
        using constraints_type = std::tuple<Op...>;
        using member_pointer_t = field_type object_type::*;
        using getter_type = G;
        using setter_type = S;

        /**
             *  Column name. Specified during construction in `make_column`.
             */
        const std::string name;

        /**
             *  Member pointer used to read/write member
             */
        member_pointer_t member_pointer /* = nullptr*/;

        /**
             *  Getter member function pointer to get a value. If member_pointer is null than
             *  `getter` and `setter` must be not null
             */
        getter_type getter /* = nullptr*/;

        /**
             *  Setter member function
             */
        setter_type setter /* = nullptr*/;

        /**
             *  Constraints tuple
             */
        constraints_type constraints;

        /**
             *  Simplified interface for `NOT NULL` constraint
             */
        bool not_null() const
        {
            return !type_is_nullable<field_type>::value;
        }

        template <class Opt>
        constexpr bool has() const
        {
            return tuple_helper::tuple_contains_type<Opt, constraints_type>::value;
        }

        template <class O1, class O2, class... Opts>
        constexpr bool has_every() const
        {
            if (has<O1>() && has<O2>()) {
                return true;
            } else {
                return has_every<Opts...>();
            }
        }

        template <class O1>
        constexpr bool has_every() const
        {
            return has<O1>();
        }

        /**
             *  Simplified interface for `DEFAULT` constraint
             *  @return string representation of default value if it exists otherwise nullptr
             */
        std::shared_ptr<std::string> default_value()
        {
            std::shared_ptr<std::string> res;
            tuple_helper::iterator<std::tuple_size<constraints_type>::value - 1, Op...>()(constraints, [&res](auto& v) {
                auto dft = internal::default_value_extractor()(v);
                if (dft) {
                    res = dft;
                }
            });
            return res;
        }
    };

    /**
         *  Column traits. Common case.
         */
    template <class T>
    struct is_column : public std::false_type {
    };

    /**
         *  Column traits. Specialized case case.
         */
    template <class O, class T, class... Op>
    struct is_column<column_t<O, T, Op...>> : public std::true_type {
    };

    template <class O, class T>
    using getter_by_value_const = T (O::*)() const;

    template <class O, class T>
    using getter_by_value = T (O::*)();

    template <class O, class T>
    using getter_by_ref_const = T& (O::*)() const;

    template <class O, class T>
    using getter_by_ref = T& (O::*)();

    template <class O, class T>
    using getter_by_const_ref_const = const T& (O::*)() const;

    template <class O, class T>
    using getter_by_const_ref = const T& (O::*)();

    template <class O, class T>
    using setter_by_value = void (O::*)(T);

    template <class O, class T>
    using setter_by_ref = void (O::*)(T&);

    template <class O, class T>
    using setter_by_const_ref = void (O::*)(const T&);

    template <class T>
    struct is_getter : std::false_type {
    };

    template <class O, class T>
    struct is_getter<getter_by_value_const<O, T>> : std::true_type {
    };

    template <class O, class T>
    struct is_getter<getter_by_value<O, T>> : std::true_type {
    };

    template <class O, class T>
    struct is_getter<getter_by_ref_const<O, T>> : std::true_type {
    };

    template <class O, class T>
    struct is_getter<getter_by_ref<O, T>> : std::true_type {
    };

    template <class O, class T>
    struct is_getter<getter_by_const_ref_const<O, T>> : std::true_type {
    };

    template <class O, class T>
    struct is_getter<getter_by_const_ref<O, T>> : std::true_type {
    };

    template <class T>
    struct is_setter : std::false_type {
    };

    template <class O, class T>
    struct is_setter<setter_by_value<O, T>> : std::true_type {
    };

    template <class O, class T>
    struct is_setter<setter_by_ref<O, T>> : std::true_type {
    };

    template <class O, class T>
    struct is_setter<setter_by_const_ref<O, T>> : std::true_type {
    };

    template <class T>
    struct getter_traits;

    template <class O, class T>
    struct getter_traits<getter_by_value_const<O, T>> {
        using object_type = O;
        using field_type = T;

        static constexpr const bool returns_lvalue = false;
    };

    template <class O, class T>
    struct getter_traits<getter_by_value<O, T>> {
        using object_type = O;
        using field_type = T;

        static constexpr const bool returns_lvalue = false;
    };

    template <class O, class T>
    struct getter_traits<getter_by_ref_const<O, T>> {
        using object_type = O;
        using field_type = T;

        static constexpr const bool returns_lvalue = true;
    };

    template <class O, class T>
    struct getter_traits<getter_by_ref<O, T>> {
        using object_type = O;
        using field_type = T;

        static constexpr const bool returns_lvalue = true;
    };

    template <class O, class T>
    struct getter_traits<getter_by_const_ref_const<O, T>> {
        using object_type = O;
        using field_type = T;

        static constexpr const bool returns_lvalue = true;
    };

    template <class O, class T>
    struct getter_traits<getter_by_const_ref<O, T>> {
        using object_type = O;
        using field_type = T;

        static constexpr const bool returns_lvalue = true;
    };

    template <class T>
    struct setter_traits;

    template <class O, class T>
    struct setter_traits<setter_by_value<O, T>> {
        using object_type = O;
        using field_type = T;
    };

    template <class O, class T>
    struct setter_traits<setter_by_ref<O, T>> {
        using object_type = O;
        using field_type = T;
    };

    template <class O, class T>
    struct setter_traits<setter_by_const_ref<O, T>> {
        using object_type = O;
        using field_type = T;
    };
}

/**
     *  Column builder function. You should use it to create columns instead of constructor
     */
template <class O, class T,
    typename = typename std::enable_if<!std::is_member_function_pointer<T O::*>::value>::type,
    class... Op>
internal::column_t<O, T, const T& (O::*)() const, void (O::*)(T), Op...> make_column(const std::string& name, T O::*m, Op... constraints)
{
    static_assert(constraints::constraints_size<Op...>::value == std::tuple_size<std::tuple<Op...>>::value, "Incorrect constraints pack");
    return { name, m, nullptr, nullptr, std::make_tuple(constraints...) };
}

/**
     *  Column builder function with setter and getter. You should use it to create columns instead of constructor
     */
template <class G, class S,
    typename = typename std::enable_if<internal::is_getter<G>::value>::type,
    typename = typename std::enable_if<internal::is_setter<S>::value>::type,
    class... Op>
internal::column_t<
    typename internal::setter_traits<S>::object_type,
    typename internal::setter_traits<S>::field_type,
    G, S, Op...>
make_column(const std::string& name,
    S setter,
    G getter,
    Op... constraints)
{
    static_assert(std::is_same<typename internal::setter_traits<S>::field_type, typename internal::getter_traits<G>::field_type>::value,
        "Getter and setter must get and set same data type");
    static_assert(constraints::constraints_size<Op...>::value == std::tuple_size<std::tuple<Op...>>::value, "Incorrect constraints pack");
    return { name, nullptr, getter, setter, std::make_tuple(constraints...) };
}

/**
     *  Column builder function with getter and setter (reverse order). You should use it to create columns instead of constructor
     */
template <class G, class S,
    typename = typename std::enable_if<internal::is_getter<G>::value>::type,
    typename = typename std::enable_if<internal::is_setter<S>::value>::type,
    class... Op>
internal::column_t<
    typename internal::setter_traits<S>::object_type,
    typename internal::setter_traits<S>::field_type,
    G, S, Op...>
make_column(const std::string& name,
    G getter,
    S setter,
    Op... constraints)
{
    static_assert(std::is_same<typename internal::setter_traits<S>::field_type, typename internal::getter_traits<G>::field_type>::value,
        "Getter and setter must get and set same data type");
    static_assert(constraints::constraints_size<Op...>::value == std::tuple_size<std::tuple<Op...>>::value, "Incorrect constraints pack");
    return { name, nullptr, getter, setter, std::make_tuple(constraints...) };
}

}
#pragma once

#include <cstddef> //  std::nullptr_t
#include <memory> //  std::shared_ptr, std::unique_ptr
#include <sstream> //  std::stringstream
#include <string> //  std::string
#include <vector> //  std::vector

namespace sqlite_orm {

/**
     *  Is used to print members mapped to objects in storage_t::dump member function.
     *  Other developers can create own specialization to map custom types
     */
template <class T>
struct field_printer {
    std::string operator()(const T& t) const
    {
        std::stringstream stream;
        stream << t;
        return stream.str();
    }
};

/**
     *  Upgrade to integer is required when using unsigned char(uint8_t)
     */
template <>
struct field_printer<unsigned char> {
    std::string operator()(const unsigned char& t) const
    {
        std::stringstream stream;
        stream << +t;
        return stream.str();
    }
};

/**
     *  Upgrade to integer is required when using signed char(int8_t)
     */
template <>
struct field_printer<signed char> {
    std::string operator()(const signed char& t) const
    {
        std::stringstream stream;
        stream << +t;
        return stream.str();
    }
};

/**
     *  char is neigher signer char nor unsigned char so it has its own specialization
     */
template <>
struct field_printer<char> {
    std::string operator()(const char& t) const
    {
        std::stringstream stream;
        stream << +t;
        return stream.str();
    }
};

template <>
struct field_printer<std::string> {
    std::string operator()(const std::string& t) const
    {
        return t;
    }
};

template <>
struct field_printer<std::vector<char>> {
    std::string operator()(const std::vector<char>& t) const
    {
        std::stringstream ss;
        ss << std::hex;
        for (auto c : t) {
            ss << c;
        }
        return ss.str();
    }
};

template <>
struct field_printer<std::nullptr_t> {
    std::string operator()(const std::nullptr_t&) const
    {
        return "null";
    }
};

template <class T>
struct field_printer<std::shared_ptr<T>> {
    std::string operator()(const std::shared_ptr<T>& t) const
    {
        if (t) {
            return field_printer<T>()(*t);
        } else {
            return field_printer<std::nullptr_t>()(nullptr);
        }
    }
};

template <class T>
struct field_printer<std::unique_ptr<T>> {
    std::string operator()(const std::unique_ptr<T>& t) const
    {
        if (t) {
            return field_printer<T>()(*t);
        } else {
            return field_printer<std::nullptr_t>()(nullptr);
        }
    }
};
}
#pragma once

#include <string> //  std::string

// #include "collate_argument.h"

// #include "constraints.h"

namespace sqlite_orm {

namespace conditions {

    /**
         *  Stores LIMIT/OFFSET info
         */
    struct limit_t {
        int lim = 0;
        bool has_offset = false;
        bool offset_is_implicit = false;
        int off = 0;

        limit_t() = default;

        limit_t(decltype(lim) lim_)
            : lim(lim_)
        {
        }

        limit_t(decltype(lim) lim_,
            decltype(has_offset) has_offset_,
            decltype(offset_is_implicit) offset_is_implicit_,
            decltype(off) off_)
            : lim(lim_)
            , has_offset(has_offset_)
            , offset_is_implicit(offset_is_implicit_)
            , off(off_)
        {
        }

        operator std::string() const
        {
            return "LIMIT";
        }
    };

    /**
         *  Stores OFFSET only info
         */
    struct offset_t {
        int off;
    };

    /**
         *  Inherit from this class if target class can be chained with other conditions with '&&' and '||' operators
         */
    struct condition_t {
    };

    /**
         *  Collated something
         */
    template <class T>
    struct collate_t : public condition_t {
        T expr;
        internal::collate_argument argument;

        collate_t(T expr_, internal::collate_argument argument_)
            : expr(expr_)
            , argument(argument_)
        {
        }

        operator std::string() const
        {
            return constraints::collate_t { this->argument };
        }
    };

    /**
         *  Collated something with custom collate function
         */
    template <class T>
    struct named_collate {
        T expr;
        std::string name;

        named_collate() = default;

        named_collate(T expr_, std::string name_)
            : expr(expr_)
            , name(std::move(name_))
        {
        }

        operator std::string() const
        {
            return "COLLATE " + this->name;
        }
    };

    /**
         *  Result of not operator
         */
    template <class C>
    struct negated_condition_t : public condition_t {
        C c;

        negated_condition_t() = default;

        negated_condition_t(C c_)
            : c(c_)
        {
        }

        operator std::string() const
        {
            return "NOT";
        }
    };

    /**
         *  Result of and operator
         */
    template <class L, class R>
    struct and_condition_t : public condition_t {
        L l;
        R r;

        and_condition_t() = default;

        and_condition_t(L l_, R r_)
            : l(l_)
            , r(r_)
        {
        }

        operator std::string() const
        {
            return "AND";
        }
    };

    /**
         *  Result of or operator
         */
    template <class L, class R>
    struct or_condition_t : public condition_t {
        L l;
        R r;

        or_condition_t() = default;

        or_condition_t(L l_, R r_)
            : l(l_)
            , r(r_)
        {
        }

        operator std::string() const
        {
            return "OR";
        }
    };

    /**
         *  Base class for binary conditions
         */
    template <class L, class R>
    struct binary_condition : public condition_t {
        L l;
        R r;

        binary_condition() = default;

        binary_condition(L l_, R r_)
            : l(l_)
            , r(r_)
        {
        }
    };

    /**
         *  = and == operators object
         */
    template <class L, class R>
    struct is_equal_t : public binary_condition<L, R> {
        using self = is_equal_t<L, R>;

        using binary_condition<L, R>::binary_condition;

        operator std::string() const
        {
            return "=";
        }

        negated_condition_t<self> operator!() const
        {
            return { *this };
        }

        collate_t<self> collate_binary() const
        {
            return { *this, internal::collate_argument::binary };
        }

        collate_t<self> collate_nocase() const
        {
            return { *this, internal::collate_argument::nocase };
        }

        collate_t<self> collate_rtrim() const
        {
            return { *this, internal::collate_argument::rtrim };
        }

        named_collate<self> collate(std::string name) const
        {
            return { *this, std::move(name) };
        }
    };

    /**
         *  != operator object
         */
    template <class L, class R>
    struct is_not_equal_t : public binary_condition<L, R> {
        using self = is_not_equal_t<L, R>;

        using binary_condition<L, R>::binary_condition;

        operator std::string() const
        {
            return "!=";
        }

        negated_condition_t<self> operator!() const
        {
            return { *this };
        }

        collate_t<self> collate_binary() const
        {
            return { *this, internal::collate_argument::binary };
        }

        collate_t<self> collate_nocase() const
        {
            return { *this, internal::collate_argument::nocase };
        }

        collate_t<self> collate_rtrim() const
        {
            return { *this, internal::collate_argument::rtrim };
        }
    };

    /**
         *  > operator object.
         */
    template <class L, class R>
    struct greater_than_t : public binary_condition<L, R> {
        using self = greater_than_t<L, R>;

        using binary_condition<L, R>::binary_condition;

        operator std::string() const
        {
            return ">";
        }

        negated_condition_t<self> operator!() const
        {
            return { *this };
        }

        collate_t<self> collate_binary() const
        {
            return { *this, internal::collate_argument::binary };
        }

        collate_t<self> collate_nocase() const
        {
            return { *this, internal::collate_argument::nocase };
        }

        collate_t<self> collate_rtrim() const
        {
            return { *this, internal::collate_argument::rtrim };
        }
    };

    /**
         *  >= operator object.
         */
    template <class L, class R>
    struct greater_or_equal_t : public binary_condition<L, R> {
        using self = greater_or_equal_t<L, R>;

        using binary_condition<L, R>::binary_condition;

        operator std::string() const
        {
            return ">=";
        }

        negated_condition_t<self> operator!() const
        {
            return { *this };
        }

        collate_t<self> collate_binary() const
        {
            return { *this, internal::collate_argument::binary };
        }

        collate_t<self> collate_nocase() const
        {
            return { *this, internal::collate_argument::nocase };
        }

        collate_t<self> collate_rtrim() const
        {
            return { *this, internal::collate_argument::rtrim };
        }
    };

    /**
         *  < operator object.
         */
    template <class L, class R>
    struct lesser_than_t : public binary_condition<L, R> {
        using self = lesser_than_t<L, R>;

        using binary_condition<L, R>::binary_condition;

        operator std::string() const
        {
            return "<";
        }

        negated_condition_t<self> operator!() const
        {
            return { *this };
        }

        collate_t<self> collate_binary() const
        {
            return { *this, internal::collate_argument::binary };
        }

        collate_t<self> collate_nocase() const
        {
            return { *this, internal::collate_argument::nocase };
        }

        collate_t<self> collate_rtrim() const
        {
            return { *this, internal::collate_argument::rtrim };
        }
    };

    /**
         *  <= operator object.
         */
    template <class L, class R>
    struct lesser_or_equal_t : public binary_condition<L, R> {
        using self = lesser_or_equal_t<L, R>;

        using binary_condition<L, R>::binary_condition;

        operator std::string() const
        {
            return "<=";
        }

        negated_condition_t<lesser_or_equal_t<L, R>> operator!() const
        {
            return { *this };
        }

        collate_t<self> collate_binary() const
        {
            return { *this, internal::collate_argument::binary };
        }

        collate_t<self> collate_nocase() const
        {
            return { *this, internal::collate_argument::nocase };
        }

        collate_t<self> collate_rtrim() const
        {
            return { *this, internal::collate_argument::rtrim };
        }
    };

    /**
         *  IN operator object.
         */
    template <class L, class A>
    struct in_t : public condition_t {
        using self = in_t<L, A>;

        L l; //  left expression
        A arg; //  in arg
        bool negative = false; //  used in not_in

        in_t() = default;

        in_t(L l_, A arg_, bool negative_)
            : l(l_)
            , arg(std::move(arg_))
            , negative(negative_)
        {
        }

        negated_condition_t<self> operator!() const
        {
            return { *this };
        }

        operator std::string() const
        {
            if (!this->negative) {
                return "IN";
            } else {
                return "NOT IN";
            }
        }
    };

    /**
         *  IS NULL operator object.
         */
    template <class T>
    struct is_null_t {
        using self = is_null_t<T>;
        T t;

        negated_condition_t<self> operator!() const
        {
            return { *this };
        }

        operator std::string() const
        {
            return "IS NULL";
        }
    };

    /**
         *  IS NOT NULL operator object.
         */
    template <class T>
    struct is_not_null_t {
        using self = is_not_null_t<T>;

        T t;

        negated_condition_t<self> operator!() const
        {
            return { *this };
        }

        operator std::string() const
        {
            return "IS NOT NULL";
        }
    };

    /**
         *  WHERE argument holder.
         */
    template <class C>
    struct where_t {
        C c;

        operator std::string() const
        {
            return "WHERE";
        }
    };

    /**
         *  ORDER BY argument holder.
         */
    template <class O>
    struct order_by_t {
        using self = order_by_t<O>;

        O o;
        int asc_desc = 0; //  1: asc, -1: desc
        std::string _collate_argument;

        order_by_t()
            : o()
        {
        }

        order_by_t(O o_)
            : o(o_)
        {
        }

        operator std::string() const
        {
            return "ORDER BY";
        }

        self asc()
        {
            auto res = *this;
            res.asc_desc = 1;
            return res;
        }

        self desc()
        {
            auto res = *this;
            res.asc_desc = -1;
            return res;
        }

        self collate_binary() const
        {
            auto res = *this;
            res._collate_argument = constraints::collate_t::string_from_collate_argument(internal::collate_argument::binary);
            return res;
        }

        self collate_nocase() const
        {
            auto res = *this;
            res._collate_argument = constraints::collate_t::string_from_collate_argument(internal::collate_argument::nocase);
            return res;
        }

        self collate_rtrim() const
        {
            auto res = *this;
            res._collate_argument = constraints::collate_t::string_from_collate_argument(internal::collate_argument::rtrim);
            return res;
        }

        self collate(std::string name) const
        {
            auto res = *this;
            res._collate_argument = std::move(name);
            return res;
        }
    };

    /**
         *  ORDER BY pack holder.
         */
    template <class... Args>
    struct multi_order_by_t {
        std::tuple<Args...> args;

        operator std::string() const
        {
            return static_cast<std::string>(order_by_t<void*>());
        }
    };

    /**
         *  GROUP BY pack holder.
         */
    template <class... Args>
    struct group_by_t {
        std::tuple<Args...> args;

        operator std::string() const
        {
            return "GROUP BY";
        }
    };

    /**
         *  BETWEEN operator object.
         */
    template <class A, class T>
    struct between_t : public condition_t {
        A expr;
        T b1;
        T b2;

        between_t() = default;

        between_t(A expr_, T b1_, T b2_)
            : expr(expr_)
            , b1(b1_)
            , b2(b2_)
        {
        }

        operator std::string() const
        {
            return "BETWEEN";
        }
    };

    /**
         *  LIKE operator object.
         */
    template <class A, class T>
    struct like_t : public condition_t {
        A a;
        T t;

        like_t() = default;

        like_t(A a_, T t_)
            : a(a_)
            , t(t_)
        {
        }

        operator std::string() const
        {
            return "LIKE";
        }
    };

    /**
         *  CROSS JOIN holder.
         *  T is joined type which represents any mapped table.
         */
    template <class T>
    struct cross_join_t {
        using type = T;

        operator std::string() const
        {
            return "CROSS JOIN";
        }
    };

    /**
         *  NATURAL JOIN holder.
         *  T is joined type which represents any mapped table.
         */
    template <class T>
    struct natural_join_t {
        using type = T;

        operator std::string() const
        {
            return "NATURAL JOIN";
        }
    };

    /**
         *  LEFT JOIN holder.
         *  T is joined type which represents any mapped table.
         *  O is on(...) argument type.
         */
    template <class T, class O>
    struct left_join_t {
        using type = T;
        using on_type = O;

        on_type constraint;

        operator std::string() const
        {
            return "LEFT JOIN";
        }
    };

    /**
         *  Simple JOIN holder.
         *  T is joined type which represents any mapped table.
         *  O is on(...) argument type.
         */
    template <class T, class O>
    struct join_t {
        using type = T;
        using on_type = O;

        on_type constraint;

        operator std::string() const
        {
            return "JOIN";
        }
    };

    /**
         *  LEFT OUTER JOIN holder.
         *  T is joined type which represents any mapped table.
         *  O is on(...) argument type.
         */
    template <class T, class O>
    struct left_outer_join_t {
        using type = T;
        using on_type = O;

        on_type constraint;

        operator std::string() const
        {
            return "LEFT OUTER JOIN";
        }
    };

    /**
         *  on(...) argument holder used for JOIN, LEFT JOIN, LEFT OUTER JOIN and INNER JOIN
         *  T is on type argument.
         */
    template <class T>
    struct on_t {
        using type = T;

        type t;

        operator std::string() const
        {
            return "ON";
        }
    };

    /**
         *  USING argument holder.
         */
    template <class F, class O>
    struct using_t {
        F O::*column;

        operator std::string() const
        {
            return "USING";
        }
    };

    /**
         *  INNER JOIN holder.
         *  T is joined type which represents any mapped table.
         *  O is on(...) argument type.
         */
    template <class T, class O>
    struct inner_join_t {
        using type = T;
        using on_type = O;

        on_type constraint;

        operator std::string() const
        {
            return "INNER JOIN";
        }
    };

    template <class T>
    struct exists_t : condition_t {
        using type = T;
        using self = exists_t<type>;

        type t;

        exists_t() = default;

        exists_t(T t_)
            : t(std::move(t_))
        {
        }

        operator std::string() const
        {
            return "EXISTS";
        }

        negated_condition_t<self> operator!() const
        {
            return { *this };
        }
    };

    /**
         *  HAVING holder.
         *  T is having argument type.
         */
    template <class T>
    struct having_t {
        using type = T;

        type t;

        operator std::string() const
        {
            return "HAVING";
        }
    };

    template <class T, class E>
    struct cast_t {
        using to_type = T;
        using expression_type = E;

        expression_type expression;

        operator std::string() const
        {
            return "CAST";
        }
    };

}

/**
     *  Cute operators for columns
     */
template <class T, class R>
conditions::lesser_than_t<T, R> operator<(internal::expression_t<T> expr, R r)
{
    return { expr.t, r };
}

template <class L, class T>
conditions::lesser_than_t<L, T> operator<(L l, internal::expression_t<T> expr)
{
    return { l, expr.t };
}

template <class T, class R>
conditions::lesser_or_equal_t<T, R> operator<=(internal::expression_t<T> expr, R r)
{
    return { expr.t, r };
}

template <class L, class T>
conditions::lesser_or_equal_t<L, T> operator<=(L l, internal::expression_t<T> expr)
{
    return { l, expr.t };
}

template <class T, class R>
conditions::greater_than_t<T, R> operator>(internal::expression_t<T> expr, R r)
{
    return { expr.t, r };
}

template <class L, class T>
conditions::greater_than_t<L, T> operator>(L l, internal::expression_t<T> expr)
{
    return { l, expr.t };
}

template <class T, class R>
conditions::greater_or_equal_t<T, R> operator>=(internal::expression_t<T> expr, R r)
{
    return { expr.t, r };
}

template <class L, class T>
conditions::greater_or_equal_t<L, T> operator>=(L l, internal::expression_t<T> expr)
{
    return { l, expr.t };
}

template <class T, class R>
conditions::is_equal_t<T, R> operator==(internal::expression_t<T> expr, R r)
{
    return { expr.t, r };
}

template <class L, class T>
conditions::is_equal_t<L, T> operator==(L l, internal::expression_t<T> expr)
{
    return { l, expr.t };
}

template <class T, class R>
conditions::is_not_equal_t<T, R> operator!=(internal::expression_t<T> expr, R r)
{
    return { expr.t, r };
}

template <class L, class T>
conditions::is_not_equal_t<L, T> operator!=(L l, internal::expression_t<T> expr)
{
    return { l, expr.t };
}

template <class T, class R>
internal::conc_t<T, R> operator||(internal::expression_t<T> expr, R r)
{
    return { expr.t, r };
}

template <class L, class T>
internal::conc_t<L, T> operator||(L l, internal::expression_t<T> expr)
{
    return { l, expr.t };
}

template <class L, class R>
internal::conc_t<L, R> operator||(internal::expression_t<L> l, internal::expression_t<R> r)
{
    return { l.t, r.t };
}

template <class T, class R>
internal::add_t<T, R> operator+(internal::expression_t<T> expr, R r)
{
    return { expr.t, r };
}

template <class L, class T>
internal::add_t<L, T> operator+(L l, internal::expression_t<T> expr)
{
    return { l, expr.t };
}

template <class L, class R>
internal::add_t<L, R> operator+(internal::expression_t<L> l, internal::expression_t<R> r)
{
    return { l.t, r.t };
}

template <class T, class R>
internal::sub_t<T, R> operator-(internal::expression_t<T> expr, R r)
{
    return { expr.t, r };
}

template <class L, class T>
internal::sub_t<L, T> operator-(L l, internal::expression_t<T> expr)
{
    return { l, expr.t };
}

template <class L, class R>
internal::sub_t<L, R> operator-(internal::expression_t<L> l, internal::expression_t<R> r)
{
    return { l.t, r.t };
}

template <class T, class R>
internal::mul_t<T, R> operator*(internal::expression_t<T> expr, R r)
{
    return { expr.t, r };
}

template <class L, class T>
internal::mul_t<L, T> operator*(L l, internal::expression_t<T> expr)
{
    return { l, expr.t };
}

template <class L, class R>
internal::mul_t<L, R> operator*(internal::expression_t<L> l, internal::expression_t<R> r)
{
    return { l.t, r.t };
}

template <class T, class R>
internal::div_t<T, R> operator/(internal::expression_t<T> expr, R r)
{
    return { expr.t, r };
}

template <class L, class T>
internal::div_t<L, T> operator/(L l, internal::expression_t<T> expr)
{
    return { l, expr.t };
}

template <class L, class R>
internal::div_t<L, R> operator/(internal::expression_t<L> l, internal::expression_t<R> r)
{
    return { l.t, r.t };
}

template <class T, class R>
internal::mod_t<T, R> operator%(internal::expression_t<T> expr, R r)
{
    return { expr.t, r };
}

template <class L, class T>
internal::mod_t<L, T> operator%(L l, internal::expression_t<T> expr)
{
    return { l, expr.t };
}

template <class L, class R>
internal::mod_t<L, R> operator%(internal::expression_t<L> l, internal::expression_t<R> r)
{
    return { l.t, r.t };
}

template <class F, class O>
conditions::using_t<F, O> using_(F O::*p)
{
    return { p };
}

template <class T>
conditions::on_t<T> on(T t)
{
    return { t };
}

template <class T>
conditions::cross_join_t<T> cross_join()
{
    return {};
}

template <class T>
conditions::natural_join_t<T> natural_join()
{
    return {};
}

template <class T, class O>
conditions::left_join_t<T, O> left_join(O o)
{
    return { o };
}

template <class T, class O>
conditions::join_t<T, O> join(O o)
{
    return { o };
}

template <class T, class O>
conditions::left_outer_join_t<T, O> left_outer_join(O o)
{
    return { o };
}

template <class T, class O>
conditions::inner_join_t<T, O> inner_join(O o)
{
    return { o };
}

inline conditions::offset_t offset(int off)
{
    return { off };
}

inline conditions::limit_t limit(int lim)
{
    return { lim };
}

inline conditions::limit_t limit(int off, int lim)
{
    return { lim, true, true, off };
}

inline conditions::limit_t limit(int lim, conditions::offset_t offt)
{
    return { lim, true, false, offt.off };
}

template <
    class L,
    class R,
    typename = typename std::enable_if<std::is_base_of<conditions::condition_t, L>::value && std::is_base_of<conditions::condition_t, R>::value>::type>
conditions::and_condition_t<L, R> operator&&(const L& l, const R& r)
{
    return { l, r };
}

template <
    class L,
    class R,
    typename = typename std::enable_if<std::is_base_of<conditions::condition_t, L>::value && std::is_base_of<conditions::condition_t, R>::value>::type>
conditions::or_condition_t<L, R> operator||(const L& l, const R& r)
{
    return { l, r };
}

template <class T>
conditions::is_not_null_t<T> is_not_null(T t)
{
    return { t };
}

template <class T>
conditions::is_null_t<T> is_null(T t)
{
    return { t };
}

template <class L, class E>
conditions::in_t<L, std::vector<E>> in(L l, std::vector<E> values)
{
    return { std::move(l), std::move(values), false };
}

template <class L, class E>
conditions::in_t<L, std::vector<E>> in(L l, std::initializer_list<E> values)
{
    return { std::move(l), std::move(values), false };
}

template <class L, class A>
conditions::in_t<L, A> in(L l, A arg)
{
    return { std::move(l), std::move(arg), false };
}

template <class L, class E>
conditions::in_t<L, std::vector<E>> not_in(L l, std::vector<E> values)
{
    return { std::move(l), std::move(values), true };
}

template <class L, class E>
conditions::in_t<L, std::vector<E>> not_in(L l, std::initializer_list<E> values)
{
    return { std::move(l), std::move(values), true };
}

template <class L, class A>
conditions::in_t<L, A> not_in(L l, A arg)
{
    return { std::move(l), std::move(arg), true };
}

template <class L, class R>
conditions::is_equal_t<L, R> is_equal(L l, R r)
{
    return { l, r };
}

template <class L, class R>
conditions::is_equal_t<L, R> eq(L l, R r)
{
    return { l, r };
}

template <class L, class R>
conditions::is_not_equal_t<L, R> is_not_equal(L l, R r)
{
    return { l, r };
}

template <class L, class R>
conditions::is_not_equal_t<L, R> ne(L l, R r)
{
    return { l, r };
}

template <class L, class R>
conditions::greater_than_t<L, R> greater_than(L l, R r)
{
    return { l, r };
}

template <class L, class R>
conditions::greater_than_t<L, R> gt(L l, R r)
{
    return { l, r };
}

template <class L, class R>
conditions::greater_or_equal_t<L, R> greater_or_equal(L l, R r)
{
    return { l, r };
}

template <class L, class R>
conditions::greater_or_equal_t<L, R> ge(L l, R r)
{
    return { l, r };
}

template <class L, class R>
conditions::lesser_than_t<L, R> lesser_than(L l, R r)
{
    return { l, r };
}

template <class L, class R>
conditions::lesser_than_t<L, R> lt(L l, R r)
{
    return { l, r };
}

template <class L, class R>
conditions::lesser_or_equal_t<L, R> lesser_or_equal(L l, R r)
{
    return { l, r };
}

template <class L, class R>
conditions::lesser_or_equal_t<L, R> le(L l, R r)
{
    return { l, r };
}

template <class C>
conditions::where_t<C> where(C c)
{
    return { c };
}

template <class O>
conditions::order_by_t<O> order_by(O o)
{
    return { o };
}

template <class... Args>
conditions::multi_order_by_t<Args...> multi_order_by(Args&&... args)
{
    return { std::make_tuple(std::forward<Args>(args)...) };
}

template <class... Args>
conditions::group_by_t<Args...> group_by(Args&&... args)
{
    return { std::make_tuple(std::forward<Args>(args)...) };
}

template <class A, class T>
conditions::between_t<A, T> between(A expr, T b1, T b2)
{
    return { expr, b1, b2 };
}

template <class A, class T>
conditions::like_t<A, T> like(A a, T t)
{
    return { a, t };
}

template <class T>
conditions::exists_t<T> exists(T t)
{
    return { std::move(t) };
}

template <class T>
conditions::having_t<T> having(T t)
{
    return { t };
}

template <class T, class E>
conditions::cast_t<T, E> cast(E e)
{
    return { e };
}
}
#pragma once

#include <sstream> //  std::stringstream
#include <string> //  std::string
#include <type_traits> //  std::enable_if, std::is_base_of, std::is_member_pointer

namespace sqlite_orm {

/**
     *  This is base class for every class which is used as a custom table alias.
     *  For more information please look through self_join.cpp example
     */
struct alias_tag {
};

namespace internal {

    /**
         *  This is a common built-in class used for custom single character table aliases.
         *  Also you can use language aliases `alias_a`, `alias_b` etc. instead
         */
    template <class T, char A>
    struct table_alias : alias_tag {
        using type = T;

        static char get()
        {
            return A;
        }
    };

    /**
         *  Column expression with table alias attached like 'C.ID'. This is not a column alias
         */
    template <class T, class C>
    struct alias_column_t {
        using alias_type = T;
        using column_type = C;

        column_type column;

        alias_column_t() {};

        alias_column_t(column_type column_)
            : column(column_)
        {
        }
    };

    template <class T, class SFINAE = void>
    struct alias_extractor;

    template <class T>
    struct alias_extractor<T, typename std::enable_if<std::is_base_of<alias_tag, T>::value>::type> {
        static std::string get()
        {
            std::stringstream ss;
            ss << T::get();
            return ss.str();
        }
    };

    template <class T>
    struct alias_extractor<T, typename std::enable_if<!std::is_base_of<alias_tag, T>::value>::type> {
        static std::string get()
        {
            return {};
        }
    };

    template <class T, class E>
    struct as_t {
        using alias_type = T;
        using expression_type = E;

        expression_type expression;
    };

    template <class T>
    struct alias_holder {
        using type = T;
    };
}

/**
     *  @return column with table alias attached. Place it instead of a column statement in case you need to specify a
     *  column with table alias prefix like 'a.column'. For more information please look through self_join.cpp example
     */
template <class T, class C>
internal::alias_column_t<T, C> alias_column(C c)
{
    static_assert(std::is_member_pointer<C>::value, "alias_column argument must be a member pointer mapped to a storage");
    return { c };
}

template <class T, class E>
internal::as_t<T, E> as(E expression)
{
    return { std::move(expression) };
}

template <class T>
internal::alias_holder<T> get()
{
    return {};
}

template <class T>
using alias_a = internal::table_alias<T, 'a'>;
template <class T>
using alias_b = internal::table_alias<T, 'b'>;
template <class T>
using alias_c = internal::table_alias<T, 'c'>;
template <class T>
using alias_d = internal::table_alias<T, 'd'>;
template <class T>
using alias_e = internal::table_alias<T, 'e'>;
template <class T>
using alias_f = internal::table_alias<T, 'f'>;
template <class T>
using alias_g = internal::table_alias<T, 'g'>;
template <class T>
using alias_h = internal::table_alias<T, 'h'>;
template <class T>
using alias_i = internal::table_alias<T, 'i'>;
template <class T>
using alias_j = internal::table_alias<T, 'j'>;
template <class T>
using alias_k = internal::table_alias<T, 'k'>;
template <class T>
using alias_l = internal::table_alias<T, 'l'>;
template <class T>
using alias_m = internal::table_alias<T, 'm'>;
template <class T>
using alias_n = internal::table_alias<T, 'n'>;
template <class T>
using alias_o = internal::table_alias<T, 'o'>;
template <class T>
using alias_p = internal::table_alias<T, 'p'>;
template <class T>
using alias_q = internal::table_alias<T, 'q'>;
template <class T>
using alias_r = internal::table_alias<T, 'r'>;
template <class T>
using alias_s = internal::table_alias<T, 's'>;
template <class T>
using alias_t = internal::table_alias<T, 't'>;
template <class T>
using alias_u = internal::table_alias<T, 'u'>;
template <class T>
using alias_v = internal::table_alias<T, 'v'>;
template <class T>
using alias_w = internal::table_alias<T, 'w'>;
template <class T>
using alias_x = internal::table_alias<T, 'x'>;
template <class T>
using alias_y = internal::table_alias<T, 'y'>;
template <class T>
using alias_z = internal::table_alias<T, 'z'>;
}
#pragma once

// #include "conditions.h"

namespace sqlite_orm {

namespace internal {

    template <class... Args>
    struct join_iterator {

        template <class L>
        void operator()(L)
        {
            //..
        }
    };

    template <>
    struct join_iterator<> {

        template <class L>
        void operator()(L)
        {
            //..
        }
    };

    template <class H, class... Tail>
    struct join_iterator<H, Tail...> : public join_iterator<Tail...> {
        using super = join_iterator<Tail...>;

        H h;

        template <class L>
        void operator()(L l)
        {
            this->super::operator()(l);
        }
    };

    template <class T, class... Tail>
    struct join_iterator<conditions::cross_join_t<T>, Tail...> : public join_iterator<Tail...> {
        using super = join_iterator<Tail...>;

        conditions::cross_join_t<T> h;

        template <class L>
        void operator()(L l)
        {
            l(h);
            this->super::operator()(l);
        }
    };

    template <class T, class... Tail>
    struct join_iterator<conditions::natural_join_t<T>, Tail...> : public join_iterator<Tail...> {
        using super = join_iterator<Tail...>;

        conditions::natural_join_t<T> h;

        template <class L>
        void operator()(L l)
        {
            l(h);
            this->super::operator()(l);
        }
    };

    template <class T, class O, class... Tail>
    struct join_iterator<conditions::left_join_t<T, O>, Tail...> : public join_iterator<Tail...> {
        using super = join_iterator<Tail...>;

        conditions::left_join_t<T, O> h;

        template <class L>
        void operator()(L l)
        {
            l(h);
            this->super::operator()(l);
        }
    };

    template <class T, class O, class... Tail>
    struct join_iterator<conditions::join_t<T, O>, Tail...> : public join_iterator<Tail...> {
        using super = join_iterator<Tail...>;

        conditions::join_t<T, O> h;

        template <class L>
        void operator()(L l)
        {
            l(h);
            this->super::operator()(l);
        }
    };

    template <class T, class O, class... Tail>
    struct join_iterator<conditions::left_outer_join_t<T, O>, Tail...> : public join_iterator<Tail...> {
        using super = join_iterator<Tail...>;

        conditions::left_outer_join_t<T, O> h;

        template <class L>
        void operator()(L l)
        {
            l(h);
            this->super::operator()(l);
        }
    };

    template <class T, class O, class... Tail>
    struct join_iterator<conditions::inner_join_t<T, O>, Tail...> : public join_iterator<Tail...> {
        using super = join_iterator<Tail...>;

        conditions::inner_join_t<T, O> h;

        template <class L>
        void operator()(L l)
        {
            l(h);
            this->super::operator()(l);
        }
    };
}
}
#pragma once

#include <string> //  std::string
#include <tuple> //  std::make_tuple
#include <type_traits> //  std::forward, std::is_base_of, std::enable_if

// #include "conditions.h"

// #include "operators.h"

namespace sqlite_orm {

namespace core_functions {

    /**
         *  Base class for operator overloading
         */
    struct core_function_t {
    };

    /**
         *  LENGTH(x) function https://sqlite.org/lang_corefunc.html#length
         */
    template <class T>
    struct length_t : public core_function_t {
        T t;

        length_t() = default;

        length_t(T t_)
            : t(t_)
        {
        }

        operator std::string() const
        {
            return "LENGTH";
        }
    };

    /**
         *  ABS(x) function https://sqlite.org/lang_corefunc.html#abs
         */
    template <class T>
    struct abs_t : public core_function_t {
        T t;

        abs_t() = default;

        abs_t(T t_)
            : t(t_)
        {
        }

        operator std::string() const
        {
            return "ABS";
        }
    };

    /**
         *  LOWER(x) function https://sqlite.org/lang_corefunc.html#lower
         */
    template <class T>
    struct lower_t : public core_function_t {
        T t;

        lower_t() = default;

        lower_t(T t_)
            : t(t_)
        {
        }

        operator std::string() const
        {
            return "LOWER";
        }
    };

    /**
         *  UPPER(x) function https://sqlite.org/lang_corefunc.html#upper
         */
    template <class T>
    struct upper_t : public core_function_t {
        T t;

        upper_t() = default;

        upper_t(T t_)
            : t(t_)
        {
        }

        operator std::string() const
        {
            return "UPPER";
        }
    };

    /**
         *  CHANGES() function https://sqlite.org/lang_corefunc.html#changes
         */
    struct changes_t : public core_function_t {

        operator std::string() const
        {
            return "CHANGES";
        }
    };

    /**
         *  TRIM(X) function https://sqlite.org/lang_corefunc.html#trim
         */
    template <class X>
    struct trim_single_t : public core_function_t {
        X x;

        trim_single_t() = default;

        trim_single_t(X x_)
            : x(x_)
        {
        }

        operator std::string() const
        {
            return "TRIM";
        }
    };

    /**
         *  TRIM(X,Y) function https://sqlite.org/lang_corefunc.html#trim
         */
    template <class X, class Y>
    struct trim_double_t : public core_function_t {
        X x;
        Y y;

        trim_double_t() = default;

        trim_double_t(X x_, Y y_)
            : x(x_)
            , y(y_)
        {
        }

        operator std::string() const
        {
            return static_cast<std::string>(trim_single_t<X>(0));
        }
    };

    /**
         *  LTRIM(X) function https://sqlite.org/lang_corefunc.html#ltrim
         */
    template <class X>
    struct ltrim_single_t : public core_function_t {
        X x;

        ltrim_single_t() = default;

        ltrim_single_t(X x_)
            : x(x_)
        {
        }

        operator std::string() const
        {
            return "LTRIM";
        }
    };

    /**
         *  LTRIM(X,Y) function https://sqlite.org/lang_corefunc.html#ltrim
         */
    template <class X, class Y>
    struct ltrim_double_t : public core_function_t {
        X x;
        Y y;

        ltrim_double_t() = default;

        ltrim_double_t(X x_, Y y_)
            : x(x_)
            , y(y_)
        {
        }

        operator std::string() const
        {
            return static_cast<std::string>(ltrim_single_t<X>(0));
        }
    };

    /**
         *  RTRIM(X) function https://sqlite.org/lang_corefunc.html#rtrim
         */
    template <class X>
    struct rtrim_single_t : public core_function_t {
        X x;

        rtrim_single_t() = default;

        rtrim_single_t(X x_)
            : x(x_)
        {
        }

        operator std::string() const
        {
            return "RTRIM";
        }
    };

    /**
         *  RTRIM(X,Y) function https://sqlite.org/lang_corefunc.html#rtrim
         */
    template <class X, class Y>
    struct rtrim_double_t : public core_function_t {
        X x;
        Y y;

        rtrim_double_t() = default;

        rtrim_double_t(X x_, Y y_)
            : x(x_)
            , y(y_)
        {
        }

        operator std::string() const
        {
            return static_cast<std::string>(rtrim_single_t<X>(0));
        }
    };

    /**
         *  CHAR(X1,X2,...,XN) function https://sqlite.org/lang_corefunc.html#char
         */
    template <class... Args>
    struct char_t_ : public core_function_t {
        using args_type = std::tuple<Args...>;

        args_type args;

        char_t_() = default;

        char_t_(args_type args_)
            : args(args_)
        {
        }

        operator std::string() const
        {
            return "CHAR";
        }
    };

    struct random_t : core_function_t, internal::arithmetic_t {

        operator std::string() const
        {
            return "RANDOM";
        }
    };

    template <class T, class... Args>
    struct date_t : core_function_t {
        using modifiers_type = std::tuple<Args...>;

        T timestring;
        modifiers_type modifiers;

        date_t() = default;

        date_t(T timestring_, modifiers_type modifiers_)
            : timestring(timestring_)
            , modifiers(modifiers_)
        {
        }

        operator std::string() const
        {
            return "DATE";
        }
    };

    template <class T, class... Args>
    struct datetime_t : core_function_t {
        using modifiers_type = std::tuple<Args...>;

        T timestring;
        modifiers_type modifiers;

        datetime_t() = default;

        datetime_t(T timestring_, modifiers_type modifiers_)
            : timestring(timestring_)
            , modifiers(modifiers_)
        {
        }

        operator std::string() const
        {
            return "DATETIME";
        }
    };

    template <class T, class... Args>
    struct julianday_t : core_function_t, internal::arithmetic_t {
        using modifiers_type = std::tuple<Args...>;

        T timestring;
        modifiers_type modifiers;

        julianday_t() = default;

        julianday_t(T timestring_, modifiers_type modifiers_)
            : timestring(timestring_)
            , modifiers(modifiers_)
        {
        }

        operator std::string() const
        {
            return "JULIANDAY";
        }
    };
}

/**
     *  Cute operators for core functions
     */

template <
    class F,
    class R,
    typename = typename std::enable_if<std::is_base_of<core_functions::core_function_t, F>::value>::type>
conditions::lesser_than_t<F, R> operator<(F f, R r)
{
    return { f, r };
}

template <
    class F,
    class R,
    typename = typename std::enable_if<std::is_base_of<core_functions::core_function_t, F>::value>::type>
conditions::lesser_or_equal_t<F, R> operator<=(F f, R r)
{
    return { f, r };
}

template <
    class F,
    class R,
    typename = typename std::enable_if<std::is_base_of<core_functions::core_function_t, F>::value>::type>
conditions::greater_than_t<F, R> operator>(F f, R r)
{
    return { f, r };
}

template <
    class F,
    class R,
    typename = typename std::enable_if<std::is_base_of<core_functions::core_function_t, F>::value>::type>
conditions::greater_or_equal_t<F, R> operator>=(F f, R r)
{
    return { f, r };
}

template <
    class F,
    class R,
    typename = typename std::enable_if<std::is_base_of<core_functions::core_function_t, F>::value>::type>
conditions::is_equal_t<F, R> operator==(F f, R r)
{
    return { f, r };
}

template <
    class F,
    class R,
    typename = typename std::enable_if<std::is_base_of<core_functions::core_function_t, F>::value>::type>
conditions::is_not_equal_t<F, R> operator!=(F f, R r)
{
    return { f, r };
}

inline core_functions::random_t random()
{
    return {};
}

template <class T, class... Args, class Res = core_functions::date_t<T, Args...>>
Res date(T timestring, Args... modifiers)
{
    return Res(timestring, std::make_tuple(std::forward<Args>(modifiers)...));
}

template <class T, class... Args, class Res = core_functions::datetime_t<T, Args...>>
Res datetime(T timestring, Args... modifiers)
{
    return Res(timestring, std::make_tuple(std::forward<Args>(modifiers)...));
}

template <class T, class... Args, class Res = core_functions::julianday_t<T, Args...>>
Res julianday(T timestring, Args... modifiers)
{
    return Res(timestring, std::make_tuple(std::forward<Args>(modifiers)...));
}

template <class... Args>
core_functions::char_t_<Args...> char_(Args&&... args)
{
    using result_type = core_functions::char_t_<Args...>;
    return result_type(std::make_tuple(std::forward<Args>(args)...));
}

template <class X, class Res = core_functions::trim_single_t<X>>
Res trim(X x)
{
    return Res(x);
}

template <class X, class Y, class Res = core_functions::trim_double_t<X, Y>>
Res trim(X x, Y y)
{
    return Res(x, y);
}

template <class X, class Res = core_functions::ltrim_single_t<X>>
Res ltrim(X x)
{
    return Res(x);
}

template <class X, class Y, class Res = core_functions::ltrim_double_t<X, Y>>
Res ltrim(X x, Y y)
{
    return Res(x, y);
}

template <class X, class Res = core_functions::rtrim_single_t<X>>
Res rtrim(X x)
{
    return Res(x);
}

template <class X, class Y, class Res = core_functions::rtrim_double_t<X, Y>>
Res rtrim(X x, Y y)
{
    return Res(x, y);
}

inline core_functions::changes_t changes()
{
    return {};
}

template <class T>
core_functions::length_t<T> length(T t)
{
    using result_type = core_functions::length_t<T>;
    return result_type(t);
}

template <class T>
core_functions::abs_t<T> abs(T t)
{
    using result_type = core_functions::abs_t<T>;
    return result_type(t);
}

template <class T, class Res = core_functions::lower_t<T>>
Res lower(T t)
{
    return Res(t);
}

template <class T, class Res = core_functions::upper_t<T>>
Res upper(T t)
{
    return Res(t);
}

template <
    class L,
    class R,
    typename = typename std::enable_if<(std::is_base_of<internal::arithmetic_t, L>::value + std::is_base_of<internal::arithmetic_t, R>::value > 0)>::type>
internal::add_t<L, R> operator+(L l, R r)
{
    return { std::move(l), std::move(r) };
}

template <
    class L,
    class R,
    typename = typename std::enable_if<(std::is_base_of<internal::arithmetic_t, L>::value + std::is_base_of<internal::arithmetic_t, R>::value > 0)>::type>
internal::sub_t<L, R> operator-(L l, R r)
{
    return { std::move(l), std::move(r) };
}

template <
    class L,
    class R,
    typename = typename std::enable_if<(std::is_base_of<internal::arithmetic_t, L>::value + std::is_base_of<internal::arithmetic_t, R>::value > 0)>::type>
internal::mul_t<L, R> operator*(L l, R r)
{
    return { std::move(l), std::move(r) };
}

template <
    class L,
    class R,
    typename = typename std::enable_if<(std::is_base_of<internal::arithmetic_t, L>::value + std::is_base_of<internal::arithmetic_t, R>::value > 0)>::type>
internal::div_t<L, R> operator/(L l, R r)
{
    return { std::move(l), std::move(r) };
}

template <
    class L,
    class R,
    typename = typename std::enable_if<(std::is_base_of<internal::arithmetic_t, L>::value + std::is_base_of<internal::arithmetic_t, R>::value > 0)>::type>
internal::mod_t<L, R> operator%(L l, R r)
{
    return { std::move(l), std::move(r) };
}
}
#pragma once

namespace sqlite_orm {

namespace aggregate_functions {

    template <class T>
    struct avg_t {
        T t;

        operator std::string() const
        {
            return "AVG";
        }
    };

    template <class T>
    struct count_t {
        T t;

        operator std::string() const
        {
            return "COUNT";
        }
    };

    /**
         *  T is use to specify type explicitly for queries like
         *  SELECT COUNT(*) FROM table_name;
         *  T can be omitted with void.
         */
    template <class T>
    struct count_asterisk_t {
        using type = T;

        operator std::string() const
        {
            return "COUNT";
        }
    };

    struct count_asterisk_without_type {
        operator std::string() const
        {
            return "COUNT";
        }
    };

    template <class T>
    struct sum_t {
        T t;

        operator std::string() const
        {
            return "SUM";
        }
    };

    template <class T>
    struct total_t {
        T t;

        operator std::string() const
        {
            return "TOTAL";
        }
    };

    template <class T>
    struct max_t {
        T t;

        operator std::string() const
        {
            return "MAX";
        }
    };

    template <class T>
    struct min_t {
        T t;

        operator std::string() const
        {
            return "MIN";
        }
    };

    template <class T>
    struct group_concat_single_t {
        T t;

        operator std::string() const
        {
            return "GROUP_CONCAT";
        }
    };

    template <class T>
    struct group_concat_double_t {
        T t;
        std::string y;

        operator std::string() const
        {
            return "GROUP_CONCAT";
        }
    };

}

template <class T>
aggregate_functions::avg_t<T> avg(T t)
{
    return { t };
}

template <class T>
aggregate_functions::count_t<T> count(T t)
{
    return { t };
}

inline aggregate_functions::count_asterisk_without_type count()
{
    return {};
}

template <class T>
aggregate_functions::count_asterisk_t<T> count()
{
    return {};
}

template <class T>
aggregate_functions::sum_t<T> sum(T t)
{
    return { t };
}

template <class T>
aggregate_functions::max_t<T> max(T t)
{
    return { t };
}

template <class T>
aggregate_functions::min_t<T> min(T t)
{
    return { t };
}

template <class T>
aggregate_functions::total_t<T> total(T t)
{
    return { t };
}

template <class T>
aggregate_functions::group_concat_single_t<T> group_concat(T t)
{
    return { t };
}

template <class T, class Y>
aggregate_functions::group_concat_double_t<T> group_concat(T t, Y y)
{
    return { t, y };
}
}
#pragma once

namespace sqlite_orm {

namespace internal {

    /**
         *  Cute class used to compare setters/getters and member pointers with each other.
         */
    template <class L, class R>
    struct typed_comparator {
        bool operator()(const L&, const R&) const
        {
            return false;
        }
    };

    template <class O>
    struct typed_comparator<O, O> {
        bool operator()(const O& lhs, const O& rhs) const
        {
            return lhs == rhs;
        }
    };

    template <class L, class R>
    bool compare_any(const L& lhs, const R& rhs)
    {
        return typed_comparator<L, R>()(lhs, rhs);
    }
}
}
#pragma once

#include <string> //  std::string

namespace sqlite_orm {

namespace internal {

    template <template <typename...> class C, typename... Ts>
    std::true_type is_base_of_template_impl(const C<Ts...>*);

    template <template <typename...> class C>
    std::false_type is_base_of_template_impl(...);

    template <typename T, template <typename...> class C>
    using is_base_of_template = decltype(is_base_of_template_impl<C>(std::declval<T*>()));

    /**
         *  DISCTINCT generic container.
         */
    template <class T>
    struct distinct_t {
        T t;

        operator std::string() const
        {
            return "DISTINCT";
        }
    };

    /**
         *  ALL generic container.
         */
    template <class T>
    struct all_t {
        T t;

        operator std::string() const
        {
            return "ALL";
        }
    };

    template <class... Args>
    struct columns_t {
        bool distinct = false;

        template <class L>
        void for_each(L) const
        {
            //..
        }

        int count() const
        {
            return 0;
        }
    };

    template <class T, class... Args>
    struct columns_t<T, Args...> : public columns_t<Args...> {
        T m;

        columns_t(decltype(m) m_, Args&&... args)
            : super(std::forward<Args>(args)...)
            , m(m_)
        {
        }

        template <class L>
        void for_each(L l) const
        {
            l(this->m);
            this->super::for_each(l);
        }

        int count() const
        {
            return 1 + this->super::count();
        }

    private:
        using super = columns_t<Args...>;
    };

    template <class... Args>
    struct set_t {

        operator std::string() const
        {
            return "SET";
        }

        template <class F>
        void for_each(F)
        {
            //..
        }
    };

    template <class L, class... Args>
    struct set_t<L, Args...> : public set_t<Args...> {
        static_assert(is_assign_t<typename std::remove_reference<L>::type>::value, "set_t argument must be assign_t");

        L l;

        using super = set_t<Args...>;
        using self = set_t<L, Args...>;

        set_t(L l_, Args&&... args)
            : super(std::forward<Args>(args)...)
            , l(std::forward<L>(l_))
        {
        }

        template <class F>
        void for_each(F f)
        {
            f(l);
            this->super::for_each(f);
        }
    };

    /**
         *  This class is used to store explicit mapped type T and its column descriptor (member pointer/getter/setter).
         *  Is useful when mapped type is derived from other type and base class has members mapped to a storage.
         */
    template <class T, class F>
    struct column_pointer {
        using type = T;
        using field_type = F;

        field_type field;
    };

    /**
         *  Subselect object type.
         */
    template <class T, class... Args>
    struct select_t {
        using return_type = T;
        using conditions_type = std::tuple<Args...>;

        return_type col;
        conditions_type conditions;
        bool highest_level = false;
    };

    /**
         *  Base for UNION, UNION ALL, EXCEPT and INTERSECT
         */
    template <class L, class R>
    struct compound_operator {
        using left_type = L;
        using right_type = R;

        left_type left;
        right_type right;

        compound_operator(left_type l, right_type r)
            : left(std::move(l))
            , right(std::move(r))
        {
            this->left.highest_level = true;
            this->right.highest_level = true;
        }
    };

    /**
         *  UNION object type.
         */
    template <class L, class R>
    struct union_t : public compound_operator<L, R> {
        using super = compound_operator<L, R>;
        using left_type = typename super::left_type;
        using right_type = typename super::right_type;

        bool all = false;

        union_t(left_type l, right_type r, decltype(all) all_)
            : super(std::move(l), std::move(r))
            , all(all_)
        {
        }

        union_t(left_type l, right_type r)
            : union_t(std::move(l), std::move(r), false)
        {
        }

        operator std::string() const
        {
            if (!this->all) {
                return "UNION";
            } else {
                return "UNION ALL";
            }
        }
    };

    /**
         *  EXCEPT object type.
         */
    template <class L, class R>
    struct except_t : public compound_operator<L, R> {
        using super = compound_operator<L, R>;
        using left_type = typename super::left_type;
        using right_type = typename super::right_type;

        using super::super;

        operator std::string() const
        {
            return "EXCEPT";
        }
    };

    /**
         *  INTERSECT object type.
         */
    template <class L, class R>
    struct intersect_t : public compound_operator<L, R> {
        using super = compound_operator<L, R>;
        using left_type = typename super::left_type;
        using right_type = typename super::right_type;

        using super::super;

        operator std::string() const
        {
            return "INTERSECT";
        }
    };

    /**
         *  Generic way to get DISTINCT value from any type.
         */
    template <class T>
    bool get_distinct(const T& t)
    {
        return false;
    }

    template <class... Args>
    bool get_distinct(const columns_t<Args...>& cols)
    {
        return cols.distinct;
    }

    template <class T>
    struct asterisk_t {
        using type = T;
    };
}

template <class T>
internal::distinct_t<T> distinct(T t)
{
    return { t };
}

template <class T>
internal::all_t<T> all(T t)
{
    return { t };
}

template <class... Args>
internal::columns_t<Args...> distinct(internal::columns_t<Args...> cols)
{
    cols.distinct = true;
    return cols;
}

/**
     *  SET keyword used in UPDATE ... SET queries.
     *  Args must have `assign_t` type. E.g. set(assign(&User::id, 5)) or set(c(&User::id) = 5)
     */
template <class... Args>
internal::set_t<Args...> set(Args&&... args)
{
    return { std::forward<Args>(args)... };
}

template <class... Args>
internal::columns_t<Args...> columns(Args&&... args)
{
    return { std::forward<Args>(args)... };
}

/**
     *  Use it like this:
     *  struct MyType : BaseType { ... };
     *  storage.select(column<MyType>(&BaseType::id));
     */
template <class T, class F>
internal::column_pointer<T, F> column(F f)
{
    return { f };
}

/**
     *  Public function for subselect query. Is useful in UNION queries.
     */
template <class T, class... Args>
internal::select_t<T, Args...> select(T t, Args... args)
{
    return { std::move(t), std::make_tuple<Args...>(std::forward<Args>(args)...) };
}

/**
     *  Public function for UNION operator.
     *  lhs and rhs are subselect objects.
     *  Look through example in examples/union.cpp
     */
template <class L, class R>
internal::union_t<L, R> union_(L lhs, R rhs)
{
    return { std::move(lhs), std::move(rhs) };
}

/**
     *  Public function for EXCEPT operator.
     *  lhs and rhs are subselect objects.
     *  Look through example in examples/except.cpp
     */
template <class L, class R>
internal::except_t<L, R> except(L lhs, R rhs)
{
    return { std::move(lhs), std::move(rhs) };
}

template <class L, class R>
internal::intersect_t<L, R> intersect(L lhs, R rhs)
{
    return { std::move(lhs), std::move(rhs) };
}

/**
     *  Public function for UNION ALL operator.
     *  lhs and rhs are subselect objects.
     *  Look through example in examples/union.cpp
     */
template <class L, class R>
internal::union_t<L, R> union_all(L lhs, R rhs)
{
    return { std::move(lhs), std::move(rhs), true };
}

template <class T>
internal::asterisk_t<T> asterisk()
{
    return {};
}
}
#pragma once

#include <string> //  std::string

// #include "database.h"

// #include "error_code.h"

#include <map>
#include <memory>
#include <system_error> //  std::error_code, std::system_error

namespace sqlite_orm {

namespace internal {

    struct database_connection {
        static database* make_database(std::string uri)
        {
            auto driverPos = uri.find("://");

            auto driver = driverPos != std::string::npos ? uri.substr(0, driverPos) : std::string { "sqlite" };
            auto args = driverPos != std::string::npos ? uri.substr(driverPos)
                                                       : uri;
            auto ldb = db_driver_factory::create(driver, args);
            auto pdb = ldb.release();
            return pdb;
        }

        database_connection(database* database)
            : db(database)
        {
            auto rc = db->open();
            if (!rc) {
                throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
            }
        }
        database_connection(std::string uri)
            : database_connection(make_database(uri))
        {
        }
        ~database_connection()
        {
            db->close();
            delete db;
        }

        database* get_db()
        {
            return db;
        }

    private:
        database* db = nullptr;
    };
}
}
#pragma once

#include <type_traits> //  std::enable_if, std::is_member_pointer

// #include "select_constraints.h"

// #include "column.h"

namespace sqlite_orm {

namespace internal {

    /**
         *  Trait class used to define table mapped type by setter/getter/member
         *  T - member pointer
         */
    template <class T, class SFINAE = void>
    struct table_type;

    template <class O, class F>
    struct table_type<F O::*, typename std::enable_if<std::is_member_pointer<F O::*>::value && !std::is_member_function_pointer<F O::*>::value>::type> {
        using type = O;
    };

    template <class T>
    struct table_type<T, typename std::enable_if<is_getter<T>::value>::type> {
        using type = typename getter_traits<T>::object_type;
    };

    template <class T>
    struct table_type<T, typename std::enable_if<is_setter<T>::value>::type> {
        using type = typename setter_traits<T>::object_type;
    };

    template <class T, class F>
    struct table_type<column_pointer<T, F>, void> {
        using type = T;
    };
}
}
#pragma once

namespace sqlite_orm {

/**
     *  Helper classes used by statement_binder and row_extractor.
     */
struct int_or_smaller_tag {
};
struct bigint_tag {
};
struct real_tag {
};

template <class V>
struct arithmetic_tag {
    using type = std::conditional_t<
        std::is_integral<V>::value,
        // Integer class
        std::conditional_t<
            sizeof(V) <= sizeof(int),
            int_or_smaller_tag,
            bigint_tag>,
        // Floating-point class
        real_tag>;
};

template <class V>
using arithmetic_tag_t = typename arithmetic_tag<V>::type;
}
#pragma once

namespace sqlite_orm {

/**
     *  Specialization for optional type (std::shared_ptr / std::unique_ptr).
     */
template <typename T>
struct is_std_ptr : std::false_type {
};

template <typename T>
struct is_std_ptr<std::shared_ptr<T>> : std::true_type {
    static std::shared_ptr<T> make(const T& v)
    {
        return std::make_shared<T>(v);
    }
};

template <typename T>
struct is_std_ptr<std::unique_ptr<T>> : std::true_type {
    static std::unique_ptr<T> make(const T& v)
    {
        return std::make_unique<T>(v);
    }
};
}
#pragma once

#include <locale> // std::wstring_convert
#include <string> //  std::string, std::wstring
#include <type_traits> //  std::enable_if_t, std::is_arithmetic, std::is_same
#ifndef SQLITE_ORM_OMITS_CODECVT
#include <codecvt> //  std::wstring_convert, std::codecvt_utf8_utf16
#endif //  SQLITE_ORM_OMITS_CODECVT
#include <cstddef> //  std::nullptr_t
#include <vector> //  std::vector

// #include "database.h"

// #include "is_std_ptr.h"

namespace sqlite_orm {

/**
     *  Helper class used for binding fields to statements.
     */
template <class V, typename Enable = void>
struct statement_binder {
    int bind(database* db, query* stmt, int index, const V& value);
};

/**
     *  Specialization for arithmetic types.
     */
template <class V>
struct statement_binder<
    V,
    std::enable_if_t<std::is_arithmetic<V>::value>> {
    int bind(database* db, query* stmt, int index, const V& value)
    {
        return bind(db, stmt, index, value, tag());
    }

private:
    using tag = arithmetic_tag_t<V>;

    int bind(database* db, query* stmt, int index, const V& value, const int_or_smaller_tag&)
    {
        db->bindInt(stmt, index, static_cast<int>(value));
        return 0;
    }

    int bind(database* db, query* stmt, int index, const V& value, const bigint_tag&)
    {
        db->bindBigInt(stmt, index, static_cast<int64_t>(value));
        return 0;
    }

    int bind(database* db, query* stmt, int index, const V& value, const real_tag&)
    {
        db->bindDouble(stmt, index, static_cast<double>(value));
        return 0;
    }
};

/**
     *  Specialization for std::string
     */
template <class V>
struct statement_binder<V, std::enable_if_t<std::is_same<V, std::string>::value || std::is_same<V, const char*>::value>> {
    int bind(database* db, query* stmt, int index, const V& value)
    {
        db->bindString(stmt, index, value);
        return 0;
    }
    int bind(database* db, query* stmt, int index, const char* value)
    {
        db->bindString(stmt, index, std::string(value));
        return 0;
    }
};

/**
     *  Specialization for const char*
     */
template <>
struct statement_binder<const char*> {
    int bind(database* db, query* stmt, int index, const char* value)
    {
        db->bindString(stmt, index, value, -1);
        return 0;
    }
};

#ifndef SQLITE_ORM_OMITS_CODECVT
/**
     *  Specialization for std::wstring and C-wstring.
     */
template <class V>
struct statement_binder<
    V,
    std::enable_if_t<
        std::is_same<V, std::wstring>::value
        || std::is_same<V, const wchar_t*>::value>> {
    int bind(database* db, query* stmt, int index, const V& value)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::string utf8Str = converter.to_bytes(value);
        return statement_binder<decltype(utf8Str)>().bind(db, stmt, index, utf8Str);
    }
};
#endif //  SQLITE_ORM_OMITS_CODECVT
/**
     *  Specialization for std::nullptr_t.
     */
template <class V>
struct statement_binder<
    V,
    std::enable_if_t<std::is_same<V, std::nullptr_t>::value>> {
    int bind(database* db, query* stmt, int index, const V&)
    {
        db->bindNull(stmt, index);
        return 0;
    }
};

template <class V>
struct statement_binder<
    V,
    std::enable_if_t<is_std_ptr<V>::value>> {
    using value_type = typename V::element_type;

    int bind(database* db, query* stmt, int index, const V& value)
    {
        if (value) {
            return statement_binder<value_type>().bind(db, stmt, index, *value);
        } else {
            return statement_binder<std::nullptr_t>().bind(db, stmt, index, nullptr);
        }
    }
};

/**
     *  Specialization for optional type (std::vector<char>).
     */
template <class V>
struct statement_binder<
    V,
    std::enable_if_t<std::is_same<V, std::vector<char>>::value>> {
    int bind(database* db, query* stmt, int index, const V& value)
    {
        if (value.size()) {
            db->bindBlob(stmt, index, value.data(), static_cast<size_t>(value.size()));
        } else {
            db->bindBlob(stmt, index, "", 0);
        }
        return 0;
    }
};
}
#pragma once

#include <cstdlib> //  atof, atoi, atoll
#include <string> //  std::string, std::wstring
#include <type_traits> //  std::enable_if_t, std::is_arithmetic, std::is_same, std::enable_if
#ifndef SQLITE_ORM_OMITS_CODECVT
#include <codecvt> //  std::wstring_convert, std::codecvt_utf8_utf16
#endif //  SQLITE_ORM_OMITS_CODECVT
#include <algorithm> //  std::copy
#include <cstring> //  strlen
#include <iterator> //  std::back_inserter
#include <tuple> //  std::tuple, std::tuple_size, std::tuple_element
#include <vector> //  std::vector

// #include "arithmetic_tag.h"

// #include "error_code.h"

// #include "journal_mode.h"

#include <algorithm> //  std::transform
#include <array> //  std::array
#include <locale> // std::toupper
#include <memory> //  std::unique_ptr
#include <string> //  std::string

namespace sqlite_orm {

/**
     *  Caps case cause of 1) delete keyword; 2) https://www.sqlite.org/pragma.html#pragma_journal_mode original spelling
     */
#ifdef DELETE
#undef DELETE
#endif
enum class journal_mode : char {
    NA = -1,
    DELETE = 0,
    TRUNCATE = 1,
    PERSIST = 2,
    MEMORY = 3,
    WAL = 4,
    OFF = 5,
};

namespace internal {

    inline const std::string& to_string(journal_mode j)
    {
        static std::string res[] = {
            "DELETE",
            "TRUNCATE",
            "PERSIST",
            "MEMORY",
            "WAL",
            "OFF",
        };
        return res[static_cast<int>(j)];
    }

    inline std::unique_ptr<journal_mode> journal_mode_from_string(const std::string& str)
    {
        std::string upper_str;
        std::transform(str.begin(), str.end(), std::back_inserter(upper_str), ::toupper);
        static std::array<journal_mode, 6> all = {
            journal_mode::DELETE,
            journal_mode::TRUNCATE,
            journal_mode::PERSIST,
            journal_mode::MEMORY,
            journal_mode::WAL,
            journal_mode::OFF,
        };
        for (auto j : all) {
            if (to_string(j) == upper_str) {
                return std::make_unique<journal_mode>(j);
            }
        }
        return std::make_unique<journal_mode>(journal_mode::NA);
    }
}
}

// #include "query.h"

namespace sqlite_orm {

/**
     *  Helper class used to cast values from argv to V class
     *  which depends from column type.
     *
     */
template <class V, typename Enable = void>
struct row_extractor {
    //  used in sqlite3_exec (select)
    V extract(const char* row_value);

    //  used in sqlite_column (iteration, get_all)
    V extract(query* stmt, int columnIndex);
};

/**
     *  Specialization for arithmetic types.
     */
template <class V>
struct row_extractor<
    V,
    std::enable_if_t<std::is_arithmetic<V>::value>> {
    V extract(const char* row_value)
    {
        return extract(row_value, tag());
    }

    V extract(query* stmt, int columnIndex)
    {
        return extract(stmt, columnIndex, tag());
    }

private:
    using tag = arithmetic_tag_t<V>;

    V extract(const char* row_value, const int_or_smaller_tag&)
    {
        return static_cast<V>(atoi(row_value));
    }

    V extract(query* stmt, int columnIndex, const int_or_smaller_tag&)
    {
        return static_cast<V>(stmt->columnInt(columnIndex));
    }

    V extract(const char* row_value, const bigint_tag&)
    {
        return static_cast<V>(atoll(row_value));
    }

    V extract(query* stmt, int columnIndex, const bigint_tag&)
    {
        return static_cast<V>(stmt->columnBigInt(columnIndex));
    }

    V extract(const char* row_value, const real_tag&)
    {
        return static_cast<V>(atof(row_value));
    }

    V extract(query* stmt, int columnIndex, const real_tag&)
    {
        return static_cast<V>(stmt->columnDouble(columnIndex));
    }
};

/**
     *  Specialization for std::string.
     */
template <class V>
struct row_extractor<
    V,
    std::enable_if_t<std::is_same<V, std::string>::value>> {
    std::string extract(const char* row_value)
    {
        if (row_value) {
            return row_value;
        } else {
            return {};
        }
    }

    std::string extract(query* stmt, int columnIndex = 0)
    {
        return stmt->columnString(columnIndex);
    }
};
#ifndef SQLITE_ORM_OMITS_CODECVT
/**
     *  Specialization for std::wstring.
     */
template <class V>
struct row_extractor<
    V,
    std::enable_if_t<std::is_same<V, std::wstring>::value>> {
    std::wstring extract(const char* row_value)
    {
        if (row_value) {
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            return converter.from_bytes(row_value);
        } else {
            return {};
        }
    }

    std::wstring extract(query* stmt, int columnIndex)
    {
        auto str = stmt->columnString(columnIndex);
        if (!str.empty()) {
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            return converter.from_bytes(str);
        }
        return {};
    }
};
#endif //  SQLITE_ORM_OMITS_CODECVT
/**
     *  Specialization for std::vector<char>.
     */
template <class V>
struct row_extractor<
    V,
    std::enable_if_t<std::is_same<V, std::vector<char>>::value>> {
    std::vector<char> extract(const char* row_value)
    {
        if (row_value) {
            auto len = ::strlen(row_value); //? WTF why stop at first null
            return this->go(row_value, static_cast<int>(len));
        } else {
            return {};
        }
    }

    std::vector<char> extract(query* stmt, int columnIndex)
    {
        return stmt->columnBlob(columnIndex);
    }

protected:
    std::vector<char> go(const char* bytes, int len)
    {
        if (len) {
            std::vector<char> res(bytes, bytes + len);
            res.reserve(len);
            std::copy(bytes,
                bytes + len,
                std::back_inserter(res));
            return res;
        } else {
            return {};
        }
    }
};

template <class V>
struct row_extractor<
    V,
    std::enable_if_t<is_std_ptr<V>::value>> {
    using value_type = typename V::element_type;

    V extract(const char* row_value)
    {
        if (row_value) {
            return is_std_ptr<V>::make(row_extractor<value_type>().extract(row_value));
        } else {
            return {};
        }
    }

    V extract(query* stmt, int columnIndex)
    {
        if (stmt->isColumnValid(columnIndex)) {
            return is_std_ptr<V>::make(row_extractor<value_type>().extract(stmt, columnIndex));
        } else {
            return {};
        }
    }
};

/**
     *  Specialization for std::vector<char>.
     */
template <>
struct row_extractor<std::vector<char>> {
    std::vector<char> extract(const char* row_value)
    {
        if (row_value) {
            auto len = ::strlen(row_value);
            return this->go(row_value, static_cast<int>(len));
        } else {
            return {};
        }
    }

    std::vector<char> extract(query* stmt, int columnIndex)
    {
        return stmt->columnBlob(columnIndex);
    }

protected:
    std::vector<char> go(const char* bytes, int len)
    {
        if (len) {
            std::vector<char> res;
            res.reserve(len);
            std::copy(bytes,
                bytes + len,
                std::back_inserter(res));
            return res;
        } else {
            return {};
        }
    }
};

template <class... Args>
struct row_extractor<std::tuple<Args...>> {

    std::tuple<Args...> extract(char** argv)
    {
        std::tuple<Args...> res;
        this->extract<std::tuple_size<decltype(res)>::value>(res, argv);
        return res;
    }

    std::tuple<Args...> extract(query* stmt, int /*columnIndex*/)
    {
        std::tuple<Args...> res;
        this->extract<std::tuple_size<decltype(res)>::value>(res, stmt);
        return res;
    }

protected:
    template <size_t I, typename std::enable_if<I != 0>::type* = nullptr>
    void extract(std::tuple<Args...>& t, query* stmt)
    {
        using tuple_type = typename std::tuple_element<I - 1, typename std::tuple<Args...>>::type;
        std::get<I - 1>(t) = row_extractor<tuple_type>().extract(stmt, I - 1);
        this->extract<I - 1>(t, stmt);
    }

    template <size_t I, typename std::enable_if<I == 0>::type* = nullptr>
    void extract(std::tuple<Args...>&, query*)
    {
        //..
    }

    template <size_t I, typename std::enable_if<I != 0>::type* = nullptr>
    void extract(std::tuple<Args...>& t, char** argv)
    {
        using tuple_type = typename std::tuple_element<I - 1, typename std::tuple<Args...>>::type;
        std::get<I - 1>(t) = row_extractor<tuple_type>().extract(argv[I - 1]);
        this->extract<I - 1>(t, argv);
    }

    template <size_t I, typename std::enable_if<I == 0>::type* = nullptr>
    void extract(std::tuple<Args...>&, char**)
    {
        //..
    }
};

/**
     *  Specialization for journal_mode.
     */
template <class V>
struct row_extractor<
    V,
    std::enable_if_t<std::is_same<V, journal_mode>::value>> {
    journal_mode extract(const char* row_value)
    {
        if (row_value) {
            if (auto res = internal::journal_mode_from_string(row_value)) {
                return std::move(*res);
            } else {
                throw std::system_error(std::make_error_code(orm_error_code::incorrect_journal_mode_string));
            }
        } else {
            throw std::system_error(std::make_error_code(orm_error_code::incorrect_journal_mode_string));
        }
    }

    journal_mode extract(query* stmt, int columnIndex)
    {
        auto str = stmt->columnString(columnIndex);
        return this->extract(str.c_str());
    }
};
}
#pragma once

#include <ostream>

namespace sqlite_orm {

enum class sync_schema_result {

    /**
         *  created new table, table with the same tablename did not exist
         */
    new_table_created,

    /**
         *  table schema is the same as storage, nothing to be done
         */
    already_in_sync,

    /**
         *  removed excess columns in table (than storage) without dropping a table
         */
    old_columns_removed,

    /**
         *  lacking columns in table (than storage) added without dropping a table
         */
    new_columns_added,

    /**
         *  both old_columns_removed and new_columns_added
         */
    new_columns_added_and_old_columns_removed,

    /**
         *  old table is dropped and new is recreated. Reasons :
         *      1. delete excess columns in the table than storage if preseve = false
         *      2. Lacking columns in the table cannot be added due to NULL and DEFAULT constraint
         *      3. Reasons 1 and 2 both together
         *      4. data_type mismatch between table and storage.
         */
    dropped_and_recreated,
};

inline std::ostream& operator<<(std::ostream& os, sync_schema_result value)
{
    switch (value) {
    case sync_schema_result::new_table_created:
        return os << "new table created";
    case sync_schema_result::already_in_sync:
        return os << "table and storage is already in sync.";
    case sync_schema_result::old_columns_removed:
        return os << "old excess columns removed";
    case sync_schema_result::new_columns_added:
        return os << "new columns added";
    case sync_schema_result::new_columns_added_and_old_columns_removed:
        return os << "old excess columns removed and new columns added";
    case sync_schema_result::dropped_and_recreated:
        return os << "old table dropped and recreated";
    default:
        return os;
    }
}
}
#pragma once

#include <string> //  std::string
#include <tuple> //  std::tuple, std::make_tuple

namespace sqlite_orm {

namespace internal {

    template <class... Cols>
    struct index_t {
        using columns_type = std::tuple<Cols...>;
        using object_type = void;

        std::string name;
        bool unique;
        columns_type columns;

        template <class L>
        void for_each_column_with_constraints(L) {}
    };
}

template <class... Cols>
internal::index_t<Cols...> make_index(const std::string& name, Cols... cols)
{
    return { name, false, std::make_tuple(cols...) };
}

template <class... Cols>
internal::index_t<Cols...> make_unique_index(const std::string& name, Cols... cols)
{
    return { name, true, std::make_tuple(cols...) };
}
}
#pragma once

// #include "alias.h"

namespace sqlite_orm {

namespace internal {

    /**
         *  If T is alias than mapped_type_proxy<T>::type is alias::type
         *  otherwise T is T.
         */
    template <class T, class sfinae = void>
    struct mapped_type_proxy {
        using type = T;
    };

    template <class T>
    struct mapped_type_proxy<T, typename std::enable_if<std::is_base_of<alias_tag, T>::value>::type> {
        using type = typename T::type;
    };
}
}
#pragma once

#include <string> //  std::string

namespace sqlite_orm {

namespace internal {

    struct rowid_t {
        operator std::string() const
        {
            return "rowid";
        }
    };

    struct oid_t {
        operator std::string() const
        {
            return "oid";
        }
    };

    struct _rowid_t {
        operator std::string() const
        {
            return "_rowid_";
        }
    };

    template <class T>
    struct table_rowid_t : public rowid_t {
        using type = T;
    };

    template <class T>
    struct table_oid_t : public oid_t {
        using type = T;
    };
    template <class T>
    struct table__rowid_t : public _rowid_t {
        using type = T;
    };

}

inline internal::rowid_t rowid()
{
    return {};
}

inline internal::oid_t oid()
{
    return {};
}

inline internal::_rowid_t _rowid_()
{
    return {};
}

template <class T>
internal::table_rowid_t<T> rowid()
{
    return {};
}

template <class T>
internal::table_oid_t<T> oid()
{
    return {};
}

template <class T>
internal::table__rowid_t<T> _rowid_()
{
    return {};
}
}
#pragma once

#include <tuple> //  std::tuple
#include <type_traits> //  std::enable_if, std::is_same, std::decay

// #include "aggregate_functions.h"

// #include "alias.h"

// #include "column.h"

// #include "core_functions.h"

// #include "operators.h"

// #include "rowid.h"

// #include "select_constraints.h"

// #include "storage_traits.h"
#include <tuple> //  std::tuple
#include <type_traits> //  std::is_same, std::enable_if, std::true_type, std::false_type, std::integral_constant

namespace sqlite_orm {

namespace internal {

    template <class... Ts>
    struct storage_impl;

    template <typename... Args>
    struct table_impl;

    namespace storage_traits {

        /**
             *  S - storage_impl type
             *  T - mapped or not mapped data type
             */
        template <class S, class T, class SFINAE = void>
        struct type_is_mapped_impl;

        /**
             *  S - storage
             *  T - mapped or not mapped data type
             */
        template <class S, class T>
        struct type_is_mapped : type_is_mapped_impl<typename S::impl_type, T> {
        };

        /**
             *  Final specialisation
             */
        template <class T>
        struct type_is_mapped_impl<storage_impl<>, T, void> : std::false_type {
        };

        template <class S, class T>
        struct type_is_mapped_impl<S, T, typename std::enable_if<std::is_same<T, typename S::table_type::object_type>::value>::type> : std::true_type {
        };

        template <class S, class T>
        struct type_is_mapped_impl<S, T, typename std::enable_if<!std::is_same<T, typename S::table_type::object_type>::value>::type>
            : type_is_mapped_impl<typename S::super, T> {
        };

        /**
             *  S - storage_impl type
             *  T - mapped or not mapped data type
             */
        template <class S, class T, class SFINAE = void>
        struct storage_columns_count_impl;

        /**
             *  S - storage
             *  T - mapped or not mapped data type
             */
        template <class S, class T>
        struct storage_columns_count : storage_columns_count_impl<typename S::impl_type, T> {
        };

        /**
             *  Final specialisation
             */
        template <class T>
        struct storage_columns_count_impl<storage_impl<>, T, void> : std::integral_constant<int, 0> {
        };

        template <class S, class T>
        struct storage_columns_count_impl<S, T, typename std::enable_if<std::is_same<T, typename S::table_type::object_type>::value>::type> : std::integral_constant<int, S::table_type::columns_count> {
        };

        template <class S, class T>
        struct storage_columns_count_impl<S, T, typename std::enable_if<!std::is_same<T, typename S::table_type::object_type>::value>::type> : storage_columns_count_impl<typename S::super, T> {
        };

        /**
             *  T - table_impl type.
             */
        template <class T>
        struct table_impl_types;

        /**
             *  type is std::tuple of field types of mapped colums.
             */
        template <typename... Args>
        struct table_impl_types<table_impl<Args...>> {
            using type = std::tuple<typename Args::field_type...>;
        };

        /**
             *  S - storage_impl type
             *  T - mapped or not mapped data type
             */
        template <class S, class T, class SFINAE = void>
        struct storage_mapped_columns_impl;

        /**
             *  S - storage
             *  T - mapped or not mapped data type
             */
        template <class S, class T>
        struct storage_mapped_columns : storage_mapped_columns_impl<typename S::impl_type, T> {
        };

        /**
             *  Final specialisation
             */
        template <class T>
        struct storage_mapped_columns_impl<storage_impl<>, T, void> {
            using type = std::tuple<>;
        };

        template <class S, class T>
        struct storage_mapped_columns_impl<S, T, typename std::enable_if<std::is_same<T, typename S::table_type::object_type>::value>::type> {
            using table_type = typename S::table_type;
            using table_impl_type = typename table_type::impl_type;
            using type = typename table_impl_types<table_impl_type>::type;
        };

        template <class S, class T>
        struct storage_mapped_columns_impl<S, T, typename std::enable_if<!std::is_same<T, typename S::table_type::object_type>::value>::type> : storage_mapped_columns_impl<typename S::super, T> {
        };

    }
}
}

namespace sqlite_orm {

namespace internal {

    /**
         *  This is a proxy class used to define what type must have result type depending on select
         *  arguments (member pointer, aggregate functions, etc). Below you can see specializations
         *  for different types. E.g. specialization for core_functions::length_t has `type` int cause
         *  LENGTH returns INTEGER in sqlite. Every column_result_t must have `type` type that equals
         *  c++ SELECT return type for T
         *  T - C++ type
         *  SFINAE - sfinae argument
         */
    template <class St, class T, class SFINAE = void>
    struct column_result_t;

    template <class St, class O, class F>
    struct column_result_t<St, F O::*, typename std::enable_if<std::is_member_pointer<F O::*>::value && !std::is_member_function_pointer<F O::*>::value>::type> {
        using type = F;
    };

    /**
         *  Common case for all getter types. Getter types are defined in column.h file
         */
    template <class St, class T>
    struct column_result_t<St, T, typename std::enable_if<is_getter<T>::value>::type> {
        using type = typename getter_traits<T>::field_type;
    };

    /**
         *  Common case for all setter types. Setter types are defined in column.h file
         */
    template <class St, class T>
    struct column_result_t<St, T, typename std::enable_if<is_setter<T>::value>::type> {
        using type = typename setter_traits<T>::field_type;
    };

    template <class St, class T>
    struct column_result_t<St, core_functions::length_t<T>, void> {
        using type = int;
    };

    template <class St, class... Args>
    struct column_result_t<St, core_functions::char_t_<Args...>, void> {
        using type = std::string;
    };

    template <class St>
    struct column_result_t<St, core_functions::random_t, void> {
        using type = int;
    };

    template <class St>
    struct column_result_t<St, core_functions::changes_t, void> {
        using type = int;
    };

    template <class St, class T>
    struct column_result_t<St, core_functions::abs_t<T>, void> {
        using type = std::shared_ptr<double>;
    };

    template <class St, class T>
    struct column_result_t<St, core_functions::lower_t<T>, void> {
        using type = std::string;
    };

    template <class St, class T>
    struct column_result_t<St, core_functions::upper_t<T>, void> {
        using type = std::string;
    };

    template <class St, class X>
    struct column_result_t<St, core_functions::trim_single_t<X>, void> {
        using type = std::string;
    };

    template <class St, class X, class Y>
    struct column_result_t<St, core_functions::trim_double_t<X, Y>, void> {
        using type = std::string;
    };

    template <class St, class X>
    struct column_result_t<St, core_functions::ltrim_single_t<X>, void> {
        using type = std::string;
    };

    template <class St, class X, class Y>
    struct column_result_t<St, core_functions::ltrim_double_t<X, Y>, void> {
        using type = std::string;
    };

    template <class St, class X>
    struct column_result_t<St, core_functions::rtrim_single_t<X>, void> {
        using type = std::string;
    };

    template <class St, class X, class Y>
    struct column_result_t<St, core_functions::rtrim_double_t<X, Y>, void> {
        using type = std::string;
    };

    template <class St, class T, class... Args>
    struct column_result_t<St, core_functions::date_t<T, Args...>, void> {
        using type = std::string;
    };

    template <class St, class T, class... Args>
    struct column_result_t<St, core_functions::julianday_t<T, Args...>, void> {
        using type = double;
    };

    template <class St, class T, class... Args>
    struct column_result_t<St, core_functions::datetime_t<T, Args...>, void> {
        using type = std::string;
    };

    template <class St, class T>
    struct column_result_t<St, aggregate_functions::avg_t<T>, void> {
        using type = double;
    };

    template <class St, class T>
    struct column_result_t<St, aggregate_functions::count_t<T>, void> {
        using type = int;
    };

    template <class St, class T>
    struct column_result_t<St, aggregate_functions::count_asterisk_t<T>, void> {
        using type = int;
    };

    template <class St, class T>
    struct column_result_t<St, aggregate_functions::sum_t<T>, void> {
        using type = std::shared_ptr<double>;
    };

    template <class St, class T>
    struct column_result_t<St, aggregate_functions::total_t<T>, void> {
        using type = double;
    };

    template <class St, class T>
    struct column_result_t<St, aggregate_functions::group_concat_single_t<T>, void> {
        using type = std::string;
    };

    template <class St, class T>
    struct column_result_t<St, aggregate_functions::group_concat_double_t<T>, void> {
        using type = std::string;
    };

    template <class St, class T>
    struct column_result_t<St, aggregate_functions::max_t<T>, void> {
        using type = std::shared_ptr<typename column_result_t<St, T>::type>;
    };

    template <class St, class T>
    struct column_result_t<St, aggregate_functions::min_t<T>, void> {
        using type = std::shared_ptr<typename column_result_t<St, T>::type>;
    };

    template <class St>
    struct column_result_t<St, aggregate_functions::count_asterisk_without_type, void> {
        using type = int;
    };

    template <class St, class T>
    struct column_result_t<St, distinct_t<T>, void> {
        using type = typename column_result_t<St, T>::type;
    };

    template <class St, class T>
    struct column_result_t<St, all_t<T>, void> {
        using type = typename column_result_t<St, T>::type;
    };

    template <class St, class L, class R>
    struct column_result_t<St, conc_t<L, R>, void> {
        using type = std::string;
    };

    template <class St, class L, class R>
    struct column_result_t<St, add_t<L, R>, void> {
        using type = double;
    };

    template <class St, class L, class R>
    struct column_result_t<St, sub_t<L, R>, void> {
        using type = double;
    };

    template <class St, class L, class R>
    struct column_result_t<St, mul_t<L, R>, void> {
        using type = double;
    };

    template <class St, class L, class R>
    struct column_result_t<St, internal::div_t<L, R>, void> {
        using type = double;
    };

    template <class St, class L, class R>
    struct column_result_t<St, mod_t<L, R>, void> {
        using type = double;
    };

    template <class St>
    struct column_result_t<St, rowid_t, void> {
        using type = int64;
    };

    template <class St>
    struct column_result_t<St, oid_t, void> {
        using type = int64;
    };

    template <class St>
    struct column_result_t<St, _rowid_t, void> {
        using type = int64;
    };

    template <class St, class T>
    struct column_result_t<St, table_rowid_t<T>, void> {
        using type = int64;
    };

    template <class St, class T>
    struct column_result_t<St, table_oid_t<T>, void> {
        using type = int64;
    };

    template <class St, class T>
    struct column_result_t<St, table__rowid_t<T>, void> {
        using type = int64;
    };

    template <class St, class T, class C>
    struct column_result_t<St, alias_column_t<T, C>, void> {
        using type = typename column_result_t<St, C>::type;
    };

    template <class St, class T, class F>
    struct column_result_t<St, column_pointer<T, F>> : column_result_t<St, F, void> {
    };

    template <class St, class... Args>
    struct column_result_t<St, columns_t<Args...>, void> {
        using type = std::tuple<typename column_result_t<St, typename std::decay<Args>::type>::type...>;
    };

    template <class St, class T, class... Args>
    struct column_result_t<St, select_t<T, Args...>> : column_result_t<St, T, void> {
    };

    template <class St, class T>
    struct column_result_t<St, T, typename std::enable_if<is_base_of_template<T, compound_operator>::value>::type> {
        using left_type = typename T::left_type;
        using right_type = typename T::right_type;
        using left_result = typename column_result_t<St, left_type>::type;
        using right_result = typename column_result_t<St, right_type>::type;
        static_assert(std::is_same<left_result, right_result>::value, "Compound subselect queries must return same types");
        using type = left_result;
    };

    /**
         *  Result for the most simple queries like `SELECT 1`
         */
    template <class St, class T>
    struct column_result_t<St, T, typename std::enable_if<std::is_arithmetic<T>::value>::type> {
        using type = T;
    };

    /**
         *  Result for the most simple queries like `SELECT 'ototo'`
         */
    template <class St>
    struct column_result_t<St, const char*, void> {
        using type = std::string;
    };

    template <class St, class T, class E>
    struct column_result_t<St, as_t<T, E>, void> : column_result_t<St, typename std::decay<E>::type, void> {
    };

    template <class St, class T>
    struct column_result_t<St, asterisk_t<T>, void> {
        using type = typename storage_traits::storage_mapped_columns<St, T>::type;
    };

    template <class St, class T, class E>
    struct column_result_t<St, conditions::cast_t<T, E>, void> {
        using type = T;
    };
}
}
#pragma once

#include <string> //  std::string
#include <tuple> //  std::tuple
#include <type_traits> //  std::is_same, std::integral_constant, std::true_type, std::false_type
#include <vector> //  std::vector

// #include "column.h"

// #include "tuple_helper.h"

// #include "constraints.h"

namespace sqlite_orm {

namespace internal {

    /**
         *  Common case for table_impl class.
         */
    template <typename... Args>
    struct table_impl;

    /**
         *  Final superclass for table_impl.
         */
    template <>
    struct table_impl<> {

        static constexpr const int columns_count = 0;

        std::vector<std::string> column_names()
        {
            return {};
        }

        template <class... Op>
        std::vector<std::string> column_names_exept()
        {
            return {};
        }

        template <class... Op>
        std::vector<std::string> column_names_with()
        {
            return {};
        }

        template <class L>
        void for_each_column(L) {}

        template <class L>
        void for_each_column_with_constraints(L) {}

        template <class F, class L>
        void for_each_column_with_field_type(L) {}

        template <class Op, class L>
        void for_each_column_exept(L) {}

        template <class Op, class L>
        void for_each_column_with(L) {}

        template <class L>
        void for_each_primary_key(L) {}
    };

    /**
         *  Regular table_impl class.
         */
    template <typename H, typename... T>
    struct table_impl<H, T...> : private table_impl<T...> {
        using column_type = H;
        using tail_types = std::tuple<T...>;
        using super = table_impl<T...>;

        table_impl(H h, T... t)
            : super(t...)
            , col(h)
        {
        }

        column_type col;

        static constexpr const int columns_count = 1 + super::columns_count;

        /**
             *  column_names_with implementation. Notice that result will be reversed.
             *  It is reversed back in `table` class.
             *  @return vector of column names that have specified Op... conditions.
             */
        template <class... Op>
        std::vector<std::string> column_names_with()
        {
            auto res = this->super::template column_names_with<Op...>();
            if (this->col.template has_every<Op...>()) {
                res.emplace_back(this->col.name);
            }
            return res;
        }

        /**
             *  For each implementation. Calls templated lambda with its column
             *  and passed call to superclass.
             */
        template <class L>
        void for_each_column(L l)
        {
            this->apply_to_col_if(l, internal::is_column<column_type> {});
            this->super::for_each_column(l);
        }

        /**
             *  For each implementation. Calls templated lambda with its column
             *  and passed call to superclass.
             */
        template <class L>
        void for_each_column_with_constraints(L l)
        {
            l(this->col);
            this->super::for_each_column_with_constraints(l);
        }

        template <class F, class L>
        void for_each_column_with_field_type(L l)
        {
            this->apply_to_col_if(l, std::is_same<F, typename column_type::field_type> {});
            this->super::template for_each_column_with_field_type<F, L>(l);
        }

        /**
             *  Working version of `for_each_column_exept`. Calls lambda if column has no option and fire super's function.
             */
        template <class Op, class L>
        void for_each_column_exept(L l)
        {
            using has_opt = tuple_helper::tuple_contains_type<Op, typename column_type::constraints_type>;
            this->apply_to_col_if(l, std::integral_constant<bool, !has_opt::value> {});
            this->super::template for_each_column_exept<Op, L>(l);
        }

        /**
             *  Working version of `for_each_column_with`. Calls lambda if column has option and fire super's function.
             */
        template <class Op, class L>
        void for_each_column_with(L l)
        {
            this->apply_to_col_if(l, tuple_helper::tuple_contains_type<Op, typename column_type::constraints_type> {});
            this->super::template for_each_column_with<Op, L>(l);
        }

        /**
             *  Calls l(this->col) if H is primary_key_t
             */
        template <class L>
        void for_each_primary_key(L l)
        {
            this->apply_to_col_if(l, internal::is_primary_key<H> {});
            this->super::for_each_primary_key(l);
        }

        template <class L>
        void apply_to_col_if(L& l, std::true_type)
        {
            l(this->col);
        }

        template <class L>
        void apply_to_col_if(L&, std::false_type) {}
    };
}
}
#pragma once

#include <algorithm> //  std::reverse, std::find_if
#include <string> //  std::string
#include <tuple> //  std::tuple_size, std::tuple_element
#include <type_traits> //  std::remove_reference, std::is_same, std::is_base_of
#include <vector> //  std::vector

// #include "table_impl.h"

// #include "column_result.h"

// #include "static_magic.h"

// #include "typed_comparator.h"

// #include "constraints.h"

// #include "tuple_helper.h"

// #include "table_info.h"

// #include "type_printer.h"

// #include "column.h"

namespace sqlite_orm {

namespace internal {

    /**
         *  Table interface class. Implementation is hidden in `table_impl` class.
         */
    template <class T, class... Cs>
    struct table_t {
        using impl_type = table_impl<Cs...>;
        using object_type = T;

        static constexpr const int columns_count = impl_type::columns_count;

        /**
             *  Table name.
             */
        const std::string name;

        /**
             *  Implementation that stores columns information.
             */
        impl_type impl;

        table_t(decltype(name) name_, decltype(impl) impl_)
            : name(std::move(name_))
            , impl(std::move(impl_))
        {
        }

        bool _without_rowid = false;

        table_t<T, Cs...> without_rowid() const
        {
            auto res = *this;
            res._without_rowid = true;
            return res;
        }

        /**
             *  Function used to get field value from object by mapped member pointer/setter/getter
             */
        template <class F, class C>
        const F* get_object_field_pointer(const object_type& obj, C c)
        {
            const F* res = nullptr;
            this->for_each_column_with_field_type<F>([&res, &c, &obj, this](auto& col) {
                using namespace static_magic;
                using column_type = typename std::remove_reference<decltype(col)>::type;
                using member_pointer_t = typename column_type::member_pointer_t;
                using getter_type = typename column_type::getter_type;
                using setter_type = typename column_type::setter_type;
                if (!res) {
                    static_if<std::is_same<C, member_pointer_t> {}>([&res, &obj, &col, &c] {
                        if (compare_any(col.member_pointer, c)) {
                            res = &(obj.*col.member_pointer);
                        }
                    })();
                }
                if (!res) {
                    static_if<std::is_same<C, getter_type> {}>([&res, &obj, &col, &c] {
                        if (compare_any(col.getter, c)) {
                            res = &((obj).*(col.getter))();
                        }
                    })();
                }
                if (!res) {
                    static_if<std::is_same<C, setter_type> {}>([&res, &obj, &col, &c] {
                        if (compare_any(col.setter, c)) {
                            res = &((obj).*(col.getter))();
                        }
                    })();
                }
            });
            return res;
        }

        /**
             *  @return vector of column names of table.
             */
        std::vector<std::string> column_names()
        {
            std::vector<std::string> res;
            this->impl.for_each_column([&res](auto& c) {
                res.push_back(c.name);
            });
            return res;
        }

        std::vector<std::string> composite_key_columns_names()
        {
            std::vector<std::string> res;
            this->impl.for_each_primary_key([this, &res](auto c) {
                res = this->composite_key_columns_names(c);
            });
            return res;
        }

        std::vector<std::string> primary_key_column_names()
        {
            std::vector<std::string> res;
            this->impl.template for_each_column_with<constraints::primary_key_t<>>([&res](auto& c) {
                res.push_back(c.name);
            });
            if (!res.size()) {
                res = this->composite_key_columns_names();
            }
            return res;
        }

        template <class... Args>
        std::vector<std::string> composite_key_columns_names(constraints::primary_key_t<Args...> pk)
        {
            std::vector<std::string> res;
            using pk_columns_tuple = decltype(pk.columns);
            res.reserve(std::tuple_size<pk_columns_tuple>::value);
            tuple_helper::iterator<std::tuple_size<pk_columns_tuple>::value - 1, Args...>()(pk.columns, [this, &res](auto& v) {
                res.push_back(this->find_column_name(v));
            });
            return res;
        }

        /**
             *  Searches column name by class member pointer passed as first argument.
             *  @return column name or empty string if nothing found.
             */
        template <
            class F,
            class O,
            typename = typename std::enable_if<std::is_member_pointer<F O::*>::value && !std::is_member_function_pointer<F O::*>::value>::type>
        std::string find_column_name(F O::*m)
        {
            std::string res;
            this->template for_each_column_with_field_type<F>([&res, m](auto c) {
                if (c.member_pointer == m) {
                    res = c.name;
                }
            });
            return res;
        }

        /**
             *  Searches column name by class getter function member pointer passed as first argument.
             *  @return column name or empty string if nothing found.
             */
        template <class G>
        std::string find_column_name(G getter, typename std::enable_if<is_getter<G>::value>::type* = nullptr)
        {
            std::string res;
            using field_type = typename getter_traits<G>::field_type;
            this->template for_each_column_with_field_type<field_type>([&res, getter](auto c) {
                if (c.getter == getter) {
                    res = c.name;
                }
            });
            return res;
        }

        /**
             *  Searches column name by class setter function member pointer passed as first argument.
             *  @return column name or empty string if nothing found.
             */
        template <class S>
        std::string find_column_name(S setter, typename std::enable_if<is_setter<S>::value>::type* = nullptr)
        {
            std::string res;
            using field_type = typename setter_traits<S>::field_type;
            this->template for_each_column_with_field_type<field_type>([&res, setter](auto c) {
                if (c.setter == setter) {
                    res = c.name;
                }
            });
            return res;
        }

        /**
             *  @return vector of column names that have constraints provided as template arguments (not_null, autoincrement).
             */
        template <class... Op>
        std::vector<std::string> column_names_with()
        {
            auto res = this->impl.template column_names_with<Op...>();
            std::reverse(res.begin(),
                res.end());
            return res;
        }

        /**
             *  Iterates all columns and fires passed lambda. Lambda must have one and only templated argument Otherwise code will
             *  not compile. Excludes table constraints (e.g. foreign_key_t) at the end of the columns list. To iterate columns with
             *  table constraints use for_each_column_with_constraints instead.
             *  L is lambda type. Do not specify it explicitly.
             *  @param l Lambda to be called per column itself. Must have signature like this [] (auto col) -> void {}
             */
        template <class L>
        void for_each_column(L l)
        {
            this->impl.for_each_column(l);
        }

        template <class L>
        void for_each_column_with_constraints(L l)
        {
            this->impl.for_each_column_with_constraints(l);
        }

        template <class F, class L>
        void for_each_column_with_field_type(L l)
        {
            this->impl.template for_each_column_with_field_type<F, L>(l);
        }

        /**
             *  Iterates all columns exept ones that have specified constraints and fires passed lambda.
             *  Lambda must have one and only templated argument Otherwise code will not compile.
             *  L is lambda type. Do not specify it explicitly.
             *  @param l Lambda to be called per column itself. Must have signature like this [] (auto col) -> void {}
             */
        template <class Op, class L>
        void for_each_column_exept(L l)
        {
            this->impl.template for_each_column_exept<Op>(l);
        }

        /**
             *  Iterates all columns that have specified constraints and fires passed lambda.
             *  Lambda must have one and only templated argument Otherwise code will not compile.
             *  L is lambda type. Do not specify it explicitly.
             *  @param l Lambda to be called per column itself. Must have signature like this [] (auto col) -> void {}
             */
        template <class Op, class L>
        void for_each_column_with(L l)
        {
            this->impl.template for_each_column_with<Op>(l);
        }

        std::vector<table_info> get_table_info()
        {
            std::vector<table_info> res;
            res.reserve(size_t(this->columns_count));
            this->for_each_column([&res](auto& col) {
                std::string dft;
                using field_type = typename std::remove_reference<decltype(col)>::type::field_type;
                if (auto d = col.default_value()) {
                    auto needQuotes = std::is_base_of<text_printer, type_printer<field_type>>::value;
                    if (needQuotes) {
                        dft = "'" + *d + "'";
                    } else {
                        dft = *d;
                    }
                }
                table_info i {
                    -1,
                    col.name,
                    type_printer<field_type>().print(),
                    col.not_null(),
                    dft,
                    col.template has<constraints::primary_key_t<>>(),
                };
                res.emplace_back(i);
            });
            std::vector<std::string> compositeKeyColumnNames;
            this->impl.for_each_primary_key([this, &compositeKeyColumnNames](auto c) {
                compositeKeyColumnNames = this->composite_key_columns_names(c);
            });
            for (size_t i = 0; i < compositeKeyColumnNames.size(); ++i) {
                auto& columnName = compositeKeyColumnNames[i];
                auto it = std::find_if(res.begin(),
                    res.end(),
                    [&columnName](const table_info& ti) {
                        return ti.name == columnName;
                    });
                if (it != res.end()) {
                    it->pk = static_cast<int>(i + 1);
                }
            }
            return res;
        }
    };
}

/**
     *  Function used for table creation. Do not use table constructor - use this function
     *  cause table class is templated and its constructing too (just like std::make_shared or std::make_pair).
     */
template <class... Cs, class T = typename std::tuple_element<0, std::tuple<Cs...>>::type::object_type>
internal::table_t<T, Cs...> make_table(const std::string& name, Cs&&... args)
{
    return { name, internal::table_impl<Cs...>(std::forward<Cs>(args)...) };
}

template <class T, class... Cs>
internal::table_t<T, Cs...> make_table(const std::string& name, Cs&&... args)
{
    return { name, internal::table_impl<Cs...>(std::forward<Cs>(args)...) };
}
}
#pragma once

#include <string> //  std::string

#include <algorithm> //  std::find_if
#include <cstddef> //  std::nullptr_t
#include <cstdlib> //  std::atoi
#include <sstream> //  std::stringstream
#include <system_error> //  std::system_error, std::error_code
#include <type_traits> //  std::forward, std::enable_if, std::is_same, std::remove_reference, std::false_type, std::true_type
#include <utility> //  std::pair, std::make_pair
#include <vector> //  std::vector

// #include "constraints.h"

// #include "database.h"

// #include "error_code.h"

// #include "field_printer.h"

// #include "field_value_holder.h"

#include <type_traits> //  std::enable_if

// #include "column.h"

namespace sqlite_orm {
namespace internal {

    template <class T, class SFINAE = void>
    struct field_value_holder;

    template <class T>
    struct field_value_holder<T, typename std::enable_if<getter_traits<T>::returns_lvalue>::type> {
        using type = typename getter_traits<T>::field_type;

        const type& value;
    };

    template <class T>
    struct field_value_holder<T, typename std::enable_if<!getter_traits<T>::returns_lvalue>::type> {
        using type = typename getter_traits<T>::field_type;

        type value;
    };
}
}

// #include "query.h"

// #include "row_extractor.h"

// #include "select_constraints.h"

// #include "sqlite_type.h"

// #include "sync_schema_result.h"

// #include "table_info.h"

namespace sqlite_orm {

namespace internal {

    /**
         *  This is a generic implementation. Used as a tail in storage_impl inheritance chain
         */
    template <class... Ts>
    struct storage_impl;

    template <class H, class... Ts>
    struct storage_impl<H, Ts...> : public storage_impl<Ts...> {
        using table_type = H;
        using super = storage_impl<Ts...>;

        storage_impl(H h, Ts... ts)
            : super(std::forward<Ts>(ts)...)
            , table(std::move(h))
        {
        }

        table_type table;

        template <class L>
        void for_each(L l)
        {
            this->super::for_each(l);
            l(this);
        }

        /**
             *  Returns foreign keys count in table definition
             */
        int foreign_keys_count()
        {
            auto res = 0;
            this->table.for_each_column_with_constraints([&res](auto c) {
                if (internal::is_foreign_key<decltype(c)>::value) {
                    ++res;
                }
            });
            return res;
        }

        /**
             *  Is used to get column name by member pointer to a base class.
             *  Main difference between `column_name` and `column_name_simple` is that
             *  `column_name` has SFINAE check for type equality but `column_name_simple` has not.
             */
        template <class O, class F>
        std::string column_name_simple(F O::*m)
        {
            return this->table.find_column_name(m);
        }

        /**
             *  Same thing as above for getter.
             */
        template <class T, typename std::enable_if<is_getter<T>::value>::type>
        std::string column_name_simple(T g)
        {
            return this->table.find_column_name(g);
        }

        /**
             *  Same thing as above for setter.
             */
        template <class T, typename std::enable_if<is_setter<T>::value>::type>
        std::string column_name_simple(T s)
        {
            return this->table.find_column_name(s);
        }

        /**
             *  Cute function used to find column name by its type and member pointer. Uses SFINAE to
             *  skip inequal type O.
             */
        template <class O, class F, class HH = typename H::object_type>
        std::string column_name(F O::*m, typename std::enable_if<std::is_same<O, HH>::value>::type* = nullptr)
        {
            return this->table.find_column_name(m);
        }

        /**
             *  Opposite version of function defined above. Just calls same function in superclass.
             */
        template <class O, class F, class HH = typename H::object_type>
        std::string column_name(F O::*m, typename std::enable_if<!std::is_same<O, HH>::value>::type* = nullptr)
        {
            return this->super::column_name(m);
        }

        /**
             *  Cute function used to find column name by its type and getter pointer. Uses SFINAE to
             *  skip inequal type O.
             */
        template <class O, class F, class HH = typename H::object_type>
        std::string column_name(const F& (O::*g)() const, typename std::enable_if<std::is_same<O, HH>::value>::type* = nullptr)
        {
            return this->table.find_column_name(g);
        }

        /**
             *  Opposite version of function defined above. Just calls same function in superclass.
             */
        template <class O, class F, class HH = typename H::object_type>
        std::string column_name(const F& (O::*g)() const, typename std::enable_if<!std::is_same<O, HH>::value>::type* = nullptr)
        {
            return this->super::column_name(g);
        }

        /**
             *  Cute function used to find column name by its type and setter pointer. Uses SFINAE to
             *  skip inequal type O.
             */
        template <class O, class F, class HH = typename H::object_type>
        std::string column_name(void (O::*s)(F), typename std::enable_if<std::is_same<O, HH>::value>::type* = nullptr)
        {
            return this->table.find_column_name(s);
        }

        /**
             *  Opposite version of function defined above. Just calls same function in superclass.
             */
        template <class O, class F, class HH = typename H::object_type>
        std::string column_name(void (O::*s)(F), typename std::enable_if<!std::is_same<O, HH>::value>::type* = nullptr)
        {
            return this->super::column_name(s);
        }

        template <class T, class F, class HH = typename H::object_type>
        std::string column_name(const column_pointer<T, F>& c, typename std::enable_if<std::is_same<T, HH>::value>::type* = nullptr)
        {
            return this->column_name_simple(c.field);
        }

        template <class T, class F, class HH = typename H::object_type>
        std::string column_name(const column_pointer<T, F>& c, typename std::enable_if<!std::is_same<T, HH>::value>::type* = nullptr)
        {
            return this->super::column_name(c);
        }

        template <class O, class HH = typename H::object_type>
        auto& get_impl(typename std::enable_if<std::is_same<O, HH>::value>::type* = nullptr)
        {
            return *this;
        }

        template <class O, class HH = typename H::object_type>
        auto& get_impl(typename std::enable_if<!std::is_same<O, HH>::value>::type* = nullptr)
        {
            return this->super::template get_impl<O>();
        }

        template <class O, class HH = typename H::object_type>
        std::string find_table_name(typename std::enable_if<std::is_same<O, HH>::value>::type* = nullptr) const
        {
            return this->table.name;
        }

        template <class O, class HH = typename H::object_type>
        std::string find_table_name(typename std::enable_if<!std::is_same<O, HH>::value>::type* = nullptr) const
        {
            return this->super::template find_table_name<O>();
        }

        template <class O, class HH = typename H::object_type>
        std::string dump(const O& o, typename std::enable_if<!std::is_same<O, HH>::value>::type* = nullptr)
        {
            return this->super::dump(o, nullptr);
        }

        template <class O, class HH = typename H::object_type>
        std::string dump(const O& o, typename std::enable_if<std::is_same<O, HH>::value>::type* = nullptr)
        {
            std::stringstream ss;
            ss << "{ ";
            using pair = std::pair<std::string, std::string>;
            std::vector<pair> pairs;
            this->table.for_each_column([&pairs, &o](auto& c) {
                using field_type = typename std::decay<decltype(c)>::type::field_type;
                pair p { c.name, "" };
                if (c.member_pointer) {
                    p.second = field_printer<field_type>()(o.*c.member_pointer);
                } else {
                    using getter_type = typename std::decay<decltype(c)>::type::getter_type;
                    field_value_holder<getter_type> valueHolder { ((o).*(c.getter))() };
                    p.second = field_printer<field_type>()(valueHolder.value);
                }
                pairs.push_back(std::move(p));
            });
            for (size_t i = 0; i < pairs.size(); ++i) {
                auto& p = pairs[i];
                ss << p.first << " : '" << p.second << "'";
                if (i < pairs.size() - 1) {
                    ss << ", ";
                } else {
                    ss << " }";
                }
            }
            return ss.str();
        }

        std::vector<table_info> get_table_info(const std::string& tableName, database* db)
        {
            return db->get_table_info(tableName);
        }

        void add_column(const table_info& ti, database* db)
        {
            db->add_column(this->table.name, ti);
        }

        /**
             *  Copies current table to another table with a given **name**.
             *  Performs CREATE TABLE %name% AS SELECT %this->table.columns_names()% FROM &this->table.name%;
             */
        void copy_table(database* db, const std::string& name)
        {
            std::stringstream ss;
            std::vector<std::string> columnNames;
            this->table.for_each_column([&columnNames](auto c) {
                columnNames.emplace_back(c.name);
            });
            auto columnNamesCount = columnNames.size();
            ss << "INSERT INTO " << name << " (";
            for (size_t i = 0; i < columnNamesCount; ++i) {
                ss << columnNames[i];
                if (i < columnNamesCount - 1) {
                    ss << ", ";
                } else {
                    ss << " ";
                }
            }
            ss << ") ";
            ss << "SELECT ";
            for (size_t i = 0; i < columnNamesCount; ++i) {
                ss << columnNames[i];
                if (i < columnNamesCount - 1) {
                    ss << ", ";
                } else {
                    ss << " ";
                }
            }
            ss << " FROM '" << this->table.name << "' ";
            auto query = db->make_query(ss.str());

            auto prepareResult = db->prepare(query.get());
            if (prepareResult) {
                if (db->exec(query.get()) != query::step::done) {
                    throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
                }
            } else {
                throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
            }
        }

        sync_schema_result schema_status(database* db, bool preserve)
        {

            auto res = sync_schema_result::already_in_sync;

            //  first let's see if table with such name exists..
            auto gottaCreateTable = !this->table_exists(this->table.name, db);
            if (!gottaCreateTable) {

                //  get table info provided in `make_table` call..
                auto storageTableInfo = this->table.get_table_info();

                //  now get current table info from db using `PRAGMA table_info` query..
                auto dbTableInfo = get_table_info(this->table.name, db);

                //  this vector will contain pointers to columns that gotta be added..
                std::vector<table_info*> columnsToAdd;

                if (get_remove_add_columns(columnsToAdd, storageTableInfo, dbTableInfo)) {
                    gottaCreateTable = true;
                }

                if (!gottaCreateTable) { //  if all storage columns are equal to actual db columns but there are excess columns at the db..
                    if (dbTableInfo.size() > 0) {
                        //extra table columns than storage columns
                        if (!preserve) {
                            gottaCreateTable = true;
                        } else {
                            res = decltype(res)::old_columns_removed;
                        }
                    }
                }
                if (gottaCreateTable) {
                    res = decltype(res)::dropped_and_recreated;
                } else {
                    if (columnsToAdd.size()) {
                        //extra storage columns than table columns
                        for (auto columnPointer : columnsToAdd) {
                            if (columnPointer->notnull && columnPointer->dflt_value.empty()) {
                                gottaCreateTable = true;
                                break;
                            }
                        }
                        if (!gottaCreateTable) {
                            if (res == decltype(res)::old_columns_removed) {
                                res = decltype(res)::new_columns_added_and_old_columns_removed;
                            } else {
                                res = decltype(res)::new_columns_added;
                            }
                        } else {
                            res = decltype(res)::dropped_and_recreated;
                        }
                    } else {
                        if (res != decltype(res)::old_columns_removed) {
                            res = decltype(res)::already_in_sync;
                        }
                    }
                }
            } else {
                res = decltype(res)::new_table_created;
            }
            return res;
        }

        static bool get_remove_add_columns(std::vector<table_info*>& columnsToAdd,
            std::vector<table_info>& storageTableInfo,
            std::vector<table_info>& dbTableInfo)
        {
            bool notEqual = false;

            //  iterate through storage columns
            for (size_t storageColumnInfoIndex = 0; storageColumnInfoIndex < storageTableInfo.size(); ++storageColumnInfoIndex) {

                //  get storage's column info
                auto& storageColumnInfo = storageTableInfo[storageColumnInfoIndex];
                auto& columnName = storageColumnInfo.name;

                //  search for a column in db eith the same name
                auto dbColumnInfoIt = std::find_if(dbTableInfo.begin(),
                    dbTableInfo.end(),
                    [&columnName](auto& ti) {
                        return ti.name == columnName;
                    });
                if (dbColumnInfoIt != dbTableInfo.end()) {
                    auto& dbColumnInfo = *dbColumnInfoIt;
                    auto dbColumnInfoType = to_sqlite_type(dbColumnInfo.type);
                    auto storageColumnInfoType = to_sqlite_type(storageColumnInfo.type);
                    if (dbColumnInfoType && storageColumnInfoType) {
                        auto columnsAreEqual = dbColumnInfo.name == storageColumnInfo.name && *dbColumnInfoType == *storageColumnInfoType && dbColumnInfo.notnull == storageColumnInfo.notnull && bool(dbColumnInfo.dflt_value.length()) == bool(storageColumnInfo.dflt_value.length()) && dbColumnInfo.pk == storageColumnInfo.pk;
                        if (!columnsAreEqual) {
                            notEqual = true;
                            break;
                        }
                        dbTableInfo.erase(dbColumnInfoIt);
                        storageTableInfo.erase(storageTableInfo.begin() + storageColumnInfoIndex);
                        --storageColumnInfoIndex;
                    } else {

                        //  undefined type/types
                        notEqual = true;
                        break;
                    }
                } else {
                    columnsToAdd.push_back(&storageColumnInfo);
                }
            }
            return notEqual;
        }

    private:
        using self = storage_impl<H, Ts...>;
    };

    template <>
    struct storage_impl<> {

        template <class O>
        std::string find_table_name() const
        {
            return {};
        }

        template <class L>
        void for_each(L) {}

        int foreign_keys_count()
        {
            return 0;
        }

        template <class O>
        std::string dump(const O&, database*, std::nullptr_t)
        {
            throw std::system_error(std::make_error_code(orm_error_code::type_is_not_mapped_to_storage));
        }

        bool table_exists(const std::string& tableName, database* db)
        {
            return db->table_exists(tableName);
        }

        void begin_transaction(database* db)
        {
            db->begin_transaction();
        }

        void commit(database* db)
        {
            db->commit_transaction();
        }

        void rollback(database* db)
        {
            db->rollback_transaction();
        }

        void rename_table(database* db, const std::string& oldName, const std::string& newName)
        {
            db->rename_table(oldName, newName);
        }

        std::string current_timestamp(database* db)
        {
            return db->current_timestamp();
        }

        std::vector<std::string> table_names(database* db)
        {
            return db->table_names();
        }

        bool threadsafe(database* db)
        {
            return db->threadsafe();
        }
    };

    template <class T>
    struct is_storage_impl : std::false_type {
    };

    template <class... Ts>
    struct is_storage_impl<storage_impl<Ts...>> : std::true_type {
    };
}
}
#pragma once

#include <memory> //  std::shared_ptr, std::make_shared
#include <string> //  std::string

#include <algorithm> //  std::find
#include <cstddef> //  std::ptrdiff_t
#include <functional> //  std::function
#include <iterator> //  std::input_iterator_tag, std::iterator_traits, std::distance
#include <map> //  std::map
#include <set> //  std::set
#include <sstream> //  std::stringstream
#include <system_error> //  std::system_error
#include <tuple> //  std::tuple_size, std::tuple, std::make_tuple
#include <type_traits> //  std::remove_reference, std::is_base_of, std::decay, std::false_type, std::true_type
#include <utility> //  std::forward, std::pair
#include <vector> //  std::vector

// #include "aggregate_functions.h"

// #include "alias.h"

// #include "column_result.h"

// #include "conditions.h"

// #include "constraints.h"

// #include "core_functions.h"

// #include "database_connection.h"

// #include "error_code.h"

// #include "field_printer.h"

// #include "field_value_holder.h"

// #include "journal_mode.h"

// #include "limit_accesor.h"

// #include "database.h"

#include <map> //  std::map

namespace sqlite_orm {
namespace internal {

    template <class S>
    struct limit_accesor {
        using storage_type = S;

        limit_accesor(storage_type& storage_)
            : storage(storage_)
        {
        }

        int length()
        {
            return get_db()->limit_length();
        }

        void length(int newValue)
        {
            get_db()->limit_set_length(newValue);
        }

        int sql_length()
        {
            return get_db()->limit_sql_length();
        }

        void sql_length(int newValue)
        {
            get_db()->limit_set_sql_length(newValue);
        }

        int column()
        {
            return get_db()->limit_columns();
        }

        void column(int newValue)
        {
            get_db()->limit_set_columns(newValue);
        }

        int expr_depth()
        {
            return get_db()->limit_expr_depth();
        }

        void expr_depth(int newValue)
        {
            get_db()->limit_set_expr_depth(newValue);
        }

        int compound_select()
        {
            return get_db()->limit_compound_select();
        }

        void compound_select(int newValue)
        {
            get_db()->limit_set_compound_select(newValue);
        }

        int vdbe_op()
        {
            return get_db()->limit_vdbe_op();
        }

        void vdbe_op(int newValue)
        {
            get_db()->limit_set_vdbe_op(newValue);
        }

        int function_arg()
        {
            return get_db()->limit_function_arg();
        }

        void function_arg(int newValue)
        {
            get_db()->limit_set_function_arg(newValue);
        }

        int attached()
        {
            return get_db()->limit_attached();
        }

        void attached(int newValue)
        {
            get_db()->limit_set_attached(newValue);
        }

        int like_pattern_length()
        {
            return get_db()->limit_like_pattern_length();
        }

        void like_pattern_length(int newValue)
        {
            get_db()->limit_set_like_pattern_length(newValue);
        }

        int variable_number()
        {
            return get_db()->limit_variable_number();
        }

        void variable_number(int newValue)
        {
            get_db()->limit_set_variable_number(newValue);
        }

        int trigger_depth()
        {
            return get_db()->limit_trigger_depth();
        }

        void trigger_depth(int newValue)
        {
            get_db()->limit_set_trigger_depth(newValue);
        }

        int worker_threads()
        {
            return get_db()->limit_worker_threads();
        }

        void worker_threads(int newValue)
        {
            get_db()->limit_set_worker_threads(newValue);
        }

    protected:
        storage_type& storage;

        template <class... Ts>
        friend struct storage_t;

        /**
             *  Stores limit set between connections.
             */
        //        std::map<int, int> limits;

        database* get_db()
        {
            auto connection = this->storage.get_or_create_connection();
            return connection->get_db();
        }
    };
}
}

// #include "mapped_type_proxy.h"

// #include "operators.h"

// #include "pragma.h"

#include <string> //  std::string

// #include "error_code.h"

// #include "journal_mode.h"

// #include "query.h"

// #include "row_extractor.h"

namespace sqlite_orm {

template <class S>
struct pragma_t {
    using storage_type = S;

    pragma_t(storage_type& storage_)
        : storage(storage_)
    {
    }

    sqlite_orm::journal_mode journal_mode()
    {
        return this->get_pragma<sqlite_orm::journal_mode>("journal_mode");
    }

    void journal_mode(sqlite_orm::journal_mode value)
    {
        this->_journal_mode = -1;
        this->set_pragma("journal_mode", value);
        this->_journal_mode = static_cast<decltype(this->_journal_mode)>(value);
    }

    int synchronous()
    {
        return this->get_pragma<int>("synchronous");
    }

    void synchronous(int value)
    {
        this->_synchronous = -1;
        this->set_pragma("synchronous", value);
        this->_synchronous = value;
    }

    int user_version()
    {
        return this->get_pragma<int>("user_version");
    }

    void user_version(int value)
    {
        this->set_pragma("user_version", value);
    }

    int auto_vacuum()
    {
        return this->get_pragma<int>("auto_vacuum");
    }

    void auto_vacuum(int value)
    {
        this->set_pragma("auto_vacuum", value);
    }

    friend storage_type;

protected:
    storage_type& storage;
    int _synchronous = -1;
    char _journal_mode = -1; //  if != -1 stores static_cast<sqlite_orm::journal_mode>(journal_mode)

    template <class T>
    T get_pragma(const std::string& name)
    {
        auto connection = this->storage.get_or_create_connection();
        auto db = connection->get_db();
        auto query = db->make_query("PRAGMA " + name);
        T res;
        if (!db->prepare(query.get())) {
            throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
        }
        auto rc = query->next(db);
        if (rc == query::step::row) {
            res = row_extractor<T>().extract(query.get(), 0);
            return res;
        } else {
            throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
        }
    }

    /**
         *  Yevgeniy Zakharov: I wanted to refactored this function with statements and value bindings
         *  but it turns out that bindings in pragma statements are not supported.
         */
    template <class T>
    void set_pragma(const std::string& name, const T& value, database* db = nullptr)
    {
        std::shared_ptr<internal::database_connection> connection;

        if (!db) {
            connection = this->storage.get_or_create_connection();
            db = connection->get_db();
        }
        std::stringstream ss;
        ss << "PRAGMA " << name << " = " << this->storage.string_from_expression(value);
        auto query = db->make_query(ss.str());
        auto rc = db->exec(query.get());
        if (rc != query::step::done) {
            throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
        }
    }

    void set_pragma(const std::string& name, const sqlite_orm::journal_mode& value, database* db = nullptr)
    {
        if (!db) {
            auto connection = this->storage.get_or_create_connection();
            db = connection->get_db();
        }
        std::stringstream ss;
        ss << "PRAGMA " << name << " = " << internal::to_string(value);
        auto query = db->make_query(ss.str());
        auto rc = db->exec(query.get());
        if (rc != query::step::done) {
            throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
        }
    }
};
}

// #include "row_extractor.h"

// #include "rowid.h"

// #include "select_constraints.h"

// #include "statement_binder.h"

// #include "storage_impl.h"

// #include "sync_schema_result.h"

// #include "table_info.h"

// #include "table_type.h"

// #include "transaction_guard.h"

namespace sqlite_orm {

namespace internal {

    /**
         *  Class used as a guard for a transaction. Calls `ROLLBACK` in destructor.
         *  Has explicit `commit()` and `rollback()` functions. After explicit function is fired
         *  guard won't do anything in d-tor. Also you can set `commit_on_destroy` to true to
         *  make it call `COMMIT` on destroy.
         *  S - storage type
         */
    template <class S>
    struct transaction_guard_t {
        using storage_type = S;

        /**
             *  This is a public lever to tell a guard what it must do in its destructor
             *  if `gotta_fire` is true
             */
        bool commit_on_destroy = false;

        transaction_guard_t(storage_type& s)
            : storage(s)
        {
        }

        ~transaction_guard_t()
        {
            if (this->gotta_fire) {
                if (!this->commit_on_destroy) {
                    this->storage.rollback();
                } else {
                    this->storage.commit();
                }
            }
        }

        /**
             *  Call `COMMIT` explicitly. After this call
             *  guard will not call `COMMIT` or `ROLLBACK`
             *  in its destructor.
             */
        void commit()
        {
            this->storage.commit();
            this->gotta_fire = false;
        }

        /**
             *  Call `ROLLBACK` explicitly. After this call
             *  guard will not call `COMMIT` or `ROLLBACK`
             *  in its destructor.
             */
        void rollback()
        {
            this->storage.rollback();
            this->gotta_fire = false;
        }

    protected:
        storage_type& storage;
        bool gotta_fire = true;
    };
}
}

// #include "tuple_helper.h"

// #include "type_is_nullable.h"

// #include "type_printer.h"

namespace sqlite_orm {

namespace internal {

    /**
         *  Storage class itself. Create an instanse to use it as an interfacto to sqlite db by calling `make_storage` function.
         */
    template <class... Ts>
    struct storage_t {
        using self = storage_t<Ts...>;
        using impl_type = storage_impl<Ts...>;

        template <class T, class... Args>
        struct view_t {
            using mapped_type = T;

            storage_t& storage;
            std::shared_ptr<internal::database_connection> connection;

            const std::string query;

            view_t(storage_t& stor, decltype(connection) conn, Args&&... args)
                : storage(stor)
                , connection(conn)
                , query([&args..., &stor] {
                    std::string q;
                    stor.template generate_select_asterisk<T>(&q, args...);
                    return q;
                }())
            {
            }

            struct iterator_t {
            protected:
                query_ptr stmt;
                view_t<T, Args...>& view;
                std::shared_ptr<T> temp;

                void extract_value(decltype(temp)& temp)
                {
                    temp = std::make_shared<T>();
                    auto& storage = this->view.storage;
                    auto& impl = storage.template get_impl<T>();
                    auto index = 0;
                    impl.table.for_each_column([&index, &temp, this](auto& c) {
                        using field_type = typename std::decay<decltype(c)>::type::field_type;
                        auto value = row_extractor<field_type>().extract(stmt.get(), index++);
                        if (c.member_pointer) {
                            auto member_pointer = c.member_pointer;
                            (*temp).*member_pointer = value;
                        } else {
                            ((*temp).*(c.setter))(std::move(value));
                        }
                    });
                }

            public:
                using value_type = T;
                using difference_type = std::ptrdiff_t;
                using pointer = value_type*;
                using reference = value_type&;
                using iterator_category = std::input_iterator_tag;

                iterator_t(query_ptr stmt_, view_t<T, Args...>& view_)
                    : stmt(stmt_)
                    , view(view_)
                {
                    this->operator++();
                }

                iterator_t(const iterator_t&) = default;

                iterator_t(iterator_t&&) = default;

                iterator_t& operator=(iterator_t&&) = default;

                iterator_t& operator=(const iterator_t&) = default;

                ~iterator_t()
                {
                }

                T& operator*()
                {
                    if (!this->stmt) {
                        throw std::system_error(std::make_error_code(orm_error_code::trying_to_dereference_null_iterator));
                    }
                    if (!this->temp) {
                        this->extract_value(this->temp);
                    }
                    return *this->temp;
                }

                T* operator->()
                {
                    if (!this->stmt) {
                        throw std::system_error(std::make_error_code(orm_error_code::trying_to_dereference_null_iterator));
                    }
                    if (!this->temp) {
                        this->extract_value(this->temp);
                    }
                    return &*this->temp;
                }

                void operator++()
                {
                    if (this->stmt) {
                        auto db = this->view.connection->get_db();
                        auto ret = stmt->next(db);
                        if (query::step::row == ret) {
                            temp = {};
                        } else if (query::step::error == ret) {
                            throw std::system_error(std::make_error_code(orm_error_code::not_found));
                        } else if (query::step::done == ret) {
                            stmt = {};
                        }
                    }
                }

                void operator++(int)
                {
                    this->operator++();
                }

                bool operator==(const iterator_t& other) const
                {
                    return stmt == other.stmt;
                }

                bool operator!=(const iterator_t& other) const
                {
                    return !(*this == other);
                }
            };

            size_t size()
            {
                return this->storage.template count<T>();
            }

            bool empty()
            {
                return !this->size();
            }

            iterator_t end()
            {
                return { nullptr, *this };
            }

            iterator_t begin()
            {
                auto db = this->connection->get_db();
                auto query = db->make_query(this->query.c_str());
                auto ret = query->prepare(db);
                if (!ret) {
                    throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
                }
                return { query, *this };
            }
        };

        std::function<void(database*)> on_open;

        transaction_guard_t<self> transaction_guard()
        {
            this->begin_transaction();
            return { *this };
        }

        template <class S>
        friend struct limit_accesor;

        /**
             *  @param filename_ database filename.
             */
        storage_t(const std::string& filename_, impl_type impl_)
            : filename(filename_)
            , impl(std::move(impl_))
            , inMemory(filename_.empty() || filename_ == ":memory:")
            , pragma(*this)
            , limit(*this)
        {
            if (inMemory) {
                this->currentTransaction = std::make_shared<internal::database_connection>(this->filename);
                this->on_open_internal(this->currentTransaction->get_db());
            }
        }

        storage_t(const storage_t& other)
            : filename(other.filename)
            , impl(other.impl)
            , currentTransaction(other.currentTransaction)
            , inMemory(other.inMemory)
            , collatingFunctions(other.collatingFunctions)
            , pragma(*this)
            , limit(*this)
        {
        }

    protected:
        using collating_function = std::function<int(int, const void*, int, const void*)>;

        std::string filename;
        impl_type impl;
        std::shared_ptr<internal::database_connection> currentTransaction;
        const bool inMemory;
        bool isOpenedForever = false;
        std::map<std::string, collating_function> collatingFunctions;

        /**
             *  Check whether connection exists and returns it if yes or creates a new one
             *  and returns it.
             */
        std::shared_ptr<internal::database_connection> get_or_create_connection()
        {
            decltype(this->currentTransaction) connection;
            if (!this->currentTransaction) {
                connection = std::make_shared<internal::database_connection>(this->filename);
                this->on_open_internal(connection->get_db());
            } else {
                connection = this->currentTransaction;
            }
            return connection;
        }

        template <class O, class T, class G, class S, class... Op>
        std::string serialize_column_schema(internal::column_t<O, T, G, S, Op...> c)
        {
            std::stringstream ss;
            ss << "'" << c.name << "' ";
            using field_type = typename decltype(c)::field_type;
            using constraints_type = typename decltype(c)::constraints_type;
            ss << type_printer<field_type>().print() << " ";
            tuple_helper::iterator<std::tuple_size<constraints_type>::value - 1, Op...>()(c.constraints, [&ss](auto& v) {
                ss << static_cast<std::string>(v) << ' ';
            });
            if (c.not_null()) {
                ss << "NOT NULL ";
            }
            return ss.str();
        }

        template <class... Cs>
        std::string serialize_column_schema(constraints::primary_key_t<Cs...> fk)
        {
            std::stringstream ss;
            ss << static_cast<std::string>(fk) << " (";
            std::vector<std::string> columnNames;
            columnNames.reserve(std::tuple_size<decltype(fk.columns)>::value);
            tuple_helper::iterator<std::tuple_size<decltype(fk.columns)>::value - 1, Cs...>()(fk.columns, [&columnNames, this](auto& c) {
                columnNames.push_back(this->impl.column_name(c));
            });
            for (size_t i = 0; i < columnNames.size(); ++i) {
                ss << columnNames[i];
                if (i < columnNames.size() - 1) {
                    ss << ", ";
                }
            }
            ss << ") ";
            return ss.str();
        }

        template <class... Cs, class... Rs>
        std::string serialize_column_schema(constraints::foreign_key_t<std::tuple<Cs...>, std::tuple<Rs...>>& fk)
        {
            std::stringstream ss;
            std::vector<std::string> columnNames;
            using columns_type_t = typename std::decay<decltype(fk)>::type::columns_type;
            constexpr const int columnsCount = std::tuple_size<columns_type_t>::value;
            columnNames.reserve(columnsCount);
            tuple_helper::iterator<columnsCount - 1, Cs...>()(fk.columns, [&columnNames, this](auto& v) {
                columnNames.push_back(this->impl.column_name(v));
            });
            ss << "FOREIGN KEY( ";
            for (size_t i = 0; i < columnNames.size(); ++i) {
                ss << columnNames[i];
                if (i < columnNames.size() - 1) {
                    ss << ",";
                }
                ss << " ";
            }
            ss << ") REFERENCES ";
            std::vector<std::string> referencesNames;
            using references_type_t = typename std::decay<decltype(fk)>::type::references_type;
            constexpr const int referencesCount = std::tuple_size<references_type_t>::value;
            referencesNames.reserve(referencesCount);
            {
                using first_reference_t = typename std::tuple_element<0, references_type_t>::type;
                using first_reference_mapped_type = typename internal::table_type<first_reference_t>::type;
                auto refTableName = this->impl.template find_table_name<first_reference_mapped_type>();
                ss << refTableName << " ";
            }
            tuple_helper::iterator<referencesCount - 1, Rs...>()(fk.references, [&referencesNames, this](auto& v) {
                referencesNames.push_back(this->impl.column_name(v));
            });
            ss << "( ";
            for (size_t i = 0; i < referencesNames.size(); ++i) {
                ss << referencesNames[i];
                if (i < referencesNames.size() - 1) {
                    ss << ",";
                }
                ss << " ";
            }
            ss << ") ";
            if (fk.on_update) {
                ss << static_cast<std::string>(fk.on_update) << " " << fk.on_update._action << " ";
            }
            if (fk.on_delete) {
                ss << static_cast<std::string>(fk.on_delete) << " " << fk.on_delete._action << " ";
            }
            return ss.str();
        }

        template <class I>
        void create_table(database* db, const std::string& tableName, I* impl)
        {
            std::stringstream ss;
            ss << "CREATE TABLE '" << tableName << "' ( ";
            auto columnsCount = impl->table.columns_count;
            auto index = 0;
            impl->table.for_each_column_with_constraints([columnsCount, &index, &ss, this](auto c) {
                ss << this->serialize_column_schema(c);
                if (index < columnsCount - 1) {
                    ss << ", ";
                }
                index++;
            });
            ss << ") ";
            if (impl->table._without_rowid) {
                ss << "WITHOUT ROWID ";
            }
            auto query = db->make_query(ss.str());
            auto ret = db->exec(query.get());
            if (ret != query::step::done) {
                throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
            }
        }

        template <class I>
        void backup_table(database* db, I* impl)
        {
            //  here we copy source table to another with a name with '_backup' suffix, but in case table with such
            //  a name already exists we append suffix 1, then 2, etc until we find a free name..
            auto backupTableName = impl->table.name + "_backup";
            if (impl->table_exists(backupTableName, db)) {
                int suffix = 1;
                do {
                    std::stringstream stream;
                    stream << suffix;
                    auto anotherBackupTableName = backupTableName + stream.str();
                    if (!impl->table_exists(anotherBackupTableName, db)) {
                        backupTableName = anotherBackupTableName;
                        break;
                    }
                    ++suffix;
                } while (true);
            }

            this->create_table(db, backupTableName, impl);

            impl->copy_table(db, backupTableName);

            this->drop_table_internal(impl->table.name, db);

            impl->rename_table(db, backupTableName, impl->table.name);
        }

        template <class O>
        void assert_mapped_type()
        {
            using mapped_types_tuples = std::tuple<typename Ts::object_type...>;
            static_assert(tuple_helper::has_type<O, mapped_types_tuples>::value, "type is not mapped to a storage");
        }

        template <class O>
        auto& get_impl()
        {
            return this->impl.template get_impl<O>();
        }

        std::string escape(std::string text)
        {
            for (size_t i = 0; i < text.length();) {
                if (text[i] == '\'') {
                    text.insert(text.begin() + i, '\'');
                    i += 2;
                } else
                    ++i;
            }
            return text;
        }

        template <class T>
        typename std::enable_if<!is_base_of_template<T, compound_operator>::value, std::string>::type string_from_expression(T t, bool /*noTableName*/ = false, bool escape = false)
        {
            auto isNullable = type_is_nullable<T>::value;
            if (isNullable && !type_is_nullable<T>()(t)) {
                return "NULL";
            } else {
                auto needQuotes = std::is_base_of<text_printer, type_printer<T>>::value;
                std::stringstream ss;
                if (needQuotes) {
                    ss << "'";
                }
                std::string text = field_printer<T>()(t);
                if (escape) {
                    text = this->escape(text);
                }
                ss << text;
                if (needQuotes) {
                    ss << "'";
                }
                return ss.str();
            }
        }

        template <class T>
        std::string string_from_expression(const alias_holder<T>& holder, bool noTableName = false, bool /*escape*/ = false)
        {
            return T::get();
        }

        template <class T, class E>
        std::string string_from_expression(const as_t<T, E>& als, bool noTableName = false, bool /*escape*/ = false)
        {
            auto tableAliasString = alias_extractor<T>::get();
            return this->string_from_expression(als.expression) + " AS " + tableAliasString;
        }

        template <class T, class C>
        std::string string_from_expression(const alias_column_t<T, C>& als, bool noTableName = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            if (!noTableName) {
                ss << "'" << T::get() << "'.";
            }
            ss << this->string_from_expression(als.column, true);
            return ss.str();
        }

        std::string string_from_expression(const std::string& t, bool /*noTableName*/ = false, bool escape = false)
        {
            std::stringstream ss;
            std::string text = t;
            if (escape) {
                text = this->escape(text);
            }
            ss << "'" << text << "'";
            return ss.str();
        }

        std::string string_from_expression(const char* t, bool /*noTableName*/ = false, bool escape = false)
        {
            std::stringstream ss;
            std::string text = t;
            if (escape) {
                text = this->escape(text);
            }
            ss << "'" << text << "'";
            return ss.str();
        }

        template <class F, class O>
        std::string string_from_expression(F O::*m, bool noTableName = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            if (!noTableName) {
                ss << "'" << this->impl.template find_table_name<O>() << "'.";
            }
            ss << "\"" << this->impl.column_name(m) << "\"";
            return ss.str();
        }

        std::string string_from_expression(const rowid_t& rid, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            return static_cast<std::string>(rid);
        }

        std::string string_from_expression(const oid_t& rid, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            return static_cast<std::string>(rid);
        }

        std::string string_from_expression(const _rowid_t& rid, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            return static_cast<std::string>(rid);
        }

        template <class O>
        std::string string_from_expression(const table_rowid_t<O>& rid, bool noTableName = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            if (!noTableName) {
                ss << "'" << this->impl.template find_table_name<O>() << "'.";
            }
            ss << static_cast<std::string>(rid);
            return ss.str();
        }

        template <class O>
        std::string string_from_expression(const table_oid_t<O>& rid, bool noTableName = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            if (!noTableName) {
                ss << "'" << this->impl.template find_table_name<O>() << "'.";
            }
            ss << static_cast<std::string>(rid);
            return ss.str();
        }

        template <class O>
        std::string string_from_expression(const table__rowid_t<O>& rid, bool noTableName = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            if (!noTableName) {
                ss << "'" << this->impl.template find_table_name<O>() << "'.";
            }
            ss << static_cast<std::string>(rid);
            return ss.str();
        }

        template <class T>
        std::string string_from_expression(const aggregate_functions::group_concat_double_t<T>& f, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            auto expr = this->string_from_expression(f.t);
            auto expr2 = this->string_from_expression(f.y);
            ss << static_cast<std::string>(f) << "(" << expr << ", " << expr2 << ") ";
            return ss.str();
        }

        template <class T>
        std::string string_from_expression(const aggregate_functions::group_concat_single_t<T>& f, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            auto expr = this->string_from_expression(f.t);
            ss << static_cast<std::string>(f) << "(" << expr << ") ";
            return ss.str();
        }

        template <class L, class R>
        std::string string_from_expression(const conc_t<L, R>& f, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            auto lhs = this->string_from_expression(f.l);
            auto rhs = this->string_from_expression(f.r);
            ss << "(" << lhs << " || " << rhs << ") ";
            return ss.str();
        }

        template <class L, class R>
        std::string string_from_expression(const add_t<L, R>& f, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            auto lhs = this->string_from_expression(f.l);
            auto rhs = this->string_from_expression(f.r);
            ss << "(" << lhs << " + " << rhs << ") ";
            return ss.str();
        }

        template <class L, class R>
        std::string string_from_expression(const sub_t<L, R>& f, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            auto lhs = this->string_from_expression(f.l);
            auto rhs = this->string_from_expression(f.r);
            ss << "(" << lhs << " - " << rhs << ") ";
            return ss.str();
        }

        template <class L, class R>
        std::string string_from_expression(const mul_t<L, R>& f, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            auto lhs = this->string_from_expression(f.l);
            auto rhs = this->string_from_expression(f.r);
            ss << "(" << lhs << " * " << rhs << ") ";
            return ss.str();
        }

        template <class L, class R>
        std::string string_from_expression(const div_t<L, R>& f, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            auto lhs = this->string_from_expression(f.l);
            auto rhs = this->string_from_expression(f.r);
            ss << "(" << lhs << " / " << rhs << ") ";
            return ss.str();
        }

        template <class L, class R>
        std::string string_from_expression(const mod_t<L, R>& f, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            auto lhs = this->string_from_expression(f.l);
            auto rhs = this->string_from_expression(f.r);
            ss << "(" << lhs << " % " << rhs << ") ";
            return ss.str();
        }

        template <class T>
        std::string string_from_expression(const aggregate_functions::min_t<T>& f, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            auto expr = this->string_from_expression(f.t);
            ss << static_cast<std::string>(f) << "(" << expr << ") ";
            return ss.str();
        }

        template <class T>
        std::string string_from_expression(const aggregate_functions::max_t<T>& f, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            auto expr = this->string_from_expression(f.t);
            ss << static_cast<std::string>(f) << "(" << expr << ") ";
            return ss.str();
        }

        template <class T>
        std::string string_from_expression(const aggregate_functions::total_t<T>& f, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            auto expr = this->string_from_expression(f.t);
            ss << static_cast<std::string>(f) << "(" << expr << ") ";
            return ss.str();
        }

        template <class T>
        std::string string_from_expression(const aggregate_functions::sum_t<T>& f, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            auto expr = this->string_from_expression(f.t);
            ss << static_cast<std::string>(f) << "(" << expr << ") ";
            return ss.str();
        }

        template <class T>
        std::string string_from_expression(const aggregate_functions::count_asterisk_t<T>& f, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            return this->string_from_expression(aggregate_functions::count_asterisk_without_type {});
        }

        std::string string_from_expression(const aggregate_functions::count_asterisk_without_type& f, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            ss << static_cast<std::string>(f) << "(*) ";
            return ss.str();
        }

        template <class T>
        std::string string_from_expression(const aggregate_functions::count_t<T>& f, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            auto expr = this->string_from_expression(f.t);
            ss << static_cast<std::string>(f) << "(" << expr << ") ";
            return ss.str();
        }

        template <class T>
        std::string string_from_expression(const aggregate_functions::avg_t<T>& a, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            auto expr = this->string_from_expression(a.t);
            ss << static_cast<std::string>(a) << "(" << expr << ") ";
            return ss.str();
        }

        template <class T>
        std::string string_from_expression(const distinct_t<T>& f, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            auto expr = this->string_from_expression(f.t);
            ss << static_cast<std::string>(f) << "(" << expr << ") ";
            return ss.str();
        }

        template <class T>
        std::string string_from_expression(const all_t<T>& f, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            auto expr = this->string_from_expression(f.t);
            ss << static_cast<std::string>(f) << "(" << expr << ") ";
            return ss.str();
        }

        template <class X, class Y>
        std::string string_from_expression(const core_functions::rtrim_double_t<X, Y>& f, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            auto expr = this->string_from_expression(f.x);
            auto expr2 = this->string_from_expression(f.y);
            ss << static_cast<std::string>(f) << "(" << expr << ", " << expr2 << ") ";
            return ss.str();
        }

        template <class X>
        std::string string_from_expression(const core_functions::rtrim_single_t<X>& f, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            auto expr = this->string_from_expression(f.x);
            ss << static_cast<std::string>(f) << "(" << expr << ") ";
            return ss.str();
        }

        template <class X, class Y>
        std::string string_from_expression(const core_functions::ltrim_double_t<X, Y>& f, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            auto expr = this->string_from_expression(f.x);
            auto expr2 = this->string_from_expression(f.y);
            ss << static_cast<std::string>(f) << "(" << expr << ", " << expr2 << ") ";
            return ss.str();
        }

        template <class X>
        std::string string_from_expression(const core_functions::ltrim_single_t<X>& f, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            auto expr = this->string_from_expression(f.x);
            ss << static_cast<std::string>(f) << "(" << expr << ") ";
            return ss.str();
        }

        template <class X, class Y>
        std::string string_from_expression(const core_functions::trim_double_t<X, Y>& f, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            auto expr = this->string_from_expression(f.x);
            auto expr2 = this->string_from_expression(f.y);
            ss << static_cast<std::string>(f) << "(" << expr << ", " << expr2 << ") ";
            return ss.str();
        }

        template <class X>
        std::string string_from_expression(const core_functions::trim_single_t<X>& f, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            auto expr = this->string_from_expression(f.x);
            ss << static_cast<std::string>(f) << "(" << expr << ") ";
            return ss.str();
        }

        std::string string_from_expression(const core_functions::changes_t& ch, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            ss << static_cast<std::string>(ch) << "() ";
            return ss.str();
        }

        template <class T>
        std::string string_from_expression(const core_functions::length_t<T>& len, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            auto expr = this->string_from_expression(len.t);
            ss << static_cast<std::string>(len) << "(" << expr << ") ";
            return ss.str();
        }

        template <class T, class... Args>
        std::string string_from_expression(const core_functions::datetime_t<T, Args...>& f, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            ss << static_cast<std::string>(f) << "(" << this->string_from_expression(f.timestring);
            using tuple_t = std::tuple<Args...>;
            tuple_helper::iterator<std::tuple_size<tuple_t>::value - 1, Args...>()(f.modifiers, [&ss, this](auto& v) {
                ss << ", " << this->string_from_expression(v);
            });
            ss << ") ";
            return ss.str();
        }

        template <class T, class... Args>
        std::string string_from_expression(const core_functions::date_t<T, Args...>& f, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            ss << static_cast<std::string>(f) << "(" << this->string_from_expression(f.timestring);
            using tuple_t = std::tuple<Args...>;
            tuple_helper::iterator<std::tuple_size<tuple_t>::value - 1, Args...>()(f.modifiers, [&ss, this](auto& v) {
                ss << ", " << this->string_from_expression(v);
            },
                false);
            ss << ") ";
            return ss.str();
        }

        template <class T, class... Args>
        std::string string_from_expression(const core_functions::julianday_t<T, Args...>& f, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            ss << static_cast<std::string>(f) << "(" << this->string_from_expression(f.timestring);
            using tuple_t = std::tuple<Args...>;
            tuple_helper::iterator<std::tuple_size<tuple_t>::value - 1, Args...>()(f.modifiers, [&ss, this](auto& v) {
                ss << ", " << this->string_from_expression(v);
            },
                false);
            ss << ") ";
            return ss.str();
        }

        std::string string_from_expression(const core_functions::random_t& f, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            ss << static_cast<std::string>(f) << "() ";
            return ss.str();
        }

        template <class... Args>
        std::string string_from_expression(const core_functions::char_t_<Args...>& f, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            using tuple_t = decltype(f.args);
            std::vector<std::string> args;
            args.reserve(std::tuple_size<tuple_t>::value);
            tuple_helper::tuple_for_each(f.args, [&args, this](auto& v) {
                auto expression = this->string_from_expression(v);
                args.emplace_back(std::move(expression));
            });
            ss << static_cast<std::string>(f) << "(";
            auto lim = int(args.size());
            for (auto i = 0; i < lim; ++i) {
                ss << args[i];
                if (i < lim - 1) {
                    ss << ", ";
                } else {
                    ss << " ";
                }
            }
            ss << ") ";
            return ss.str();
        }

        template <class T>
        std::string string_from_expression(const core_functions::upper_t<T>& a, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            auto expr = this->string_from_expression(a.t);
            ss << static_cast<std::string>(a) << "(" << expr << ") ";
            return ss.str();
        }

        template <class T>
        std::string string_from_expression(const core_functions::lower_t<T>& a, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            auto expr = this->string_from_expression(a.t);
            ss << static_cast<std::string>(a) << "(" << expr << ") ";
            return ss.str();
        }

        template <class T>
        std::string string_from_expression(const core_functions::abs_t<T>& a, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            auto expr = this->string_from_expression(a.t);
            ss << static_cast<std::string>(a) << "(" << expr << ") ";
            return ss.str();
        }

        template <class T, class F>
        std::string string_from_expression(const column_pointer<T, F>& c, bool noTableName = false, bool escape = false)
        {
            std::stringstream ss;
            if (!noTableName) {
                ss << "'" << this->impl.template find_table_name<T>() << "'.";
            }
            auto& impl = this->get_impl<T>();
            ss << "\"" << impl.column_name_simple(c.field) << "\"";
            return ss.str();
        }

        template <class T>
        std::vector<std::string> get_column_names(const T& t)
        {
            auto columnName = this->string_from_expression(t);
            if (columnName.length()) {
                return { columnName };
            } else {
                throw std::system_error(std::make_error_code(orm_error_code::column_not_found));
            }
        }

        template <class T>
        std::vector<std::string> get_column_names(const internal::asterisk_t<T>& ast)
        {
            std::vector<std::string> res;
            res.push_back("*");
            return res;
        }

        template <class... Args>
        std::vector<std::string> get_column_names(const internal::columns_t<Args...>& cols)
        {
            std::vector<std::string> columnNames;
            columnNames.reserve(cols.count());
            cols.for_each([&columnNames, this](auto& m) {
                auto columnName = this->string_from_expression(m);
                if (columnName.length()) {
                    columnNames.push_back(columnName);
                } else {
                    throw std::system_error(std::make_error_code(orm_error_code::column_not_found));
                }
            });
            return columnNames;
        }

        /**
             *  Takes select_t object and returns SELECT query string
             */
        template <class T, class... Args>
        std::string string_from_expression(const internal::select_t<T, Args...>& sel, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            if (!is_base_of_template<T, compound_operator>::value) {
                if (!sel.highest_level) {
                    ss << "( ";
                }
                ss << "SELECT ";
            }
            if (get_distinct(sel.col)) {
                ss << static_cast<std::string>(distinct(0)) << " ";
            }
            auto columnNames = this->get_column_names(sel.col);
            for (size_t i = 0; i < columnNames.size(); ++i) {
                ss << columnNames[i];
                if (i < columnNames.size() - 1) {
                    ss << ",";
                }
                ss << " ";
            }
            auto tableNamesSet = this->parse_table_names(sel.col);
            internal::join_iterator<Args...>()([&tableNamesSet, this](const auto& c) {
                using original_join_type = typename std::decay<decltype(c)>::type::type;
                using cross_join_type = typename internal::mapped_type_proxy<original_join_type>::type;
                auto crossJoinedTableName = this->impl.template find_table_name<cross_join_type>();
                auto tableAliasString = alias_extractor<original_join_type>::get();
                std::pair<std::string, std::string> tableNameWithAlias(std::move(crossJoinedTableName), std::move(tableAliasString));
                tableNamesSet.erase(tableNameWithAlias);
            });
            if (!tableNamesSet.empty()) {
                ss << "FROM ";
                std::vector<std::pair<std::string, std::string>> tableNames(tableNamesSet.begin(), tableNamesSet.end());
                for (size_t i = 0; i < tableNames.size(); ++i) {
                    auto& tableNamePair = tableNames[i];
                    ss << "'" << tableNamePair.first << "' ";
                    if (!tableNamePair.second.empty()) {
                        ss << tableNamePair.second << " ";
                    }
                    if (int(i) < int(tableNames.size()) - 1) {
                        ss << ",";
                    }
                    ss << " ";
                }
            }
            using tuple_t = typename std::decay<decltype(sel)>::type::conditions_type;
            tuple_helper::iterator<std::tuple_size<tuple_t>::value - 1, Args...>()(sel.conditions, [&ss, this](auto& v) {
                this->process_single_condition(ss, v);
            },
                false);
            if (!is_base_of_template<T, compound_operator>::value) {
                if (!sel.highest_level) {
                    ss << ") ";
                }
            }
            return ss.str();
        }

        template <class T, class E>
        std::string string_from_expression(const conditions::cast_t<T, E>& c, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            ss << static_cast<std::string>(c) << " ( " << this->string_from_expression(c.expression) << " AS " << type_printer<T>().print() << ") ";
            return ss.str();
        }

        template <class T>
        typename std::enable_if<is_base_of_template<T, compound_operator>::value, std::string>::type string_from_expression(const T& op, bool /*noTableName*/ = false, bool /*escape*/ = false)
        {
            std::stringstream ss;
            ss << this->string_from_expression(op.left) << " ";
            ss << static_cast<std::string>(op) << " ";
            ss << this->string_from_expression(op.right) << " ";
            return ss.str();
        }

        template <class T>
        std::string process_where(const conditions::is_null_t<T>& c)
        {
            std::stringstream ss;
            ss << this->string_from_expression(c.t) << " " << static_cast<std::string>(c) << " ";
            return ss.str();
        }

        template <class T>
        std::string process_where(const conditions::is_not_null_t<T>& c)
        {
            std::stringstream ss;
            ss << this->string_from_expression(c.t) << " " << static_cast<std::string>(c) << " ";
            return ss.str();
        }

        template <class C>
        std::string process_where(const conditions::negated_condition_t<C>& c)
        {
            std::stringstream ss;
            ss << " " << static_cast<std::string>(c) << " ";
            auto cString = this->process_where(c.c);
            ss << " (" << cString << " ) ";
            return ss.str();
        }

        template <class L, class R>
        std::string process_where(const conditions::and_condition_t<L, R>& c)
        {
            std::stringstream ss;
            ss << " (" << this->process_where(c.l) << ") " << static_cast<std::string>(c) << " (" << this->process_where(c.r) << ") ";
            return ss.str();
        }

        template <class L, class R>
        std::string process_where(const conditions::or_condition_t<L, R>& c)
        {
            std::stringstream ss;
            ss << " (" << this->process_where(c.l) << ") " << static_cast<std::string>(c) << " (" << this->process_where(c.r) << ") ";
            return ss.str();
        }

        /**
             *  Common case. Is used to process binary conditions like is_equal, not_equal
             */
        template <class C>
        std::string process_where(const C& c)
        {
            auto leftString = this->string_from_expression(c.l, false, true);
            auto rightString = this->string_from_expression(c.r, false, true);
            std::stringstream ss;
            ss << leftString << " " << static_cast<std::string>(c) << " " << rightString;
            return ss.str();
        }

        template <class T>
        std::string process_where(const conditions::named_collate<T>& col)
        {
            auto res = this->process_where(col.expr);
            return res + " " + static_cast<std::string>(col);
        }

        template <class T>
        std::string process_where(const conditions::collate_t<T>& col)
        {
            auto res = this->process_where(col.expr);
            return res + " " + static_cast<std::string>(col);
        }

        template <class L, class A>
        std::string process_where(const conditions::in_t<L, A>& inCondition)
        {
            std::stringstream ss;
            auto leftString = this->string_from_expression(inCondition.l);
            ss << leftString << " " << static_cast<std::string>(inCondition) << " ";
            ss << this->string_from_expression(inCondition.arg);
            ss << " ";
            return ss.str();
        }

        template <class L, class E>
        std::string process_where(const conditions::in_t<L, std::vector<E>>& inCondition)
        {
            std::stringstream ss;
            auto leftString = this->string_from_expression(inCondition.l);
            ss << leftString << " " << static_cast<std::string>(inCondition) << " ( ";
            for (size_t index = 0; index < inCondition.arg.size(); ++index) {
                auto& value = inCondition.arg[index];
                ss << " " << this->string_from_expression(value);
                if (index < inCondition.arg.size() - 1) {
                    ss << ", ";
                }
            }
            ss << " )";
            return ss.str();
        }

        template <class A, class T>
        std::string process_where(const conditions::like_t<A, T>& l)
        {
            std::stringstream ss;
            ss << this->string_from_expression(l.a) << " " << static_cast<std::string>(l) << " " << this->string_from_expression(l.t) << " ";
            return ss.str();
        }

        template <class A, class T>
        std::string process_where(const conditions::between_t<A, T>& bw)
        {
            std::stringstream ss;
            auto expr = this->string_from_expression(bw.expr);
            ss << expr << " " << static_cast<std::string>(bw) << " " << this->string_from_expression(bw.b1) << " AND " << this->string_from_expression(bw.b2) << " ";
            return ss.str();
        }

        template <class T>
        std::string process_where(const conditions::exists_t<T>& e)
        {
            std::stringstream ss;
            ss << static_cast<std::string>(e) << " " << this->string_from_expression(e.t) << " ";
            return ss.str();
        }

        template <class O>
        std::string process_order_by(const conditions::order_by_t<O>& orderBy)
        {
            std::stringstream ss;
            auto columnName = this->string_from_expression(orderBy.o);
            ss << columnName << " ";
            if (orderBy._collate_argument.length()) {
                ss << "COLLATE " << orderBy._collate_argument << " ";
            }
            switch (orderBy.asc_desc) {
            case 1:
                ss << "ASC ";
                break;
            case -1:
                ss << "DESC ";
                break;
            }
            return ss.str();
        }

        template <class T>
        void process_join_constraint(std::stringstream& ss, const conditions::on_t<T>& t)
        {
            ss << static_cast<std::string>(t) << " " << this->process_where(t.t) << " ";
        }

        template <class F, class O>
        void process_join_constraint(std::stringstream& ss, const conditions::using_t<F, O>& u)
        {
            ss << static_cast<std::string>(u) << " (" << this->string_from_expression(u.column, true) << " ) ";
        }

        void process_single_condition(std::stringstream& ss, const conditions::limit_t& limt)
        {
            ss << static_cast<std::string>(limt) << " ";
            if (limt.has_offset) {
                if (limt.offset_is_implicit) {
                    ss << limt.off << ", " << limt.lim;
                } else {
                    ss << limt.lim << " OFFSET " << limt.off;
                }
            } else {
                ss << limt.lim;
            }
            ss << " ";
        }

        template <class O>
        void process_single_condition(std::stringstream& ss, const conditions::cross_join_t<O>& c)
        {
            ss << static_cast<std::string>(c) << " ";
            ss << " '" << this->impl.template find_table_name<O>() << "' ";
        }

        template <class O>
        void process_single_condition(std::stringstream& ss, const conditions::natural_join_t<O>& c)
        {
            ss << static_cast<std::string>(c) << " ";
            ss << " '" << this->impl.template find_table_name<O>() << "' ";
        }

        template <class T, class O>
        void process_single_condition(std::stringstream& ss, const conditions::inner_join_t<T, O>& l)
        {
            ss << static_cast<std::string>(l) << " ";
            auto aliasString = alias_extractor<T>::get();
            ss << " '" << this->impl.template find_table_name<typename mapped_type_proxy<T>::type>() << "' ";
            if (aliasString.length()) {
                ss << "'" << aliasString << "' ";
            }
            this->process_join_constraint(ss, l.constraint);
        }

        template <class T, class O>
        void process_single_condition(std::stringstream& ss, const conditions::left_outer_join_t<T, O>& l)
        {
            ss << static_cast<std::string>(l) << " ";
            ss << " '" << this->impl.template find_table_name<T>() << "' ";
            this->process_join_constraint(ss, l.constraint);
        }

        template <class T, class O>
        void process_single_condition(std::stringstream& ss, const conditions::left_join_t<T, O>& l)
        {
            ss << static_cast<std::string>(l) << " ";
            ss << " '" << this->impl.template find_table_name<T>() << "' ";
            this->process_join_constraint(ss, l.constraint);
        }

        template <class T, class O>
        void process_single_condition(std::stringstream& ss, const conditions::join_t<T, O>& l)
        {
            ss << static_cast<std::string>(l) << " ";
            ss << " '" << this->impl.template find_table_name<T>() << "' ";
            this->process_join_constraint(ss, l.constraint);
        }

        template <class C>
        void process_single_condition(std::stringstream& ss, const conditions::where_t<C>& w)
        {
            ss << static_cast<std::string>(w) << " ";
            auto whereString = this->process_where(w.c);
            ss << "( " << whereString << ") ";
        }

        template <class O>
        void process_single_condition(std::stringstream& ss, const conditions::order_by_t<O>& orderBy)
        {
            ss << static_cast<std::string>(orderBy) << " ";
            auto orderByString = this->process_order_by(orderBy);
            ss << orderByString << " ";
        }

        template <class... Args>
        void process_single_condition(std::stringstream& ss, const conditions::multi_order_by_t<Args...>& orderBy)
        {
            std::vector<std::string> expressions;
            using tuple_t = std::tuple<Args...>;
            tuple_helper::iterator<std::tuple_size<tuple_t>::value - 1, Args...>()(orderBy.args, [&expressions, this](auto& v) {
                auto expression = this->process_order_by(v);
                expressions.insert(expressions.begin(), expression);
            });
            ss << static_cast<std::string>(orderBy) << " ";
            for (size_t i = 0; i < expressions.size(); ++i) {
                ss << expressions[i];
                if (i < expressions.size() - 1) {
                    ss << ", ";
                }
            }
            ss << " ";
        }

        template <class... Args>
        void process_single_condition(std::stringstream& ss, const conditions::group_by_t<Args...>& groupBy)
        {
            std::vector<std::string> expressions;
            using tuple_t = std::tuple<Args...>;
            tuple_helper::iterator<std::tuple_size<tuple_t>::value - 1, Args...>()(groupBy.args, [&expressions, this](auto& v) {
                auto expression = this->string_from_expression(v);
                expressions.push_back(expression);
            });
            ss << static_cast<std::string>(groupBy) << " ";
            for (size_t i = 0; i < expressions.size(); ++i) {
                ss << expressions[i];
                if (i < expressions.size() - 1) {
                    ss << ", ";
                }
            }
            ss << " ";
        }

        template <class T>
        void process_single_condition(std::stringstream& ss, const conditions::having_t<T>& hav)
        {
            ss << static_cast<std::string>(hav) << " ";
            ss << this->process_where(hav.t) << " ";
        }

        /**
             *  Recursion end.
             */
        template <class... Args>
        void process_conditions(std::stringstream&, Args... /*args*/)
        {
            //..
        }

        template <class C, class... Args>
        void process_conditions(std::stringstream& ss, C c, Args&&... args)
        {
            this->process_single_condition(ss, c);
            this->process_conditions(ss, std::forward<Args>(args)...);
        }

        void on_open_internal(database* db)
        {

            if (this->foreign_keys_count()) {
                this->foreign_keys(db, true);
            }
            if (this->pragma._synchronous != -1) {
                this->pragma.synchronous(this->pragma._synchronous);
            }

            if (this->pragma._journal_mode != -1) {
                this->pragma.set_pragma("journal_mode", static_cast<journal_mode>(this->pragma._journal_mode), db);
            }

            //            for (auto& p : this->collatingFunctions) {
            //                if (sqlite3_create_collation(db,
            //                        p.first.c_str(),
            //                        SQLITE_UTF8,
            //                        &p.second,
            //                        collate_callback)
            //                    != SQLITE_OK) {
            //                    throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
            //                }
            //            }

            //            for (auto& p : this->limit.limits) {
            //                sqlite3_limit(db, p.first, p.second);
            //            }

            if (this->on_open) {
                this->on_open(db);
            }
        }

        //  returns foreign keys count in storage definition
        int foreign_keys_count()
        {
            auto res = 0;
            this->impl.for_each([&res](auto impl) {
                res += impl->foreign_keys_count();
            });
            return res;
        }
        static int collate_callback(void* arg, int leftLen, const void* lhs, int rightLen, const void* rhs)
        {
            auto& f = *(collating_function*)arg;
            return f(leftLen, lhs, rightLen, rhs);
        }

    public:
        template <class T, class... Args>
        view_t<T, Args...> iterate(Args&&... args)
        {
            this->assert_mapped_type<T>();

            auto connection = this->get_or_create_connection();
            return { *this, connection, std::forward<Args>(args)... };
        }

        void create_collation(const std::string& name, collating_function f)
        {
            collating_function* functionPointer = nullptr;
            if (f) {
                functionPointer = &(collatingFunctions[name] = f);
            } else {
                collatingFunctions.erase(name);
            }

            //  create collations if db is open
            //            if (this->currentTransaction) {
            //                auto db = this->currentTransaction->get_db();
            //                if (sqlite3_create_collation(db,
            //                        name.c_str(),
            //                        SQLITE_UTF8,
            //                        functionPointer,
            //                        f ? collate_callback : nullptr)
            //                    != SQLITE_OK) {
            //                    throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
            //                }
            //            }
        }

        template <class O, class... Args>
        void remove_all(Args&&... args)
        {
            this->assert_mapped_type<O>();

            auto connection = this->get_or_create_connection();
            auto& impl = this->get_impl<O>();
            std::stringstream ss;
            ss << "DELETE FROM '" << impl.table.name << "' ";
            this->process_conditions(ss, std::forward<Args>(args)...);
            auto db = connection->get_db();
            auto query = db->make_query(ss.str());

            if (db->exec(query.get()) != query::step::done) {
                throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
            }
        }

        /**
             *  Delete routine.
             *  O is an object's type. Must be specified explicitly.
             *  @param id id of object to be removed.
             */
        template <class O, class I>
        void remove(I id)
        {
            this->assert_mapped_type<O>();

            auto connection = this->get_or_create_connection();
            auto& impl = this->get_impl<O>();
            std::stringstream ss;
            ss << "DELETE FROM '" << impl.table.name << "' ";
            ss << "WHERE ";
            auto primaryKeyColumnNames = impl.table.primary_key_column_names();
            for (size_t i = 0; i < primaryKeyColumnNames.size(); ++i) {
                ss << "\"" << primaryKeyColumnNames[i] << "\""
                   << " =  ?";
                if (i < primaryKeyColumnNames.size() - 1) {
                    ss << " AND ";
                } else {
                    ss << " ";
                }
            }
            auto db = connection->get_db();
            auto query = db->make_query(ss.str());
            if (db->prepare(query.get())) {
                auto index = 0;
                statement_binder<I>().bind(db, query.get(), index, id);

                if (query->next(db) != query::step::done) {
                    throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
                }
            } else {
                throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
            }
        }

        /**
             *  Update routine. Sets all non primary key fields where primary key is equal.
             *  O is an object type. May be not specified explicitly cause it can be deduced by
             *      compiler from first parameter.
             *  @param o object to be updated.
             */
        template <class O>
        void update(const O& o)
        {
            this->assert_mapped_type<O>();

            auto connection = this->get_or_create_connection();
            auto& impl = this->get_impl<O>();
            std::stringstream ss;
            ss << "UPDATE '" << impl.table.name << "' SET ";
            std::vector<std::string> setColumnNames;
            impl.table.for_each_column([&setColumnNames](auto c) {
                if (!c.template has<constraints::primary_key_t<>>()) {
                    setColumnNames.emplace_back(c.name);
                }
            });
            for (size_t i = 0; i < setColumnNames.size(); ++i) {
                ss << "\"" << setColumnNames[i] << "\""
                   << " = ?";
                if (i < setColumnNames.size() - 1) {
                    ss << ",";
                }
                ss << " ";
            }
            ss << "WHERE ";
            auto primaryKeyColumnNames = impl.table.primary_key_column_names();
            for (size_t i = 0; i < primaryKeyColumnNames.size(); ++i) {
                ss << "\"" << primaryKeyColumnNames[i] << "\""
                   << " = ?";
                if (i < primaryKeyColumnNames.size() - 1) {
                    ss << " AND";
                }
                ss << " ";
            }
            auto db = connection->get_db();
            auto query = db->make_query(ss.str());
            if (db->prepare(query.get())) {
                auto index = 0;
                impl.table.for_each_column([db, &o, query, &index](auto& c) {
                    if (!c.template has<constraints::primary_key_t<>>()) {
                        using field_type = typename std::decay<decltype(c)>::type::field_type;
                        const field_type* value = nullptr;
                        if (c.member_pointer) {
                            value = &(o.*c.member_pointer);
                        } else {
                            value = &((o).*(c.getter))();
                        }
                        statement_binder<field_type>().bind(db, query.get(), index++, *value);
                    }
                });
                impl.table.for_each_column([db, &o, query, &index](auto& c) {
                    if (c.template has<constraints::primary_key_t<>>()) {
                        using field_type = typename std::decay<decltype(c)>::type::field_type;
                        const field_type* value = nullptr;
                        if (c.member_pointer) {
                            value = &(o.*c.member_pointer);
                        } else {
                            value = &((o).*(c.getter))();
                        }
                        statement_binder<field_type>().bind(db, query.get(), index++, *value);
                    }
                });
                //                if (sqlite3_step(stmt) == SQLITE_DONE) {
                //                    //  done..
                //                } else {
                //                    throw std::system_error(std::error_code(sqlite3_errcode(connection->get_db()), db->error_category()));
                //                }
            } else {
                throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
            }
        }

        template <class... Args, class... Wargs>
        void update_all(internal::set_t<Args...> set, Wargs... wh)
        {
            auto connection = this->get_or_create_connection();

            std::stringstream ss;
            ss << "UPDATE ";
            std::set<std::pair<std::string, std::string>> tableNamesSet;
            set.for_each([this, &tableNamesSet](auto& asgn) {
                auto tableName = this->parse_table_name(asgn.l);
                tableNamesSet.insert(tableName.begin(), tableName.end());
            });
            if (!tableNamesSet.empty()) {
                if (tableNamesSet.size() == 1) {
                    ss << " '" << tableNamesSet.begin()->first << "' ";
                    ss << static_cast<std::string>(set) << " ";
                    std::vector<std::string> setPairs;
                    set.for_each([this, &setPairs](auto& asgn) {
                        std::stringstream sss;
                        sss << this->string_from_expression(asgn.l, true) << " = " << this->string_from_expression(asgn.r) << " ";
                        setPairs.push_back(sss.str());
                    });
                    auto setPairsCount = setPairs.size();
                    for (size_t i = 0; i < setPairsCount; ++i) {
                        ss << setPairs[i] << " ";
                        if (i < setPairsCount - 1) {
                            ss << ", ";
                        }
                    }
                    this->process_conditions(ss, wh...);
                    auto db = connection->get_db();
                    auto query = db->make_query(ss.str());
                    if (db->exec(query.get()) != query::step::done) {
                        throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
                    }

                } else {
                    throw std::system_error(std::make_error_code(orm_error_code::too_many_tables_specified));
                }
            } else {
                throw std::system_error(std::make_error_code(orm_error_code::incorrect_set_fields_specified));
            }
        }

    protected:
        /**
             *  O - mapped type
             *  Args - conditions
             *  @param query - result query string
             *  @return impl for O
             */
        template <class O, class... Args>
        auto& generate_select_asterisk(std::string* query, Args&&... args)
        {
            std::stringstream ss;
            ss << "SELECT ";
            auto& impl = this->get_impl<O>();
            auto columnNames = impl.table.column_names();
            for (size_t i = 0; i < columnNames.size(); ++i) {
                ss
                    << "'" << impl.table.name << "'."
                    << "\""
                    << columnNames[i]
                    << "\"";
                if (i < columnNames.size() - 1) {
                    ss << ", ";
                } else {
                    ss << " ";
                }
            }
            ss << "FROM '" << impl.table.name << "' ";
            this->process_conditions(ss, std::forward<Args>(args)...);
            if (query) {
                *query = ss.str();
            }
            return impl;
        }

        template <class T>
        std::set<std::pair<std::string, std::string>> parse_table_name(const T&)
        {
            return {};
        }

        template <class F, class O>
        std::set<std::pair<std::string, std::string>> parse_table_name(F O::*, std::string alias = {})
        {
            return { std::make_pair(this->impl.template find_table_name<O>(), std::move(alias)) };
        }

        template <class T>
        std::set<std::pair<std::string, std::string>> parse_table_name(const aggregate_functions::min_t<T>& f)
        {
            return this->parse_table_name(f.t);
        }

        template <class T>
        std::set<std::pair<std::string, std::string>> parse_table_name(const aggregate_functions::max_t<T>& f)
        {
            return this->parse_table_name(f.t);
        }

        template <class T>
        std::set<std::pair<std::string, std::string>> parse_table_name(const aggregate_functions::sum_t<T>& f)
        {
            return this->parse_table_name(f.t);
        }

        template <class T>
        std::set<std::pair<std::string, std::string>> parse_table_name(const aggregate_functions::total_t<T>& f)
        {
            return this->parse_table_name(f.t);
        }

        template <class T>
        std::set<std::pair<std::string, std::string>> parse_table_name(const aggregate_functions::group_concat_double_t<T>& f)
        {
            auto res = this->parse_table_name(f.t);
            auto secondSet = this->parse_table_name(f.y);
            res.insert(secondSet.begin(), secondSet.end());
            return res;
        }

        template <class T>
        std::set<std::pair<std::string, std::string>> parse_table_name(const aggregate_functions::group_concat_single_t<T>& f)
        {
            return this->parse_table_name(f.t);
        }

        template <class T>
        std::set<std::pair<std::string, std::string>> parse_table_name(const aggregate_functions::count_t<T>& f)
        {
            return this->parse_table_name(f.t);
        }

        template <class T>
        std::set<std::pair<std::string, std::string>> parse_table_name(const aggregate_functions::avg_t<T>& a)
        {
            return this->parse_table_name(a.t);
        }

        template <class T>
        std::set<std::pair<std::string, std::string>> parse_table_name(const core_functions::length_t<T>& len)
        {
            return this->parse_table_name(len.t);
        }

        template <class T, class... Args>
        std::set<std::pair<std::string, std::string>> parse_table_name(const core_functions::date_t<T, Args...>& f)
        {
            auto res = this->parse_table_name(f.timestring);
            using tuple_t = decltype(f.modifiers);
            tuple_helper::iterator<std::tuple_size<tuple_t>::value - 1, Args...>()(f.modifiers, [&res, this](auto& v) {
                auto tableNames = this->parse_table_name(v);
                res.insert(tableNames.begin(), tableNames.end());
            });
            return res;
        }

        template <class T, class... Args>
        std::set<std::pair<std::string, std::string>> parse_table_name(const core_functions::datetime_t<T, Args...>& f)
        {
            auto res = this->parse_table_name(f.timestring);
            using tuple_t = decltype(f.modifiers);
            tuple_helper::iterator<std::tuple_size<tuple_t>::value - 1, Args...>()(f.modifiers, [&res, this](auto& v) {
                auto tableNames = this->parse_table_name(v);
                res.insert(tableNames.begin(), tableNames.end());
            });
            return res;
        }

        template <class X>
        std::set<std::pair<std::string, std::string>> parse_table_name(const core_functions::trim_single_t<X>& f)
        {
            return this->parse_table_name(f.x);
        }

        template <class X, class Y>
        std::set<std::pair<std::string, std::string>> parse_table_name(const core_functions::trim_double_t<X, Y>& f)
        {
            auto res = this->parse_table_name(f.x);
            auto res2 = this->parse_table_name(f.y);
            res.insert(res2.begin(), res2.end());
            return res;
        }

        template <class X>
        std::set<std::pair<std::string, std::string>> parse_table_name(const core_functions::rtrim_single_t<X>& f)
        {
            return this->parse_table_name(f.x);
        }

        template <class X, class Y>
        std::set<std::pair<std::string, std::string>> parse_table_name(const core_functions::rtrim_double_t<X, Y>& f)
        {
            auto res = this->parse_table_name(f.x);
            auto res2 = this->parse_table_name(f.y);
            res.insert(res2.begin(), res2.end());
            return res;
        }

        template <class X>
        std::set<std::pair<std::string, std::string>> parse_table_name(const core_functions::ltrim_single_t<X>& f)
        {
            return this->parse_table_name(f.x);
        }

        template <class X, class Y>
        std::set<std::pair<std::string, std::string>> parse_table_name(const core_functions::ltrim_double_t<X, Y>& f)
        {
            auto res = this->parse_table_name(f.x);
            auto res2 = this->parse_table_name(f.y);
            res.insert(res2.begin(), res2.end());
            return res;
        }

        template <class... Args>
        std::set<std::pair<std::string, std::string>> parse_table_name(const core_functions::char_t_<Args...>& f)
        {
            std::set<std::pair<std::string, std::string>> res;
            using tuple_t = decltype(f.args);
            tuple_helper::iterator<std::tuple_size<tuple_t>::value - 1, Args...>()(f.args, [&res, this](auto& v) {
                auto tableNames = this->parse_table_name(v);
                res.insert(tableNames.begin(), tableNames.end());
            });
            return res;
        }

        std::set<std::pair<std::string, std::string>> parse_table_name(const core_functions::random_t&)
        {
            return {};
        }

        template <class T>
        std::set<std::pair<std::string, std::string>> parse_table_name(const core_functions::upper_t<T>& a)
        {
            return this->parse_table_name(a.t);
        }

        template <class T>
        std::set<std::pair<std::string, std::string>> parse_table_name(const core_functions::lower_t<T>& a)
        {
            return this->parse_table_name(a.t);
        }

        template <class T>
        std::set<std::pair<std::string, std::string>> parse_table_name(const core_functions::abs_t<T>& a)
        {
            return this->parse_table_name(a.t);
        }

        template <class T>
        std::set<std::pair<std::string, std::string>> parse_table_name(const distinct_t<T>& f)
        {
            return this->parse_table_name(f.t);
        }

        template <class T>
        std::set<std::pair<std::string, std::string>> parse_table_name(const all_t<T>& f)
        {
            return this->parse_table_name(f.t);
        }

        template <class L, class R, class... Args>
        std::set<std::pair<std::string, std::string>> parse_table_name(const conc_t<L, R>& f)
        {
            std::set<std::pair<std::string, std::string>> res;
            auto leftSet = this->parse_table_names(f.l);
            res.insert(leftSet.begin(), leftSet.end());
            auto rightSet = this->parse_table_names(f.r);
            res.insert(rightSet.begin(), rightSet.end());
            return res;
        }

        template <class L, class R, class... Args>
        std::set<std::pair<std::string, std::string>> parse_table_name(const add_t<L, R>& f)
        {
            std::set<std::pair<std::string, std::string>> res;
            auto leftSet = this->parse_table_names(f.l);
            res.insert(leftSet.begin(), leftSet.end());
            auto rightSet = this->parse_table_names(f.r);
            res.insert(rightSet.begin(), rightSet.end());
            return res;
        }

        template <class L, class R, class... Args>
        std::set<std::pair<std::string, std::string>> parse_table_name(const sub_t<L, R>& f)
        {
            std::set<std::pair<std::string, std::string>> res;
            auto leftSet = this->parse_table_names(f.l);
            res.insert(leftSet.begin(), leftSet.end());
            auto rightSet = this->parse_table_names(f.r);
            res.insert(rightSet.begin(), rightSet.end());
            return res;
        }

        template <class L, class R, class... Args>
        std::set<std::pair<std::string, std::string>> parse_table_name(const mul_t<L, R>& f)
        {
            std::set<std::pair<std::string, std::string>> res;
            auto leftSet = this->parse_table_names(f.l);
            res.insert(leftSet.begin(), leftSet.end());
            auto rightSet = this->parse_table_names(f.r);
            res.insert(rightSet.begin(), rightSet.end());
            return res;
        }

        template <class L, class R, class... Args>
        std::set<std::pair<std::string, std::string>> parse_table_name(const div_t<L, R>& f)
        {
            std::set<std::pair<std::string, std::string>> res;
            auto leftSet = this->parse_table_names(f.l);
            res.insert(leftSet.begin(), leftSet.end());
            auto rightSet = this->parse_table_names(f.r);
            res.insert(rightSet.begin(), rightSet.end());
            return res;
        }

        template <class L, class R, class... Args>
        std::set<std::pair<std::string, std::string>> parse_table_name(const mod_t<L, R>& f)
        {
            std::set<std::pair<std::string, std::string>> res;
            auto leftSet = this->parse_table_names(f.l);
            res.insert(leftSet.begin(), leftSet.end());
            auto rightSet = this->parse_table_names(f.r);
            res.insert(rightSet.begin(), rightSet.end());
            return res;
        }

        template <class T, class F>
        std::set<std::pair<std::string, std::string>> parse_table_name(const column_pointer<T, F>& c)
        {
            std::set<std::pair<std::string, std::string>> res;
            res.insert({ this->impl.template find_table_name<T>(), "" });
            return res;
        }

        template <class T, class C>
        std::set<std::pair<std::string, std::string>> parse_table_name(const alias_column_t<T, C>& a)
        {
            return this->parse_table_name(a.column, alias_extractor<T>::get());
        }

        template <class T>
        std::set<std::pair<std::string, std::string>> parse_table_name(const aggregate_functions::count_asterisk_t<T>& c)
        {
            auto tableName = this->impl.template find_table_name<T>();
            if (!tableName.empty()) {
                return { std::make_pair(std::move(tableName), "") };
            } else {
                return {};
            }
        }

        std::set<std::pair<std::string, std::string>> parse_table_name(const aggregate_functions::count_asterisk_without_type& c)
        {
            return {};
        }

        template <class T>
        std::set<std::pair<std::string, std::string>> parse_table_name(const asterisk_t<T>& ast)
        {
            auto tableName = this->impl.template find_table_name<T>();
            return { std::make_pair(std::move(tableName), "") };
        }

        template <class T, class E>
        std::set<std::pair<std::string, std::string>> parse_table_name(const conditions::cast_t<T, E>& c)
        {
            return this->parse_table_name(c.expression);
        }

        template <class... Args>
        std::set<std::pair<std::string, std::string>> parse_table_names(Args...)
        {
            return {};
        }

        template <class H, class... Args>
        std::set<std::pair<std::string, std::string>> parse_table_names(H h, Args&&... args)
        {
            auto res = this->parse_table_names(std::forward<Args>(args)...);
            auto tableName = this->parse_table_name(h);
            res.insert(tableName.begin(), tableName.end());
            return res;
        }

        template <class... Args>
        std::set<std::pair<std::string, std::string>> parse_table_names(const internal::columns_t<Args...>& cols)
        {
            std::set<std::pair<std::string, std::string>> res;
            cols.for_each([&res, this](auto& m) {
                auto tableName = this->parse_table_name(m);
                res.insert(tableName.begin(), tableName.end());
            });
            return res;
        }

        template <class F, class O, class... Args>
        std::string group_concat_internal(F O::*m, std::shared_ptr<const std::string> y, Args&&... args)
        {
            this->assert_mapped_type<O>();

            auto connection = this->get_or_create_connection();
            auto& impl = this->get_impl<O>();
            std::string res;
            std::stringstream ss;
            ss << "SELECT " << static_cast<std::string>(sqlite_orm::group_concat(0)) << "(";
            auto columnName = this->string_from_expression(m);
            if (columnName.length()) {
                ss << columnName;
                if (y) {
                    ss << ",\"" << *y << "\"";
                }
                ss << ") FROM '" << impl.table.name << "' ";
                this->process_conditions(ss, std::forward<Args>(args)...);
                auto db = connection->get_db();
                auto query = db->make_query(ss.str());

                if (db->exec(query.get()) != query::step::done) {
                    throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
                }
                res = row_extractor<std::string>().extract(query.get());
            } else {
                throw std::system_error(std::make_error_code(orm_error_code::column_not_found));
            }
            return res;
        }

    public:
        /**
             *  Select * with no conditions routine.
             *  O is an object type to be extracted. Must be specified explicitly.
             *  @return All objects of type O stored in database at the moment.
             */
        template <class O, class C = std::vector<O>, class... Args>
        C get_all(Args&&... args)
        {
            this->assert_mapped_type<O>();

            auto connection = this->get_or_create_connection();
            C res;
            std::string squery;
            auto& impl = this->generate_select_asterisk<O>(&squery, std::forward<Args>(args)...);
            auto db = connection->get_db();
            auto query = db->make_query(squery);

            if (!db->prepare(query.get())) {
                throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
            }

            query::step stepRes;
            do {
                stepRes = query->next(db);
                if (stepRes == query::step::row) {
                    O obj;
                    auto index = 0;
                    impl.table.for_each_column([&index, &obj, query](auto c) {
                        using field_type = typename decltype(c)::field_type;
                        auto value = row_extractor<field_type>().extract(query.get(), index++);
                        if (c.member_pointer) {
                            obj.*c.member_pointer = value;
                        } else {
                            ((obj).*(c.setter))(std::move(value));
                        }
                    });
                    res.push_back(std::move(obj));
                } else if (stepRes == query::step::done) {
                    break;
                }
            } while (stepRes == query::step::row);
            return res;
        }

        /**
             *  Select * by id routine.
             *  throws std::system_error(orm_error_code::not_found, orm_error_category) if object not found with given id.
             *  throws std::system_error with orm_error_category in case of db error.
             *  O is an object type to be extracted. Must be specified explicitly.
             *  @return Object of type O where id is equal parameter passed or throws `std::system_error(orm_error_code::not_found, orm_error_category)`
             *  if there is no object with such id.
             */
        template <class O, class... Ids>
        O get(Ids... ids)
        {
            this->assert_mapped_type<O>();

            auto connection = this->get_or_create_connection();
            auto& impl = this->get_impl<O>();
            std::shared_ptr<O> res;
            std::stringstream ss;
            ss << "SELECT ";
            auto columnNames = impl.table.column_names();
            for (size_t i = 0; i < columnNames.size(); ++i) {
                ss << "\"" << columnNames[i] << "\"";
                if (i < columnNames.size() - 1) {
                    ss << ", ";
                } else {
                    ss << " ";
                }
            }
            ss << "FROM '" << impl.table.name << "' WHERE ";
            auto primaryKeyColumnNames = impl.table.primary_key_column_names();
            if (primaryKeyColumnNames.size()) {
                for (size_t i = 0; i < primaryKeyColumnNames.size(); ++i) {
                    ss << "\"" << primaryKeyColumnNames[i] << "\""
                       << " = ? ";
                    if (i < primaryKeyColumnNames.size() - 1) {
                        ss << "AND ";
                    }
                    ss << ' ';
                }
                auto db = connection->get_db();
                auto query = db->make_query(ss.str());

                if (db->prepare(query.get())) {
                    auto index = 0;
                    auto idsTuple = std::make_tuple(std::forward<Ids>(ids)...);
                    constexpr const auto idsCount = std::tuple_size<decltype(idsTuple)>::value;
                    tuple_helper::iterator<idsCount - 1, Ids...>()(idsTuple, [db, query, &index](auto& v) {
                        using field_type = typename std::decay<decltype(v)>::type;
                        statement_binder<field_type>().bind(db, query.get(), index++, v);
                    });
                    index = 0;
                    auto stepRes = query->next(db);
                    if (stepRes == query::step::row) {
                        O res;
                        impl.table.for_each_column([&index, &res, query](auto c) {
                            using field_type = typename decltype(c)::field_type;
                            auto value = row_extractor<field_type>().extract(query.get(), index++);
                            if (c.member_pointer) {
                                res.*c.member_pointer = value;
                            } else {
                                ((res).*(c.setter))(std::move(value));
                            }
                        });
                        return res;
                    } else {
                        throw std::system_error(std::make_error_code(sqlite_orm::orm_error_code::not_found));
                    }
                }
            } else {
                throw std::system_error(std::make_error_code(sqlite_orm::orm_error_code::column_not_found));
            }
            return *res;
        }

        /**
             *  The same as `get` function but doesn't throw an exception if noting found but returns std::shared_ptr with null value.
             *  throws std::system_error in case of db error.
             */
        template <class O, class... Ids>
        std::shared_ptr<O> get_no_throw(Ids... ids)
        {
            this->assert_mapped_type<O>();

            auto connection = this->get_or_create_connection();
            auto& impl = this->get_impl<O>();
            std::shared_ptr<O> res;
            std::stringstream ss;
            ss << "SELECT ";
            auto columnNames = impl.table.column_names();
            for (size_t i = 0; i < columnNames.size(); ++i) {
                ss << "\"" << columnNames[i] << "\"";
                if (i < columnNames.size() - 1) {
                    ss << ", ";
                } else {
                    ss << " ";
                }
            }
            ss << "FROM '" << impl.table.name << "' WHERE ";
            auto primaryKeyColumnNames = impl.table.primary_key_column_names();
            if (primaryKeyColumnNames.size() && primaryKeyColumnNames.front().length()) {
                for (size_t i = 0; i < primaryKeyColumnNames.size(); ++i) {
                    ss << "\"" << primaryKeyColumnNames[i] << "\""
                       << " = ? ";
                    if (i < primaryKeyColumnNames.size() - 1) {
                        ss << "AND ";
                    }
                    ss << ' ';
                }
                auto db = connection->get_db();
                auto query = db->make_query(ss.str());

                if (db->prepare(query.get())) {
                    auto index = 0;
                    auto idsTuple = std::make_tuple(std::forward<Ids>(ids)...);
                    constexpr const auto idsCount = std::tuple_size<decltype(idsTuple)>::value;
                    tuple_helper::iterator<idsCount - 1, Ids...>()(idsTuple, [db, query, &index](auto& v) {
                        using field_type = typename std::decay<decltype(v)>::type;
                        statement_binder<field_type>().bind(db, query.get(), index++, v);
                    });
                    index = 0;
                    auto stepRes = query->next(db);
                    if (stepRes == query::step::row) {
                        O res;
                        impl.table.for_each_column([&index, &res, query](auto c) {
                            using field_type = typename decltype(c)::field_type;
                            auto value = row_extractor<field_type>().extract(query.get(), index++);
                            if (c.member_pointer) {
                                res.*c.member_pointer = value;
                            } else {
                                ((res).*(c.setter))(std::move(value));
                            }
                        });
                        return std::make_shared<O>(std::move(res));
                    } else if (stepRes == query::step::done) {
                        return {};
                    } else {
                        throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
                    }
                } else {
                    throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
                }
            } else {
                throw std::system_error(std::make_error_code(orm_error_code::table_has_no_primary_key_column));
            }
        }

        /**
             *  SELECT COUNT(*) with no conditions routine. https://www.sqlite.org/lang_aggfunc.html#count
             *  @return Number of O object in table.
             */
        template <class O, class... Args, class R = typename mapped_type_proxy<O>::type>
        int count(Args&&... args)
        {
            this->assert_mapped_type<R>();
            auto tableAliasString = alias_extractor<O>::get();

            auto connection = this->get_or_create_connection();
            auto& impl = this->get_impl<R>();
            int res = 0;
            std::stringstream ss;
            ss << "SELECT " << static_cast<std::string>(sqlite_orm::count()) << "(*) FROM '" << impl.table.name << "' ";
            if (!tableAliasString.empty()) {
                ss << "'" << tableAliasString << "' ";
            }
            this->process_conditions(ss, args...);
            auto db = connection->get_db();
            auto query = db->make_query(ss.str());
            if (db->prepare(query.get())) {
                auto rc = query->next(db);
                if (rc != query::step::row) {
                    throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
                }
                res = row_extractor<int>().extract(query.get(), 0);
            } else {
                throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
            }

            return res;
        }

        /**
             *  SELECT COUNT(X) https://www.sqlite.org/lang_aggfunc.html#count
             *  @param m member pointer to class mapped to the storage.
             */
        template <class F, class O, class... Args>
        int count(F O::*m, Args&&... args)
        {
            this->assert_mapped_type<O>();

            auto connection = this->get_or_create_connection();
            auto& impl = this->get_impl<O>();
            int res = 0;
            std::stringstream ss;
            ss << "SELECT " << static_cast<std::string>(sqlite_orm::count(0)) << "(";
            auto columnName = this->string_from_expression(m);
            if (columnName.length()) {
                ss << columnName << ") FROM '" << impl.table.name << "' ";
                this->process_conditions(ss, std::forward<Args>(args)...);
                auto db = connection->get_db();
                auto query = db->make_query(ss.str());
                if (!db->prepare(query.get())) {
                    throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
                }
                auto rc = query->next(db);

                if (rc == query::step::done) {
                    res = row_extractor<int>().extract(query.get(), 0);
                } else {
                    throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
                }
            } else {
                throw std::system_error(std::make_error_code(orm_error_code::column_not_found));
            }
            return res;
        }

        /**
             *  AVG(X) query.   https://www.sqlite.org/lang_aggfunc.html#avg
             *  @param m is a class member pointer (the same you passed into make_column).
             *  @return average value from db.
             */
        template <class F, class O, class... Args>
        double avg(F O::*m, Args&&... args)
        {
            this->assert_mapped_type<O>();

            auto connection = this->get_or_create_connection();
            auto& impl = this->get_impl<O>();
            double res = 0;
            std::stringstream ss;
            ss << "SELECT " << static_cast<std::string>(sqlite_orm::avg(0)) << "(";
            auto columnName = this->string_from_expression(m);
            if (columnName.length()) {
                ss << columnName << ") FROM '" << impl.table.name << "' ";
                this->process_conditions(ss, std::forward<Args>(args)...);
                auto db = connection->get_db();
                auto query = db->make_query(ss.str());
                if (!db->prepare(query.get())) {
                    throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
                }
                auto rc = query->next(db);
                if (rc == query::step::row) {
                    res = row_extractor<double>().extract(query.get(), 0);
                } else {
                    throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
                }
                return res;
            } else {
                throw std::system_error(std::make_error_code(orm_error_code::column_not_found));
            }
            return res;
        }

        template <class F, class O>
        std::string group_concat(F O::*m)
        {
            return this->group_concat_internal(m, {});
        }

        /**
             *  GROUP_CONCAT(X) query.  https://www.sqlite.org/lang_aggfunc.html#groupconcat
             *  @param m is a class member pointer (the same you passed into make_column).
             *  @return group_concat query result.
             */
        template <class F, class O, class... Args,
            class Tuple = std::tuple<Args...>,
            typename sfinae = typename std::enable_if<std::tuple_size<std::tuple<Args...>>::value >= 1>::type>
        std::string group_concat(F O::*m, Args&&... args)
        {
            return this->group_concat_internal(m, {}, std::forward<Args>(args)...);
        }

        /**
             *  GROUP_CONCAT(X, Y) query.   https://www.sqlite.org/lang_aggfunc.html#groupconcat
             *  @param m is a class member pointer (the same you passed into make_column).
             *  @return group_concat query result.
             */
        template <class F, class O, class... Args>
        std::string group_concat(F O::*m, const std::string& y, Args&&... args)
        {
            return this->group_concat_internal(m, std::make_shared<std::string>(y), std::forward<Args>(args)...);
        }

        template <class F, class O, class... Args>
        std::string group_concat(F O::*m, const char* y, Args&&... args)
        {
            return this->group_concat_internal(m, std::make_shared<std::string>(y), std::forward<Args>(args)...);
        }

        /**
             *  MAX(x) query.
             *  @param m is a class member pointer (the same you passed into make_column).
             *  @return std::shared_ptr with max value or null if sqlite engine returned null.
             */
        template <class F, class O, class... Args, class Ret = typename column_result_t<self, F O::*>::type>
        std::shared_ptr<Ret> max(F O::*m, Args&&... args)
        {
            this->assert_mapped_type<O>();

            auto connection = this->get_or_create_connection();
            auto& impl = this->get_impl<O>();
            std::shared_ptr<Ret> res;
            std::stringstream ss;
            ss << "SELECT " << static_cast<std::string>(sqlite_orm::max(0)) << "(";
            auto columnName = this->string_from_expression(m);
            if (columnName.length()) {
                ss << columnName << ") FROM '" << impl.table.name << "' ";
                this->process_conditions(ss, std::forward<Args>(args)...);
                auto db = connection->get_db();
                auto query = db->make_query(ss.str());
                if (!db->prepare(query.get())) {
                    throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
                }
                auto rc = query->next(db);
                if (rc == query::step::done) {
                    res = std::make_shared<Ret>(row_extractor<Ret>().extract(query.get(), 0));
                } else {
                    throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
                }
            } else {
                throw std::system_error(std::make_error_code(orm_error_code::column_not_found));
            }
            return res;
        }

        /**
             *  MIN(x) query.
             *  @param m is a class member pointer (the same you passed into make_column).
             *  @return std::shared_ptr with min value or null if sqlite engine returned null.
             */
        template <class F, class O, class... Args, class Ret = typename column_result_t<self, F O::*>::type>
        std::shared_ptr<Ret> min(F O::*m, Args&&... args)
        {
            this->assert_mapped_type<O>();

            auto connection = this->get_or_create_connection();
            auto& impl = this->get_impl<O>();
            std::shared_ptr<Ret> res;
            std::stringstream ss;
            ss << "SELECT " << static_cast<std::string>(sqlite_orm::min(0)) << "(";
            auto columnName = this->string_from_expression(m);
            if (columnName.length()) {
                ss << columnName << ") FROM '" << impl.table.name << "' ";
                this->process_conditions(ss, std::forward<Args>(args)...);
                auto db = connection->get_db();
                auto query = db->make_query(ss.str());

                if (!db->prepare(query.get())) {
                    throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
                }
                auto rc = query->next(db);
                if (rc == query::step::done) {
                    res = std::make_shared<Ret>(row_extractor<Ret>().extract(query.get(), 0));
                } else {
                    throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
                }
            } else {
                throw std::system_error(std::make_error_code(orm_error_code::column_not_found));
            }
            return res;
        }

        /**
             *  SUM(x) query.
             *  @param m is a class member pointer (the same you passed into make_column).
             *  @return std::shared_ptr with sum value or null if sqlite engine returned null.
             */
        template <class F, class O, class... Args, class Ret = typename column_result_t<self, F O::*>::type>
        std::shared_ptr<Ret> sum(F O::*m, Args&&... args)
        {
            this->assert_mapped_type<O>();

            auto connection = this->get_or_create_connection();
            auto& impl = this->get_impl<O>();
            std::shared_ptr<Ret> res;
            std::stringstream ss;
            ss << "SELECT " << static_cast<std::string>(sqlite_orm::sum(0)) << "(";
            auto columnName = this->string_from_expression(m);
            if (columnName.length()) {
                ss << columnName << ") FROM '" << impl.table.name << "' ";
                this->process_conditions(ss, std::forward<Args>(args)...);
                auto db = connection->get_db();
                auto query = db->make_query(ss.str());

                if (!db->prepare(query.get())) {
                    throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
                }
                auto rc = query->next(db);
                if (rc == query::step::done) {
                    res = std::make_shared<Ret>(row_extractor<Ret>().extract(query.get(), 0));
                } else {
                    throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
                }
            } else {
                throw std::system_error(std::make_error_code(orm_error_code::column_not_found));
            }
            return res;
        }

        /**
             *  TOTAL(x) query.
             *  @param m is a class member pointer (the same you passed into make_column).
             *  @return total value (the same as SUM but not nullable. More details here https://www.sqlite.org/lang_aggfunc.html)
             */
        template <class F, class O, class... Args>
        double total(F O::*m, Args&&... args)
        {
            this->assert_mapped_type<O>();

            auto connection = this->get_or_create_connection();
            double res;
            std::stringstream ss;
            ss << "SELECT " << static_cast<std::string>(sqlite_orm::total(0)) << "(";
            auto columnName = this->string_from_expression(m);
            if (!columnName.empty()) {
                ss << columnName << ") ";
                auto tableNamesSet = this->parse_table_names(m);
                if (!tableNamesSet.empty()) {
                    ss << "FROM ";
                    std::vector<std::pair<std::string, std::string>> tableNames(tableNamesSet.begin(), tableNamesSet.end());
                    for (size_t i = 0; i < tableNames.size(); ++i) {
                        ss << "'" << tableNames[i].first << "' ";
                        if (i < tableNames.size() - 1) {
                            ss << ",";
                        }
                        ss << " ";
                    }
                }
                this->process_conditions(ss, std::forward<Args>(args)...);
                auto db = connection->get_db();
                auto query = db->make_query(ss.str());

                if (!db->prepare(query.get())) {
                    throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
                }
                auto rc = query->next(db);
                if (rc == query::step::done) {
                    res = *std::make_shared<double>(row_extractor<double>().extract(query.get(), 0));
                } else {
                    throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
                }
            } else {
                throw std::system_error(std::make_error_code(orm_error_code::column_not_found));
            }
            return res;
        }

        /**
             *  Select a single column into std::vector<T> or multiple columns into std::vector<std::tuple<...>>.
             *  For a single column use `auto rows = storage.select(&User::id, where(...));
             *  For multicolumns user `auto rows = storage.select(columns(&User::id, &User::name), where(...));
             */
        template <
            class T,
            class... Args,
            class R = typename column_result_t<self, T>::type>
        std::vector<R> select(T m, Args... args)
        {
            static_assert(!is_base_of_template<T, compound_operator>::value || std::tuple_size<std::tuple<Args...>>::value == 0,
                "Cannot use args with a compound operator");
            using select_type = select_t<T, Args...>;
            auto sql_query = this->string_from_expression(select_type { std::move(m), std::make_tuple<Args...>(std::forward<Args>(args)...), true });
            auto connection = this->get_or_create_connection();
            auto db = connection->get_db();
            auto query = db->make_query(sql_query);
            if (db->prepare(query.get())) {
                std::vector<R> res;
                query::step stepRes;
                do {
                    stepRes = query->next(db);
                    if (stepRes == query::step::row) {
                        res.push_back(row_extractor<R>().extract(query.get(), 0));
                    } else if (stepRes == query::step::done) {
                        break;
                    } else {
                        throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
                    }
                } while (stepRes == query::step::row);
                return res;
            } else {
                throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
            }
        }

        /**
             *  Returns a string representation of object of a class mapped to the storage.
             *  Type of string has json-like style.
             */
        template <class O>
        std::string dump(const O& o)
        {
            this->assert_mapped_type<O>();
            return this->impl.dump(o);
        }

        /**
             *  This is REPLACE (INSERT OR REPLACE) function.
             *  Also if you need to insert value with knows id you should
             *  also you this function instead of insert cause inserts ignores
             *  id and creates own one.
             */
        template <class O>
        void replace(const O& o)
        {
            this->assert_mapped_type<O>();

            auto connection = this->get_or_create_connection();
            auto& impl = get_impl<O>();
            std::stringstream ss;
            ss << "REPLACE INTO '" << impl.table.name << "' (";
            auto columnNames = impl.table.column_names();
            auto columnNamesCount = columnNames.size();
            for (size_t i = 0; i < columnNamesCount; ++i) {
                ss << "\"" << columnNames[i] << "\"";
                if (i < columnNamesCount - 1) {
                    ss << ", ";
                } else {
                    ss << ") ";
                }
            }
            ss << "VALUES(";
            for (size_t i = 0; i < columnNamesCount; ++i) {
                ss << "?";
                if (i < columnNamesCount - 1) {
                    ss << ", ";
                } else {
                    ss << ")";
                }
            }
            auto db = connection->get_db();
            auto query = db->make_query(ss.str());
            if (db->prepare(query.get())) {
                auto index = 0;
                impl.table.for_each_column([&o, &index, &query, db](auto c) {
                    using field_type = typename decltype(c)::field_type;
                    const field_type* value = nullptr;
                    if (c.member_pointer) {
                        value = &(o.*c.member_pointer);
                    } else {
                        value = &((o).*(c.getter))();
                    }
                    statement_binder<field_type>().bind(db, query.get(), index++, *value);
                });
                if (query->next(db) == query::step::done) {
                    //..
                } else {
                    throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
                }
            } else {
                throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
            }
        }

        template <class It>
        void replace_range(It from, It to)
        {
            using O = typename std::iterator_traits<It>::value_type;
            this->assert_mapped_type<O>();
            if (from == to) {
                return;
            }

            auto connection = this->get_or_create_connection();
            auto& impl = get_impl<O>();
            std::stringstream ss;
            ss << "REPLACE INTO '" << impl.table.name << "' (";
            auto columnNames = impl.table.column_names();
            auto columnNamesCount = columnNames.size();
            for (size_t i = 0; i < columnNamesCount; ++i) {
                ss << "\"" << columnNames[i] << "\"";
                if (i < columnNamesCount - 1) {
                    ss << ", ";
                } else {
                    ss << ") ";
                }
            }
            ss << "VALUES ";
            auto valuesString = [columnNamesCount] {
                std::stringstream ss;
                ss << "(";
                for (size_t i = 0; i < columnNamesCount; ++i) {
                    ss << "?";
                    if (i < columnNamesCount - 1) {
                        ss << ", ";
                    } else {
                        ss << ")";
                    }
                }
                return ss.str();
            }();
            auto valuesCount = static_cast<int>(std::distance(from, to));
            for (auto i = 0; i < valuesCount; ++i) {
                ss << valuesString;
                if (i < valuesCount - 1) {
                    ss << ",";
                }
                ss << " ";
            }
            auto db = connection->get_db();
            auto query = db->make_query(ss.str());
            if (db->prepare(query.get())) {
                auto index = 0;
                for (auto it = from; it != to; ++it) {
                    auto& o = *it;
                    impl.table.for_each_column([&o, &index, db, query](auto c) {
                        using field_type = typename decltype(c)::field_type;
                        const field_type* value = nullptr;
                        if (c.member_pointer) {
                            value = &(o.*c.member_pointer);
                        } else {
                            value = &((o).*(c.getter))();
                        }
                        statement_binder<field_type>().bind(db, query.get(), index++, *value);
                    });
                }
                if (query->next(db) == query::step::done) {
                    //..
                } else {
                    throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
                }
            } else {
                throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
            }
        }

        template <class O, class... Cols>
        int insert(const O& o, columns_t<Cols...> cols)
        {
            constexpr const size_t colsCount = std::tuple_size<std::tuple<Cols...>>::value;
            static_assert(colsCount > 0, "Use insert or replace with 1 argument instead");
            this->assert_mapped_type<O>();
            auto connection = this->get_or_create_connection();
            auto& impl = get_impl<O>();
            std::stringstream ss;
            ss << "INSERT INTO '" << impl.table.name << "' ";
            std::vector<std::string> columnNames;
            columnNames.reserve(colsCount);
            cols.for_each([&columnNames, this](auto& m) {
                auto columnName = this->string_from_expression(m, true);
                if (columnName.length()) {
                    columnNames.push_back(columnName);
                } else {
                    throw std::system_error(std::make_error_code(orm_error_code::column_not_found));
                }
            });
            ss << "(";
            for (size_t i = 0; i < columnNames.size(); ++i) {
                ss << columnNames[i];
                if (i < columnNames.size() - 1) {
                    ss << ",";
                } else {
                    ss << ")";
                }
                ss << " ";
            }
            ss << "VALUES (";
            for (size_t i = 0; i < columnNames.size(); ++i) {
                ss << "?";
                if (i < columnNames.size() - 1) {
                    ss << ",";
                } else {
                    ss << ")";
                }
                ss << " ";
            }
            auto db = connection->get_db();
            auto query = db->make_query(ss.str());
            if (db->prepare(query.get())) {
                auto index = 0;
                cols.for_each([&o, &index, query, db, &impl](auto& m) {
                    using column_type = typename std::decay<decltype(m)>::type;
                    using field_type = typename column_result_t<self, column_type>::type;
                    const field_type* value = impl.table.template get_object_field_pointer<field_type>(o, m);
                    statement_binder<field_type>().bind(db, query.get(), index++, *value);
                });
                if (query->next(db) == query::step::done) {
                    return db->last_insert_rowid();
                } else {
                    throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
                }
            } else {
                throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
            }
        }

        /**
             *  Insert routine. Inserts object with all non primary key fields in passed object. Id of passed
             *  object doesn't matter.
             *  @return id of just created object.
             */
        template <class O>
        int insert(const O& o)
        {
            this->assert_mapped_type<O>();

            auto connection = this->get_or_create_connection();
            auto& impl = get_impl<O>();
            int res = 0;
            std::stringstream ss;
            ss << "INSERT INTO '" << impl.table.name << "' ";
            std::vector<std::string> columnNames;
            auto compositeKeyColumnNames = impl.table.composite_key_columns_names();

            impl.table.for_each_column([&impl, &columnNames, &compositeKeyColumnNames](auto c) {
                if (impl.table._without_rowid || !c.template has<constraints::primary_key_t<>>()) {
                    auto it = std::find(compositeKeyColumnNames.begin(),
                        compositeKeyColumnNames.end(),
                        c.name);
                    if (it == compositeKeyColumnNames.end()) {
                        columnNames.emplace_back(c.name);
                    }
                }
            });

            auto columnNamesCount = columnNames.size();
            if (columnNamesCount) {
                ss << "( ";
                for (size_t i = 0; i < columnNamesCount; ++i) {
                    ss << "\"" << columnNames[i] << "\"";
                    if (i < columnNamesCount - 1) {
                        ss << ", ";
                    } else {
                        ss << ") ";
                    }
                }
            } else {
                ss << "DEFAULT ";
            }
            ss << "VALUES ";
            if (columnNamesCount) {
                ss << "( ";
                for (size_t i = 0; i < columnNamesCount; ++i) {
                    ss << "?";
                    if (i < columnNamesCount - 1) {
                        ss << ", ";
                    } else {
                        ss << ")";
                    }
                }
            }
            auto db = connection->get_db();
            auto query = db->make_query(ss.str());

            if (db->prepare(query.get())) {
                auto index = 0;
                impl.table.for_each_column([&o, &index, query, db, &impl, &compositeKeyColumnNames](auto c) {
                    if (impl.table._without_rowid || !c.template has<constraints::primary_key_t<>>()) {
                        auto it = std::find(compositeKeyColumnNames.begin(),
                            compositeKeyColumnNames.end(),
                            c.name);
                        if (it == compositeKeyColumnNames.end()) {
                            using field_type = typename decltype(c)::field_type;
                            if (c.member_pointer) {
                                statement_binder<field_type>().bind(db, query.get(), index++, o.*c.member_pointer);
                            } else {
                                using getter_type = typename decltype(c)::getter_type;
                                field_value_holder<getter_type> valueHolder { ((o).*(c.getter))() };
                                statement_binder<field_type>().bind(db, query.get(), index++, valueHolder.value);
                            }
                        }
                    }
                });
                if (query->next(db) == query::step::done) {
                    res = db->last_insert_rowid();
                } else {
                    throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
                }
            } else {
                throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
            }
            return res;
        }

        template <class It>
        void insert_range(It from, It to)
        {
            using O = typename std::iterator_traits<It>::value_type;
            this->assert_mapped_type<O>();
            if (from == to) {
                return;
            }

            auto connection = this->get_or_create_connection();
            auto& impl = get_impl<O>();

            std::stringstream ss;
            ss << "INSERT INTO '" << impl.table.name << "' (";
            std::vector<std::string> columnNames;
            impl.table.for_each_column([&columnNames](auto c) {
                if (!c.template has<constraints::primary_key_t<>>()) {
                    columnNames.emplace_back(c.name);
                }
            });

            auto columnNamesCount = columnNames.size();
            for (size_t i = 0; i < columnNamesCount; ++i) {
                ss << "\"" << columnNames[i] << "\"";
                if (i < columnNamesCount - 1) {
                    ss << ", ";
                } else {
                    ss << ") ";
                }
            }
            ss << "VALUES ";
            auto valuesString = [columnNamesCount] {
                std::stringstream ss;
                ss << "(";
                for (size_t i = 0; i < columnNamesCount; ++i) {
                    ss << "?";
                    if (i < columnNamesCount - 1) {
                        ss << ", ";
                    } else {
                        ss << ")";
                    }
                }
                return ss.str();
            }();
            auto valuesCount = static_cast<int>(std::distance(from, to));
            for (auto i = 0; i < valuesCount; ++i) {
                ss << valuesString;
                if (i < valuesCount - 1) {
                    ss << ",";
                }
                ss << " ";
            }
            auto db = connection->get_db();
            auto query = db->make_query(ss.str());

            if (db->prepare(query.get())) {
                auto index = 0;
                for (auto it = from; it != to; ++it) {
                    auto& o = *it;
                    impl.table.for_each_column([&o, &index, query, db](auto c) {
                        if (!c.template has<constraints::primary_key_t<>>()) {
                            typedef typename decltype(c)::field_type field_type;
                            const field_type* value = nullptr;
                            if (c.member_pointer) {
                                value = &(o.*c.member_pointer);
                            } else {
                                value = &((o).*(c.getter))();
                            }
                            statement_binder<field_type>().bind(db, query.get(), index++, *value);
                        }
                    });
                }
                if (query->next(db) == query::step::done) {
                    //..
                } else {
                    throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
                }
            } else {
                throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
            }
        }

        void drop_index(const std::string& indexName)
        {
            auto connection = this->get_or_create_connection();
            std::stringstream ss;
            ss << "DROP INDEX '" << indexName + "'";
            auto db = connection->get_db();
            auto query = db->make_query(ss.str());

            if (db->prepare(query.get())) {
                if (query->next(db) == query::step::done) {
                    //  done..
                } else {
                    throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
                }
            } else {
                throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
            }
        }

        void vacuum()
        {
            auto connection = this->get_or_create_connection();
            std::string sqlquery = "VACUUM";
            auto db = connection->get_db();
            auto query = db->make_query(sqlquery);
            if (db->prepare(query.get())) {
                if (query->next(db) == query::step::done) {
                    //  done..
                } else {
                    throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
                }
            } else {
                throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
            }
        }

    protected:
        void drop_table_internal(const std::string& tableName, database* db)
        {
            std::stringstream ss;
            ss << "DROP TABLE '" << tableName + "'";
            auto query = db->make_query(ss.str());
            if (db->prepare(query.get())) {
                if (query->next(db) == query::step::done) {
                    //  done..
                } else {
                    throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
                }
            } else {
                throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
            }
        }

    public:
        /**
             *  Drops table with given name.
             */
        void drop_table(const std::string& tableName)
        {
            auto connection = this->get_or_create_connection();
            this->drop_table_internal(tableName, connection->get_db());
        }

        /**
             *  sqlite3_changes function.
             */
        int changes()
        {
            auto connection = this->get_or_create_connection();
            auto db = connection->get_db();
            return db->last_changed_rows();
        }

        /**
             *  sqlite3_total_changes function.
             */
        int total_changes()
        {
            auto connection = this->get_or_create_connection();
            auto db = connection->get_db();
            return db->total_changed_rows();
        }

        int64 last_insert_rowid()
        {
            auto connection = this->get_or_create_connection();
            auto db = connection->get_db();
            return db->last_insert_rowid();
        }

        int busy_timeout(int ms)
        {
            auto connection = this->get_or_create_connection();
            auto db = connection->get_db();
            return db->busy_timeout(ms);
        }

        /**
             *  Returns libsqltie3 lib version, not sqlite_orm
             */
        std::string libversion()
        {
            auto connection = this->get_or_create_connection();
            auto db = connection->get_db();
            return db->version();
        }

    protected:
        template <class... Tss, class... Cols>
        sync_schema_result sync_table(storage_impl<internal::index_t<Cols...>, Tss...>* impl, database* db, bool)
        {
            auto res = sync_schema_result::already_in_sync;
            std::stringstream ss;
            ss << "CREATE ";
            if (impl->table.unique) {
                ss << "UNIQUE ";
            }
            using columns_type = typename decltype(impl->table)::columns_type;
            using head_t = typename std::tuple_element<0, columns_type>::type;
            using indexed_type = typename internal::table_type<head_t>::type;
            ss << "INDEX IF NOT EXISTS '" << impl->table.name << "' ON '" << this->impl.template find_table_name<indexed_type>() << "' ( ";
            std::vector<std::string> columnNames;
            tuple_helper::iterator<std::tuple_size<columns_type>::value - 1, Cols...>()(impl->table.columns, [&columnNames, this](auto& v) {
                columnNames.push_back(this->impl.column_name(v));
            });
            for (size_t i = 0; i < columnNames.size(); ++i) {
                ss << "'" << columnNames[i] << "'";
                if (i < columnNames.size() - 1) {
                    ss << ",";
                }
                ss << " ";
            }
            ss << ") ";
            auto query = db->make_query(ss.str());

            auto rc = db->exec(query.get());
            if (rc != query::step::done) {
                throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
            }
            return res;
        }

        template <class... Tss, class... Cs>
        sync_schema_result sync_table(storage_impl<table_t<Cs...>, Tss...>* impl, database* db, bool preserve)
        {
            auto res = sync_schema_result::already_in_sync;

            auto schema_stat = impl->schema_status(db, preserve);
            if (schema_stat != decltype(schema_stat)::already_in_sync) {
                if (schema_stat == decltype(schema_stat)::new_table_created) {
                    this->create_table(db, impl->table.name, impl);
                    res = decltype(res)::new_table_created;
                } else {
                    if (schema_stat == sync_schema_result::old_columns_removed
                        || schema_stat == sync_schema_result::new_columns_added
                        || schema_stat == sync_schema_result::new_columns_added_and_old_columns_removed) {

                        //  get table info provided in `make_table` call..
                        auto storageTableInfo = impl->table.get_table_info();

                        //  now get current table info from db using `PRAGMA table_info` query..
                        auto dbTableInfo = impl->get_table_info(impl->table.name, db);

                        //  this vector will contain pointers to columns that gotta be added..
                        std::vector<table_info*> columnsToAdd;

                        impl->get_remove_add_columns(columnsToAdd, storageTableInfo, dbTableInfo);

                        if (schema_stat == sync_schema_result::old_columns_removed) {

                            //  extra table columns than storage columns
                            this->backup_table(db, impl);
                            res = decltype(res)::old_columns_removed;
                        }

                        if (schema_stat == sync_schema_result::new_columns_added) {
                            for (auto columnPointer : columnsToAdd) {
                                impl->add_column(*columnPointer, db);
                            }
                            res = decltype(res)::new_columns_added;
                        }

                        if (schema_stat == sync_schema_result::new_columns_added_and_old_columns_removed) {

                            //remove extra columns
                            this->backup_table(db, impl);
                            for (auto columnPointer : columnsToAdd) {
                                impl->add_column(*columnPointer, db);
                            }
                            res = decltype(res)::new_columns_added_and_old_columns_removed;
                        }
                    } else if (schema_stat == sync_schema_result::dropped_and_recreated) {
                        this->drop_table_internal(impl->table.name, db);
                        this->create_table(db, impl->table.name, impl);
                        res = decltype(res)::dropped_and_recreated;
                    }
                }
            }
            return res;
        }

    public:
        /**
             *  This is a cute function used to replace migration up/down functionality.
             *  It performs check storage schema with actual db schema and:
             *  * if there are excess tables exist in db they are ignored (not dropped)
             *  * every table from storage is compared with it's db analog and
             *      * if table doesn't exist it is being created
             *      * if table exists its colums are being compared with table_info from db and
             *          * if there are columns in db that do not exist in storage (excess) table will be dropped and recreated
             *          * if there are columns in storage that do not exist in db they will be added using `ALTER TABLE ... ADD COLUMN ...' command
             *          * if there is any column existing in both db and storage but differs by any of properties/constraints (type, pk, notnull, dflt_value) table will be dropped and recreated
             *  Be aware that `sync_schema` doesn't guarantee that data will not be dropped. It guarantees only that it will make db schema the same
             *  as you specified in `make_storage` function call. A good point is that if you have no db file at all it will be created and
             *  all tables also will be created with exact tables and columns you specified in `make_storage`, `make_table` and `make_column` call.
             *  The best practice is to call this function right after storage creation.
             *  @param preserve affects on function behaviour in case it is needed to remove a column. If it is `false` so table will be dropped
             *  if there is column to remove, if `true` -  table is being copied into another table, dropped and copied table is renamed with source table name.
             *  Warning: sync_schema doesn't check foreign keys cause it is unable to do so in sqlite3. If you know how to get foreign key info
             *  please submit an issue https://github.com/fnc12/sqlite_orm/issues
             *  @return std::map with std::string key equal table name and `sync_schema_result` as value. `sync_schema_result` is a enum value that stores
             *  table state after syncing a schema. `sync_schema_result` can be printed out on std::ostream with `operator<<`.
             */
        std::map<std::string, sync_schema_result> sync_schema(bool preserve = false)
        {
            auto connection = this->get_or_create_connection();
            std::map<std::string, sync_schema_result> result;
            auto db = connection->get_db();
            this->impl.for_each([&result, db, preserve, this](auto impl) {
                auto res = this->sync_table(impl, db, preserve);
                result.insert({ impl->table.name, res });
            });
            return result;
        }

        /**
             *  This function returns the same map that `sync_schema` returns but it
             *  doesn't perform `sync_schema` actually - just simulates it in case you want to know
             *  what will happen if you sync your schema.
             */
        std::map<std::string, sync_schema_result> sync_schema_simulate(bool preserve = false)
        {
            auto connection = this->get_or_create_connection();
            std::map<std::string, sync_schema_result> result;
            auto db = connection->get_db();
            this->impl.for_each([&result, db, preserve](auto impl) {
                result.insert({ impl->table.name, impl->schema_status(db, preserve) });
            });
            return result;
        }

        bool transaction(std::function<bool()> f)
        {
            this->begin_transaction();
            auto db = this->currentTransaction->get_db();
            auto shouldCommit = f();
            if (shouldCommit) {
                this->impl.commit(db);
            } else {
                this->impl.rollback(db);
            }
            if (!this->inMemory && !this->isOpenedForever) {
                this->currentTransaction = nullptr;
            }
            return shouldCommit;
        }

    public:
        void begin_transaction()
        {
            if (!this->inMemory) {
                if (!this->isOpenedForever) {
                    if (this->currentTransaction)
                        throw std::system_error(std::make_error_code(orm_error_code::cannot_start_a_transaction_within_a_transaction));
                    this->currentTransaction = std::make_shared<internal::database_connection>(this->filename);
                    this->on_open_internal(this->currentTransaction->get_db());
                }
            }
            auto db = this->currentTransaction->get_db();
            this->impl.begin_transaction(db);
        }

        void commit()
        {
            if (!this->inMemory) {
                if (!this->currentTransaction)
                    throw std::system_error(std::make_error_code(orm_error_code::no_active_transaction));
            }
            auto db = this->currentTransaction->get_db();
            this->impl.commit(db);
            if (!this->inMemory && !this->isOpenedForever) {
                this->currentTransaction = nullptr;
            }
        }

        void rollback()
        {
            if (!this->inMemory) {
                if (!this->currentTransaction)
                    throw std::system_error(std::make_error_code(orm_error_code::no_active_transaction));
            }
            auto db = this->currentTransaction->get_db();
            this->impl.rollback(db);
            if (!this->inMemory && !this->isOpenedForever) {
                this->currentTransaction = nullptr;
            }
        }

        std::string current_timestamp()
        {
            auto connection = this->get_or_create_connection();
            return this->impl.current_timestamp(connection->get_db());
        }

    protected:
        void foreign_keys(database* db, bool value)
        {
            std::stringstream ss;
            ss << "PRAGMA foreign_keys = " << value;
            auto query = db->make_query(ss.str());
            auto rc = db->exec(query.get());
            if (rc != query::step::done) {
                throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
            }
        }

        bool foreign_keys(database* db)
        {
            std::string sqlquery = "PRAGMA foreign_keys";
            auto res = false;
            auto query = db->make_query(sqlquery);
            if (!db->prepare(query.get())) {
                throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
            }
            auto rc = query->next(db);
            if (rc == query::step::done) {
                res = row_extractor<int>().extract(query.get(), 0);
            } else {
                throw std::system_error(std::error_code(db->last_error_code(), db->error_category()));
            }
            return res;
        }

    public:
        /**
             * \fn db_release_memory
             * \brief Releases freeable memory of database. It is function can/should be called periodically by application,
             * if application has less memory usage constraint.
             * \note sqlite3_db_release_memory added in 3.7.10 https://sqlite.org/changes.html
             */
        int db_release_memory()
        {
            //TODO: add release mem
            return 0;
            //            auto connection = this->get_or_create_connection();
            //            return sqlite3_db_release_memory(connection->get_db());
        }

        /**
             *  Checks whether table exists in db. Doesn't check storage itself - works only with actual database.
             *  Note: table can be not mapped to a storage
             *  @return true if table with a given name exists in db, false otherwise.
             */
        bool table_exists(const std::string& tableName)
        {
            auto connection = this->get_or_create_connection();
            return this->impl.table_exists(tableName, connection->get_db());
        }

        /**
             *  Returns existing permanent table names in database. Doesn't check storage itself - works only with actual database.
             *  @return Returns list of tables in database.
             */
        std::vector<std::string> table_names()
        {
            auto connection = this->get_or_create_connection();
            return this->impl.table_names();
        }

        bool threadsafe()
        {
            auto connection = this->get_or_create_connection();
            return this->impl.threadsafe(connection->get_db());
        }

        void open_forever()
        {
            this->isOpenedForever = true;
            if (!this->currentTransaction) {
                this->currentTransaction = std::make_shared<internal::database_connection>(this->filename);
                this->on_open_internal(this->currentTransaction->get_db());
            }
        }

        using pragma_type = pragma_t<self>;

        friend pragma_type;

    public:
        pragma_type pragma;
        limit_accesor<self> limit;
    };

    template <class T>
    struct is_storage : std::false_type {
    };

    template <class... Ts>
    struct is_storage<storage_t<Ts...>> : std::true_type {
    };
}

template <class... Ts>
internal::storage_t<Ts...> make_storage(const std::string& filename, Ts... tables)
{
    return { filename, internal::storage_impl<Ts...>(tables...) };
}
}
#pragma once

#if defined(_MSC_VER)
#if defined(__RESTORE_MIN__)
__pragma(pop_macro("min"))
#undef __RESTORE_MIN__
#endif
#if defined(__RESTORE_MAX__)
    __pragma(pop_macro("max"))
#undef __RESTORE_MAX__
#endif
#endif // defined(_MSC_VER)
#pragma once

// #include "database.h"

#include <iostream>
#include <memory>
#include <sqlite3.h>

        namespace sqlite_orm
{

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

    static void throw_if_not_sqlite_ok(database * db, int result)
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
