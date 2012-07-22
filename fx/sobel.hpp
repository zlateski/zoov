#ifndef ZOOV_FX_SOBEL_HPP
#define ZOOV_FX_SOBEL_HPP 1

#include "fx.hpp"

namespace zoov {

class EFFECT(sobel, STATIC(), image_ptr, int, int, int)
{
public:
    void init()
    {
    }

    image_ptr process(image_ptr im, int a, int b, int c)
    {
        image_ptr r = image_pool::get(im);
        cvSobel( im.get(), r.get(), a, b, c );
        return r;
    }

}; // class sobel

} // namespace zoov

#endif
