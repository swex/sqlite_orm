
#ifndef not_found_exception_h
#define not_found_exception_h

#include <stdexcept>

namespace sqlite_orm {
    
    /**
     *  Exeption thrown if nothing was found in database with specified id.
     */
    struct not_found_exception : public std::exception {
        
        const char* what() const throw() override {
            return "Not found";
        };
    };
}

#endif /* not_found_exception_h */
