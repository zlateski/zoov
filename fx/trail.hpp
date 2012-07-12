#ifndef ZOOV_FX_TRAIL_HPP
#define ZOOV_FX_TRAIL_HPP 1

#include "fx.hpp"

namespace zoov {

class EFFECT(trail, STATIC(), image_ptr, double )
{
private:
    image_ptr image_   ;

public:
    trail()
        : image_(cvCreateImage( cvSize(800, 600), IPL_DEPTH_8U, 4 ))
    { }

    ~trail()
    {
        cvReleaseImage(&image_);
    }

    void init()
    { }

    image_ptr process(image_ptr im, double a1 )
    {
        if ( im->nChannels != image_->nChannels )
        {
            cvReleaseImage(&image_);
            image_ = cvCreateImage( cvSize(800, 600), IPL_DEPTH_8U, im->nChannels );
        }

        cvAddWeighted( image_, a1, im, static_cast<double>(1)-a1,
                       static_cast<double>(0), image_ );

        return image_;
    }

}; // class canny

} // namespace zoov

#endif // ZOOV_FX_TRAIL_HPP
