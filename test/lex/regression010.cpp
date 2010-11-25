//  Copyright (c) 2001-2010 Hartmut Kaiser
//  Copyright (c) 2010 Mathias Gaunard
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// This test makes sure that the BOL state (begin of line) is properly reset
// if a token matched at the beginning of a line is discarded using 
// lex::pass_fail.

// #define BOOST_SPIRIT_LEXERTL_DEBUG 1

#include <boost/config/warning_disable.hpp>
#include <boost/detail/lightweight_test.hpp>

#include <boost/spirit/include/support_multi_pass.hpp>
#include <boost/spirit/include/classic_position_iterator.hpp>
#include <boost/spirit/include/lex_lexertl.hpp>

#include <boost/spirit/home/phoenix/core.hpp>
#include <boost/spirit/home/phoenix/operator.hpp>
#include <boost/spirit/home/phoenix/statement.hpp>
#include <boost/spirit/home/phoenix/object.hpp>
#include <boost/spirit/home/phoenix/stl.hpp>

namespace spirit = boost::spirit;
namespace lex = spirit::lex;
namespace phoenix = boost::phoenix;

typedef spirit::classic::position_iterator2<
    spirit::multi_pass<std::istreambuf_iterator<char> >
> file_iterator;

inline file_iterator 
make_file_iterator(std::istream& input, const std::string& filename)
{
    return file_iterator(
        spirit::make_default_multi_pass(
            std::istreambuf_iterator<char>(input)),
        spirit::multi_pass<std::istreambuf_iterator<char> >(),
        filename);
}

typedef lex::lexertl::token<file_iterator> token_type;

struct lexer
  : lex::lexer<lex::lexertl::actor_lexer<token_type> >
{
    lexer() : word("^[a-zA-Z0-9]+$", 1)
    {
        self =  word [ 
                    lex::_state = "O" 
                ]
            |   lex::token_def<>("!.*$") [ 
                    lex::_state = "O"
                  , lex::_pass = lex::pass_flags::pass_ignore 
                ]
            |   lex::token_def<>('\n', 2) [ 
                    lex::_state = "O" 
                ] 
            ;
        
        self("O") = 
                lex::token_def<>(".") [ 
                    lex::_state = "INITIAL"
                  , lex::_pass = lex::pass_flags::pass_fail 
                ]
            ;
    }
    
    lex::token_def<> word;
};

typedef lexer::iterator_type token_iterator;

int main()
{
    std::stringstream ss;
    ss << "!foo\nbar\n!baz";
    
    file_iterator begin = make_file_iterator(ss, "SS");
    file_iterator end;
    
    lexer l;
    token_iterator begin2 = l.begin(begin, end);
    token_iterator end2 = l.end();
    
    int test_data[] = { 2, 1, 2 };
    std::size_t const test_data_size = sizeof(test_data)/sizeof(test_data[0]);

    token_iterator it = begin2;
    int i = 0;
    for (/**/; it != end2 && i < test_data_size; ++it, ++i)
    {
        BOOST_TEST(it->id() == test_data[i]);
    }
    BOOST_TEST(it == end2);
    BOOST_TEST(i == test_data_size);

    return boost::report_errors();
}
