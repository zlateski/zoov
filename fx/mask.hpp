#ifndef ZOOV_FX_MASK_HPP
#define ZOOV_FX_MASK_HPP 1

#include "fx.hpp"

namespace zoov {

class EFFECT(mask, STATIC(), image_ptr, image_ptr)
{
public:
    void init()
    {
    }

    image_ptr process(image_ptr im, image_ptr m)
    {
        image_ptr r = image_pool::get(im);
        cvZero( r.get() );
        cvCopy( im.get(), r.get(), m.get());
        return r;
    }

}; // class mask

} // namespace zoov

#endif // ZOOV_FX_MASK_HPP
