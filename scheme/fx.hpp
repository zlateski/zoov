#ifndef ZOOV_SCHEME_FX_HPP
#define ZOOV_SCHEME_FX_HPP 1

#include "fx_fwd.hpp"
#include "scheme/cell.hpp"
#include <zi/bits/type_traits.hpp>
#include <zi/meta/meta.hpp>
#include <zi/meta/enable_if.hpp>
#include <zi/utility/singleton.hpp>
#include <zi/utility/non_copyable.hpp>
#include <cv.h>


namespace zoov {



namespace detail {

class fx_locker: zi::non_copyable
{
private:
    fx_ptr    fx_ ;
    image_ptr img_;

public:
    explicit fx_locker(const fx_ptr& p)
        : fx_(p)
        , img_(fx_->get_image())
    {
        fx_->acquire_read();
    }

    image_ptr& get_image()
    {
        return img_;
    }

    ~fx_locker()
    {
        fx_->release_read();
    }

    const fx_ptr& get() const
    {
        return fx_;
    }
};

template <std::size_t T, typename Z>
struct cat;

template <template<std::size_t...> class C, std::size_t T, std::size_t... Args>
struct cat<T,C<Args...>>
{
    typedef C<T,Args...> type;
};

template<typename T> struct arg_type;
template<> struct arg_type<int> { typedef int type; };
template<> struct arg_type<float> { typedef float type; };
template<> struct arg_type<double> { typedef double type; };
template<> struct arg_type<std::string> { typedef const std::string& type; };
template<> struct arg_type<image_ptr> { typedef image_ptr type; };

template<typename T> struct arg_extractor
{ T& operator()(T& v) const { return v; } };

template<> struct arg_extractor<image_ptr>
{
    image_ptr& operator()(fx_locker& v) const
    {
        return v.get_image(); //.get_image(); //v->get()->get_image();
    }
};


template<typename T> struct locked_type;
template<> struct locked_type<int> { typedef int type; };
template<> struct locked_type<float> { typedef float type; };
template<> struct locked_type<double> { typedef double type; };
template<> struct locked_type<std::string> { typedef std::string type; };
template<> struct locked_type<image_ptr> { typedef fx_locker type; };

template<typename T> struct locker_it;

template<> struct locker_it<float> {
    typedef float type;
    float operator()(cell_ptr& l) { return static_cast<float>(pop_number(l)); } };

template<> struct locker_it<double> {
    typedef double type;
    double operator()(cell_ptr& l) { return static_cast<double>(pop_number(l)); } };

template<> struct locker_it<int> {
    typedef int type;
    int operator()(cell_ptr& l) { return pop_int(l); } };

template<> struct locker_it<std::string> {
    typedef std::string type;
    std::string operator()(cell_ptr& l) { return pop_string(l); } };

template<> struct locker_it<image_ptr> {
    typedef fx_ptr type;
    fx_ptr operator()(cell_ptr& l) { return pop_image(l); } };

template<typename T>
typename locker_it<T>::type
do_lock(cell_ptr& l)
{
    locker_it<T> e;
    return e(l);
}

template<typename ...Ts>
class locker;

template<>
class locker<>: zi::non_copyable
{
public:
    locker(cell_ptr&) {}
};


template<typename Head, typename... Tail>
class locker<Head,Tail...>: zi::non_copyable
{
public:
    typedef Head                                  original_t ;
    typedef typename locked_type<Head>::type      storead_t  ;
    typedef locker<Tail...>                       tail_type  ;

private:
    storead_t       h_;
    locker<Tail...> t_;

public:
    locker(cell_ptr& l)
        : h_(do_lock<original_t>(l))
        , t_(l)
    { }

    original_t& head()
    {
        arg_extractor<original_t> f;
        return f(h_);
    }

    locker<Tail...>& tail()
    {
        return t_;
    }
};

template<std::size_t I, class P>
struct locker_type_at
{
    typedef typename locker_type_at<I-1,typename P::tail_type>::type type;
};

template<class P>
struct locker_type_at<0,P>
{
    typedef typename P::original_t type;
};

template<std::size_t I, typename ...Ts>
typename zi::meta::enable_if_c<I==0,typename locker<Ts...>::original_t&>::type
locker_get(locker<Ts...>& t)
{
    return t.head();
};

template<std::size_t I, typename ...Ts >
typename zi::meta::enable_if_c<
    (I>0), typename locker_type_at<I,locker<Ts...>>::type& >::type
    locker_get(locker<Ts...>& t)
{
    return locker_get<I-1>(t.tail());
};

template<std::size_t... Ids>
struct locker_indices {};

template<class T, std::size_t N>
struct make_locker_indices_h;

template<template<typename...> class C, std::size_t N>
struct make_locker_indices_h<C<>, N>
{
    typedef locker_indices<> type;
};

template<template<typename...> class C, std::size_t N, typename Head, typename ...Tail>
struct make_locker_indices_h<C<Head,Tail...>, N>
{
    typedef typename cat
    < N - sizeof...(Tail) - 1
      , typename make_locker_indices_h<C<Tail...>, N>::type
      >::type type;
};

template<class T>
struct make_locker_indices;

template<template<typename...> class C, typename Head, typename ...Tail>
struct make_locker_indices<C<Head,Tail...>>
{
    typedef typename make_locker_indices_h<C<Head,
        Tail...>,
        sizeof...(Tail)+1>::type type;
};

template<template<typename...> class C>
struct make_locker_indices<C<>>
{
    typedef locker_indices<> type;
};

// template<template<typename...> class C>
// struct make_locker_indices<C<>>
// {
//     typedef typename locker_indices<> type;
// };



} // namespace detail

template< typename... > struct static_params {};
template< typename... > struct dynamic_params {};

template< typename S = static_params<>, typename D = dynamic_params<> >
class effect;

template< typename... Ts, typename... Zs >
class effect<static_params<Ts...>,dynamic_params<Zs...>>
    : virtual public fx
{
public:
    typedef detail::locker<Ts...>                                      init_locker    ;
    typedef typename detail::make_locker_indices<init_locker>::type    init_indices   ;
    typedef detail::locker<Zs...>                                      process_locker ;
    typedef typename detail::make_locker_indices<process_locker>::type process_indices;

    virtual void init(typename detail::arg_type<Ts>::type...) = 0;
    virtual image_ptr process(typename detail::arg_type<Zs>::type...) = 0;

    virtual void close() {}
    virtual ~effect() { }

    std::size_t init_len() const
    {
        return sizeof...(Ts);
    }

    template<std::size_t... IDs>
        void call_init(cell_ptr& l, const detail::locker_indices<IDs...>&)
    {
        init_locker params(l);
        init(detail::locker_get<IDs>(params)...);
    }

    cell_ptr init(cell_ptr l)
    {
        call_init(l, init_indices());
        return l;
    }

    template<std::size_t... IDs>
        image_ptr call_process(cell_ptr& l, const detail::locker_indices<IDs...>&)
    {
        process_locker params(l);
        fx::acquire_write();
        image_ptr r = process(detail::locker_get<IDs>(params)...);
        fx::release_write();
        return r;
    }

    image_ptr process(cell_ptr& l)
    {
        return call_process(l, process_indices());
    }

    effect()
    {

    }
};


} // namespace zoov

#define STATIC(...)  zoov::static_params< __VA_ARGS__ >
#define DYNAMIC(...) zoov::dynamic_params< __VA_ARGS__ >

#define EFFECT(name,stat,...)                                           \
    name;                                                               \
                                                                        \
    struct _____ ## name ## _register_____                              \
    {                                                                   \
        _____ ## name ## _register_____()                               \
        {                                                               \
            zoov::register_fx<name> x(#name);                           \
        }                                                               \
    };                                                                  \
                                                                        \
    namespace {                                                         \
        const _____ ## name ## _register_____ &                         \
        _____ ## name ## _register_instance_____ =                      \
            zi::singleton<_____ ## name ## _register_____>              \
            ::instance();                                               \
            }                                                           \
                                                                        \
    void _____use_ ## name ## _register_instance_____()                 \
    {                                                                   \
        static_cast<void>(_____ ## name ## _register_instance_____);    \
    }                                                                   \
                                                                        \
    class name: public zoov::effect<stat,zoov::dynamic_params<__VA_ARGS__>>

#endif // ZOOV_SCHEME_FX_HPP

