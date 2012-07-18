#ifndef ZOOV_SCHEME_DETAIL_IMAGE_POOL_HPP
#define ZOOV_SCHEME_DETAIL_IMAGE_POOL_HPP 1

#include <opencv/cv.h>
#include <zi/concurrency.hpp>
#include <zi/utility/for_each.hpp>
#include <boost/shared_ptr.hpp>
#include <map>
#include <iostream>
#include <list>

namespace zoov {

class image_pool
{
private:

    class pool
    {
    private:
        int       width_ ;
        int       height_;
        int       depth_ ;
        int       chans_ ;
        zi::mutex m_     ;

        std::list<IplImage*> list_;

        struct deleter
        {
        private:
            pool* pool_;
        public:
            explicit deleter(pool* p)
                : pool_(p)
            { }

            void operator()(void* v) const
            {
                pool_->return_image(reinterpret_cast<IplImage*>(v));
            }
        };

    public:
        pool(int w, int h, int d, int c)
            : width_(w)
            , height_(h)
            , depth_(d)
            , chans_(c)
            , m_()
            , list_()
        { }

        boost::shared_ptr<IplImage> get()
        {
            {
                zi::mutex::guard g(m_);
                if ( list_.size() )
                {
                    IplImage* m = list_.back();
                    list_.pop_back();
                    return boost::shared_ptr<IplImage>(m, deleter(this));
                }
            }

            return boost::shared_ptr<IplImage>
                (cvCreateImage( cvSize( width_, height_ ), depth_, chans_),
                 deleter(this));
        }

        void return_image(IplImage* m)
        {
            zi::mutex::guard g(m_);
            list_.push_back(m);
            //std::cout << "Pool Size: " << list_.size() << "\n";
        }

    };

private:
    std::map<std::tuple<int,int,int,int>,pool*> map_;
    zi::mutex                                   m_  ;

    pool* get_pool(int w, int h, int d, int c)
    {
        std::tuple<int,int,int,int> idx(w,h,d,c);
        zi::mutex::guard g(m_);

        if ( map_.count(idx) == 0 )
        {
            map_[idx] = new pool(w,h,d,c);
        }

        return map_[idx];
    }

public:

    image_pool()
        : map_()
        , m_()
    { }

    boost::shared_ptr<IplImage> get_it(int w, int h, int d, int c)
    {
        pool* p = get_pool(w,h,d,c);
        return p->get();
    }

    static boost::shared_ptr<IplImage> get(int w, int h, int d, int c)
    {
        return zi::singleton<image_pool>::instance().get_it(w,h,d,c);
    }

    static boost::shared_ptr<IplImage> get(IplImage* i)
    {
        return zi::singleton<image_pool>::instance().get_it
            (i->width,i->height,i->depth,i->nChannels);
    }

    static boost::shared_ptr<IplImage> get(boost::shared_ptr<IplImage> i)
    {
        return zi::singleton<image_pool>::instance().get_it
            (i->width,i->height,i->depth,i->nChannels);
    }

    static boost::shared_ptr<IplImage> get(IplImage* i, int c)
    {
        return zi::singleton<image_pool>::instance().get_it(i->width,i->height,i->depth,c);
    }

    static boost::shared_ptr<IplImage> get(boost::shared_ptr<IplImage> i, int c)
    {
        return zi::singleton<image_pool>::instance().get_it(i->width,i->height,i->depth,c);
    }

    ~image_pool()
    {
        FOR_EACH( it, map_ )
        {
            delete it->second;
        }
    }
};

} // namespace zoov

#endif // ZOOV_SCHEME_DETAIL_IMAGE_POOL_HPP
