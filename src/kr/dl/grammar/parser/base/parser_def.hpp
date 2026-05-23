#ifndef RUNIR_SRC_KR_DL_GRAMMAR_PARSER_GENERIC_PARSER_DEF_HPP_
#define RUNIR_SRC_KR_DL_GRAMMAR_PARSER_GENERIC_PARSER_DEF_HPP_

#include "runir/kr/dl/grammar/parser/ext/ast_adapted.hpp"
#include "runir/kr/dl/grammar/parser/base/ast_adapted.hpp"
#include "runir/kr/dl/grammar/parser/base/parsers.hpp"
#include "runir/kr/dl/grammar/parser/error_handler.hpp"
#include "runir/kr/dl/grammar/parser/ext/parsers.hpp"

#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>

namespace runir::kr::dl::grammar::parser::ext
{
namespace x3 = boost::spirit::x3;

using x3::lit;
using x3::uint_;

concept_register_type const concept_register = "concept_register";
const auto concept_register_def = (((lit("(") >> lit(ast::ConceptRegister::keyword)) > uint_ > lit(")")) | (lit(ast::ConceptRegister::keyword) > uint_));

BOOST_SPIRIT_DEFINE(concept_register)

struct ConceptRegisterClass : x3::annotate_on_success
{
};

concept_register_type const& concept_register_parser() { return concept_register; }

}  // namespace runir::kr::dl::grammar::parser::ext

namespace runir::kr::dl::grammar::parser::base
{
namespace x3 = boost::spirit::x3;

using x3::eoi;
using x3::lexeme;
using x3::lit;
using x3::omit;
using x3::raw;
using x3::ascii::alnum;
using x3::ascii::alpha;
using x3::ascii::char_;
using x3::ascii::digit;

base_concept_type const base_concept = "base_concept";
base_concept_root_type const base_concept_root = "base_concept_root";
base_role_type const base_role = "base_role";
base_role_root_type const base_role_root = "base_role_root";
base_concept_choice_type const base_concept_choice = "base_concept_choice";
base_role_choice_type const base_role_choice = "base_role_choice";

ext_concept_type const ext_concept = "ext_concept";
ext_concept_root_type const ext_concept_root = "ext_concept_root";
ext_role_type const ext_role = "ext_role";
ext_role_root_type const ext_role_root = "ext_role_root";
ext_concept_choice_type const ext_concept_choice = "ext_concept_choice";
ext_role_choice_type const ext_role_choice = "ext_role_choice";

inline auto concept_non_terminal_string_parser() { return raw[lexeme[lit("c_") >> +digit]]; }
inline auto role_non_terminal_string_parser() { return raw[lexeme[lit("r_") >> +digit]]; }
inline auto predicate_name_string_parser()
{
    return lexeme[omit[lit('"')]] > raw[lexeme[(alpha >> *(alnum | char_('-') | char_('_'))) | char_('=')]] > lexeme[omit[lit('"')]];
}
inline auto object_name_string_parser() { return lexeme[omit[lit('"')]] > raw[lexeme[alpha >> *(alnum | char_('-') | char_('_'))]] > lexeme[omit[lit('"')]]; }
inline auto true_string_parser() { return x3::lit(runir::kr::dl::TrueTag::keyword) >> x3::attr(true); }
inline auto false_string_parser() { return x3::lit(runir::kr::dl::FalseTag::keyword) >> x3::attr(false); }
inline auto bool_string_parser() { return x3::lexeme[true_string_parser() | false_string_parser()]; }

template<typename Parser>
auto with_constructor_parentheses(Parser parser)
{
    return (lit("(") >> parser >> lit(")")) | parser;
}

const auto base_role_def = with_constructor_parentheses(lit(ast::RoleUniversal::keyword) >> x3::attr(ast::RoleUniversal {}));
const auto base_role_root_def = base_role > eoi;
const auto base_role_choice_def = base_role;

const auto ext_role_def = with_constructor_parentheses(lit(ast::RoleUniversal::keyword) >> x3::attr(ast::RoleUniversal {}));
const auto ext_role_root_def = ext_role > eoi;
const auto ext_role_choice_def = ext_role;

const auto base_concept_def = with_constructor_parentheses(lit(ast::ConceptBot::keyword) >> x3::attr(ast::ConceptBot {}))
                              | with_constructor_parentheses(lit(ast::ConceptTop::keyword) >> x3::attr(ast::ConceptTop {}));
const auto base_concept_root_def = base_concept > eoi;
const auto base_concept_choice_def = base_concept;

const auto ext_concept_def = with_constructor_parentheses(lit(ast::ConceptBot::keyword) >> x3::attr(ast::ConceptBot {}))
                             | with_constructor_parentheses(lit(ast::ConceptTop::keyword) >> x3::attr(ast::ConceptTop {}))
                             | runir::kr::dl::grammar::parser::ext::concept_register;
const auto ext_concept_root_def = ext_concept > eoi;
const auto ext_concept_choice_def = ext_concept;

BOOST_SPIRIT_DEFINE(base_concept, base_concept_root, base_role, base_role_root, base_concept_choice, base_role_choice)
BOOST_SPIRIT_DEFINE(ext_concept, ext_concept_root, ext_role, ext_role_root, ext_concept_choice, ext_role_choice)

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct ConstructorClass : x3::annotate_on_success
{
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct ConstructorRootClass : ErrorHandlerBase
{
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct ConstructorOrNonTerminalClass : x3::annotate_on_success
{
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct NonTerminalClass : x3::annotate_on_success
{
};

base_concept_type const& base_concept_parser() { return base_concept; }
base_concept_root_type const& base_concept_root_parser() { return base_concept_root; }
base_role_type const& base_role_parser() { return base_role; }
base_role_root_type const& base_role_root_parser() { return base_role_root; }
ext_concept_type const& ext_concept_parser() { return ext_concept; }
ext_concept_root_type const& ext_concept_root_parser() { return ext_concept_root; }
ext_role_type const& ext_role_parser() { return ext_role; }
ext_role_root_type const& ext_role_root_parser() { return ext_role_root; }

}  // namespace runir::kr::dl::grammar::parser::base

#endif
