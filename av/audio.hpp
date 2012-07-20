#ifndef ZOOV_AV_AUDIO_HPP
#define ZOOV_AV_AUDIO_HPP 1

#include <SDL/SDL.h>
#include <SDL/SDL_audio.h>
#include <SDL/SDL_sound.h>
#include <SDL/SDL_mixer.h>

#include <zi/concurrency.hpp>
#include <zi/utility/singleton.hpp>
#include <zi/utility/for_each.hpp>

#include <cstddef>
#include <map>
#include <list>
#include <string>
#include <sstream>
#include <cstring>
#include <deque>

#include <unistd.h>

namespace zoov {

inline void audio_mixer_fill_audio(void* m, Uint8* stream, int len);

class audio_mixer
{
private:
    SDL_AudioSpec          spec_;
    zi::mutex              m_   ;
    zi::condition_variable cv_  ;
    bool                   initialized_;
    std::map<void*, int > read_ ;
    std::map<void*, int > write_;

public:
    audio_mixer()
        : spec_()
        , m_()
        , cv_()
        , initialized_(0)
        , read_()
        , write_()
    {
    }

    void init()
    {
        spec_.freq     = 48000;
        spec_.format   = AUDIO_S16LSB;
        spec_.channels = 2;
        spec_.samples  = 4800;
        spec_.callback = audio_mixer_fill_audio;
        spec_.userdata = this;

        SDL_AudioSpec got;

        std::cout << "Will try to open SDL Audio\n";

        if ( SDL_OpenAudio(&spec_, &got) < 0 )
        {
            std::cout << "Can't open SDL_AudioSpec\n" << std::flush;
            exit(0);
        }

        std::cout << "GOT: "
                  << "    freq: " << got.freq << "\n"
                  << "    channels: " << got.channels << "\n"
                  << "    samples: " << static_cast<int>(got.samples) << "\n";

        std::cout << "SDL Audio Initialized\n";

        initialized_ = true;
    }

    ~audio_mixer()
    {
        SDL_CloseAudio();
    }

    void add_audio(void* who)
    {
        zi::mutex::guard g(m_);

        if ( !initialized_ )
        {
            init();
        }

        if ( read_.count(who) )
        {
            exit(0);
        }

        int f[2];
        static_cast<void>(::pipe(f));

        read_[who] = f[0];
        write_[who] = f[1];

        if ( read_.size() == 1 )
        {
            SDL_PauseAudio(0);
        }
    }

    void remove_audio(void* who)
    {
        zi::mutex::guard g(m_);
        if ( read_.count(who) == 0 )
        {
            exit(0);
        }

        ::close(read_[who]);
        ::close(write_[who]);

        read_.erase(who);
        write_.erase(who);

        if ( read_.size() == 0 )
        {
            SDL_PauseAudio(1);
        }
    }

    void add_samples(void* who, const void* samples, unsigned len)
    {
        std::cout << "Adding to: " <<  who << " len: " << len*4 << "\n";
        zi::mutex::guard g(m_);
        int w = ::write(write_[who],reinterpret_cast<const char*>(samples),len*4);
        cv_.notify_all();
    }

    void fill_audio(Uint8* stream, int len)
    {
        std::cout << "Audio mixer fill getting lock\n";
        zi::mutex::guard g(m_);
        std::cout << "Audio mixer fill got lock " << read_.size() << "\n";

        std::memset(stream, 0, len);

        FOR_EACH( it, read_ )
        {
            std::cout << "Needs to fill: " << it->first  << " for " << len << "\n";

            //if ( it->second->str().size() < 100000 ) return;

            char buff[len];
            char* cpos = buff;
            char* dpos = reinterpret_cast<char*>(stream);
            int l = len;

            //for (;;)
            {
                int r = ::read(it->second, buff, l);
                SDL_MixAudio(reinterpret_cast<Uint8*>(dpos),
                             reinterpret_cast<Uint8*>(cpos), r, SDL_MIX_MAXVOLUME);
                std::cout << "Read: " << r << "\n";
            }

            std::cout << "Mixed: " << it->first << "\n";
        }
    }
};

inline void audio_mixer_fill_audio(void* m, Uint8* stream, int len)
{
    std::cout << "Audio mixer fill called\n";
    reinterpret_cast<audio_mixer*>(m)->fill_audio(stream, len);
}

namespace {
audio_mixer& mixer = zi::singleton<audio_mixer>::instance();
}

} // namespace zoov

#endif // ZOOV_AV_AUDIO_HPP

