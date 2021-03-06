#ifndef ZOOV_SCHEME_SCHEME_HPP
#define ZOOV_SCHEME_SCHEME_HPP

#include "cell.hpp"
#include "env.hpp"
#include "eval.hpp"
#include "parse.hpp"
#include "detail/file.hpp"

#include <zi/utility/singleton.hpp>

#include "detail/dlfcn.hpp"

namespace zoov {

struct get_carr
{
    cell_ptr operator()(cell_ptr args, env_ptr) const
    {
        return args->car()->car();
    }
};

class scheme
{
private:
    env_ptr env_;

private:
    static cell_ptr rand_fn(cell_ptr args, env_ptr)
    {
        assure(args->cdr()->is_nil(), "nil? needs only one arg");
        int n = args->car()->get_integral();
        return cell_t::make_number(std::rand() % n);
    }

    static cell_ptr add(cell_ptr args, env_ptr)
    {
        double d = args->car()->get_number();
        args = args->cdr();
        while ( !args->is_nil() )
        {
            d += args->car()->get_number();
            args = args->cdr();
        }

        return cell_t::make_number(d);
    }

    static cell_ptr is_nil(cell_ptr args, env_ptr)
    {
        //std::cout << args << "\n";
        assure(args->cdr()->is_nil(), "nil? needs only one arg");
        return cell_t::make_boolean(args->car()->is_nil());
    }

    static cell_ptr get_car(cell_ptr args, env_ptr)
    {
        return args->car()->car();
    }

    static cell_ptr make_list(cell_ptr args, env_ptr)
    {
        return args;
    }

    static cell_ptr is_list(cell_ptr args, env_ptr)
    {
        return cell_t::make_boolean(args->car()->is_pair());
    }

    static cell_ptr print_env(cell_ptr args, env_ptr)
    {
        scheme::env()->print();
        return cell_t::make_undefined();
    }

    static cell_ptr get_cdr(cell_ptr args, env_ptr)
    {
        return args->car()->cdr();
    }

    static cell_ptr subtract(cell_ptr args, env_ptr)
    {
        double d = args->car()->get_number();
        args = args->cdr();
        while ( !args->is_nil() )
        {
            d -= args->car()->get_number();
            args = args->cdr();
        }

        return cell_t::make_number(d);
    }

    static cell_ptr is_gt(cell_ptr args, env_ptr)
    {
        double a = args->car()->get_number();
        args = args->cdr();
        double b = args->car()->get_number();
        return cell_t::make_boolean(a>b);
    }

    static cell_ptr is_gte(cell_ptr args, env_ptr)
    {
        double a = args->car()->get_number();
        args = args->cdr();
        double b = args->car()->get_number();
        return cell_t::make_boolean(a>=b);
    }

    static cell_ptr is_lt(cell_ptr args, env_ptr)
    {
        double a = args->car()->get_number();
        args = args->cdr();
        double b = args->car()->get_number();
        return cell_t::make_boolean(a<b);
    }

    static cell_ptr is_lte(cell_ptr args, env_ptr)
    {
        double a = args->car()->get_number();
        args = args->cdr();
        double b = args->car()->get_number();
        return cell_t::make_boolean(a<=b);
    }

    static cell_ptr multiply(cell_ptr args, env_ptr)
    {
        double d = args->car()->get_number();
        args = args->cdr();
        while ( !args->is_nil() )
        {
            d *= args->car()->get_number();
            args = args->cdr();
        }

        return cell_t::make_number(d);
    }

    static cell_ptr divide(cell_ptr args, env_ptr)
    {
        double d = args->car()->get_number();
        args = args->cdr();
        while ( !args->is_nil() )
        {
            d /= args->car()->get_number();
            args = args->cdr();
        }

        return cell_t::make_number(d);
    }

    static cell_ptr int_divide(cell_ptr args, env_ptr)
    {
        int d = args->car()->get_int();
        args = args->cdr();
        while ( !args->is_nil() )
        {
            d /= args->car()->get_int();
            args = args->cdr();
        }

        return cell_t::make_number(d);
    }

    static cell_ptr int_mod(cell_ptr args, env_ptr)
    {
        int d = args->car()->get_int();
        args = args->cdr();
        while ( !args->is_nil() )
        {
            d %= args->car()->get_int();
            args = args->cdr();
        }

        return cell_t::make_number(d);
    }

    static cell_ptr if_fn(cell_ptr args, env_ptr)
    {
        assure(args->cddr()->is_nil(), "Too many arguments");
        cell_ptr a = args->car();
        cell_ptr b = args->cadr();

        if ( a->is_integral() && b->is_integral() )
        {
            return cell_t::make_boolean(a->get_int() == b->get_int());
        }

        if ( a->is_number() && b->is_number() )
        {
            return cell_t::make_boolean(a->get_number() == b->get_number());
        }

        if ( a->is_symbol() && b->is_symbol() )
        {
            return cell_t::make_boolean(a->get_symbol() == b->get_symbol());
        }

        return cell_t::make_boolean(false);
    }

    static cell_ptr concat(cell_ptr args, env_ptr)
    {
        std::string d = args->car()->get_string();
        args = args->cdr();
        while ( !args->is_nil() )
        {
            d += args->car()->get_string();
            args = args->cdr();
        }

        return cell_t::make_string(d);
    }

    static cell_ptr make_cons(cell_ptr args, env_ptr)
    {
        cell_ptr r = cell_t::make_pair(args->car(), args->cadr());
        assure(args->cddr()->is_nil(), "Cons needs two args");
        return r;
    }

    static cell_ptr print_it(cell_ptr args, env_ptr)
    {
        while ( !args->is_nil() )
        {
            std::cout << args->car() << "\n";
            args = args->cdr();
        }
        return cell_t::make_undefined();
    }


public:
    static void register_builtin(const std::string& s, const builtin_t& f)
    {
        scheme::env()->set(s, cell_t::make_builtin(f));
    }

    static void unregister_builtin(const std::string& s)
    {
        scheme::env()->erase(s);
    }

    static void register_effect(const std::string& s, const builtin_t& f)
    {
        scheme::env()->set(s, cell_t::make_creator(s, f));
    }

public:
    scheme()
        : env_(new env_t)
    {
        env_->set("#t", cell_t::make_boolean(true));
        env_->set("#t", cell_t::make_boolean(true));
        env_->set("#f", cell_t::make_boolean(false));
        env_->set("nil?", cell_t::make_builtin(&scheme::is_nil));
        env_->set("nil", cell_t::make_nil());
        env_->set(">?", cell_t::make_builtin(&scheme::is_gt));
        env_->set(">=?", cell_t::make_builtin(&scheme::is_gte));
        env_->set("<?", cell_t::make_builtin(&scheme::is_lt));
        env_->set("<=?", cell_t::make_builtin(&scheme::is_lte));
        env_->set("+", cell_t::make_builtin(&scheme::add));
        env_->set("-", cell_t::make_builtin(&scheme::subtract));
        env_->set("*", cell_t::make_builtin(&scheme::multiply));
        env_->set("/", cell_t::make_builtin(&scheme::divide));
        env_->set("div", cell_t::make_builtin(&scheme::int_divide));
        env_->set("mod", cell_t::make_builtin(&scheme::int_mod));
        env_->set("eq?", cell_t::make_builtin(&scheme::if_fn));
        env_->set("list?", cell_t::make_builtin(&scheme::is_list));
        env_->set("pair?", cell_t::make_builtin(&scheme::is_list));
        env_->set("car", cell_t::make_builtin(&scheme::get_car));
        env_->set("cdr", cell_t::make_builtin(&scheme::get_cdr));
        env_->set("list", cell_t::make_builtin(&scheme::make_list));
        env_->set("cons", cell_t::make_builtin(&scheme::make_cons));
        env_->set("concat", cell_t::make_builtin(&scheme::concat));
        env_->set("print", cell_t::make_builtin(&scheme::print_it));
        env_->set("printenv", cell_t::make_builtin(&scheme::print_env));
        env_->set("rand", cell_t::make_builtin(&scheme::rand_fn));
    }

    env_ptr get_env() const
    {
        return env_;
    }

    static env_ptr env()
    {
        return zi::singleton<scheme>::instance().env_;
    }

    static cell_ptr evaluate(cell_ptr c)
    {
        cell_ptr r = zoov::eval(c, scheme::env());
        return r;
    }

    static cell_ptr evaluate_in_env(cell_ptr c, env_ptr e)
    {
        return zoov::eval(c, e);
    }

    static cell_ptr evaluate_line(const std::string& line )
    {
        std::list<std::string> s = zoov::tokenize(line);
        zoov::cell_ptr c = zoov::parse(s);
        return scheme::evaluate(c);
    }


}; // class scheme

inline env_ptr get_global_env()
{
    return scheme::env();
}

template< typename Fx >
struct register_fx
{
    struct creator_fn
    {
        const std::string name;

        cell_ptr operator ()(cell_ptr l, env_ptr) const
        {
            fx_ptr   fxp(new Fx);
            cell_ptr args = fxp->init_fx(l->cdr());

            l->set_car(cell_t::make_effect(name,fxp));
            l->set_cdr(args);

            return l;
        }
    };

    register_fx(const std::string& s)
    {
        scheme::register_effect(s, creator_fn{s});
    }
};


} // namespace zoov


#endif // ZOOV_SCHEME_EVAL_HPP
