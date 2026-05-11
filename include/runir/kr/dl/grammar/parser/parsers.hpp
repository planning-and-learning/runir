#ifndef RUNIR_GRAMMAR_PARSER_PARSERS_HPP_
#define RUNIR_GRAMMAR_PARSER_PARSERS_HPP_

#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/dl/grammar/parser/declarations.hpp"

#include <boost/spirit/home/x3.hpp>

namespace runir::kr::dl::grammar::parser
{
namespace x3 = boost::spirit::x3;

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
struct ConceptRoleValueMapContainmentClass;
struct ConceptRoleValueMapEqualityClass;
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
struct BooleanNonemptyClass;

struct NumericalCountClass;
struct NumericalDistanceClass;

struct GrammarHeadClass;
struct GrammarBodyClass;
struct GrammarClass;
struct GrammarRootClass;

using concept_type = x3::rule<ConstructorClass<runir::kr::dl::ConceptTag>, ast::Constructor<runir::kr::dl::ConceptTag>>;
using concept_root_type = x3::rule<ConstructorRootClass<runir::kr::dl::ConceptTag>, ast::Constructor<runir::kr::dl::ConceptTag>>;
using concept_bot_type = x3::rule<ConceptBotClass, ast::ConceptBot>;
using concept_top_type = x3::rule<ConceptTopClass, ast::ConceptTop>;
using concept_atomic_state_type = x3::rule<ConceptAtomicStateClass, ast::ConceptAtomicState>;
using concept_atomic_goal_type = x3::rule<ConceptAtomicGoalClass, ast::ConceptAtomicGoal>;
using concept_intersection_type = x3::rule<ConceptIntersectionClass, ast::ConceptIntersection>;
using concept_union_type = x3::rule<ConceptUnionClass, ast::ConceptUnion>;
using concept_negation_type = x3::rule<ConceptNegationClass, ast::ConceptNegation>;
using concept_value_restriction_type = x3::rule<ConceptValueRestrictionClass, ast::ConceptValueRestriction>;
using concept_existential_quantification_type = x3::rule<ConceptExistentialQuantificationClass, ast::ConceptExistentialQuantification>;
using concept_role_value_map_containment_type = x3::rule<ConceptRoleValueMapContainmentClass, ast::ConceptRoleValueMapContainment>;
using concept_role_value_map_equality_type = x3::rule<ConceptRoleValueMapEqualityClass, ast::ConceptRoleValueMapEquality>;
using concept_nominal_type = x3::rule<ConceptNominalClass, ast::ConceptNominal>;
using concept_non_terminal_type = x3::rule<NonTerminalClass<runir::kr::dl::ConceptTag>, ast::NonTerminal<runir::kr::dl::ConceptTag>>;
using concept_choice_type = x3::rule<ConstructorOrNonTerminalClass<runir::kr::dl::ConceptTag>, ast::ConstructorOrNonTerminal<runir::kr::dl::ConceptTag>>;
using concept_derivation_rule_type = x3::rule<DerivationRuleClass<runir::kr::dl::ConceptTag>, ast::DerivationRule<runir::kr::dl::ConceptTag>>;

using role_type = x3::rule<ConstructorClass<runir::kr::dl::RoleTag>, ast::Constructor<runir::kr::dl::RoleTag>>;
using role_root_type = x3::rule<ConstructorRootClass<runir::kr::dl::RoleTag>, ast::Constructor<runir::kr::dl::RoleTag>>;
using role_universal_type = x3::rule<RoleUniversalClass, ast::RoleUniversal>;
using role_atomic_state_type = x3::rule<RoleAtomicStateClass, ast::RoleAtomicState>;
using role_atomic_goal_type = x3::rule<RoleAtomicGoalClass, ast::RoleAtomicGoal>;
using role_intersection_type = x3::rule<RoleIntersectionClass, ast::RoleIntersection>;
using role_union_type = x3::rule<RoleUnionClass, ast::RoleUnion>;
using role_complement_type = x3::rule<RoleComplementClass, ast::RoleComplement>;
using role_inverse_type = x3::rule<RoleInverseClass, ast::RoleInverse>;
using role_composition_type = x3::rule<RoleCompositionClass, ast::RoleComposition>;
using role_transitive_closure_type = x3::rule<RoleTransitiveClosureClass, ast::RoleTransitiveClosure>;
using role_reflexive_transitive_closure_type = x3::rule<RoleReflexiveTransitiveClosureClass, ast::RoleReflexiveTransitiveClosure>;
using role_restriction_type = x3::rule<RoleRestrictionClass, ast::RoleRestriction>;
using role_identity_type = x3::rule<RoleIdentityClass, ast::RoleIdentity>;
using role_non_terminal_type = x3::rule<NonTerminalClass<runir::kr::dl::RoleTag>, ast::NonTerminal<runir::kr::dl::RoleTag>>;
using role_choice_type = x3::rule<ConstructorOrNonTerminalClass<runir::kr::dl::RoleTag>, ast::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>>;
using role_derivation_rule_type = x3::rule<DerivationRuleClass<runir::kr::dl::RoleTag>, ast::DerivationRule<runir::kr::dl::RoleTag>>;

using constructor_or_non_terminal_variant_type = x3::rule<ConstructorOrNonTerminalVariantClass, ast::ConstructorOrNonTerminalVariant>;

using boolean_type = x3::rule<ConstructorClass<runir::kr::dl::BooleanTag>, ast::Constructor<runir::kr::dl::BooleanTag>>;
using boolean_root_type = x3::rule<ConstructorRootClass<runir::kr::dl::BooleanTag>, ast::Constructor<runir::kr::dl::BooleanTag>>;
using boolean_atomic_state_type = x3::rule<BooleanAtomicStateClass, ast::BooleanAtomicState>;
using boolean_nonempty_type = x3::rule<BooleanNonemptyClass, ast::BooleanNonempty>;
using boolean_non_terminal_type = x3::rule<NonTerminalClass<runir::kr::dl::BooleanTag>, ast::NonTerminal<runir::kr::dl::BooleanTag>>;
using boolean_choice_type = x3::rule<ConstructorOrNonTerminalClass<runir::kr::dl::BooleanTag>, ast::ConstructorOrNonTerminal<runir::kr::dl::BooleanTag>>;
using boolean_derivation_rule_type = x3::rule<DerivationRuleClass<runir::kr::dl::BooleanTag>, ast::DerivationRule<runir::kr::dl::BooleanTag>>;

using numerical_type = x3::rule<ConstructorClass<runir::kr::dl::NumericalTag>, ast::Constructor<runir::kr::dl::NumericalTag>>;
using numerical_root_type = x3::rule<ConstructorRootClass<runir::kr::dl::NumericalTag>, ast::Constructor<runir::kr::dl::NumericalTag>>;
using numerical_count_type = x3::rule<NumericalCountClass, ast::NumericalCount>;
using numerical_distance_type = x3::rule<NumericalDistanceClass, ast::NumericalDistance>;
using numerical_non_terminal_type = x3::rule<NonTerminalClass<runir::kr::dl::NumericalTag>, ast::NonTerminal<runir::kr::dl::NumericalTag>>;
using numerical_choice_type = x3::rule<ConstructorOrNonTerminalClass<runir::kr::dl::NumericalTag>, ast::ConstructorOrNonTerminal<runir::kr::dl::NumericalTag>>;
using numerical_derivation_rule_type = x3::rule<DerivationRuleClass<runir::kr::dl::NumericalTag>, ast::DerivationRule<runir::kr::dl::NumericalTag>>;

using derivation_rule_variant_type = x3::rule<DerivationRuleVariantClass, ast::DerivationRuleVariant>;

using grammar_head_type = x3::rule<GrammarHeadClass, ast::GrammarHead>;
using grammar_body_type = x3::rule<GrammarBodyClass, ast::GrammarBody>;
using grammar_type = x3::rule<GrammarClass, ast::Grammar>;
using grammar_root_type = x3::rule<GrammarRootClass, ast::Grammar>;

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
                     concept_role_value_map_containment_type,
                     concept_role_value_map_equality_type,
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
                     boolean_nonempty_type,
                     boolean_non_terminal_type,
                     boolean_choice_type,
                     boolean_derivation_rule_type)

BOOST_SPIRIT_DECLARE(numerical_type,
                     numerical_root_type,
                     numerical_count_type,
                     numerical_distance_type,
                     numerical_non_terminal_type,
                     numerical_choice_type,
                     numerical_derivation_rule_type)

BOOST_SPIRIT_DECLARE(derivation_rule_variant_type, grammar_head_type, grammar_body_type)
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
concept_role_value_map_containment_type const& concept_role_value_map_containment_parser();
concept_role_value_map_equality_type const& concept_role_value_map_equality_parser();
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
boolean_nonempty_type const& boolean_nonempty_parser();
boolean_non_terminal_type const& boolean_non_terminal_parser();
boolean_choice_type const& boolean_choice_parser();
boolean_derivation_rule_type const& boolean_derivation_rule_parser();

numerical_type const& numerical_parser();
numerical_root_type const& numerical_root_parser();
numerical_count_type const& numerical_count_parser();
numerical_distance_type const& numerical_distance_parser();
numerical_non_terminal_type const& numerical_non_terminal_parser();
numerical_choice_type const& numerical_choice_parser();
numerical_derivation_rule_type const& numerical_derivation_rule_parser();

derivation_rule_variant_type const& derivation_rule_variant_parser();

grammar_head_type const& grammar_head_parser();
grammar_body_type const& grammar_body_parser();

grammar_type const& grammar_parser();
grammar_root_type const& grammar_root_parser();

}

#endif
