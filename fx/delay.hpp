#ifndef ZOOV_FX_DELAY_HPP
#define ZOOV_FX_DELAY_HPP 1

#include "fx.hpp"
#include <vector>

namespace zoov {

class EFFECT(delay, STATIC(int), image_ptr)
{
private:
    std::size_t            size_;
    std::size_t            curr_;
    std::vector<image_ptr> imgs_;

public:
    void init(int l)
    {
        size_ = static_cast<std::size_t>(l);
        curr_ = 0;
    }

    image_ptr process(image_ptr im)
    {
        if ( size_ <= 0 )
        {
            image_ptr r = image_pool::get(im);
            cvCopy(im.get(), r.get());
            return r;
        }

        if ( imgs_.size() != size_ )
        {
            imgs_.resize(size_);
            for ( std::size_t i = 0; i < size_; ++i )
            {
                imgs_[i] = image_pool::get(im);
                cvZero(imgs_[i].get());
            }
        }

        std::size_t curr = curr_;
        curr_ = (curr_ + 1) % size_;

        cvCopy(im.get(), imgs_[curr].get());

        return imgs_[curr_];
    }

}; // class delay

} // namespace zoov

#endif
