#ifndef ZOOV_SCHEME_DETAIL_DEBUG_HPP
#define ZOOV_SCHEME_DETAIL_DEBUG_HPP

#ifdef NDEBUG
#  define dout if (0) ::std::cout
#else
#  define dout ::std::cout
#endif

#endif // ZOOV_SCHEME_DETAIL_DEBUG_HPP
