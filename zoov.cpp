// 7783

#include "scheme/scheme.hpp"
#include "scheme/parse.hpp"
#include "scheme/detail/image_pool.hpp"
#include "fx/fx.hpp"
#include "fx/trail.hpp"
#include "fx/add.hpp"
#include "fx/image.hpp"
#include "fx/smooth.hpp"
#include "fx/sobel.hpp"
#include "fx/absdiff.hpp"
#include "fx/polygons.hpp"
#include "fx/threshold.hpp"
#include "fx/grayscale.hpp"
#include "fx/contours.hpp"
#include "fx/video.hpp"
#include "fx/canny.hpp"
#include "fx/delay.hpp"
#include "fx/webcam.hpp"
#include "fx/mask.hpp"
#include "fx/fps_counter.hpp"

#include <iostream>
#include <string>
#include <utility>
#include <list>
#include <boost/lexical_cast.hpp>
#include <boost/any.hpp>

#include <zi/utility/for_each.hpp>
#include <zi/bits/type_traits.hpp>
#include <zi/meta/meta.hpp>
#include <zi/meta/enable_if.hpp>
#include <zi/concurrency.hpp>

#include <SDL/SDL.h>
#include <zi/concurrency.hpp>
#include <zi/timer.hpp>
#include <SDL/SDL_ttf.h>

#include <zi/logging.hpp>
#include <sys/resource.h>

void setstacksize()
{
    const rlim_t kStackSize = 1024 * 1024 * 1024;   // min stack size = 16 MB
    struct rlimit rl;
    int result;

    result = getrlimit(RLIMIT_STACK, &rl);

    printf("%d\n", result);

    if (result == 0)
    {
        printf("%ld %ld\n", rl.rlim_cur, kStackSize);
        if (rl.rlim_cur < kStackSize)
        {
            rl.rlim_cur = kStackSize;
            result = setrlimit(RLIMIT_STACK, &rl);
            if (result != 0)
            {
                fprintf(stderr, "setrlimit returned result = %d\n", result);
            }
        }
    }

    // ...

//    return 0;
}

USE_ZiLOGGING( STDOUT );

class sdl_t
{
    sdl_t()
    {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0 )
        {
            exit(1);
        }
    }

    ~sdl_t()
    {
        SDL_Quit();
    }
};

//sdl_t& sdl = zi::singleton<sdl_t>::instance();

class some_effect: public zoov::effect<zoov::static_params<std::string, int>>
{
public:
    ~some_effect()
    {
        std::cout << "Killed Some Effect\n";
    }

    void init(const std::string& b, int a)
    {
        std::cout << a << ' ' << b << "\n";
    }

    zoov::image_ptr process()
    {
        //std::cout << "[rpcess\n";
        //std::cout << a << ' ' << b << "\n";
        return zoov::image_ptr();
    }

    ///    void pr
};

namespace zoov {

struct effect_thread
{
private:
    bool         is_running_;
    zi::mutex    m_         ;
    IplImage*    out_       ;

public:
    void stop()
    {
        zi::mutex::guard g(m_);
        is_running_ = false;
    }

    effect_thread()
        : is_running_(true)
        , m_()
        , out_(cvCreateImageHeader( cvSize(800, 600), IPL_DEPTH_8U, 4 ))
    {
    }

    ~effect_thread()
    {
        cvReleaseImageHeader(&out_);
    }

    void loop(zoov::cell_ptr l, zoov::env_ptr e, SDL_Surface* screen) const
    {
        out_->imageData = reinterpret_cast<char*>(screen->pixels);

        std::pair<cell_ptr,env_ptr> p = std::make_pair(l,e);
//                                      scheme::env());// =

        while (1)
        {
            bool is_running;

            {
                zi::mutex::guard g(m_);
                is_running = is_running_;
            }

            if ( !is_running_ )
            {
                //std::memset(screen->pixels, 0, screen->pitch * screen->h);
                return;
            }
            else
            {
                ++zoov::ticker;
                cell_ptr r = zoov::scheme::evaluate_in_env(p.first,p.second);
                p.second->run_gc(p.first);

                r->get_image()->acquire_write();
                image_ptr im = r->get_image()->get_image();

                if (SDL_MUSTLOCK(screen))
                {
                    if (SDL_LockSurface(screen) < 0)
                    {
                        std::cout << "CAN'T LOCK SURFACE!\n\n";
                        continue;
                    }
                }

                switch ( im->nChannels )
                {
                case 1:
                    cvCvtColor( im.get(), out_, CV_GRAY2RGBA );
                    break;
                case 3:
                    cvCvtColor( im.get(), out_, CV_RGB2RGBA );
                    break;
                case 4:
                    std::memcpy(screen->pixels, im->imageData,
                                screen->pitch * screen->h);
                    break;
                }

                //p.second->run_gc(p.first);
                //zoov::env_garbage_collector.collect(zoov::scheme::env());

                if (SDL_MUSTLOCK(screen))
                {
                    SDL_UnlockSurface(screen);
                }

                SDL_Flip(screen);

                r->get_image()->release_write();

                //zoov::scheme::env()->run_gc(p.first);
            }

            if ( !is_running )
            {
                return;
            }
        }
    }
};

cell_ptr quit_function(cell_ptr args, env_ptr)
{
    SDL_Quit();
    scheme::env()->run_gc();
    exit(0);
}

} // namespace zoov



int main(int argc, char **argv)
{
    setstacksize();

    zi::parse_arguments( argc, argv, true );

    zoov::scheme::register_builtin("quit", &zoov::quit_function);
    zoov::register_fx<some_effect> reg("some");

    std::string line;
    int par = 0;

    bool done = false;

    SDL_Surface *screen;
    SDL_Event event;

    int keypress = 0;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        exit(1);
    }

    std::cout << "SDL Initialized\n";

    std::cout << "Try init mixer\n";

    zoov::mixer.init();

    std::cout << "Try init mixer - DONE\n";

    if (!(screen = SDL_SetVideoMode(800, 600, 32, SDL_HWSURFACE)))
    {
        SDL_Quit();
        return 1;
    }

    //zoov::shareds.open("libtest.so");

    while (1)
    {
        done = false;

        std::string x;
        std::getline(std::cin, x);
        x += ' ';

        for ( std::size_t z = 0; z < x.size(); ++z )
        {
            line += x[z];
            if ( x[z] == '(' )
            {
                ++par;
            }

            if ( x[z] == ')' )
            {
                --par;
                if ( par == 0 )
                {
                    std::list<std::string> s = zoov::tokenize(line);
                    zoov::cell_ptr c = zoov::parse(s);

                    bool to_run = false;

                    {
                        zoov::cell_ptr l = zoov::scheme::evaluate(c);
                        zoov::scheme::env()->run_gc(c);
                        to_run = l->is_pair() && l->car()->is_effect();
                        std::cout << " = " << l << "\n";
                    }


                    // std::cout << (l->is_effect() ? "Image Found\n" : "Nada\n");
                    // std::cout << "Will be evaluating:\n" << c << "\n\n";

                    if ( to_run )
                    {
                        zoov::effect_thread et1;
                        //zoov::effect_thread et2;

                        std::pair<zoov::cell_ptr,zoov::env_ptr> p2 // (c,zoov::scheme::env());
                                                                   = zoov::scheme::env()->clone(c);

                        //c.reset();
                        //zoov::scheme::env()->run_gc();
                        p2.second->print();


                        //zi::thread t1(zi::bind(&zoov::effect_thread::loop,
                        //                     &et1, c, zoov::scheme::env(), screen ));
                        zi::thread t1(zi::bind(&zoov::effect_thread::loop,
                                               &et1, p2.first, p2.second, screen ));
                        t1.start();
                        //t2.start();


                        while(SDL_PollEvent(&event))
                        { }

                        while(!keypress)
                        {
                            //DrawScreen(screen,h++);
                            while(SDL_PollEvent(&event))
                            {
                                switch (event.type)
                                {
                                case SDL_KEYDOWN:
                                    if ( event.key.keysym.sym == 'q' )
                                    {
                                        keypress = 1;
                                    }
                                }
                            }
                        }

                        et1.stop();
                        //et2.stop();
                        t1.join();
                        //t2.join();

                        //p2.second->clear();
                        //p2.second->run_gc();
                        p2.second->print();

                        keypress = 0;
                    }

                    zoov::scheme::env()->run_gc();

                    zoov::printncels();
                    zoov::printnenv();
                    //zoov::env_garbage_collector.collect(zoov::scheme::env());
                    line = "";
                    done = true;
                }
            }
        }

        if ( line != "" && par == 0 && !done)
        {
            std::list<std::string> s = zoov::tokenize(line);
            zoov::cell_ptr c = zoov::parse(s);
            std::cout << c << "\n";
            std::cout << zoov::scheme::evaluate(c) << "\n";
            line = "";
        }

        //std::cout << line << "\n";
    }

}
