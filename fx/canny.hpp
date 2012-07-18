#ifndef ZOOV_FX_CANNY_HPP
#define ZOOV_FX_CANNY_HPP 1

#include "fx.hpp"

namespace zoov {

class EFFECT(canny, STATIC(), image_ptr, double, double, int)
{
public:
    void init()
    { }

    image_ptr process(image_ptr im, double a1, double a2, int a3 )
    {
        image_ptr r = image_pool::get(im);
        cvCanny( im.get(), r.get(), a1, a2, a3 );
        return r;
    }

}; // class canny

} // namespace zoov

#endif // ZOOV_FX_CANNY_HPP
