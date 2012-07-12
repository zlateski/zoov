#ifndef ZOOV_FX_ADD_HPP
#define ZOOV_FX_ADD_HPP 1

#include "fx.hpp"

#include <zi/concurrency.hpp>

#include <cv.h>

namespace zoov {

class EFFECT(add, STATIC(), image_ptr, image_ptr, double, double, double)
{
private:
    image_ptr result_;

public:
    add()
        : result_(cvCreateImage( cvSize(800, 600), IPL_DEPTH_8U, 4))
    { }

    ~add()
    {
        cvReleaseImage(&result_);
    }

    void init()
    { }

    image_ptr process(image_ptr a, image_ptr b, double alpha, double beta, double gamma)
    {
        cvAddWeighted(a, alpha, b, beta, gamma, result_);
        return result_;
    }

};

} // namespace zoov

#endif // ZOOV_FX_ADD_HPP
