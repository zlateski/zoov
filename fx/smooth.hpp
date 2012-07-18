#ifndef ZOOV_FX_SMOOTH_HPP
#define ZOOV_FX_SMOOTH_HPP 1

#include "fx.hpp"

namespace zoov {

class EFFECT(smooth, STATIC(), image_ptr, int, int)
{
public:
    void init()
    {
    }

    image_ptr process(image_ptr im, int a, int b)
    {
        image_ptr r = image_pool::get(im);
        cvSmooth( im.get(), r.get(), CV_BLUR, a, b, 0, 0 );
        return r;
    }

}; // class smooth

} // namespace zoov

#endif
