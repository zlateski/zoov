#ifndef ZOOV_FX_IMAGE
#define ZOOV_FX_IMAGE 1

#include "fx.hpp"

#include <string>

namespace zoov {

class EFFECT(image, STATIC(std::string))
{
private:
    image_ptr image_;

public:
    image()
        : image_(cvCreateImage( cvSize(800, 600), IPL_DEPTH_8U, 4 ))
    { }

    void init(const std::string& filename)
    {
        IplImage* original  = cvLoadImage( filename.c_str(), CV_LOAD_IMAGE_COLOR );
        IplImage* resampled = cvCreateImage( cvSize(800, 600), IPL_DEPTH_8U, 3 );

        cvResize( original, resampled, CV_INTER_CUBIC );
        cvCvtColor( resampled, image_, CV_RGB2RGBA );

        cvReleaseImage(&original);
        cvReleaseImage(&resampled);
    }

    image_ptr process()
    {
        return image_;
    }

    bool has_changed() const
    {
        return false;
    }

    ~image()
    {
        if (image_)
        {
            cvReleaseImage(&image_);
        }
    }

}; // class image

} // namespace zoov

#endif
