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
