#ifndef ZOOV_FX_VIDEO_HPP
#define ZOOV_FX_VIDEO_HPP 1

#include "fx.hpp"

#include <zi/concurrency.hpp>
#include <zi/time.hpp>

#include <vlc/vlc.h>

#include <algorithm>
#include <utility>
#include <cstddef>
#include <string>
#include <iostream>

#include "av/audio.hpp"

namespace zoov {

namespace detail {

inline void* vlc_on_lock(void *what, void** planes);
inline void  vlc_on_unlock(void *what, void *picture, void *const *planes);
inline void  vlc_on_display(void *what, void *picture);

inline void  vlc_on_audio_play(void* what, const void* samples, unsigned count, int64_t pts);
inline void  vlc_on_audio_pause(void* what, int64_t pts);
inline void  vlc_on_audio_resume(void* what, int64_t pts);
inline void  vlc_on_audio_flush(void* what, int64_t pts);
inline void  vlc_on_audio_drain(void* what);

inline int   vlc_on_audio_setup(void **data, char *format, unsigned *rate, unsigned *channels);
inline void  vlc_on_audio_cleanup(void *data);

} // namespace detail

class EFFECT(video, STATIC(std::string))
{
private:
    image_ptr current_;
    image_ptr buffer_;
    image_ptr result_;

    bool changed_;

    zi::mutex               mutex_;
    zi::condition_variable  cv_   ;

    libvlc_instance_t*     vlc_instance_;
    libvlc_media_player_t* vlc_mp_;
    libvlc_media_t*        vlc_media_;

public:
    void init(const std::string& filename)
    {
        int width = 800;
        int height = 600;

        current_ = image_pool::get(width,height,IPL_DEPTH_8U,4);
        buffer_  = image_pool::get(width,height,IPL_DEPTH_8U,4);
        result_  = image_pool::get(width,height,IPL_DEPTH_8U,4);
        vlc_instance_ = libvlc_new(0,0);

        vlc_media_ = libvlc_media_new_path(vlc_instance_, filename.c_str());
        vlc_mp_    = libvlc_media_player_new_from_media(vlc_media_);

        libvlc_audio_set_format_callbacks( vlc_mp_,
                                           detail::vlc_on_audio_setup,
                                           detail::vlc_on_audio_cleanup );

        libvlc_video_set_format(vlc_mp_, "RV32", width, height, width*4);
        libvlc_media_release(vlc_media_);

        libvlc_video_set_callbacks(vlc_mp_, detail::vlc_on_lock,
                                   detail::vlc_on_unlock, detail::vlc_on_display, this);

        libvlc_audio_set_callbacks(vlc_mp_,
                                   detail::vlc_on_audio_play,
                                   detail::vlc_on_audio_pause, detail::vlc_on_audio_resume,
                                   detail::vlc_on_audio_flush, detail::vlc_on_audio_drain,
                                   this);

        //mixer.add_audio(this);

        libvlc_media_player_play(vlc_mp_);

        libvlc_audio_output_t* outs = libvlc_audio_output_list_get(vlc_instance_);

        //libvlc_audio_output_t* outs = outsl;

        std::cout << "AUDIO:\n";

        while (outs)
        {
            std::cout << outs->psz_name << ": " << outs->psz_description << "\n";

            outs = outs->p_next;
        }

        std::cout << "//\n";

        libvlc_audio_output_list_release(outs);
    }

    video()
        : current_()
        , buffer_()
        , result_()
        , changed_(false)
        , mutex_()
        , cv_()
        , vlc_instance_()
        , vlc_mp_()
        , vlc_media_()
    { }

    ~video()
    {
        //mixer.remove_audio(this);

        std::cout << "Erased video effect\n";

        libvlc_media_player_stop(vlc_mp_);
        libvlc_media_player_release(vlc_mp_);
        libvlc_release(vlc_instance_);
    }

    void on_lock(void** planes)
    {
        *planes = buffer_->imageData;
    }

    void on_unlock(void *, void *const *)
    {
    }

    void on_display(void *)
    {
        zi::mutex::guard g(mutex_);
        changed_ = true;
        std::swap( current_, buffer_ );
        cv_.notify_all();
    }

    void on_audio_play(const void* samples, unsigned count, int64_t pts)
    {
        //mixer.add_samples(this, samples, count);
        std::cout << "Audio Play (" << count << ") : " << pts << ' ' << libvlc_clock() << "\n";
        const uint16_t* p = reinterpret_cast<const uint16_t*>(samples);
        //std::cout << p[0] << "\n";
        //std::cout << p[1] << "\n";
        //std::cout << p[2] << "\n";

    }

    void on_audio_pause(int64_t pts)
    {
        std::cout << "Audio Pause : " << pts << "\n";
    }

    void on_audio_resume(int64_t pts)
    {
        std::cout << "Audio Resume : " << pts << "\n";
    }

    void on_audio_flush(int64_t pts)
    {
        std::cout << "Audio Flush : " << pts << "\n";
    }

    void on_audio_drain()
    {
        std::cout << "Audio Drain\n";
    }


    image_ptr process()
    {
        zi::mutex::guard g(mutex_);
        while (!changed_)
        {
            cv_.wait(mutex_);
        }
        //if ( changed_ )
        //{
        std::swap( current_, result_ );
        ////}
        changed_ = false;
        return result_;
    }

}; // class video

namespace detail {

inline void* vlc_on_lock(void *what, void** planes)
{
    reinterpret_cast<video*>(what)->on_lock(planes);
    return what;
}

inline void vlc_on_unlock(void *what, void *picture, void *const *planes)
{
    reinterpret_cast<video*>(what)->on_unlock(picture, planes);
}

inline void vlc_on_display(void *what, void *picture)
{
    reinterpret_cast<video*>(what)->on_display(picture);
}


inline void vlc_on_audio_play(void* what, const void* samples, unsigned count, int64_t pts)
{
    reinterpret_cast<video*>(what)->on_audio_play(samples, count, pts);
}

inline void vlc_on_audio_pause(void* what, int64_t pts)
{
    reinterpret_cast<video*>(what)->on_audio_pause(pts);
}

inline void vlc_on_audio_resume(void* what, int64_t pts)
{
    reinterpret_cast<video*>(what)->on_audio_resume(pts);
}

inline void vlc_on_audio_flush(void* what, int64_t pts)
{
    reinterpret_cast<video*>(what)->on_audio_flush(pts);
}

inline void vlc_on_audio_drain(void* what)
{
    reinterpret_cast<video*>(what)->on_audio_drain();
}

inline int  vlc_on_audio_setup(void **data, char *format, unsigned *rate, unsigned *channels)
{
    std::cout << "Format: " << format << "\nRate: " << *rate << "\nChann: " << *channels << "\n";
    return 1;
}

inline void vlc_on_audio_cleanup(void *data)
{
}




} // namespace detail

ALIAS(vid, video);

} // namespace zoov

#endif // ZOOV_FX_VIDEO_HPP
