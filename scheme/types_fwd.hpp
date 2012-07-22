#ifndef ZOOV_SCHEME_TYPES_FWD_HPP
#define ZOOV_SCHEME_TYPES_FWD_HPP 1

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/function.hpp>

#include <list>
#include <string>

namespace zoov {

class cell_t;
class env_t ;
class scheme;

typedef boost::shared_ptr<cell_t> cell_ptr;
typedef boost::shared_ptr<env_t>  env_ptr;

struct undefined_tag {};
struct nil_tag       {};
struct true_tag      {};
struct false_tag     {};

typedef boost::function<cell_ptr (cell_ptr, env_ptr)> builtin_t;

namespace {

undefined_tag undefined_v;
true_tag      true_v     ;
false_tag     false_v    ;
nil_tag       nil_v      ;

}

void ___use__them__all__()
{
    static_cast<void>(undefined_v);
    static_cast<void>(true_v);
    static_cast<void>(false_v);
    static_cast<void>(nil_v);
}

inline std::list<std::string> tokenize(const std::string&);
inline cell_ptr parse(std::list<std::string>&);

inline env_ptr get_global_env();

} // namespace zoov

#endif // ZOOV_SCHEME_TYPES_FWD_HPP
