#ifndef ZOOV_FX_ADD_HPP
#define ZOOV_FX_ADD_HPP 1

#include "fx.hpp"

#include <zi/concurrency.hpp>

namespace zoov {

class EFFECT(add, STATIC(), image_ptr, image_ptr, double, double, double)
{
public:
    void init()
    { }

    image_ptr process(image_ptr a, image_ptr b, double alpha, double beta, double gamma)
    {
        image_ptr r = image_pool::get(a);
        cvAddWeighted(a.get(), alpha, b.get(), beta, gamma, r.get());
        return r;
    }

};

} // namespace zoov

#endif // ZOOV_FX_ADD_HPP
