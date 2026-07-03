#ifndef RUNIR_KR_DL_GRAMMAR_PARSER_UNS_GRAMMAR_PARSERS_HPP_
#define RUNIR_KR_DL_GRAMMAR_PARSER_UNS_GRAMMAR_PARSERS_HPP_

#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/dl/grammar/parser/error_handler.hpp"

#include <boost/spirit/home/x3.hpp>

namespace runir::kr::dl::grammar::parser::uns::grammar
{
namespace x3 = boost::spirit::x3;
namespace grammar_ast = runir::kr::dl::grammar::ast;

template<runir::kr::dl::CategoryTag D>
struct ConstructorClass;

template<runir::kr::dl::CategoryTag D>
struct ConstructorRootClass;

template<runir::kr::dl::CategoryTag D>
struct NonTerminalClass;

template<runir::kr::dl::CategoryTag D>
struct ConstructorOrNonTerminalClass;

template<runir::kr::dl::CategoryTag D>
struct DerivationRuleClass;

struct DerivationRuleVariantClass;

struct ConceptBotClass;
struct ConceptTopClass;
struct ConceptAtomicStateClass;
struct ConceptAtomicGoalClass;
struct ConceptIntersectionClass;
struct ConceptUnionClass;
struct ConceptNegationClass;
struct ConceptValueRestrictionClass;
struct ConceptExistentialQuantificationClass;
struct ConceptAtLeastNumberRestrictionClass;
struct ConceptAtMostNumberRestrictionClass;
struct ConceptExactNumberRestrictionClass;
struct ConceptQualifiedAtLeastNumberRestrictionClass;
struct ConceptQualifiedAtMostNumberRestrictionClass;
struct ConceptQualifiedExactNumberRestrictionClass;
struct ConceptRoleValueMapClass;
struct ConceptAgreementClass;
struct ConceptRoleFillersClass;
struct ConceptOneOfClass;
struct ConceptNominalClass;

struct RoleUniversalClass;
struct RoleAtomicStateClass;
struct RoleAtomicGoalClass;
struct RoleIntersectionClass;
struct RoleUnionClass;
struct RoleComplementClass;
struct RoleInverseClass;
struct RoleCompositionClass;
struct RoleTransitiveClosureClass;
struct RoleReflexiveTransitiveClosureClass;
struct RoleRestrictionClass;
struct RoleIdentityClass;

struct ConstructorOrNonTerminalVariantClass;

struct BooleanAtomicStateClass;
struct BooleanAtomicGoalClass;
struct BooleanNonemptyClass;
struct BooleanEqClass;
struct BooleanNeqClass;
struct BooleanLtClass;
struct BooleanLeClass;
struct BooleanGtClass;
struct BooleanGeClass;
struct BooleanConstantClass;
struct BooleanAndClass;
struct BooleanOrClass;
struct BooleanNotClass;

struct NumericalCountClass;
struct NumericalDistanceClass;
struct NumericalEqClass;
struct NumericalNeqClass;
struct NumericalLtClass;
struct NumericalLeClass;
struct NumericalGtClass;
struct NumericalGeClass;
struct NumericalConstantClass;
struct NumericalAddClass;
struct NumericalSubClass;
struct NumericalMulClass;
struct NumericalDivClass;
struct NumericalMinClass;
struct NumericalMaxClass;

struct GrammarBodyClass;
struct GrammarClass;
struct GrammarRootClass;

using concept_type = x3::rule<ConstructorClass<runir::kr::dl::ConceptTag>, grammar_ast::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::ConceptTag>>;
using concept_root_type =
    x3::rule<ConstructorRootClass<runir::kr::dl::ConceptTag>, grammar_ast::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::ConceptTag>>;
using concept_bot_type = x3::rule<ConceptBotClass, grammar_ast::ConceptBot<runir::kr::UnsFamilyTag>>;
using concept_top_type = x3::rule<ConceptTopClass, grammar_ast::ConceptTop<runir::kr::UnsFamilyTag>>;
using concept_atomic_state_type = x3::rule<ConceptAtomicStateClass, grammar_ast::ConceptAtomicState<runir::kr::UnsFamilyTag>>;
using concept_atomic_goal_type = x3::rule<ConceptAtomicGoalClass, grammar_ast::ConceptAtomicGoal<runir::kr::UnsFamilyTag>>;
using concept_intersection_type = x3::rule<ConceptIntersectionClass, grammar_ast::ConceptIntersection<runir::kr::UnsFamilyTag>>;
using concept_union_type = x3::rule<ConceptUnionClass, grammar_ast::ConceptUnion<runir::kr::UnsFamilyTag>>;
using concept_negation_type = x3::rule<ConceptNegationClass, grammar_ast::ConceptNegation<runir::kr::UnsFamilyTag>>;
using concept_value_restriction_type = x3::rule<ConceptValueRestrictionClass, grammar_ast::ConceptValueRestriction<runir::kr::UnsFamilyTag>>;
using concept_existential_quantification_type =
    x3::rule<ConceptExistentialQuantificationClass, grammar_ast::ConceptExistentialQuantification<runir::kr::UnsFamilyTag>>;
using concept_at_least_number_restriction_type =
    x3::rule<ConceptAtLeastNumberRestrictionClass, grammar_ast::ConceptAtLeastNumberRestriction<runir::kr::UnsFamilyTag>>;
using concept_at_most_number_restriction_type =
    x3::rule<ConceptAtMostNumberRestrictionClass, grammar_ast::ConceptAtMostNumberRestriction<runir::kr::UnsFamilyTag>>;
using concept_exact_number_restriction_type = x3::rule<ConceptExactNumberRestrictionClass, grammar_ast::ConceptExactNumberRestriction<runir::kr::UnsFamilyTag>>;
using concept_qualified_at_least_number_restriction_type =
    x3::rule<ConceptQualifiedAtLeastNumberRestrictionClass, grammar_ast::ConceptQualifiedAtLeastNumberRestriction<runir::kr::UnsFamilyTag>>;
using concept_qualified_at_most_number_restriction_type =
    x3::rule<ConceptQualifiedAtMostNumberRestrictionClass, grammar_ast::ConceptQualifiedAtMostNumberRestriction<runir::kr::UnsFamilyTag>>;
using concept_qualified_exact_number_restriction_type =
    x3::rule<ConceptQualifiedExactNumberRestrictionClass, grammar_ast::ConceptQualifiedExactNumberRestriction<runir::kr::UnsFamilyTag>>;
using concept_role_value_map_type = x3::rule<ConceptRoleValueMapClass, grammar_ast::ConceptRoleValueMap<runir::kr::UnsFamilyTag>>;
using concept_agreement_type = x3::rule<ConceptAgreementClass, grammar_ast::ConceptAgreement<runir::kr::UnsFamilyTag>>;
using concept_role_fillers_type = x3::rule<ConceptRoleFillersClass, grammar_ast::ConceptRoleFillers<runir::kr::UnsFamilyTag>>;
using concept_one_of_type = x3::rule<ConceptOneOfClass, grammar_ast::ConceptOneOf<runir::kr::UnsFamilyTag>>;
using concept_nominal_type = x3::rule<ConceptNominalClass, grammar_ast::ConceptNominal<runir::kr::UnsFamilyTag>>;
using concept_non_terminal_type =
    x3::rule<NonTerminalClass<runir::kr::dl::ConceptTag>, grammar_ast::NonTerminal<runir::kr::UnsFamilyTag, runir::kr::dl::ConceptTag>>;
using concept_choice_type = x3::rule<ConstructorOrNonTerminalClass<runir::kr::dl::ConceptTag>,
                                     grammar_ast::ConstructorOrNonTerminal<runir::kr::UnsFamilyTag, runir::kr::dl::ConceptTag>>;
using concept_derivation_rule_type =
    x3::rule<DerivationRuleClass<runir::kr::dl::ConceptTag>, grammar_ast::DerivationRule<runir::kr::UnsFamilyTag, runir::kr::dl::ConceptTag>>;

using role_type = x3::rule<ConstructorClass<runir::kr::dl::RoleTag>, grammar_ast::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::RoleTag>>;
using role_root_type = x3::rule<ConstructorRootClass<runir::kr::dl::RoleTag>, grammar_ast::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::RoleTag>>;
using role_universal_type = x3::rule<RoleUniversalClass, grammar_ast::RoleUniversal<runir::kr::UnsFamilyTag>>;
using role_atomic_state_type = x3::rule<RoleAtomicStateClass, grammar_ast::RoleAtomicState<runir::kr::UnsFamilyTag>>;
using role_atomic_goal_type = x3::rule<RoleAtomicGoalClass, grammar_ast::RoleAtomicGoal<runir::kr::UnsFamilyTag>>;
using role_intersection_type = x3::rule<RoleIntersectionClass, grammar_ast::RoleIntersection<runir::kr::UnsFamilyTag>>;
using role_union_type = x3::rule<RoleUnionClass, grammar_ast::RoleUnion<runir::kr::UnsFamilyTag>>;
using role_complement_type = x3::rule<RoleComplementClass, grammar_ast::RoleComplement<runir::kr::UnsFamilyTag>>;
using role_inverse_type = x3::rule<RoleInverseClass, grammar_ast::RoleInverse<runir::kr::UnsFamilyTag>>;
using role_composition_type = x3::rule<RoleCompositionClass, grammar_ast::RoleComposition<runir::kr::UnsFamilyTag>>;
using role_transitive_closure_type = x3::rule<RoleTransitiveClosureClass, grammar_ast::RoleTransitiveClosure<runir::kr::UnsFamilyTag>>;
using role_reflexive_transitive_closure_type =
    x3::rule<RoleReflexiveTransitiveClosureClass, grammar_ast::RoleReflexiveTransitiveClosure<runir::kr::UnsFamilyTag>>;
using role_restriction_type = x3::rule<RoleRestrictionClass, grammar_ast::RoleRestriction<runir::kr::UnsFamilyTag>>;
using role_identity_type = x3::rule<RoleIdentityClass, grammar_ast::RoleIdentity<runir::kr::UnsFamilyTag>>;
using role_non_terminal_type = x3::rule<NonTerminalClass<runir::kr::dl::RoleTag>, grammar_ast::NonTerminal<runir::kr::UnsFamilyTag, runir::kr::dl::RoleTag>>;
using role_choice_type =
    x3::rule<ConstructorOrNonTerminalClass<runir::kr::dl::RoleTag>, grammar_ast::ConstructorOrNonTerminal<runir::kr::UnsFamilyTag, runir::kr::dl::RoleTag>>;
using role_derivation_rule_type =
    x3::rule<DerivationRuleClass<runir::kr::dl::RoleTag>, grammar_ast::DerivationRule<runir::kr::UnsFamilyTag, runir::kr::dl::RoleTag>>;

using constructor_or_non_terminal_variant_type =
    x3::rule<ConstructorOrNonTerminalVariantClass, grammar_ast::ConstructorOrNonTerminalVariant<runir::kr::UnsFamilyTag>>;

using boolean_type = x3::rule<ConstructorClass<runir::kr::dl::BooleanTag>, grammar_ast::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>>;
using boolean_root_type =
    x3::rule<ConstructorRootClass<runir::kr::dl::BooleanTag>, grammar_ast::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>>;
using boolean_atomic_state_type = x3::rule<BooleanAtomicStateClass, grammar_ast::BooleanAtomicState<runir::kr::UnsFamilyTag>>;
using boolean_atomic_goal_type = x3::rule<BooleanAtomicGoalClass, grammar_ast::BooleanAtomicGoal<runir::kr::UnsFamilyTag>>;
using boolean_nonempty_type = x3::rule<BooleanNonemptyClass, grammar_ast::BooleanNonempty<runir::kr::UnsFamilyTag>>;
using boolean_eq_type = x3::rule<BooleanEqClass, grammar_ast::BooleanEq<runir::kr::UnsFamilyTag>>;
using boolean_neq_type = x3::rule<BooleanNeqClass, grammar_ast::BooleanNeq<runir::kr::UnsFamilyTag>>;
using boolean_lt_type = x3::rule<BooleanLtClass, grammar_ast::BooleanLt<runir::kr::UnsFamilyTag>>;
using boolean_le_type = x3::rule<BooleanLeClass, grammar_ast::BooleanLe<runir::kr::UnsFamilyTag>>;
using boolean_gt_type = x3::rule<BooleanGtClass, grammar_ast::BooleanGt<runir::kr::UnsFamilyTag>>;
using boolean_ge_type = x3::rule<BooleanGeClass, grammar_ast::BooleanGe<runir::kr::UnsFamilyTag>>;
using boolean_constant_type = x3::rule<BooleanConstantClass, grammar_ast::BooleanConstant<runir::kr::UnsFamilyTag>>;
using boolean_and_type = x3::rule<BooleanAndClass, grammar_ast::BooleanAnd<runir::kr::UnsFamilyTag>>;
using boolean_or_type = x3::rule<BooleanOrClass, grammar_ast::BooleanOr<runir::kr::UnsFamilyTag>>;
using boolean_not_type = x3::rule<BooleanNotClass, grammar_ast::BooleanNot<runir::kr::UnsFamilyTag>>;
using boolean_non_terminal_type =
    x3::rule<NonTerminalClass<runir::kr::dl::BooleanTag>, grammar_ast::NonTerminal<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>>;
using boolean_choice_type = x3::rule<ConstructorOrNonTerminalClass<runir::kr::dl::BooleanTag>,
                                     grammar_ast::ConstructorOrNonTerminal<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>>;
using boolean_derivation_rule_type =
    x3::rule<DerivationRuleClass<runir::kr::dl::BooleanTag>, grammar_ast::DerivationRule<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>>;

using numerical_type = x3::rule<ConstructorClass<runir::kr::dl::NumericalTag>, grammar_ast::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::NumericalTag>>;
using numerical_root_type =
    x3::rule<ConstructorRootClass<runir::kr::dl::NumericalTag>, grammar_ast::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::NumericalTag>>;
using numerical_count_type = x3::rule<NumericalCountClass, grammar_ast::NumericalCount<runir::kr::UnsFamilyTag>>;
using numerical_distance_type = x3::rule<NumericalDistanceClass, grammar_ast::NumericalDistance<runir::kr::UnsFamilyTag>>;
using numerical_eq_type = x3::rule<NumericalEqClass, grammar_ast::NumericalEq<runir::kr::UnsFamilyTag>>;
using numerical_neq_type = x3::rule<NumericalNeqClass, grammar_ast::NumericalNeq<runir::kr::UnsFamilyTag>>;
using numerical_lt_type = x3::rule<NumericalLtClass, grammar_ast::NumericalLt<runir::kr::UnsFamilyTag>>;
using numerical_le_type = x3::rule<NumericalLeClass, grammar_ast::NumericalLe<runir::kr::UnsFamilyTag>>;
using numerical_gt_type = x3::rule<NumericalGtClass, grammar_ast::NumericalGt<runir::kr::UnsFamilyTag>>;
using numerical_ge_type = x3::rule<NumericalGeClass, grammar_ast::NumericalGe<runir::kr::UnsFamilyTag>>;
using numerical_constant_type = x3::rule<NumericalConstantClass, grammar_ast::NumericalConstant<runir::kr::UnsFamilyTag>>;
using numerical_add_type = x3::rule<NumericalAddClass, grammar_ast::NumericalAdd<runir::kr::UnsFamilyTag>>;
using numerical_sub_type = x3::rule<NumericalSubClass, grammar_ast::NumericalSub<runir::kr::UnsFamilyTag>>;
using numerical_mul_type = x3::rule<NumericalMulClass, grammar_ast::NumericalMul<runir::kr::UnsFamilyTag>>;
using numerical_div_type = x3::rule<NumericalDivClass, grammar_ast::NumericalDiv<runir::kr::UnsFamilyTag>>;
using numerical_min_type = x3::rule<NumericalMinClass, grammar_ast::NumericalMin<runir::kr::UnsFamilyTag>>;
using numerical_max_type = x3::rule<NumericalMaxClass, grammar_ast::NumericalMax<runir::kr::UnsFamilyTag>>;
using numerical_non_terminal_type =
    x3::rule<NonTerminalClass<runir::kr::dl::NumericalTag>, grammar_ast::NonTerminal<runir::kr::UnsFamilyTag, runir::kr::dl::NumericalTag>>;
using numerical_choice_type = x3::rule<ConstructorOrNonTerminalClass<runir::kr::dl::NumericalTag>,
                                       grammar_ast::ConstructorOrNonTerminal<runir::kr::UnsFamilyTag, runir::kr::dl::NumericalTag>>;
using numerical_derivation_rule_type =
    x3::rule<DerivationRuleClass<runir::kr::dl::NumericalTag>, grammar_ast::DerivationRule<runir::kr::UnsFamilyTag, runir::kr::dl::NumericalTag>>;

using derivation_rule_variant_type = x3::rule<DerivationRuleVariantClass, grammar_ast::DerivationRuleVariant<runir::kr::UnsFamilyTag>>;

using grammar_body_type = x3::rule<GrammarBodyClass, std::vector<grammar_ast::DerivationRuleVariant<runir::kr::UnsFamilyTag>>>;
using grammar_type = x3::rule<GrammarClass, grammar_ast::Grammar<runir::kr::UnsFamilyTag>>;
using grammar_root_type = x3::rule<GrammarRootClass, grammar_ast::Grammar<runir::kr::UnsFamilyTag>>;

BOOST_SPIRIT_DECLARE(concept_type,
                     concept_root_type,
                     concept_bot_type,
                     concept_top_type,
                     concept_atomic_state_type,
                     concept_atomic_goal_type,
                     concept_intersection_type,
                     concept_union_type,
                     concept_negation_type,
                     concept_value_restriction_type,
                     concept_existential_quantification_type,
                     concept_at_least_number_restriction_type,
                     concept_at_most_number_restriction_type,
                     concept_exact_number_restriction_type,
                     concept_qualified_at_least_number_restriction_type,
                     concept_qualified_at_most_number_restriction_type,
                     concept_qualified_exact_number_restriction_type,
                     concept_role_value_map_type,
                     concept_agreement_type,
                     concept_role_fillers_type,
                     concept_one_of_type,
                     concept_nominal_type,
                     concept_non_terminal_type,
                     concept_choice_type,
                     concept_derivation_rule_type)

BOOST_SPIRIT_DECLARE(role_type,
                     role_root_type,
                     role_universal_type,
                     role_atomic_state_type,
                     role_atomic_goal_type,
                     role_intersection_type,
                     role_union_type,
                     role_complement_type,
                     role_inverse_type,
                     role_composition_type,
                     role_transitive_closure_type,
                     role_reflexive_transitive_closure_type,
                     role_restriction_type,
                     role_identity_type,
                     role_non_terminal_type,
                     role_choice_type,
                     role_derivation_rule_type)

BOOST_SPIRIT_DECLARE(constructor_or_non_terminal_variant_type)

BOOST_SPIRIT_DECLARE(boolean_type,
                     boolean_root_type,
                     boolean_atomic_state_type,
                     boolean_atomic_goal_type,
                     boolean_nonempty_type,
                     boolean_eq_type,
                     boolean_neq_type,
                     boolean_lt_type,
                     boolean_le_type,
                     boolean_gt_type,
                     boolean_ge_type,
                     boolean_constant_type,
                     boolean_and_type,
                     boolean_or_type,
                     boolean_not_type,
                     boolean_non_terminal_type,
                     boolean_choice_type,
                     boolean_derivation_rule_type)

BOOST_SPIRIT_DECLARE(numerical_type,
                     numerical_root_type,
                     numerical_count_type,
                     numerical_distance_type,
                     numerical_eq_type,
                     numerical_neq_type,
                     numerical_lt_type,
                     numerical_le_type,
                     numerical_gt_type,
                     numerical_ge_type,
                     numerical_constant_type,
                     numerical_add_type,
                     numerical_sub_type,
                     numerical_mul_type,
                     numerical_div_type,
                     numerical_min_type,
                     numerical_max_type,
                     numerical_non_terminal_type,
                     numerical_choice_type,
                     numerical_derivation_rule_type)

BOOST_SPIRIT_DECLARE(derivation_rule_variant_type, grammar_body_type)
BOOST_SPIRIT_DECLARE(grammar_type, grammar_root_type)

concept_type const& concept_parser();
concept_root_type const& concept_root_parser();
concept_bot_type const& concept_bot_parser();
concept_top_type const& concept_top_parser();
concept_atomic_state_type const& concept_atomic_state_parser();
concept_atomic_goal_type const& concept_atomic_goal_parser();
concept_intersection_type const& concept_intersection_parser();
concept_union_type const& concept_union_parser();
concept_negation_type const& concept_negation_parser();
concept_value_restriction_type const& concept_value_restriction_parser();
concept_existential_quantification_type const& concept_existential_quantification_parser();
concept_at_least_number_restriction_type const& concept_at_least_number_restriction_parser();
concept_at_most_number_restriction_type const& concept_at_most_number_restriction_parser();
concept_exact_number_restriction_type const& concept_exact_number_restriction_parser();
concept_qualified_at_least_number_restriction_type const& concept_qualified_at_least_number_restriction_parser();
concept_qualified_at_most_number_restriction_type const& concept_qualified_at_most_number_restriction_parser();
concept_qualified_exact_number_restriction_type const& concept_qualified_exact_number_restriction_parser();
concept_role_value_map_type const& concept_role_value_map_parser();
concept_agreement_type const& concept_agreement_parser();
concept_role_fillers_type const& concept_role_fillers_parser();
concept_one_of_type const& concept_one_of_parser();
concept_nominal_type const& concept_nominal_parser();
concept_non_terminal_type const& concept_non_terminal_parser();
concept_choice_type const& concept_choice_parser();
concept_derivation_rule_type const& concept_derivation_rule_parser();

role_type const& role_parser();
role_root_type const& role_root_parser();
role_universal_type const& role_universal_parser();
role_atomic_state_type const& role_atomic_state_parser();
role_atomic_goal_type const& role_atomic_goal_parser();
role_intersection_type const& role_intersection_parser();
role_union_type const& role_union_parser();
role_complement_type const& role_complement_parser();
role_inverse_type const& role_inverse_parser();
role_composition_type const& role_composition_parser();
role_transitive_closure_type const& role_transitive_closure_parser();
role_reflexive_transitive_closure_type const& role_reflexive_transitive_closure_parser();
role_restriction_type const& role_restriction_parser();
role_identity_type const& role_identity_parser();
role_non_terminal_type const& role_non_terminal_parser();
role_choice_type const& role_choice_parser();
role_derivation_rule_type const& role_derivation_rule_parser();

constructor_or_non_terminal_variant_type const& constructor_or_non_terminal_variant_parser();

boolean_type const& boolean_parser();
boolean_root_type const& boolean_root_parser();
boolean_atomic_state_type const& boolean_atomic_state_parser();
boolean_atomic_goal_type const& boolean_atomic_goal_parser();
boolean_nonempty_type const& boolean_nonempty_parser();
boolean_eq_type const& boolean_eq_parser();
boolean_neq_type const& boolean_neq_parser();
boolean_lt_type const& boolean_lt_parser();
boolean_le_type const& boolean_le_parser();
boolean_gt_type const& boolean_gt_parser();
boolean_ge_type const& boolean_ge_parser();
boolean_constant_type const& boolean_constant_parser();
boolean_and_type const& boolean_and_parser();
boolean_or_type const& boolean_or_parser();
boolean_not_type const& boolean_not_parser();
boolean_non_terminal_type const& boolean_non_terminal_parser();
boolean_choice_type const& boolean_choice_parser();
boolean_derivation_rule_type const& boolean_derivation_rule_parser();

numerical_type const& numerical_parser();
numerical_root_type const& numerical_root_parser();
numerical_count_type const& numerical_count_parser();
numerical_distance_type const& numerical_distance_parser();
numerical_eq_type const& numerical_eq_parser();
numerical_neq_type const& numerical_neq_parser();
numerical_lt_type const& numerical_lt_parser();
numerical_le_type const& numerical_le_parser();
numerical_gt_type const& numerical_gt_parser();
numerical_ge_type const& numerical_ge_parser();
numerical_constant_type const& numerical_constant_parser();
numerical_add_type const& numerical_add_parser();
numerical_sub_type const& numerical_sub_parser();
numerical_mul_type const& numerical_mul_parser();
numerical_div_type const& numerical_div_parser();
numerical_min_type const& numerical_min_parser();
numerical_max_type const& numerical_max_parser();
numerical_non_terminal_type const& numerical_non_terminal_parser();
numerical_choice_type const& numerical_choice_parser();
numerical_derivation_rule_type const& numerical_derivation_rule_parser();

derivation_rule_variant_type const& derivation_rule_variant_parser();

grammar_body_type const& grammar_body_parser();

grammar_type const& grammar_parser();
grammar_root_type const& grammar_root_parser();

}  // namespace runir::kr::dl::grammar::parser::uns::grammar

#endif
