#ifndef ZOOV_FX_CONTOURS_HPP
#define ZOOV_FX_CONTOURS_HPP 1

#include "fx.hpp"
#include <vector>

namespace zoov {

class EFFECT(contours, STATIC(), image_ptr, int, int )
{
public:

    void init()
    { }

    image_ptr process(image_ptr im, int color, int thick)
    {
        std::vector<std::vector<cv::Point> > cs;
        image_ptr t = image_pool::get(im);

        cvCopy(im.get(), t.get());
        cv::Mat co(t.get());

        cv::Scalar col((color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);

        cv::findContours( co, cs, CV_RETR_TREE, CV_CHAIN_APPROX_NONE );
        image_ptr r = image_pool::get(im, 4);
        cvZero(r.get());
        cv::Mat ro(r.get());

        cv::drawContours(ro, cs, -1, col, thick);

        return r;
    }

}; // class canny

} // namespace zoov

#endif // ZOOV_FX_CONTOURS_HPP
