#ifndef RUNIR_SRC_KR_DL_GRAMMAR_PARSER_EXT_PARSER_DEF_HPP_
#define RUNIR_SRC_KR_DL_GRAMMAR_PARSER_EXT_PARSER_DEF_HPP_

#include "runir/kr/dl/grammar/parser/base/ast_adapted.hpp"
#include "runir/kr/dl/grammar/parser/error_handler.hpp"
#include "runir/kr/dl/grammar/parser/ext/ast_adapted.hpp"
#include "runir/kr/dl/grammar/parser/ext/parsers.hpp"

#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>

namespace runir::kr::dl::grammar::parser::ext
{
namespace x3 = boost::spirit::x3;

using x3::eoi;
using x3::lit;
using x3::uint_;

template<typename Ast, typename Parser>
auto constructor_identifier_parser(Parser parser)
{
    return (((lit("(") >> lit(Ast::keyword)) > parser > lit(")")) | (lit(Ast::keyword) > parser));
}

ext_concept_type const ext_concept = "ext_concept";
ext_concept_root_type const ext_concept_root = "ext_concept_root";
ext_role_type const ext_role = "ext_role";
ext_role_root_type const ext_role_root = "ext_role_root";
ext_concept_choice_type const ext_concept_choice = "ext_concept_choice";
ext_role_choice_type const ext_role_choice = "ext_role_choice";

concept_register_type const concept_register = "concept_register";
role_register_type const role_register = "role_register";
concept_argument_type const concept_argument = "concept_argument";
role_argument_type const role_argument = "role_argument";
boolean_argument_type const boolean_argument = "boolean_argument";
numerical_argument_type const numerical_argument = "numerical_argument";

template<typename Parser>
auto with_constructor_parentheses(Parser parser)
{
    return (lit("(") >> parser >> lit(")")) | parser;
}

const auto ext_role_def = with_constructor_parentheses(lit(runir::kr::dl::grammar::parser::base::ast::RoleUniversal::keyword)
                                                       >> x3::attr(runir::kr::dl::grammar::parser::base::ast::RoleUniversal {}))
                          | role_register | role_argument;
const auto ext_role_root_def = ext_role > eoi;
const auto ext_role_choice_def = ext_role;

const auto ext_concept_def = with_constructor_parentheses(lit(runir::kr::dl::grammar::parser::base::ast::ConceptBot::keyword)
                                                          >> x3::attr(runir::kr::dl::grammar::parser::base::ast::ConceptBot {}))
                             | with_constructor_parentheses(lit(runir::kr::dl::grammar::parser::base::ast::ConceptTop::keyword)
                                                            >> x3::attr(runir::kr::dl::grammar::parser::base::ast::ConceptTop {}))
                             | concept_register | concept_argument;
const auto ext_concept_root_def = ext_concept > eoi;
const auto ext_concept_choice_def = ext_concept;

const auto concept_register_def = constructor_identifier_parser<ast::ConceptRegister>(uint_);
const auto role_register_def = constructor_identifier_parser<ast::RoleRegister>(uint_);
const auto concept_argument_def = constructor_identifier_parser<ast::Argument<runir::kr::dl::ConceptTag>>(uint_);
const auto role_argument_def = constructor_identifier_parser<ast::Argument<runir::kr::dl::RoleTag>>(uint_);
const auto boolean_argument_def = constructor_identifier_parser<ast::Argument<runir::kr::dl::BooleanTag>>(uint_);
const auto numerical_argument_def = constructor_identifier_parser<ast::Argument<runir::kr::dl::NumericalTag>>(uint_);

BOOST_SPIRIT_DEFINE(ext_concept, ext_concept_root, ext_role, ext_role_root, ext_concept_choice, ext_role_choice)
BOOST_SPIRIT_DEFINE(concept_register, role_register, concept_argument, role_argument, boolean_argument, numerical_argument)

struct ConceptRegisterClass : x3::annotate_on_success
{
};

struct RoleRegisterClass : x3::annotate_on_success
{
};

template<runir::kr::dl::CategoryTag Category>
struct ArgumentClass : x3::annotate_on_success
{
};

ext_concept_type const& ext_concept_parser() { return ext_concept; }
ext_concept_root_type const& ext_concept_root_parser() { return ext_concept_root; }
ext_role_type const& ext_role_parser() { return ext_role; }
ext_role_root_type const& ext_role_root_parser() { return ext_role_root; }

concept_register_type const& concept_register_parser() { return concept_register; }
role_register_type const& role_register_parser() { return role_register; }
concept_argument_type const& concept_argument_parser() { return concept_argument; }
role_argument_type const& role_argument_parser() { return role_argument; }
boolean_argument_type const& boolean_argument_parser() { return boolean_argument; }
numerical_argument_type const& numerical_argument_parser() { return numerical_argument; }

}  // namespace runir::kr::dl::grammar::parser::ext

#endif
