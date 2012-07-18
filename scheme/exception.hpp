#ifndef ZOOV_SCHEME_EXCEPTION_HPP
#define ZOOV_SCHEME_EXCEPTION_HPP 1

#include <exception>
#include <string>
#include <zi/zpp/stringify.hpp>

namespace zoov {

class scheme_exception: public std::exception
{
protected:
    const std::string message_;

public:
    scheme_exception( const std::string& m )
        : message_(m)
    { }

    scheme_exception( const std::string& m, const char* fname, const char* line )
        : message_(m + "\n    in " + fname + ": " + line)
    { }

    virtual ~scheme_exception() throw()
    { }

    virtual const char* what() const throw()
    {
        return message_.c_str();
    }

};

void assure(bool what, const std::string& m = "")
{
    if (!what)
    {
        throw scheme_exception(m);
    }
}

#define ASSURE(what)                                                    \
    if ( !(what) )                                                      \
    {                                                                   \
        std::istringstream ss;                                          \
        ss << ZI_ZPP_STRINGIFY_HPP(what);                               \
        ss << "\n    in " << __FILE__ << ": " << __LINE__;              \
        throw ::zoov::scheme_exception(m);                              \
    }                                                                   \
    static_cast<void>(0)


} // namespace zoov

#endif // ZOOV_SCHEME_EXCEPTION_HPP
