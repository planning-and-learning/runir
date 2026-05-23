#ifndef RUNIR_KR_DL_GRAMMAR_PARSER_BASE_PARSERS_HPP_
#define RUNIR_KR_DL_GRAMMAR_PARSER_BASE_PARSERS_HPP_

#include "runir/kr/dl/grammar/parser/base/ast.hpp"

#include <boost/spirit/home/x3.hpp>

namespace runir::kr::dl::grammar::parser::base
{
namespace x3 = boost::spirit::x3;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct ConstructorClass;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct ConstructorRootClass;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct NonTerminalClass;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct ConstructorOrNonTerminalClass;

template<runir::kr::dl::FamilyTag Family>
using concept_type = x3::rule<ConstructorClass<Family, runir::kr::dl::ConceptTag>, ast::ConceptConstructor<Family>>;

template<runir::kr::dl::FamilyTag Family>
using concept_root_type = x3::rule<ConstructorRootClass<Family, runir::kr::dl::ConceptTag>, ast::ConceptConstructor<Family>>;

template<runir::kr::dl::FamilyTag Family>
using role_type = x3::rule<ConstructorClass<Family, runir::kr::dl::RoleTag>, ast::RoleConstructor<Family>>;

template<runir::kr::dl::FamilyTag Family>
using role_root_type = x3::rule<ConstructorRootClass<Family, runir::kr::dl::RoleTag>, ast::RoleConstructor<Family>>;

template<runir::kr::dl::FamilyTag Family>
using concept_choice_type = x3::rule<ConstructorOrNonTerminalClass<Family, runir::kr::dl::ConceptTag>, ast::ConceptChoice<Family>>;

template<runir::kr::dl::FamilyTag Family>
using role_choice_type = x3::rule<ConstructorOrNonTerminalClass<Family, runir::kr::dl::RoleTag>, ast::RoleChoice<Family>>;

using base_concept_type = concept_type<runir::kr::dl::BaseFamilyTag>;
using base_concept_root_type = concept_root_type<runir::kr::dl::BaseFamilyTag>;
using base_role_type = role_type<runir::kr::dl::BaseFamilyTag>;
using base_role_root_type = role_root_type<runir::kr::dl::BaseFamilyTag>;
using base_concept_choice_type = concept_choice_type<runir::kr::dl::BaseFamilyTag>;
using base_role_choice_type = role_choice_type<runir::kr::dl::BaseFamilyTag>;

using ext_concept_type = concept_type<runir::kr::dl::ExtFamilyTag>;
using ext_concept_root_type = concept_root_type<runir::kr::dl::ExtFamilyTag>;
using ext_role_type = role_type<runir::kr::dl::ExtFamilyTag>;
using ext_role_root_type = role_root_type<runir::kr::dl::ExtFamilyTag>;
using ext_concept_choice_type = concept_choice_type<runir::kr::dl::ExtFamilyTag>;
using ext_role_choice_type = role_choice_type<runir::kr::dl::ExtFamilyTag>;

BOOST_SPIRIT_DECLARE(base_concept_type, base_concept_root_type, base_role_type, base_role_root_type, base_concept_choice_type, base_role_choice_type)
BOOST_SPIRIT_DECLARE(ext_concept_type, ext_concept_root_type, ext_role_type, ext_role_root_type, ext_concept_choice_type, ext_role_choice_type)

base_concept_type const& base_concept_parser();
base_concept_root_type const& base_concept_root_parser();
base_role_type const& base_role_parser();
base_role_root_type const& base_role_root_parser();

ext_concept_type const& ext_concept_parser();
ext_concept_root_type const& ext_concept_root_parser();
ext_role_type const& ext_role_parser();
ext_role_root_type const& ext_role_root_parser();

}  // namespace runir::kr::dl::grammar::parser::base

#endif
