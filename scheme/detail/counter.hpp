#ifndef ZOOV_LISP_DETAIL_COUNTER_HPP
#define ZOOV_LISP_DETAIL_COUNTER_HPP

#include <atomic>
#include <zi/utility/singleton.hpp>

#define COUNTER(name)                                                   \
    struct ___struct_for_##name##_counter___: ::std::atomic<int> {};    \
    namespace {                                                         \
        ___struct_for_##name##_counter___& name =                       \
            ::zi::singleton< ___struct_for_##name##_counter___>::instance(); \
    }                                                                   \

#endif // ZOOV_LISP_DETAIL_COUNTER_HPP
