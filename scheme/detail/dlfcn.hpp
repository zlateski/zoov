#ifndef ZOOV_SCHEME_DETAIL_DLFCN_HPP
#define ZOOV_SCHEME_DETAIL_DLFCN_HPP

#include <zi/utility/for_each.hpp>
#include <zi/utility/singleton.hpp>
#include <zi/concurrency.hpp>
#include "../types_fwd.hpp"

#include <map>
#include <string>
#include <cstdlib>
#include <dlfcn.h>

namespace zoov {
namespace dlfcn {



class shareds
{
private:
    std::map<std::string, void*>  map_;
    zi::mutex                     m_  ;

    typedef void (*proc_t)(void);

    void* do_open(const char* f)
    {
        void* handle = dlopen(f, RTLD_LAZY);

        if ( !handle )
        {
            std::cout << "Error opening " << f << ": " << dlerror() << "\n";
            exit(1);
        }

        dlerror();
        proc_t initp = reinterpret_cast<proc_t>(dlsym(handle, "initialize"));

        char* error;
        if ( error = dlerror() )
        {
            std::cout << "Error: " << error << "\n";
            exit(1);
        }

        (*initp)();

        return handle;
    }

    void do_close(void* handle)
    {
        dlerror();

        proc_t uninitp = reinterpret_cast<proc_t>(dlsym(handle, "initialize"));

        char* error;
        if ( error = dlerror() )
        {
            std::cout << "Error: " << error << "\n";
            exit(1);
        }

        (*uninitp)();

        dlclose(handle);
    }

public:
    shareds()
        : map_()
        , m_()
    { }

    void open(const std::string& what)
    {
        zi::mutex::guard g(m_);
        if ( map_.count(what) )
        {
            do_close(map_[what]);
        }
        map_[what] = do_open(what.c_str());
    }

    void close(const std::string& what)
    {
        zi::mutex::guard g(m_);
        if ( map_.count(what) )
        {
            do_close(map_[what]);
            map_.erase(what);
        }
    }

}; // class loader

} // namespace dlfcn

namespace {
dlfcn::shareds& shareds = zi::singleton<dlfcn::shareds>::instance();
}

} // namespace zoov

#endif // ZOOV_SCHEME_DETAIL_DLFCN_HPP
