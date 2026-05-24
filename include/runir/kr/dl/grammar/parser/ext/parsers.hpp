#ifndef RUNIR_KR_DL_GRAMMAR_PARSER_EXT_PARSERS_HPP_
#define RUNIR_KR_DL_GRAMMAR_PARSER_EXT_PARSERS_HPP_

#include "runir/kr/dl/grammar/parser/base/parsers.hpp"
#include "runir/kr/dl/grammar/parser/ext/ast.hpp"

#include <boost/spirit/home/x3.hpp>

namespace runir::kr::dl::grammar::parser::ext
{
namespace x3 = boost::spirit::x3;

using ext_concept_type = runir::kr::dl::grammar::parser::base::concept_type<runir::kr::dl::ExtFamilyTag>;
using ext_concept_root_type = runir::kr::dl::grammar::parser::base::concept_root_type<runir::kr::dl::ExtFamilyTag>;
using ext_role_type = runir::kr::dl::grammar::parser::base::role_type<runir::kr::dl::ExtFamilyTag>;
using ext_role_root_type = runir::kr::dl::grammar::parser::base::role_root_type<runir::kr::dl::ExtFamilyTag>;
using ext_concept_choice_type = runir::kr::dl::grammar::parser::base::concept_choice_type<runir::kr::dl::ExtFamilyTag>;
using ext_role_choice_type = runir::kr::dl::grammar::parser::base::role_choice_type<runir::kr::dl::ExtFamilyTag>;

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

BOOST_SPIRIT_DECLARE(ext_concept_type, ext_concept_root_type, ext_role_type, ext_role_root_type, ext_concept_choice_type, ext_role_choice_type)

BOOST_SPIRIT_DECLARE(concept_register_type, role_register_type, concept_argument_type, role_argument_type, boolean_argument_type, numerical_argument_type)

extern ext_concept_type const ext_concept;
extern ext_concept_root_type const ext_concept_root;
extern ext_role_type const ext_role;
extern ext_role_root_type const ext_role_root;
extern ext_concept_choice_type const ext_concept_choice;
extern ext_role_choice_type const ext_role_choice;

ext_concept_type const& ext_concept_parser();
ext_concept_root_type const& ext_concept_root_parser();
ext_role_type const& ext_role_parser();
ext_role_root_type const& ext_role_root_parser();

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
