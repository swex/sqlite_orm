#pragma once

#include <string> //  std::string

#include "database.h"
#include "error_code.h"
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
