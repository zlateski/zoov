#ifndef ZOOV_SCHEME_PARSE_HPP
#define ZOOV_SCHEME_PARSE_HPP

#include "scheme.hpp"

#include <boost/lexical_cast.hpp>
#include <list>

namespace zoov {

inline std::list<std::string> tokenize(const std::string & str)
{
    std::list<std::string> tokens;
    const char* s = str.c_str();

    while (*s)
    {
        while (*s == ' ' || *s == '\n' || *s == '\r' || *s == '\t')
        {
            ++s;
        }

        if ( *s == '"' )
        {
            const char* t = s;

            ++t;
            while ( *t && *t != '"' )
            {
                ++t;
            }

            tokens.push_back(std::string(s, t));

            ++t;
            s = t;
        }
        else if (*s == '(' || *s == ')')
        {
            tokens.push_back(*s++ == '(' ? "(" : ")");
        }
        else
        {
            const char* t = s;
            while (*t && *t != ' ' && *t != '(' && *t != ')')
            {
                ++t;
            }

            tokens.push_back(std::string(s, t));

            s = t;
        }
    }
    return tokens;
}

inline cell_ptr atom(const std::string& token)
{
    try
    {
        double v = boost::lexical_cast<double>(token);
        return cell_t::make_number(v);
    }
    catch(boost::bad_lexical_cast&)
    {
        if ( token[0] == '"' )
        {
            return cell_t::make_string(token.substr(1));
        }
        return cell_t::make_symbol(token);
    }
}

inline cell_ptr parse(std::list<std::string>&);

inline cell_ptr parse_list(std::list<std::string>& l)
{
    if ( l.size() == 0 )
    {
        return cell_t::make_nil();
    }

    if ( l.front() == ")" )
    {
        l.pop_front();
        return cell_t::make_nil();
    }

    cell_ptr c;

    if ( l.front() == "." )
    {
        l.pop_front();
        c = parse(l);

        assure( l.front() == ")", "Syntax error");

        l.pop_front();
        return c;
    }

    if ( l.front() == "(" )
    {
        l.pop_front();
        c = parse_list(l);
    }
    else
    {
        c = atom(l.front());
        l.pop_front();
    }

    return cell_t::make_pair(c, parse_list(l));
}

inline cell_ptr parse(std::list<std::string>& l)
{
    if ( l.front() == "(" )
    {
        l.pop_front();
        return parse_list(l);
    }
    else
    {
        cell_ptr c = atom(l.front());
        l.pop_front();
        return c;
    }
}

} // namespace zoov


#endif // ZOOV_SCHEME_PARSE_HPP
