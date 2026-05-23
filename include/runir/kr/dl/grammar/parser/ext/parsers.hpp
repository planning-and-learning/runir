#ifndef RUNIR_KR_DL_GRAMMAR_PARSER_EXT_PARSERS_HPP_
#define RUNIR_KR_DL_GRAMMAR_PARSER_EXT_PARSERS_HPP_

#include "runir/kr/dl/grammar/parser/ext/ast.hpp"

#include <boost/spirit/home/x3.hpp>

namespace runir::kr::dl::grammar::parser::ext
{
namespace x3 = boost::spirit::x3;

struct ConceptRegisterClass;

using concept_register_type = x3::rule<ConceptRegisterClass, ast::ConceptRegister>;

BOOST_SPIRIT_DECLARE(concept_register_type)

extern concept_register_type const concept_register;

concept_register_type const& concept_register_parser();

}  // namespace runir::kr::dl::grammar::parser::ext

#endif
