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

#include "arithmetic_tag.h"
#include "error_code.h"
#include "journal_mode.h"
#include "query.h"

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
