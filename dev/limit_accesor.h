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
