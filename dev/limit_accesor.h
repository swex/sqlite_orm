#pragma once

#include "database.h"
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
            auto connection = get_connection();
            return connection->get_db()->limit_length();
        }

        void length(int newValue)
        {
            auto connection = get_connection();
            connection->get_db()->limit_set_length(newValue);
            limits[database::limit_type::length] = newValue;
        }

        int sql_length()
        {
            auto connection = get_connection();
            return connection->get_db()->limit_sql_length();
        }

        void sql_length(int newValue)
        {
            auto connection = get_connection();
            connection->get_db()->limit_set_sql_length(newValue);
            limits[database::limit_type::sql_length] = newValue;
        }

        int column()
        {
            auto connection = get_connection();
            return connection->get_db()->limit_columns();
        }

        void column(int newValue)
        {
            auto connection = get_connection();
            connection->get_db()->limit_set_columns(newValue);
            limits[database::limit_type::columns] = newValue;
        }

        int expr_depth()
        {
            auto connection = get_connection();
            return connection->get_db()->limit_expr_depth();
        }

        void expr_depth(int newValue)
        {
            auto connection = get_connection();
            connection->get_db()->limit_set_expr_depth(newValue);
            limits[database::limit_type::expr_depth] = newValue;
        }

        int compound_select()
        {
            auto connection = get_connection();
            return connection->get_db()->limit_compound_select();
        }

        void compound_select(int newValue)
        {
            auto connection = get_connection();
            connection->get_db()->limit_set_compound_select(newValue);
            limits[database::limit_type::compound_select] = newValue;
        }

        int vdbe_op()
        {
            auto connection = get_connection();
            return connection->get_db()->limit_vdbe_op();
        }

        void vdbe_op(int newValue)
        {
            auto connection = get_connection();
            connection->get_db()->limit_set_vdbe_op(newValue);
            limits[database::limit_type::vdbe_op] = newValue;
        }

        int function_arg()
        {
            auto connection = get_connection();
            return connection->get_db()->limit_function_arg();
        }

        void function_arg(int newValue)
        {
            auto connection = get_connection();
            connection->get_db()->limit_set_function_arg(newValue);
            limits[database::limit_type::function_arg] = newValue;
        }

        int attached()
        {
            auto connection = get_connection();
            return connection->get_db()->limit_attached();
        }

        void attached(int newValue)
        {
            auto connection = get_connection();
            connection->get_db()->limit_set_attached(newValue);
            limits[database::limit_type::attached] = newValue;
        }

        int like_pattern_length()
        {
            auto connection = get_connection();
            return connection->get_db()->limit_like_pattern_length();
        }

        void like_pattern_length(int newValue)
        {
            auto connection = get_connection();
            connection->get_db()->limit_set_like_pattern_length(newValue);
            limits[database::limit_type::like_pattern_length] = newValue;
        }

        int variable_number()
        {
            auto connection = get_connection();
            return connection->get_db()->limit_variable_number();
        }

        void variable_number(int newValue)
        {
            auto connection = get_connection();
            connection->get_db()->limit_set_variable_number(newValue);
            limits[database::limit_type::variable_number] = newValue;
        }

        int trigger_depth()
        {
            auto connection = get_connection();
            return connection->get_db()->limit_trigger_depth();
        }

        void trigger_depth(int newValue)
        {
            auto connection = get_connection();
            connection->get_db()->limit_set_trigger_depth(newValue);
            limits[database::limit_type::trigger_depth] = newValue;
        }

        int worker_threads()
        {
            auto connection = get_connection();
            return connection->get_db()->limit_worker_threads();
        }

        void worker_threads(int newValue)
        {
            auto connection = get_connection();
            connection->get_db()->limit_set_worker_threads(newValue);
            limits[database::limit_type::worker_threads] = newValue;
        }

    protected:
        storage_type& storage;

        template <class... Ts>
        friend struct storage_t;

        /**
             *  Stores limit set between connections.
             */
        std::map<database::limit_type, int> limits;

        std::shared_ptr<internal::database_connection> get_connection()
        {
            auto connection = this->storage.get_or_create_connection();
            return connection;
        }
    };
}
}
