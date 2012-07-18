#ifndef ZOOV_SCHEME_FX_FWD_HPP
#define ZOOV_SCHEME_FX_FWD_HPP 1

#include <boost/shared_ptr.hpp>
#include <zi/concurrency.hpp>
#include "detail/image_pool.hpp"

#include <cstddef>
#include <utility>
#include <iostream>
#include <atomic>

#include <opencv/cv.h>
#include <opencv/cv.hpp>
#include <opencv/highgui.h>


namespace zoov {

struct ticker_t: std::atomic<int> {};

namespace {
ticker_t& ticker = zi::singleton<ticker_t>::instance();
}


//typedef boost::shared_ptr<cv::Mat> image_ptr;
typedef boost::shared_ptr<IplImage> image_ptr;

inline void null_deleter(void*)
{ }

inline image_ptr from_ptr(IplImage* i)
{
    return boost::shared_ptr<IplImage>(i,&null_deleter);
}


class fx;
typedef boost::shared_ptr<fx> fx_ptr;

class fx
{
private:
    image_ptr   image_      ;
    bool        initialized_;
    zi::rwmutex m_          ;
    int         tick_       ;

public:
    virtual std::size_t init_len() const = 0;

    void acquire_read() const
    {
        m_.acquire_read();
    }

    void release_read() const
    {
        m_.release_read();
    }

    void acquire_write() const
    {
        m_.acquire_write();
    }

    void release_write() const
    {
        m_.release_write();
    }

public:
    fx()
        : image_()
        , initialized_(false)
        , m_()
        , tick_()
    { }

    virtual ~fx()
    { }

    virtual bool has_changed() const
    {
        return true;
    }

    virtual image_ptr process(cell_ptr&) = 0;

    void process_fx(cell_ptr l)
    {
        if ( !initialized_ )
        {
            std::cout << "Process before Init!!!!!!\n";
        }

        if ( !image_ )
        {
            tick_ = ticker;
            image_ = process(l);
        }

        if ( has_changed() && tick_ < ticker )
        {
            tick_ = ticker;
            image_ = process(l);
        }
    }

    image_ptr get_image() const
    {
        if ( !image_ )
        {
            std::cout << "NOOOOO IMAGEEEE\n";
        }
        return image_;
    }

    virtual cell_ptr init(cell_ptr) = 0;

    cell_ptr init_fx(cell_ptr l)
    {
        initialized_ = true;
        return init(l);
    }

}; // class fx

} // namespace zoov

#endif // ZOOV_SCHEME_FX_FWD_HPP
