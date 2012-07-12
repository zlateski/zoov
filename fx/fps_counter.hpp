#ifndef ZOOV_FX_FPS_COUNTER_HPP
#define ZOOV_FX_FPS_COUNTER_HPP 1

#include "fx.hpp"

#include <zi/concurrency.hpp>
#include <zi/time/timer.hpp>

#include <algorithm>
#include <utility>
#include <cstddef>
#include <cstdio>
#include <string>
#include <iostream>

namespace zoov {

class EFFECT(fps_counter, STATIC(), image_ptr)
{
private:
    zi::timer::wall     timer_;
    std::vector<double> times_;
    std::size_t         index_;

public:
    fps_counter()
        : timer_()
        , times_(32)
        , index_()
    {
    }

    ~fps_counter()
    {
    }

    void init()
    {
        timer_.reset();
    }

    image_ptr process(image_ptr x)
    {
        std::size_t next_index = (index_ + 1) % 32;
        times_[index_] = static_cast<double>(zi::now::usecs());
        if ( index_ == 30 )
        {
            std::cout << (static_cast<double>(32000000)/(times_[index_]-times_[next_index])) << " fps\n";
        }
        index_ = next_index;

        return x;
    }

}; // class fps_counter

} // namespace zoov

#endif // ZOOV_FX_FPS_COUNTER_HPP
