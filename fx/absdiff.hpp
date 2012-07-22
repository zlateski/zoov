#ifndef ZOOV_FX_ABSDIFF_HPP
#define ZOOV_FX_ABSDIFF_HPP 1

#include "fx.hpp"

namespace zoov {

class EFFECT(absdiff, STATIC(), image_ptr, image_ptr )
{
public:

    void init()
    { }

    image_ptr process(image_ptr im1, image_ptr im2 )
    {
        image_ptr r = image_pool::get(im1);

        cvAbsDiff( im1.get(), im2.get(), r.get() );

        return r;
    }

}; // class canny

} // namespace zoov

#endif // ZOOV_FX_ABSDIFF_HPP
