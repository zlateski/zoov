#ifndef ZOOV_LISP_ENV_HPP
#define ZOOV_LISP_ENV_HPP

#include <boost/any.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include <map>
#include <string>
#include <zi/utility/singleton.hpp>
#include <zi/utility/for_each.hpp>

#include "cell.hpp"

namespace zoov {

static int nenv  = 0;
static int nrenv = 0;
static int cps   = 0;

class env_t
{
public:
    class garbage_collector_t
    {
    private:
        env_t*               root_;
        std::list<env_ptr>   envs_;
        std::size_t          iter_;

    public:
        garbage_collector_t( env_t* root )
            : root_(root)
            , envs_()
            , iter_(0)
        { }

        void collect(cell_ptr c)
        {
            ++iter_;

            root_->visit(iter_);
            visit(c, iter_);

            if ( c->env_ )
            {
                c->env_->visit(iter_);
            }

            std::list<env_ptr> envs;
            FOR_EACH( it, envs_ )
            {
                if ( (*it)->visited_ == iter_ )
                {
                    envs.push_back(*it);
                }
                else
                {
                    (*it)->clear();
                }
            }

            std::swap(envs, envs_);
        }

        void erase_all()
        {
            FOR_EACH( it, envs_ )
            {
                (*it)->clear();
            }
            envs_.clear();
            root_->clear();
        }

        void register_env(env_ptr e)
        {
            std::cout << "Registered env [ " << envs_.size() << " envs]\n";
            envs_.push_back(e);
        }

    }; // class garbage_collector_t

    env_ptr bind(cell_ptr vars, cell_ptr vals, env_ptr e)
    {
        env_ptr r(new env_t(e));
        gc_->register_env(r);
        r->pair_up(vars, vals);
        return r;
    }

    env_ptr bind(cell_ptr vals, env_ptr e)
    {
        env_ptr r(new env_t(e));
        gc_->register_env(r);
        r->pair_up(vals);
        return r;
    }

    env_ptr bind(env_ptr e)
    {
        env_ptr r(new env_t(e));
        gc_->register_env(r);
        return r;
    }

    void run_gc(cell_ptr c = cell_t::make_nil())
    {
        gc_->collect(c);
    }

private:
    typedef std::map<std::string, cell_ptr> map_type;

private:
    map_type             map_    ;
    env_ptr              parent_ ;
    std::size_t          visited_; // for garbage collection
    env_ptr              copy_   ;
    garbage_collector_t* gc_     ;
    bool                 root_   ;

    void reset_copy(cell_ptr c)
    {
        if ( c && c->copy_ )
        {
            cell_ptr cp = c->copy_;
            c->copy_.reset();
            reset_copy(cp);

            if ( c->env_ )
            {
                c->env_->reset_copy();
            }

            reset_copy(c->pair_.first);
            reset_copy(c->pair_.second);
        }
    }

    void reset_copy()
    {
        if ( copy_ )
        {
            env_ptr cp = copy_;
            copy_.reset();
            cp->reset_copy();

            FOR_EACH( it, map_ )
            {
                reset_copy(it->second);
            }

            if ( parent_ )
            {
                parent_->reset_copy();
            }
        }
    }

    static cell_ptr copy(cell_ptr o)
    {
        if ( o->copy_ )
        {
            return o->copy_;
        }

        cell_t* c = new cell_t(o->type_);
        c->copy_    = cell_ptr(c);
        o->copy_    = c->copy_;

        c->bool_    = o->bool_;
        c->number_  = o->number_;
        c->string_  = o->string_;
        c->pair_    = o->pair_;
        c->env_     = o->env_;
        c->effect_  = o->effect_;
        c->builtin_ = o->builtin_;

        if ( o->env_ )
        {
            c->env_ = o->env_->copy();
        }

        if ( c->pair_.first )
        {
            c->pair_.first = copy(c->pair_.first);
        }

        if ( c->pair_.second )
        {
            c->pair_.second = copy(c->pair_.second);
        }

        return o->copy_;
    }


    const env_ptr& copy()
    {
        if ( copy_ )
        {
            return copy_;
        }

        env_t* e = parent_ ? new env_t(parent_->copy()) : new env_t;
        copy_        = env_ptr(e);
        copy_->copy_ = copy_;

        if ( !root_ )
        {
            gc_ = copy_->parent_->gc_;
            gc_->register_env(copy_);
        }

        FOR_EACH( it, map_ )
        {
            e->map_[it->first] = copy(it->second);
        }

        return copy_;
    }


public:
    env_t()
        : map_()
        , parent_()
        , visited_()
        , copy_()
        , gc_(new garbage_collector_t(this))
        , root_(1)
    {
        ++nenv;
        ++nrenv;
        std::cout << "---> Nenv: " << nenv << ' ' << nrenv << "\n";
    }

    env_t( env_ptr parent )
        : map_()
        , parent_(parent)
        , visited_(false)
        , copy_()
        , gc_(parent->gc_)
        , root_(0)
    {
        ++nenv;
        std::cout << "---> Nenv: " << nenv << ' ' << nrenv << "\n";
    }

    ~env_t()
    {
        --nenv;
        if ( root_ ) --nrenv;
        std::cout << "---> Nenv: " << nenv << ' ' << nrenv << "\n";

        if ( root_ )
        {
            gc_->erase_all();
            delete gc_;
        }
    }

    void print()
    {
        FOR_EACH( it, map_ )
        {
            std::cout << it->first << '\n' << '\t'
                      << it->second << '\n';
        }
    }

    std::pair<cell_ptr,env_ptr> deep_copy(cell_ptr c)
    {
        assure(!parent_, "Can deep copy only top level environment");
        env_ptr   ec = copy();
        cell_ptr  cc = copy(c);
        reset_copy();
        reset_copy(c);
        reset_copy(cc);
        return std::make_pair(cc,ec);
    }

    static void visit(cell_ptr c, std::size_t iter)
    {
        if ( !c )
        {
            return;
        }

        if ( c->visited_ == iter )
        {
            return;
        }

        c->visited_ = iter;

        if ( c->env_ )
        {
            c->env_->visit(iter);
        }

        visit(c->pair_.first,  iter);
        visit(c->pair_.second, iter);
    }

    void visit(std::size_t iter)
    {
        if ( visited_ == iter )
        {
            return;
        }

        visited_ = iter;

        if ( parent_ )
        {
            parent_->visit(iter);
        }

        FOR_EACH( it, map_ )
        {
            visit(it->second, iter);
        }
    }

    void clear()
    {
        map_.clear();
        parent_.reset();
    }

    cell_ptr find(const std::string& name)
    {
        const map_type::const_iterator it = map_.find(name);
        if ( it != map_.end() )
        {
            return it->second;
        }
        return cell_ptr();
    }

    bool has(const std::string& name)
    {
        return map_.find(name) != map_.end();
    }

    void set(const std::string& name, const cell_ptr& cell)
    {
        map_[name] = cell;
    }

    cell_ptr& operator[](const std::string& name)
    {
        return map_[name];
    }

    env_ptr get_parent() const
    {
        return parent_;
    }

    void pair_up(cell_ptr vars, cell_ptr vals)
    {
        //std::cout << "Pairup: " << vars << ": " << vals << "\n";

        if ( vars->is_nil() )
        {
            assure(vals->is_nil(), "Too many values given");
            return;
        }

        if ( vars->is_symbol() )
        {
            assure(!has(vars->get_symbol()), "Symbol " + vars->get_symbol() + " already defined");
            set(vars->get_symbol(), vals);
        }
        else
        {
            if ( vals->is_nil() )
            {
                assure(false, "Too few arguments given");
            }

            assure(vars->is_pair(), "Argument list not well defined");
            assure(vals->is_pair(), "Value list not well defined");
            assure(vars->car()->is_symbol(), "Not a symbol");

            set(vars->car()->get_symbol(), vals->car());
            pair_up(vars->cdr(), vals->cdr());
        }
    }

    void pair_up(cell_ptr vals)
    {
        int i = 0;

        set("$$", vals);

        while ( !vals->is_nil() )
        {
            set("$" + boost::lexical_cast<std::string>(++i), vals->car());
            vals = vals->cdr();
        }
    }


}; // class env


inline std::pair<cell_ptr, env_ptr> lookup_env(env_ptr e, const std::string& what)
{
    while (e)
    {
        if ( e->has(what) )
        {
            return std::make_pair(e->find(what), e);
        }
        e = e->get_parent();
    }

    assure(false, "Symbol " + what + " not found");

    return std::make_pair(cell_t::make_undefined(), e);
}

inline cell_ptr lookup(env_ptr e, const std::string& what)
{
    return lookup_env(e, what).first;
}

} // namespace zoov


#endif // ZOOV_LISP_ENV_HPP
