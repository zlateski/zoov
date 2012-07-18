#ifndef ZOOV_FX_GRAYSCALE_HPP
#define ZOOV_FX_GRAYSCALE_HPP 1

#include "fx.hpp"

namespace zoov {

class EFFECT(grayscale, STATIC(), image_ptr)
{
public:
    void init() {}

    image_ptr process(image_ptr im)
    {
        image_ptr r = image_pool::get(im,1);

        switch ( im->nChannels )
        {
        case 4:
            cvCvtColor( im.get(), r.get(), CV_RGBA2GRAY );
            break;
        case 3:
            cvCvtColor( im.get(), r.get(), CV_RGB2GRAY );
            break;
        case 1:
            std::memcpy( r->imageData, im->imageData, im->imageSize);
            break;
        }
        return r;
    }

}; // class grayscale

} // namespace zoov

#endif // ZOOV_FX_GRAYSCALE_HPP
