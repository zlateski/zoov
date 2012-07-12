#ifndef ZOOV_FX_MASK_HPP
#define ZOOV_FX_MASK_HPP 1

#include "fx.hpp"

namespace zoov {

class EFFECT(mask, STATIC(), image_ptr, image_ptr)
{
private:
    image_ptr image_;

public:
    mask()
        : image_(cvCreateImage( cvSize(800, 600), IPL_DEPTH_8U, 4 ))
    { }

    ~mask()
    {
        cvReleaseImage(&image_);
    }

    void init()
    {
    }

    image_ptr process(image_ptr im, image_ptr m)
    {
        if ( im->nChannels != image_->nChannels )
        {
            cvReleaseImage(&image_);
            image_ = cvCreateImage( cvSize(800, 600), IPL_DEPTH_8U, im->nChannels );
        }

        cvZero( image_ );
        cvCopy( im, image_, m);

        return image_;
    }

}; // class mask

} // namespace zoov

#endif // ZOOV_FX_MASK_HPP
