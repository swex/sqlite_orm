#pragma once

#include <locale> // std::wstring_convert
#include <string> //  std::string, std::wstring
#include <type_traits> //  std::enable_if_t, std::is_arithmetic, std::is_same
#ifndef SQLITE_ORM_OMITS_CODECVT
#include <codecvt> //  std::wstring_convert, std::codecvt_utf8_utf16
#endif //  SQLITE_ORM_OMITS_CODECVT
#include <cstddef> //  std::nullptr_t
#include <vector> //  std::vector

#include "database.h"
#include "is_std_ptr.h"

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
