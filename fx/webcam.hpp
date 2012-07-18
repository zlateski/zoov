#ifndef ZOOV_FX_WEBCAM_HPP
#define ZOOV_FX_WEBCAM_HPP 1

#include "fx.hpp"

#include <cv.h>
#include <highgui.h>

#include <zi/concurrency.hpp>
#include <zi/time.hpp>

namespace zoov {

class EFFECT(webcam, STATIC(int))
{
private:
    image_ptr resampled_;
    image_ptr current_  ;
    image_ptr buffer_   ;
    image_ptr result_   ;
    bool      changed_  ;

    zi::mutex               mutex_;
    zi::condition_variable  cv_   ;

    CvCapture* capture_;
    bool       die_    ;

private:
    void capture_loop()
    {
        zi::int64_t t = 0;

        for (;;)
        {
            {
                zi::mutex::guard g(mutex_);
                if ( die_ )
                {
                    die_ = false;
                    cv_.notify_all();
                    return;
                }
            }

            IplImage* original;

            original = cvQueryFrame( capture_ );
            cvResize( original, resampled_.get(), CV_INTER_CUBIC );
            cvCvtColor( resampled_.get(), current_.get(), CV_RGB2RGBA );

            //zi::int64_t n = zi::now::msec();
            //std::cout << ( n - t ) << '\n';
            //t = n;
            //std::cout << "FPS: " << cvGetCaptureProperty(capture_, CV_CAP_PROP_FPS) << '\n';
            //std::cout << original_->width << "\n";
            //std::cout << original_->height << "\n";
            //std::cout << original_->imageSize << " :: "
            //          << ( original_->width * original_->height ) << "\n";

            {
                zi::mutex::guard g(mutex_);
                std::swap( current_, buffer_ );
                changed_ = true;
                cv_.notify_all();
            }
        }
    }

public:
    void init(int camid)
    {
        int width  = 800;
        int height = 600;

        changed_   = false;

        resampled_ = image_pool::get(width,height,IPL_DEPTH_8U,3);
        current_   = image_pool::get(width,height,IPL_DEPTH_8U,4);
        buffer_    = image_pool::get(width,height,IPL_DEPTH_8U,4);
        result_    = image_pool::get(width,height,IPL_DEPTH_8U,4);

        capture_ = cvCaptureFromCAM(camid);
        die_ = false;

        zi::thread t(zi::bind(&webcam::capture_loop, this));
        t.start();
        t.detach();
    }

    ~webcam()
    {
        {
            zi::mutex::guard g(mutex_);
            die_ = true;
            while ( die_ )
            {
                cv_.wait(mutex_);
            }
        }
        cvReleaseCapture(&capture_);
    }

    image_ptr process()
    {
        zi::mutex::guard g(mutex_);
        while (!changed_)
        {
            cv_.wait(mutex_);
        }
        std::swap( current_, result_ );
        changed_ = false;
        return result_;
    }
};

} // namespace zoov

#endif // ZOOV_FX_WEBCAM_HPP
