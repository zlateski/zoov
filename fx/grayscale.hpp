#ifndef ZOOV_FX_GRAYSCALE_HPP
#define ZOOV_FX_GRAYSCALE_HPP 1

#include "fx.hpp"

namespace zoov {

class EFFECT(grayscale, STATIC(), image_ptr)
{
private:
    image_ptr image_;

public:
    grayscale()
        : image_(cvCreateImage( cvSize(800, 600), IPL_DEPTH_8U, 1 ))
    { }

    void init() {}

    image_ptr process(image_ptr im)
    {

        switch ( im->nChannels )
        {
        case 4:
            cvCvtColor( im, image_, CV_RGBA2GRAY );
            break;
        case 3:
            cvCvtColor( im, image_, CV_RGB2GRAY );
            break;
        case 1:
            std::memcpy( image_->imageData, im->imageData, im->imageSize);
            break;
        }
        return image_;
    }

    ~grayscale()
    {
        if ( image_ )
        {
            cvReleaseImage(&image_);
        }
    }

}; // class grayscale

} // namespace zoov

#endif // ZOOV_FX_GRAYSCALE_HPP
