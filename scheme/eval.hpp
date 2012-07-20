#ifndef ZOOV_SCHEME_EVAL_HPP
#define ZOOV_SCHEME_EVAL_HPP

#include "cell.hpp"
#include "env.hpp"
#include "detail/file.hpp"

#include <zi/concurrency.hpp>
#include <zi/utility/for_each.hpp>
#include <list>
#include <string>

namespace zoov {

inline cell_ptr apply(cell_ptr, cell_ptr);
inline cell_ptr evcond(cell_ptr, env_ptr);
inline cell_ptr evif(cell_ptr, env_ptr);
inline cell_ptr eval(cell_ptr, env_ptr);
inline cell_ptr evlet(cell_ptr, env_ptr);
inline cell_ptr evlist(cell_ptr, env_ptr);
inline cell_ptr evalall(cell_ptr, env_ptr);

class evaluator
{
    struct job
    {
    private:
        cell_ptr to_eval_ ;
        cell_ptr args_    ;
        cell_ptr to_store_;
        bool     is_done_ ;
        zi::mutex              m_ ;
        zi::condition_variable cv_;

    public:
        job(cell_ptr e, cell_ptr a, cell_ptr s)
            : to_eval_(e)
            , args_(a)
            , to_store_(s)
            , is_done_(0)
            , m_()
            , cv_()
        {}

        void run()
        {
            to_eval_->get_effect()->process_fx(args_);
            to_store_->set_car(cell_t::make_image(to_eval_->get_effect()));

            {
                zi::mutex::guard g(m_);
                is_done_ = true;
                cv_.notify_all();
            }
        }

        void wait()
        {
            zi::mutex::guard g(m_);
            while (!is_done_)
            {
                cv_.wait(m_);
            }
        }

    };

    struct task_manager: zi::task_manager::deque
    {
        task_manager()
            : zi::task_manager::deque(10)
        {
            zi::task_manager::deque::start();
        }

        ~task_manager()
        {
            zi::task_manager::deque::join();
        }
    };

private:
    std::list<job*> jobs_;
    task_manager&   tm_  ;

public:
    evaluator()
        : jobs_()
        , tm_(zi::singleton<task_manager>::instance())
    { }

    ~evaluator()
    {
        wait();
    }

    void add(cell_ptr e, cell_ptr a, cell_ptr s)
    {
        jobs_.push_back(new job(e,a,s));
        //jobs_.back()->run();
        tm_.push_back(zi::bind(&job::run,jobs_.back()));
    }

    void wait()
    {
        FOR_EACH_ERASE( it, jobs_ )
        {
            if ( *it )
            {
                (*it)->wait();
                delete (*it);
            }
        }
    }
};

inline cell_ptr evlet(cell_ptr exp, env_ptr e)
{
    env_ptr r = e->drop_frame();

    //env_garbage_collector.register_env(r);

    cell_ptr pairs = exp->car();

    while ( !pairs->is_nil() )
    {
        r->set(pairs->caar()->get_symbol(), evalall(pairs->cdar(), r));
        pairs = pairs->cdr();
    }

    return evalall(exp->cdr(), r);
}

inline cell_ptr evlist(cell_ptr l, env_ptr e)
{
    //std::cout << "EVLIST: " << l << "\n";

    if ( l->is_nil() )
    {
        return l;
    }

    cell_ptr ret(cell_t::make_pair());
    cell_ptr sto(ret);


    evaluator evx;

    while ( !l->is_nil() )
    {
        assure(l->is_pair(), "Not a list");

        if ( l->car()->is_pair() && l->car()->car()->is_effect() )
        {
            //std::cout << "effect this: " << l->car() << "\n";
            cell_ptr a = evlist(l->car()->cdr(),e); //l->car()->car()->get_env());
            evx.add(l->car()->car(),a,sto);
        }
        else
        {
            sto->set_car(eval(l->car(),e));
        }

        l = l->cdr();
        if ( !l->is_nil() )
        {
            sto->set_cdr(cell_t::make_pair());
            sto = sto->cdr();
        }
    }

    evx.wait();

    return ret;
}

inline cell_ptr evcond(cell_ptr exp, env_ptr e)
{
    //std::cout << "Evcond: " << exp << "\n";

    if ( exp->is_nil() )
    {
        return exp;
    }

    assure( exp->is_pair(), "Bad cond format");
    assure( exp->car()->is_pair(), "Bad case cond format");

    bool else_case = exp->caar()->is_symbol() && exp->caar()->get_symbol() == "else";

    if ( else_case )
    {
        //std::cout << "Else: " << exp->cdar() << '\n';
        exp = exp->cdar();
        return evalall(exp, e);
    }

    cell_ptr r;
    r = eval(exp->caar(), e);

    //std::cout << "R: " << r << "\n";

    if ( r->is_true() )
    {
        exp = exp->cdar();

        //std::cout << "It's true: " << exp << "\n";

        if (exp->is_nil())
        {
            return r;
        }
        else
        {
            return evalall(exp, e);
        }
    }
    else
    {
        return evcond(exp->cdr(), e);
    }
}

// todo more
inline cell_ptr evif(cell_ptr exp, env_ptr e)
{
    if ( eval(exp->cadr(), e)->is_true() )
    {
        return eval(exp->cddr()->car(), e);
    }
    else
    {
        return eval(exp->cddr()->cadr(), e);
    }
}


inline cell_ptr eval(cell_ptr exp, env_ptr e)
{

    std::cout << "Eval : " << exp << "\n";

    if ( exp->is_number() || exp->is_string() ||
         exp->is_boolean() || exp->is_promise() ||
         exp->is_image() )
    {
        return exp;
    }

    if ( exp->is_symbol() )
    {
        try
        {
            return e->lookup(exp->get_symbol());
        }
        catch ( scheme_exception& ecp )
        {
            if ( detail::file_exists("./lib/" + exp->get_symbol() + ".zcm") )
            {
                std::string x = detail::file_get_contents("./lib/" + exp->get_symbol() + ".zcm");
                std::list<std::string> s = tokenize(x);
                cell_ptr c = parse(s);
                return cell_t::make_infile
                    (cell_t::make_pair(c,cell_t::make_nil()), get_global_env());
            }
            throw ecp;
        }
    }

    if ( exp->is_quote() )
    {
        return exp->get_quote();
    }

    assure( exp->is_pair(), "Bad expression [not a pair]" );
    //assure( exp->car()->is_symbol(), "Bad expression [not a symbol]");

    if ( exp->car()->is_symbol() )
    {

        std::string s = exp->car()->get_symbol();

        if ( s == "promise" )
        {
            assure(exp->cddr()->is_nil(), "Syntax error");
            return cell_t::make_promise(exp->cadr(), e);
        }

        if ( s == "force" )
        {
            cell_ptr f = eval(exp->cadr(), e);
            assure(exp->cddr()->is_nil(), "Syntax error");
            assure(f->is_promise(), "Not a promise");

            cell_ptr r = f->get_promise();

            if ( !f->is_promise_evaluated() )
            {
                r = eval(f->get_promise(), f->get_env());
                f->promise_evaluated(r);
            }

            return r;
        }

        if ( s == "quote" )
        {
            return exp->cadr();
        }

        if ( s == "load" )
        {
            exp = exp->cdr();
            while ( !exp->is_nil() )
            {
                std::string f = exp->car()->get_string();
                if ( !detail::file_exists(f) )
                {
                    return cell_t::make_boolean(false);
                }

                std::string cnt = detail::file_get_contents(f);
                std::list<std::string> cntl = tokenize(cnt);

                while ( cntl.size() )
                {
                    cell_ptr c = parse(cntl);
                    //std::cout << "File content: " << c << "\n";
                    eval(c, e);
                }
                exp = exp->cdr();
            }
            return cell_t::make_boolean(true);
        }

        if ( s == "string" )
        {
            return cell_t::make_string(exp->cadr()->get_symbol());
        }

        if ( s == "cond" )
        {
            return evcond(exp->cdr(), e);
        }

        if ( s == "if" )
        {
            return evif(exp, e);
        }

        if ( s == "let" || s == "let*" )
        {
            return evlet(exp->cdr(), e);
        }

        if ( s == "define" )
        {
            if ( exp->cadr()->is_symbol() )
            {
                std::string s = exp->cadr()->get_symbol();
                assure( !e->has(s), s + " already defined in this environment");
                cell_ptr c = eval(exp->cddr()->car(), e);
                e->set(s, c);
                return cell_t::make_undefined();
            }
            else
            {
                // syntactic sugar for defining lambdas
                assure(exp->cadr()->car()->is_symbol(), "Not a symbol");
                std::string s = exp->cadr()->car()->get_symbol();
                cell_ptr args = exp->cadr()->cdr();
                cell_ptr body = exp->cddr();
                e->set(s, cell_t::make_compound(args, body, e));
                return cell_t::make_undefined();
            }
        }

        if ( s == "lambda" )
        {
            return cell_t::make_compound(exp->cadr(), exp->cddr(), e);
        }

        if ( s == "set!" )
        {
            assure( exp->cadr()->is_symbol(), "Not a symbol");
            assure( exp->cddr()->cdr()->is_nil(), "Too many arguments");
            std::pair<cell_ptr, env_ptr> p = e->lookup_env(exp->cadr()->get_symbol() );
            cell_ptr r = eval(exp->cddr()->car(), e);
            p.second->set( exp->cadr()->get_symbol(), r);
            return p.first;
        }
    }

    if ( exp->car()->is_effect() )
    {
        exp->car()->get_effect()->process_fx(evlist(exp->cdr(), e));//exp->car()->get_env()));
        return cell_t::make_image(exp->car()->get_effect());
    }

    cell_ptr f = eval(exp->car(), e);

    if ( f->is_creator() )
    {
        cell_ptr r = cell_t::make_pair(exp->car(),evlist(exp->cdr(),e));
        f->get_creator()(r);

        exp->set_car(r->car());
        //r->car()->set_env(e);

        for ( std::size_t i = r->car()->get_effect()->init_len(); i > 0; --i )
        {
            exp->set_cdr(exp->cdr()->cdr());
        }

        //std::cout << "Created: " << f->get_creator_name() << "\n";
        return exp;
    }

    if ( f->is_function() )
    {
        return apply(f, evlist(exp->cdr(), e));
    }

    return f;
}

inline cell_ptr evalall(cell_ptr l, env_ptr e)
{
    cell_ptr r = cell_t::make_undefined();
    while ( !l->is_nil() )
    {
        r = eval(l->car(), e);
        l = l->cdr();
    }

    return r;
}

inline cell_ptr apply(cell_ptr f, cell_ptr args)
{
    if ( f->is_compound() )
    {
        return evalall(f->get_body(),f->get_env()->drop_frame(f->get_args(), args));
    }

    if ( f->is_builtin() )
    {
        builtin_t t = f->get_builtin();
        cell_ptr a = t(args);
        return a;
    }

    if ( f->is_infile() )
    {
        return evalall(f->get_body(),f->get_env()->drop_frame(args));
    }

    return cell_t::make_undefined();
}

} // namespace zoov


#endif // ZOOV_SCHEME_EVAL_HPP
