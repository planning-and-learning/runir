#ifndef RUNIR_KR_DL_GRAMMAR_PARSER_PARSERS_HPP_
#define RUNIR_KR_DL_GRAMMAR_PARSER_PARSERS_HPP_

#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/dl/grammar/parser/error_handler.hpp"

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>
#include <vector>

namespace runir::kr::dl::grammar::parser
{
namespace x3 = boost::spirit::x3;
namespace ast = runir::kr::dl::grammar::ast;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct ConstructorClass : x3::annotate_on_success
{
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct ConstructorRootClass : ErrorHandlerBase
{
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct NonTerminalClass : x3::annotate_on_success
{
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct ConstructorOrNonTerminalClass : x3::annotate_on_success
{
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct DerivationRuleClass : x3::annotate_on_success
{
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename Tag>
struct ConstructorTagClass : x3::annotate_on_success
{
};

template<runir::kr::dl::FamilyTag Family>
struct ConstructorOrNonTerminalVariantClass : x3::annotate_on_success
{
};

template<runir::kr::dl::FamilyTag Family>
struct DerivationRuleVariantClass : x3::annotate_on_success
{
};

template<runir::kr::dl::FamilyTag Family>
struct GrammarBodyClass : x3::annotate_on_success
{
};

template<runir::kr::dl::FamilyTag Family>
struct GrammarClass : x3::annotate_on_success
{
};

template<runir::kr::dl::FamilyTag Family>
struct GrammarRootClass : ErrorHandlerBase
{
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
using constructor_type = x3::rule<ConstructorClass<Family, Category>, ast::Constructor<Family, Category>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
using constructor_root_type = x3::rule<ConstructorRootClass<Family, Category>, ast::Constructor<Family, Category>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
using non_terminal_type = x3::rule<NonTerminalClass<Family, Category>, ast::NonTerminal<Family, Category>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
using choice_type = x3::rule<ConstructorOrNonTerminalClass<Family, Category>, ast::ConstructorOrNonTerminal<Family, Category>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
using derivation_rule_type = x3::rule<DerivationRuleClass<Family, Category>, ast::DerivationRule<Family, Category>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename Tag>
using constructor_tag_type = x3::rule<ConstructorTagClass<Family, Category, Tag>, ast::Constructor<Family, Category, Tag>>;

template<runir::kr::dl::FamilyTag Family>
using constructor_or_non_terminal_variant_type = x3::rule<ConstructorOrNonTerminalVariantClass<Family>, ast::ConstructorOrNonTerminalVariant<Family>>;

template<runir::kr::dl::FamilyTag Family>
using derivation_rule_variant_type = x3::rule<DerivationRuleVariantClass<Family>, ast::DerivationRuleVariant<Family>>;

template<runir::kr::dl::FamilyTag Family>
using grammar_body_type = x3::rule<GrammarBodyClass<Family>, std::vector<ast::DerivationRuleVariant<Family>>>;

template<runir::kr::dl::FamilyTag Family>
using grammar_type = x3::rule<GrammarClass<Family>, ast::Grammar<Family>>;

template<runir::kr::dl::FamilyTag Family>
using grammar_root_type = x3::rule<GrammarRootClass<Family>, ast::Grammar<Family>>;

template<runir::kr::dl::FamilyTag Family>
using concept_type = constructor_type<Family, runir::kr::dl::ConceptTag>;

template<runir::kr::dl::FamilyTag Family>
using concept_root_type = constructor_root_type<Family, runir::kr::dl::ConceptTag>;

template<runir::kr::dl::FamilyTag Family>
using role_type = constructor_type<Family, runir::kr::dl::RoleTag>;

template<runir::kr::dl::FamilyTag Family>
using role_root_type = constructor_root_type<Family, runir::kr::dl::RoleTag>;

template<runir::kr::dl::FamilyTag Family>
using boolean_type = constructor_type<Family, runir::kr::dl::BooleanTag>;

template<runir::kr::dl::FamilyTag Family>
using boolean_root_type = constructor_root_type<Family, runir::kr::dl::BooleanTag>;

template<runir::kr::dl::FamilyTag Family>
using numerical_type = constructor_type<Family, runir::kr::dl::NumericalTag>;

template<runir::kr::dl::FamilyTag Family>
using numerical_root_type = constructor_root_type<Family, runir::kr::dl::NumericalTag>;


#define RUNIR_DECLARE_RULE_ALIASES(Prefix, Family)                                                                                                  \
    using Prefix##_concept_root_type = constructor_root_type<Family, runir::kr::dl::ConceptTag>;                                                    \
    using Prefix##_role_root_type = constructor_root_type<Family, runir::kr::dl::RoleTag>;                                                          \
    using Prefix##_boolean_root_type = constructor_root_type<Family, runir::kr::dl::BooleanTag>;                                                    \
    using Prefix##_numerical_root_type = constructor_root_type<Family, runir::kr::dl::NumericalTag>;                                                \
    using Prefix##_concept_type = constructor_type<Family, runir::kr::dl::ConceptTag>;                                                              \
    using Prefix##_role_type = constructor_type<Family, runir::kr::dl::RoleTag>;                                                                    \
    using Prefix##_boolean_type = constructor_type<Family, runir::kr::dl::BooleanTag>;                                                              \
    using Prefix##_numerical_type = constructor_type<Family, runir::kr::dl::NumericalTag>;                                                          \
    using Prefix##_concept_choice_type = choice_type<Family, runir::kr::dl::ConceptTag>;                                                            \
    using Prefix##_role_choice_type = choice_type<Family, runir::kr::dl::RoleTag>;                                                                  \
    using Prefix##_boolean_choice_type = choice_type<Family, runir::kr::dl::BooleanTag>;                                                            \
    using Prefix##_numerical_choice_type = choice_type<Family, runir::kr::dl::NumericalTag>;                                                        \
    using Prefix##_grammar_root_type = grammar_root_type<Family>;

RUNIR_DECLARE_RULE_ALIASES(family0, runir::kr::BaseFamilyTag)
RUNIR_DECLARE_RULE_ALIASES(family1, runir::kr::ExtFamilyTag)
RUNIR_DECLARE_RULE_ALIASES(family2, runir::kr::UnsFamilyTag)

#undef RUNIR_DECLARE_RULE_ALIASES

BOOST_SPIRIT_DECLARE(family0_concept_root_type,
                     family0_role_root_type,
                     family0_boolean_root_type,
                     family0_numerical_root_type,
                     family0_concept_type,
                     family0_role_type,
                     family0_boolean_type,
                     family0_numerical_type,
                     family0_concept_choice_type,
                     family0_role_choice_type,
                     family0_boolean_choice_type,
                     family0_numerical_choice_type,
                     family0_grammar_root_type)

BOOST_SPIRIT_DECLARE(family1_concept_root_type,
                     family1_role_root_type,
                     family1_boolean_root_type,
                     family1_numerical_root_type,
                     family1_concept_type,
                     family1_role_type,
                     family1_boolean_type,
                     family1_numerical_type,
                     family1_concept_choice_type,
                     family1_role_choice_type,
                     family1_boolean_choice_type,
                     family1_numerical_choice_type,
                     family1_grammar_root_type)

BOOST_SPIRIT_DECLARE(family2_concept_root_type,
                     family2_role_root_type,
                     family2_boolean_root_type,
                     family2_numerical_root_type,
                     family2_concept_type,
                     family2_role_type,
                     family2_boolean_type,
                     family2_numerical_type,
                     family2_concept_choice_type,
                     family2_role_choice_type,
                     family2_boolean_choice_type,
                     family2_numerical_choice_type,
                     family2_grammar_root_type)

template<runir::kr::dl::FamilyTag Family>
concept_type<Family> const& concept_parser();

template<runir::kr::dl::FamilyTag Family>
concept_root_type<Family> const& concept_root_parser();

template<runir::kr::dl::FamilyTag Family>
role_type<Family> const& role_parser();

template<runir::kr::dl::FamilyTag Family>
role_root_type<Family> const& role_root_parser();

template<runir::kr::dl::FamilyTag Family>
boolean_type<Family> const& boolean_parser();

template<runir::kr::dl::FamilyTag Family>
boolean_root_type<Family> const& boolean_root_parser();

template<runir::kr::dl::FamilyTag Family>
numerical_type<Family> const& numerical_parser();

template<runir::kr::dl::FamilyTag Family>
numerical_root_type<Family> const& numerical_root_parser();

template<runir::kr::dl::FamilyTag Family>
grammar_root_type<Family> const& grammar_root_parser();

}  // namespace runir::kr::dl::grammar::parser

#endif
