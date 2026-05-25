#ifndef RUNIR_KR_DL_GRAMMAR_PARSER_EXT_PARSERS_HPP_
#define RUNIR_KR_DL_GRAMMAR_PARSER_EXT_PARSERS_HPP_

#include "runir/kr/dl/grammar/parser/error_handler.hpp"
#include "runir/kr/dl/grammar/parser/ext/ast.hpp"

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>

namespace runir::kr::dl::grammar::parser::ext
{
namespace x3 = boost::spirit::x3;

template<runir::kr::dl::CategoryTag Category>
struct ConstructorClass : x3::annotate_on_success
{
};

template<runir::kr::dl::CategoryTag Category>
struct ConstructorRootClass : ErrorHandlerBase
{
};

template<runir::kr::dl::CategoryTag Category>
struct ConstructorOrNonTerminalClass : x3::annotate_on_success
{
};

template<runir::kr::dl::CategoryTag Category>
struct NonTerminalClass : x3::annotate_on_success
{
};

using ext_concept_type =
    x3::rule<ConstructorClass<runir::kr::dl::ConceptTag>, runir::kr::dl::grammar::parser::base::ast::ConceptConstructor<runir::kr::ExtFamilyTag>>;
using ext_concept_root_type =
    x3::rule<ConstructorRootClass<runir::kr::dl::ConceptTag>, runir::kr::dl::grammar::parser::base::ast::ConceptConstructor<runir::kr::ExtFamilyTag>>;
using ext_role_type = x3::rule<ConstructorClass<runir::kr::dl::RoleTag>, runir::kr::dl::grammar::parser::base::ast::RoleConstructor<runir::kr::ExtFamilyTag>>;
using ext_role_root_type =
    x3::rule<ConstructorRootClass<runir::kr::dl::RoleTag>, runir::kr::dl::grammar::parser::base::ast::RoleConstructor<runir::kr::ExtFamilyTag>>;
using ext_concept_non_terminal_type = x3::rule<NonTerminalClass<runir::kr::dl::ConceptTag>,
                                               runir::kr::dl::grammar::parser::base::ast::NonTerminal<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>;
using ext_role_non_terminal_type =
    x3::rule<NonTerminalClass<runir::kr::dl::RoleTag>, runir::kr::dl::grammar::parser::base::ast::NonTerminal<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>>;
using ext_concept_choice_type =
    x3::rule<ConstructorOrNonTerminalClass<runir::kr::dl::ConceptTag>, runir::kr::dl::grammar::parser::base::ast::ConceptChoice<runir::kr::ExtFamilyTag>>;
using ext_role_choice_type =
    x3::rule<ConstructorOrNonTerminalClass<runir::kr::dl::RoleTag>, runir::kr::dl::grammar::parser::base::ast::RoleChoice<runir::kr::ExtFamilyTag>>;

struct ConceptRegisterClass;
struct RoleRegisterClass;
template<runir::kr::dl::CategoryTag Category>
struct ArgumentClass;

using concept_register_type = x3::rule<ConceptRegisterClass, ast::ConceptRegister>;
using role_register_type = x3::rule<RoleRegisterClass, ast::RoleRegister>;
template<runir::kr::dl::CategoryTag Category>
using argument_type = x3::rule<ArgumentClass<Category>, ast::Argument<Category>>;
using concept_argument_type = argument_type<runir::kr::dl::ConceptTag>;
using role_argument_type = argument_type<runir::kr::dl::RoleTag>;
using boolean_argument_type = argument_type<runir::kr::dl::BooleanTag>;
using numerical_argument_type = argument_type<runir::kr::dl::NumericalTag>;

BOOST_SPIRIT_DECLARE(ext_concept_type,
                     ext_concept_root_type,
                     ext_role_type,
                     ext_role_root_type,
                     ext_concept_non_terminal_type,
                     ext_role_non_terminal_type,
                     ext_concept_choice_type,
                     ext_role_choice_type)

BOOST_SPIRIT_DECLARE(concept_register_type, role_register_type, concept_argument_type, role_argument_type, boolean_argument_type, numerical_argument_type)

extern ext_concept_type const ext_concept;
extern ext_concept_root_type const ext_concept_root;
extern ext_role_type const ext_role;
extern ext_role_root_type const ext_role_root;
extern ext_concept_non_terminal_type const ext_concept_non_terminal;
extern ext_role_non_terminal_type const ext_role_non_terminal;
extern ext_concept_choice_type const ext_concept_choice;
extern ext_role_choice_type const ext_role_choice;

ext_concept_type const& ext_concept_parser();
ext_concept_root_type const& ext_concept_root_parser();
ext_role_type const& ext_role_parser();
ext_role_root_type const& ext_role_root_parser();
ext_concept_non_terminal_type const& ext_concept_non_terminal_parser();
ext_role_non_terminal_type const& ext_role_non_terminal_parser();
ext_concept_choice_type const& ext_concept_choice_parser();
ext_role_choice_type const& ext_role_choice_parser();

extern concept_register_type const concept_register;
extern role_register_type const role_register;
extern concept_argument_type const concept_argument;
extern role_argument_type const role_argument;
extern boolean_argument_type const boolean_argument;
extern numerical_argument_type const numerical_argument;

concept_register_type const& concept_register_parser();
role_register_type const& role_register_parser();
concept_argument_type const& concept_argument_parser();
role_argument_type const& role_argument_parser();
boolean_argument_type const& boolean_argument_parser();
numerical_argument_type const& numerical_argument_parser();

}  // namespace runir::kr::dl::grammar::parser::ext

#endif
