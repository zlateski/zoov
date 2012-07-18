#ifndef ZOOV_SCHEME_CELL_HPP
#define ZOOV_SCHEME_CELL_HPP 1

#include "types_fwd.hpp"
#include "exception.hpp"
#include "fx_fwd.hpp"

#include <zi/utility/singleton.hpp>

#include <utility>
#include <ios>
#include <iomanip>
#include <string>
#include <cmath>
#include <limits>

namespace zoov {

class fx;
typedef boost::shared_ptr<fx> fx_ptr;


double round(double number)
{
    return number < 0.0 ? std::ceil(number - 0.5) : std::floor(number + 0.5);
}

class cell_t
{
private:
    typedef std::pair<cell_ptr,cell_ptr> cell_ptr_pair;

public:
    enum
    {
        undefined = 0,
        nil,
        boolean,
        number,
        symbol,
        quote,
        pair,
        builtin,
        compound,
        infile,
        effect,
        image,
        string,
        promise,
        creator
    };

private:
    typedef cell_t this_t;

private:
    int           type_   ;
    bool          bool_   ;
    double        number_ ;
    std::string   string_ ;
    cell_ptr_pair pair_   ;
    env_ptr       env_    ;
    builtin_t     builtin_;
    fx_ptr        effect_ ;
    cell_ptr      copy_   ;
    std::size_t   visited_;

    friend class env_t;
    //template <typename> friend class garbage_collector_t;

public:
    cell_ptr copy()
    {
        cell_t* c = new cell_t(type_);
        c->bool_ = bool_;
        c->number_ = number_;
        c->string_ = string_;
        if ( pair_.first ) c->pair_.first = pair_.first->copy();
        if ( pair_.second ) c->pair_.second = pair_.second->copy();
        c->env_ = env_;
        c->builtin_ = builtin_;
        c->effect_ = effect_;
        c->copy_ = copy_;
        c->visited_ = visited_;
        return cell_ptr(c);
    }


public:
    cell_t(int t)
        : type_(t)
        , bool_()
        , number_()
        , string_()
        , pair_()
        , env_()
        , builtin_()
        , effect_()
        , copy_()
        , visited_(0)
    { }

    cell_t()
        : type_(nil)
        , bool_()
        , number_()
        , string_()
        , pair_()
        , env_()
        , builtin_()
        , effect_()
        , copy_()
        , visited_(0)
    { }

    ~cell_t()
    {
        if ( type_ == effect )
        {
            std::cout << "Erased: " << get_effect_name() << '\n';
        }
    }

public:
    static cell_ptr make_undefined()
    {
        cell_ptr c = boost::make_shared<cell_t>(this_t::undefined);
        return c;
    }

    static cell_ptr make_nil()
    {
        static cell_ptr n = boost::make_shared<cell_t>();
        return n;
    }

    static cell_ptr make_boolean(bool b)
    {
        cell_t* c = new cell_t(boolean);
        c->bool_ = b;
        return cell_ptr(c);
    }

    static cell_ptr make_number(double n)
    {
        cell_t* c = new cell_t(number);
        c->number_ = n;
        return cell_ptr(c);
    }

    static cell_ptr make_symbol(const std::string& s)
    {
        cell_t* c = new cell_t(symbol);
        c->string_ = s;
        return cell_ptr(c);
    }

    static cell_ptr make_promise(cell_ptr p, env_ptr env)
    {
        cell_t* c = new cell_t(promise);
        c->pair_.first = p;
        c->env_ = env;
        c->bool_ = false;
        return cell_ptr(c);
    }

    static cell_ptr make_string(const std::string& s)
    {
        cell_t* c = new cell_t(string);
        c->string_ = s;
        return cell_ptr(c);
    }

    static cell_ptr make_quote(cell_ptr q)
    {
        cell_t* c = new cell_t(quote);
        c->pair_.first = q;
        return cell_ptr(c);
    }

    static cell_ptr make_pair(cell_ptr a = cell_t::make_nil(), cell_ptr b = cell_t::make_nil())
    {
        cell_t* c = new cell_t(pair);
        c->pair_.first = a;
        c->pair_.second = b;
        return cell_ptr(c);
    }

    static cell_ptr make_builtin(builtin_t f)
    {
        cell_t* c = new cell_t(builtin);
        c->builtin_ = f;
        return cell_ptr(c);
    }

    static cell_ptr make_creator(const std::string& name, builtin_t f)
    {
        cell_t* c = new cell_t(creator);
        c->string_ = name;
        c->builtin_ = f;
        return cell_ptr(c);
    }

    static cell_ptr make_compound(cell_ptr arg, cell_ptr body, env_ptr env)
    {
        cell_t* c = new cell_t(compound);
        c->pair_.first = arg;
        c->pair_.second = body;
        c->env_ = env;
        return cell_ptr(c);
    }

    static cell_ptr make_infile(cell_ptr body, env_ptr env)
    {
        cell_t* c = new cell_t(infile);
        c->pair_.second = body;
        c->env_ = env;
        return cell_ptr(c);
    }

    static cell_ptr make_effect(const std::string& name, fx_ptr f)
    {
        cell_t* c = new cell_t(effect);
        c->effect_  = f;
        c->string_  = name;
        return cell_ptr(c);
    }

    static cell_ptr make_image(fx_ptr f)
    {
        cell_t* c = new cell_t(image);
        c->effect_  = f;
        return cell_ptr(c);
    }

public:
    int get_type() const
    {
        return type_;
    }

    bool is_nil() const
    {
        return type_ == this_t::nil;
    }

    bool is_undefined() const
    {
        return type_ == this_t::undefined;
    }

    bool is_boolean() const
    {
        return type_ == this_t::boolean;
    }

    bool is_number() const
    {
        return type_ == this_t::number;
    }

    bool is_integral() const
    {
        if (!is_number())
        {
            return false;
        }

        return (std::abs(number_ - round(number_)) < std::numeric_limits<double>::epsilon());
    }

    bool is_double() const
    {
        return is_number();
    }

    bool is_quote() const
    {
        return type_ == this_t::quote;
    }

    bool is_symbol() const
    {
        return type_ == this_t::symbol;
    }

    bool is_promise() const
    {
        return type_ == this_t::promise;
    }

    bool is_promise_evaluated() const
    {
        return bool_;
    }

    void promise_evaluated(cell_ptr v)
    {
        bool_ = 1;
        pair_.first = v;
    }

    bool is_string() const
    {
        return type_ == this_t::string;
    }

    bool is_pair() const
    {
        return type_ == this_t::this_t::pair;
    }

    void set_car(cell_ptr a)
    {
        assure(is_pair(), "Not a pair");
        pair_.first = a;
    }

    void set_env(env_ptr e)
    {
        env_ = e;
    }

    void set_cdr(cell_ptr b)
    {
        assure(is_pair(), "Not a pair");
        pair_.second = b;
    }

    bool is_builtin() const
    {
        return type_ == this_t::builtin;
    }

    bool is_creator() const
    {
        return type_ == this_t::creator;
    }

    bool is_compound() const
    {
        return type_ == this_t::compound;
    }

    bool is_infile() const
    {
        return type_ == this_t::infile;
    }

    bool is_effect() const
    {
        return type_ == this_t::effect;
    }

    bool is_image() const
    {
        return type_ == this_t::image;
    }

    bool is_function() const
    {
        return is_builtin() || is_compound() || is_infile();
    }


public:
    int get_integral() const
    {
        assure(is_integral(), "Not an integral type");
        return static_cast<int>(number_);
    }

    fx_ptr get_effect() const
    {
        assure(is_effect(), "Not an effect type");
        return effect_;
    }

    fx_ptr get_image() const
    {
        assure(is_image(), "Not an effect type");
        return effect_;
    }

    int get_int() const
    {
        return get_integral();
    }

    double get_number() const
    {
        assure(is_number(), "Not a number type");
        return number_;
    }

    double get_double() const
    {
        return get_number();
    }

    bool get_bool() const
    {
        assure(is_boolean(), "Not a boolean type");
        return bool_;
    }

    bool get_boolean() const
    {
        return get_bool();
    }

    std::string get_symbol() const
    {
        assure(is_symbol(), "Not a symbol");
        return string_;
    }

    cell_ptr get_promise() const
    {
        assure(is_promise(), "Not a promise");
        return pair_.first;
    }

    std::string set_symbol(const std::string& s)
    {
        assure(is_symbol(), "Not a symbol");
        string_ = s;
        return s;
    }

    std::string get_string() const
    {
        assure(is_string(), "Not a string");
        return string_;
    }

    std::string get_effect_name() const
    {
        assure(is_effect(), "Not a string");
        return string_;
    }

    cell_ptr get_quote() const
    {
        assure(is_quote(), "Not a quote");
        return pair_.first;
    }

    cell_ptr car() const
    {
        assure(is_pair(), "Not a pair");
        return pair_.first;
    }

    cell_ptr cdr() const
    {
        assure(is_pair(), "Not a pair");
        return pair_.second;
    }

    cell_ptr caar() const
    {
        return car()->car();
    }

    cell_ptr cadr() const
    {
        return cdr()->car();
    }

    cell_ptr cddr() const
    {
        return cdr()->cdr();
    }

    cell_ptr cdar() const
    {
        return car()->cdr();
    }

    builtin_t get_builtin() const
    {
        assure(is_builtin(), "Not a builtin procedure");
        return builtin_;
    }

    builtin_t get_creator() const
    {
        assure(is_creator(), "Not a creator procedure");
        return builtin_;
    }

    const std::string& get_creator_name() const
    {
        assure(is_creator(), "Not a creator procedure");
        return string_;
    }

    cell_ptr get_args() const
    {
        assure(is_compound(), "Not a compound procedure");
        return pair_.first;
    }

    cell_ptr get_body() const
    {
        assure(is_compound() || is_infile(), "Not a compound or infile  procedure");
        return pair_.second;
    }

    env_ptr get_env() const
    {
        assure(is_effect() || is_promise() ||
               is_compound() || is_infile(), "Not a compound or infile procedure");
        return env_;
    }

    bool is_false() const
    {
        return type_ == this_t::nil || (type_ == this_t::boolean && !bool_);
    }

    bool is_true() const
    {
        return !is_false();
    }
};

template <class CharT, class Traits>
inline std::basic_ostream<CharT,Traits>&
operator<< (std::basic_ostream<CharT,Traits>& os, cell_ptr c );

template <class CharT, class Traits>
inline void print_list(std::basic_ostream<CharT,Traits>& os, cell_ptr c )
{
    os << '(';
    os << c->car();
    c = c->cdr();
    while ( c->is_pair() )
    {
        os << ' ' << c->car();
        c = c->cdr();
    }

    if ( !c->is_nil() )
    {
        os << " . " << c;
    }

    os << ')';
}

template <class CharT, class Traits>
inline std::basic_ostream<CharT,Traits>&
operator<< (std::basic_ostream<CharT,Traits>& os, cell_ptr c )
{
    switch ( c->get_type() )
    {
    case cell_t::nil:
        os << "()";
        break;

    case cell_t::boolean:
        os << '#' << ( c->get_bool()?'t':'f');
        break;

    case cell_t::number:
        os << c->get_number();
        break;

    case cell_t::quote:
        os << "(quote " << c->car() << ")";
        break;

    case cell_t::symbol:
        os << c->get_symbol();
        break;

    case cell_t::promise:
        os << "[promise]";
        break;

    case cell_t::string:
        os << '"' << c->get_string() << '"';
        break;

    case cell_t::effect:
        os << "[effect<"
           << c->get_effect_name() << "> "
           << std::hex
           << reinterpret_cast<void*>(c->get_effect().get())
           << std::dec << ']';
        break;

    case cell_t::image:
        os << "[image "
           << std::hex
           << reinterpret_cast<void*>(c->get_image().get())
           << std::dec << ']';
        break;

    case cell_t::pair:
        print_list(os, c);
        break;

    case cell_t::builtin:
        os << "[builtin procedure]";
        break;

    case cell_t::creator:
        os << "[creator procedure<"
           << c->get_creator_name()
           << ">]";
        break;

    case cell_t::compound:
        os << "[compound procedure] of ";
        os << c->get_args() << ": ";
        os << c->get_body();
        break;

    case cell_t::infile:
        os << "[infile procedure]";
        break;

    case cell_t::undefined:
        os << "[undefined value]";
        break;

    }

    return os;
}


inline int pop_integral(cell_ptr& l)
{
    assure(l->is_pair(), "Not a list");
    int r = l->car()->get_integral();
    l = l->cdr();
    return r;
}

inline fx_ptr pop_effect(cell_ptr& l)
{
    assure(l->is_pair(), "Not a list");
    fx_ptr r = l->car()->get_effect();
    l = l->cdr();
    return r;
}

inline fx_ptr pop_image(cell_ptr& l)
{
    assure(l->is_pair(), "Not a list");
    fx_ptr r = l->car()->get_image();
    l = l->cdr();
    return r;
}

inline int pop_int(cell_ptr& l)
{
    return pop_integral(l);
}

inline double pop_number(cell_ptr& l)
{
    assure(l->is_pair(), "Not a list");
    double r = l->car()->get_double();
    l = l->cdr();
    return r;
}

inline double pop_double(cell_ptr& l)
{
    return pop_number(l);
}

inline bool pop_bool(cell_ptr& l)
{
    assure(l->is_pair(), "Not a list");
    bool r = l->car()->get_boolean();
    l = l->cdr();
    return r;
}

inline bool get_boolean(cell_ptr& l)
{
    return pop_bool(l);
}

inline std::string pop_symbol(cell_ptr& l)
{
    assure(l->is_pair(), "Not a list");
    std::string r = l->car()->get_symbol();
    l = l->cdr();
    return r;
}

inline std::string pop_string(cell_ptr& l)
{
    assure(l->is_pair(), "Not a list");
    std::string r = l->car()->get_string();
    l = l->cdr();
    return r;
}

} // namespace zoov

#endif // ZOOV_SCHEME_CELL_HPP
