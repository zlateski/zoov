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
#include <boost/enable_shared_from_this.hpp>

#include "cell.hpp"

namespace zoov {

static int nenv  = 0;
static int nrenv = 0;
static int cps   = 0;

static int rcopy  = 0;


class env_t: public boost::enable_shared_from_this<env_t>
{
private:
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
            std::cout << "Garbage collector of Env #" << root_->env_id_ << " running\n";

            ++iter_;

            root_->visit(iter_);
            visit(c, iter_);

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

        void collect_all()
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

private:
    static void visit(cell_ptr c, std::size_t n)
    {
        if ( c && (c->visited_ < n) )
        {
            c->visited_ = n;
            if ( c->env_ )
            {
                c->env_->visit(n);
            }
            visit(c->pair_.first, n);
            visit(c->pair_.second, n);
        }
    }

    void visit(std::size_t n)
    {
        if ( visited_ < n )
        {
            visited_ = n;
            if ( parent_ )
            {
                parent_->visit(n);
            }
            FOR_EACH( it, map_ )
            {
                visit(it->second, n);
            }
        }
    }

    static void erase_all(cell_ptr c)
    {
        if ( c )
        {
            if ( c->env_ )
            {
                c->env_->erase_all();
                cell_ptr x;
                x.swap(c->pair_.first);
                erase_all(x);
                x.swap(c->pair_.second);
                erase_all(x);
            }
        }
    }

private:
    typedef std::map<std::string, cell_ptr>  map_t          ;
    typedef boost::shared_ptr<env_t> env_ptr;

private:
    map_t                map_    ;
    env_ptr              parent_ ;
    std::size_t          visited_;
    env_ptr              copy_   ;
    garbage_collector_t* gc_     ;
    bool                 root_   ;
    int                  env_id_ ;

public:
    env_t()
        : map_()
        , parent_()
        , visited_(0)
        , copy_()
        , gc_(new garbage_collector_t(this))
        , root_(1)
        , env_id_(nenv++)
    {
        ++nrenv;
        std::cout << "---> Created Root Env #" << env_id_ << "\n";
        std::cout << "---> Nenv: " << nenv << ' ' << nrenv << "\n";
    }

    explicit env_t(env_ptr p)
        : map_()
        , parent_(p)
        , visited_(0)
        , copy_()
        , gc_(p->gc_)
        , root_(0)
        , env_id_(nenv++)
    {
        std::cout << "---> Created Env #" << env_id_ << "\n";
        std::cout << "---> Nenv: " << nenv << ' ' << nrenv << "\n";
    }

    ~env_t()
    {
        --nenv;
        if ( root_ )
        {
            --nrenv;
            std::cout << "---> Erased Root Env #" << env_id_ << "\n";
        }
        else
        {
            std::cout << "---> Erased Env #" << env_id_ << "\n";
        }

        std::cout << "---> Nenv: " << nenv << ' ' << nrenv << "\n";

        if ( root_ )
        {
            gc_->collect_all();//clear();
        }

    }

    void clear()
    {
        map_.clear();
        parent_.reset();
        // map_t m;
        // m.swap(map_);

        // FOR_EACH( it, m )
        // {
        //     //clear(it->second);
        // }

        // env_ptr x;
        // x.swap(parent_);

        // if ( x )
        // {
        //     x->clear();
        // }
    }

    void erase_all()
    {
        map_t m;
        m.swap(map_);

        FOR_EACH( it, m )
        {
            erase_all(it->second);
        }

        env_ptr x;
        x.swap(parent_);

        if ( x )
        {
            x->erase_all();
        }
    }

    void bind(cell_ptr vars, cell_ptr vals)
    {
        if ( vars->is_nil() )
        {
            assure(vals->is_nil());
            return;
        }

        if ( vars->is_symbol() )
        {
            assure(!has(vars->get_symbol()));
            add(vars->get_symbol(), vals);
        }
        else
        {
            assure(!vals->is_nil());
            assure(vars->is_pair());
            assure(vals->is_pair());
            assure(vars->car()->is_symbol());

            add(vars->car()->get_symbol(), vals->car());
            bind(vars->cdr(), vals->cdr());
        }
    }

    std::pair<cell_ptr,env_ptr> clone(cell_ptr c = cell_ptr())
    {
        assure(!parent_);
        env_ptr   ec = copy();
        cell_ptr  cc = copy(c);
        reset_copy();
        reset_copy(c);
        reset_copy(cc);
        ec->reset_copy();
        return std::make_pair(cc,ec);
    }

    void bind(cell_ptr vals)
    {
        int i = 0;
        add("$$", vals);

        while ( !vals->is_nil() )
        {
            add("$" + boost::lexical_cast<std::string>(++i), vals->car());
            vals = vals->cdr();
        }
    }


    cell_ptr find(const std::string& name)
    {
        const map_t::const_iterator it = map_.find(name);
        if ( it != map_.end() )
        {
            return it->second;
        }
        return cell_ptr();
    }

    bool has(const std::string& name)
    {
        return map_.count(name);
    }

    void set(const std::string& name, const cell_ptr& cell)
    {
        map_[name] = cell;
    }

    void add(const std::string& name, const cell_ptr& cell)
    {
        assure(!map_.count(name));
        map_[name] = cell;
    }

    void print()
    {
        FOR_EACH( it, map_ )
        {
            std::cout << it->first << '\n' << '\t'
                      << it->second << '\n';
        }
    }

    env_ptr drop_frame(cell_ptr vars, cell_ptr vals)
    {
        env_ptr r(new env_t(shared_from_this()));
        gc_->register_env(r);
        r->bind(vars, vals);
        return r;
    }

    env_ptr drop_frame(cell_ptr vals)
    {
        env_ptr r(new env_t(shared_from_this()));
        gc_->register_env(r);
        r->bind(vals);
        return r;
    }

    env_ptr drop_frame()
    {
        env_ptr r(new env_t(shared_from_this()));
        gc_->register_env(r);
        return r;
    }

    env_ptr get_parent() const
    {
        return parent_;
    }

    std::pair<cell_ptr, env_ptr> lookup_env(const std::string& what)
    {
        env_ptr e = shared_from_this();
        while (e)
        {
            if ( e->has(what) )
            {
                return std::make_pair(e->find(what), e);
            }
            e = e->get_parent();
        }

        assure(false);
        return std::make_pair(cell_t::make_undefined(), e);
    }

    cell_ptr lookup(const std::string& what)
    {
        return lookup_env(what).first;
    }

    void run_gc(cell_ptr c = cell_t::make_nil())
    {
        gc_->collect(c);
    }

private:
    void reset_copy(cell_ptr c)
    {
        if ( c && c->copy_ )
        {
            rcopy--;
            std::cout << "RCP: " << rcopy << '\n';

            cell_ptr cp;
            cp.swap(c->copy_);

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
            rcopy--;
            std::cout << "RCP: " << rcopy << '\n';

            env_ptr cp;
            cp.swap(copy_);
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

    cell_ptr copy(cell_ptr o)
    {
        if ( o->copy_ )
        {
            return o->copy_;
        }

        rcopy += 2;
        std::cout << "RCP: " << rcopy << '\n';

        cell_t* c = new cell_t(o->type_);
        c->copy_    = cell_ptr(c);
        o->copy_    = c->copy_;

        c->bool_    = o->bool_;
        c->number_  = o->number_;
        c->string_  = o->string_;
        c->effect_  = o->effect_;
        c->builtin_ = o->builtin_;

        if ( o->env_ )
        {
            c->env_ = o->env_->copy();
        }

        if ( o->pair_.first )
        {
            c->pair_.first = copy(o->pair_.first);
        }

        if ( o->pair_.second )
        {
            c->pair_.second = copy(o->pair_.second);
        }

        return o->copy_;
    }

    env_ptr copy()
    {
        if ( copy_ )
        {
            return copy_;
        }

        rcopy += 2;
        std::cout << "RCP: " << rcopy << '\n';

        env_t* e = parent_ ? new env_t(parent_->copy()) : new env_t;
        copy_        = env_ptr(e);
        copy_->copy_ = copy_;

        if ( !root_ )
        {
            e->gc_ = copy_->parent_->gc_;
            e->gc_->register_env(copy_);
        }

        FOR_EACH( it, map_ )
        {
            e->map_[it->first] = copy(it->second);
        }

        return copy_;
    }
};

} // namespace zoov


#endif // ZOOV_LISP_ENV_HPP
