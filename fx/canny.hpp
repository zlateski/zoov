#ifndef ZOOV_FX_CANNY_HPP
#define ZOOV_FX_CANNY_HPP 1

#include "fx.hpp"

namespace zoov {

class EFFECT(canny, STATIC(), image_ptr, double, double, int)
{
private:
    image_ptr image_   ;

public:
    canny()
        : image_(cvCreateImage( cvSize(800, 600), IPL_DEPTH_8U, 4 ))
    { }

    ~canny()
    {
        cvReleaseImage(&image_);
    }

    void init()
    { }

    image_ptr process(image_ptr im, double a1, double a2, int a3 )
    {
        if ( im->nChannels != image_->nChannels )
        {
            cvReleaseImage(&image_);
            image_ = cvCreateImage( cvSize(800, 600), IPL_DEPTH_8U, im->nChannels );
        }

        cvCanny( im, image_, a1, a2, a3 );

        return image_;
    }

}; // class canny

} // namespace zoov

#endif // ZOOV_FX_CANNY_HPP
