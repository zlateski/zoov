#ifndef ZOOV_FX_SMOOTH_HPP
#define ZOOV_FX_SMOOTH_HPP 1

#include "fx.hpp"

namespace zoov {

class EFFECT(smooth, STATIC(), image_ptr, int, int)
{
private:
    image_ptr image_ ;

public:
    smooth()
        : image_(cvCreateImage( cvSize(800, 600), IPL_DEPTH_8U, 4 ))
    { }

    ~smooth()
    {
        cvReleaseImage(&image_);
    }

    void init()
    {
    }

    image_ptr process(image_ptr im, int a, int b)
    {
        if ( im->nChannels != image_->nChannels )
        {
            cvReleaseImage(&image_);
            image_ = cvCreateImage( cvSize(800, 600), IPL_DEPTH_8U, im->nChannels );
        }

        cvSmooth( im, image_, CV_BLUR, a, b, 0, 0 );
        return image_;
    }

}; // class smooth

} // namespace zoov

#endif
