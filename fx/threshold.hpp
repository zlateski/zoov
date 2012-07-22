#ifndef ZOOV_FX_THRESHOLD_HPP
#define ZOOV_FX_THRESHOLD_HPP 1

#include "fx.hpp"

namespace zoov {

class EFFECT(threshold, STATIC(), image_ptr, int, int)
{
public:
    void init()
    {
    }

    image_ptr process(image_ptr im, int a, int b)
    {
        image_ptr r = image_pool::get(im);
        cvThreshold( im.get(), r.get(), a, b, CV_THRESH_BINARY);
        return r;
    }

}; // class threshold

} // namespace zoov

#endif
