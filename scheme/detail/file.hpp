#ifndef ZOOV_SCHEME_DETAIL_FILE_HPP
#define ZOOV_SCHEME_DETAIL_FILE_HPP

#include <string>
#include <fstream>
#include <sstream>

namespace zoov {
namespace detail {

inline bool file_exists(const std::string& f)
{
    std::ifstream t(f.c_str());
    return t;
}

inline std::string file_get_contents(const std::string& f)
{
    std::ifstream t(f.c_str());
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}

} // namespace detail
} // namespace zoov


#endif // ZOOV_SCHEME_DETAIL_FILE_HPP
