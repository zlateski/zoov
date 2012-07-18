#ifndef ZOOV_FX_TRAIL_HPP
#define ZOOV_FX_TRAIL_HPP 1

#include "fx.hpp"

namespace zoov {

class EFFECT(trail, STATIC(), image_ptr, double )
{
public:

    void init()
    { }

    image_ptr process(image_ptr im, double a1 )
    {
        image_ptr r = image_pool::get(im);

        cvAddWeighted( r.get(), a1, im.get(), static_cast<double>(1)-a1,
                       static_cast<double>(0), r.get() );

        return r;
    }

}; // class canny

} // namespace zoov

#endif // ZOOV_FX_TRAIL_HPP
