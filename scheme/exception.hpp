#ifndef ZOOV_SCHEME_EXCEPTION_HPP
#define ZOOV_SCHEME_EXCEPTION_HPP 1

#include <exception>
#include <string>

namespace zoov {

class scheme_exception: public std::exception
{
protected:
    const std::string message_;

public:
    scheme_exception( const std::string& m )
        : message_(m)
    { }

    virtual ~scheme_exception() throw()
    { }

    virtual const char* what() const throw()
    {
        return message_.c_str();
    }

};

void assure(bool what, const std::string& m)
{
    if (!what)
    {
        throw scheme_exception(m);
    }
}

} // namespace zoov

#endif // ZOOV_SCHEME_EXCEPTION_HPP
