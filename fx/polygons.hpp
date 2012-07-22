#ifndef ZOOV_FX_POLYGONS_HPP
#define ZOOV_FX_POLYGONS_HPP 1

#include "fx.hpp"
#include <vector>

namespace zoov {

class EFFECT(polygons, STATIC(), image_ptr, int )
{
public:

    void init()
    { }

    image_ptr process(image_ptr im, int color)
    {
        std::vector<std::vector<cv::Point> > cs;
        image_ptr t = image_pool::get(im);

        cvCopy(im.get(), t.get());
        cv::Mat co(t.get());

        cv::Scalar col((color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);

        cv::findContours( co, cs, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE );

        image_ptr r = image_pool::get(im, 4);
        cvZero(r.get());
        cv::Mat ro(r.get());

        cv::Point** pts = new cv::Point*[cs.size()];
        int*        npts = new int[cs.size()];

        for ( std::size_t i = 0; i < cs.size(); ++i )
        {
            pts[i] = new cv::Point[cs[i].size()];
            std::copy(cs[i].begin(), cs[i].end(), pts[i]);
            npts[i] = static_cast<int>(cs[i].size());
        }

        cv::fillPoly(ro, const_cast<const cv::Point**>(pts),
                     npts, static_cast<int>(cs.size()), col);

        for ( std::size_t i = 0; i < cs.size(); ++i )
        {
            delete [] pts[i];
        }

        delete [] pts;
        delete [] npts;

        return r;
    }

}; // class canny

} // namespace zoov

#endif // ZOOV_FX_POLYGONS_HPP
