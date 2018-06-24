#pragma once

#include <string>   //  std::string
#include <tuple>    //  std::tuple
#include <utility>  //  std::forward

namespace sqlite_orm {
    
    namespace internal {
        
        template<class T>
        struct distinct_t {
            T t;
            
            operator std::string() const {
                return "DISTINCT";
            }
        };
        
        template<class T>
        struct all_t {
            T t;
            
            operator std::string() const {
                return "ALL";
            }
        };
        
        template<class ...Args>
        struct columns_t {
            bool distinct = false;
            
            template<class L>
            void for_each(L) {
                //..
            }
            
            int count() {
                return 0;
            }
        };
        
        template<class T, class ...Args>
        struct columns_t<T, Args...> : public columns_t<Args...> {
            T m;
            
            columns_t(decltype(m) m_, Args&& ...args): super(std::forward<Args>(args)...), m(m_) {}
            
            template<class L>
            void for_each(L l) {
                l(this->m);
                this->super::for_each(l);
            }
            
            int count() {
                return 1 + this->super::count();
            }
        private:
            using super = columns_t<Args...>;
        };
        
        template<class ...Args>
        struct set_t {
            
            operator std::string() const {
                return "SET";
            }
            
            template<class F>
            void for_each(F) {
                //..
            }
        };
        
        template<class L, class ...Args>
        struct set_t<L, Args...> : public set_t<Args...> {
            static_assert(is_assign_t<typename std::remove_reference<L>::type>::value, "set_t argument must be assign_t");
            
            L l;
            
            using super = set_t<Args...>;
            using self = set_t<L, Args...>;
            
            set_t(L l_, Args&& ...args) : super(std::forward<Args>(args)...), l(std::forward<L>(l_)) {}
            
            template<class F>
            void for_each(F f) {
                f(l);
                this->super::for_each(f);
            }
        };
        
        /**
         *  This class is used to store explicit mapped type T and its column descriptor (member pointer/getter/setter).
         *  Is useful when mapped type is derived from other type and base class has members mapped to a storage.
         */
        template<class T, class F>
        struct column_pointer {
            using type = T;
            using field_type = F;
            
            field_type field;
        };
        
        template<class T, class ...Args>
        struct single_column_subselect_t {
            T column;
            std::tuple<Args...> conditions;
        };
        
        template<class ...Cols, class ...Args>
        struct multi_column_subselect_t {
            columns_t<Cols..>> cols;
            std::tuple<Args...> conditions;
        };
        
        /**
         *  `UNION` and `UNION ALL` query holder.
         */
        template<class L, class R>
        struct union_t {
            bool all = false;
            L l;
            R r;
        };
    }
    
    template<class T>
    internal::distinct_t<T> distinct(T t) {
        return {t};
    }
    
    template<class T>
    internal::all_t<T> all(T t) {
        return {t};
    }
    
    template<class ...Args>
    internal::columns_t<Args...> distinct(internal::columns_t<Args...> cols) {
        cols.distinct = true;
        return cols;
    }
    
    template<class ...Args>
    internal::set_t<Args...> set(Args&& ...args) {
        return {std::forward<Args>(args)...};
    }
    
    template<class ...Args>
    internal::columns_t<Args...> columns(Args&& ...args) {
        return {std::forward<Args>(args)...};
    }
    
    /**
     *  Use it like this:
     *  struct MyType : BaseType { ... };
     *  storage.select(column<MyType>(&BaseType::id));
     */
    template<class T, class F>
    internal::column_pointer<T, F> column(F f) {
        return {f};
    }
    
    template<class T, class ...Args>
    internal::single_column_subselect_t<T, Args...> select(T t, Args ...args) {
        return {std::move(t), std::forward<Args>(args)...};
    }
    
    template<class ...Cols, class ...Args>
    internal::multi_column_subselect_t<Cols...
    
    template<class L, class R>
    internal::union_t union_(L l, R r) {
        return {false, std::move(l), std::move(r)};
    }
    
    template<class L, class R>
    internal::union_t union_all(L l, R r) {
        return {true, std::move(l), std::move(r)};
    }
}
