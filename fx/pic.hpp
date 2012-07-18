#ifndef ZOOV_FX_PIC_HPP
#define ZOOV_FX_PIC_HPP 1

#include "fx.hpp"

#include <string>

namespace zoov {

class EFFECT(pic, STATIC(std::string))
{
private:
    image_ptr image_;

public:
    pic()
        : image_()
    { }

    void init(const std::string& filename)
    {
        IplImage* original  = cvLoadImage( filename.c_str(), CV_LOAD_IMAGE_COLOR );
        IplImage* resampled = cvCreateImage( cvSize(800, 600), IPL_DEPTH_8U, 3 );

        cvResize( original, resampled, CV_INTER_CUBIC );

        image_ = image_pool::get(800,600,IPL_DEPTH_8U,4);

        cvCvtColor( resampled, image_.get(), CV_RGB2RGBA );

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

}; // class image

} // namespace zoov

#endif // ZOOV_FX_PIC_HPP
