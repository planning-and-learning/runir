#ifndef RUNIR_SRC_GRAMMAR_PARSER_PARSER_DEF_HPP_
#define RUNIR_SRC_GRAMMAR_PARSER_PARSER_DEF_HPP_

#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/dl/grammar/ast/ast_adapted.hpp"
#include "runir/kr/dl/grammar/parser/error_handler.hpp"
#include "runir/kr/dl/grammar/parser/parsers.hpp"

#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>

namespace runir::kr::dl::grammar::parser
{
namespace x3 = boost::spirit::x3;

using x3::eoi;
using x3::eps;
using x3::lexeme;
using x3::lit;
using x3::omit;
using x3::raw;

using x3::ascii::alnum;
using x3::ascii::alpha;
using x3::ascii::char_;
using x3::ascii::digit;

concept_type const concept_ = "concept";
concept_root_type const concept_root = "concept_root";
concept_bot_type const concept_bot = "concept_bot";
concept_top_type const concept_top = "concept_top";
concept_atomic_state_type const concept_atomic_state = "concept_atomic_state";
concept_atomic_goal_type const concept_atomic_goal = "concept_atomic_goal";
concept_intersection_type const concept_intersection = "concept_intersection";
concept_union_type const concept_union = "concept_union";
concept_negation_type const concept_negation = "concept_negation";
concept_value_restriction_type const concept_value_restriction = "concept_value_restriction";
concept_existential_quantification_type const concept_existential_quantification = "concept_existential_quantification";
concept_role_value_map_containment_type const concept_role_value_map_containment = "concept_role_value_map_containment";
concept_role_value_map_equality_type const concept_role_value_map_equality = "concept_role_value_map_equality";
concept_nominal_type const concept_nominal = "concept_nominal";
concept_non_terminal_type const concept_non_terminal = "concept_non_terminal";
concept_choice_type const concept_choice = "concept_choice";
concept_derivation_rule_type const concept_derivation_rule = "concept_derivation_rule";

role_type const role = "role";
role_root_type const role_root = "role_root";
role_universal_type const role_universal = "role_universal";
role_atomic_state_type const role_atomic_state = "role_atomic_state";
role_atomic_goal_type const role_atomic_goal = "role_atomic_goal";
role_intersection_type const role_intersection = "role_intersection";
role_union_type const role_union = "role_union";
role_complement_type const role_complement = "role_complement";
role_inverse_type const role_inverse = "role_inverse";
role_composition_type const role_composition = "role_composition";
role_transitive_closure_type const role_transitive_closure = "role_transitive_closure";
role_reflexive_transitive_closure_type const role_reflexive_transitive_closure = "role_reflexive_transitive_closure";
role_restriction_type const role_restriction = "role_restriction";
role_identity_type const role_identity = "role_identity";
role_non_terminal_type const role_non_terminal = "role_non_terminal";
role_choice_type const role_choice = "role_choice";
role_derivation_rule_type const role_derivation_rule = "role_derivation_rule";

constructor_or_non_terminal_variant_type const constructor_or_non_terminal_variant = "constructor_or_non_terminal_variant";

boolean_type const boolean = "boolean";
boolean_root_type const boolean_root = "boolean_root";
boolean_atomic_state_type const boolean_atomic_state = "boolean_atomic_state";
boolean_nonempty_type const boolean_nonempty = "boolean_nonempty";
boolean_non_terminal_type const boolean_non_terminal = "boolean_non_terminal";
boolean_choice_type const boolean_choice = "boolean_choice";
boolean_derivation_rule_type const boolean_derivation_rule = "boolean_derivation_rule";

numerical_type const numerical = "numerical";
numerical_root_type const numerical_root = "numerical_root";
numerical_count_type const numerical_count = "numerical_count";
numerical_distance_type const numerical_distance = "numerical_distance";
numerical_non_terminal_type const numerical_non_terminal = "numerical_non_terminal";
numerical_choice_type const numerical_choice = "numerical_choice";
numerical_derivation_rule_type const numerical_derivation_rule = "numerical_derivation_rule";

derivation_rule_variant_type const derivation_rule_variant = "derivation_rule_variant";

grammar_body_type const grammar_body = "grammar_body";
grammar_type const grammar = "grammar";
grammar_root_type const grammar_root = "grammar_root";

inline auto concept_non_terminal_string_parser() { return raw[lexeme[lit("c_") >> +digit]]; }

inline auto role_non_terminal_string_parser() { return raw[lexeme[lit("r_") >> +digit]]; }

inline auto boolean_non_terminal_string_parser() { return raw[lexeme[lit("b_") >> +digit]]; }

inline auto numerical_non_terminal_string_parser() { return raw[lexeme[lit("n_") >> +digit]]; }

inline auto predicate_name_string_parser()
{
    return lexeme[omit[lit('"')]] > raw[lexeme[(alpha >> *(alnum | char_('-') | char_('_'))) | char_('=')]] > lexeme[omit[lit('"')]];
}

inline auto object_name_string_parser() { return lexeme[omit[lit('"')]] > raw[lexeme[alpha >> *(alnum | char_('-') | char_('_'))]] > lexeme[omit[lit('"')]]; }

inline auto bool_string_parser()
{
    return x3::lexeme[(x3::lit("true") >> x3::attr(true)) | (x3::lit("false") >> x3::attr(false)) | (x3::lit("1") >> x3::attr(true))
                      | (x3::lit("0") >> x3::attr(false))];
}

template<typename Parser>
auto with_constructor_parentheses(Parser parser)
{
    return (lit("(") >> parser >> lit(")")) | parser;
}

const auto concept__def = concept_bot | concept_top | concept_atomic_state | concept_atomic_goal | concept_intersection | concept_union | concept_negation
                          | concept_value_restriction | concept_existential_quantification | concept_role_value_map_containment
                          | concept_role_value_map_equality | concept_nominal;
const auto concept_root_def = concept_ > eoi;

const auto concept_bot_def = with_constructor_parentheses(lit(ast::ConceptBot::keyword) >> x3::attr(ast::ConceptBot {}));
const auto concept_top_def = with_constructor_parentheses(lit(ast::ConceptTop::keyword) >> x3::attr(ast::ConceptTop {}));
const auto concept_atomic_state_def = with_constructor_parentheses(lit(ast::ConceptAtomicState::keyword) > predicate_name_string_parser());
const auto concept_atomic_goal_def = with_constructor_parentheses(lit(ast::ConceptAtomicGoal::keyword) > predicate_name_string_parser() > bool_string_parser());
const auto concept_intersection_def = with_constructor_parentheses(lit(ast::ConceptIntersection::keyword) > concept_choice > concept_choice);
const auto concept_union_def = with_constructor_parentheses(lit(ast::ConceptUnion::keyword) > concept_choice > concept_choice);
const auto concept_negation_def = with_constructor_parentheses(lit(ast::ConceptNegation::keyword) > concept_choice);
const auto concept_value_restriction_def = with_constructor_parentheses(lit(ast::ConceptValueRestriction::keyword) > role_choice > concept_choice);
const auto concept_existential_quantification_def =
    with_constructor_parentheses(lit(ast::ConceptExistentialQuantification::keyword) > role_choice > concept_choice);
const auto concept_role_value_map_containment_def = with_constructor_parentheses(lit(ast::ConceptRoleValueMapContainment::keyword) > role_choice > role_choice);
const auto concept_role_value_map_equality_def = with_constructor_parentheses(lit(ast::ConceptRoleValueMapEquality::keyword) > role_choice > role_choice);
const auto concept_nominal_def = with_constructor_parentheses(lit(ast::ConceptNominal::keyword) > object_name_string_parser());
const auto concept_non_terminal_def = concept_non_terminal_string_parser();
const auto concept_choice_def = concept_non_terminal | concept_;
const auto concept_derivation_rule_def = (lit("(") >> concept_non_terminal)
                                         > ((lit("(") > (concept_choice % lit("or")) > lit(")")) | x3::repeat(1)[concept_choice]) > lit(")");

const auto role_def = role_universal | role_atomic_state | role_atomic_goal | role_intersection | role_union | role_complement | role_inverse | role_composition
                      | role_transitive_closure | role_reflexive_transitive_closure | role_restriction | role_identity;
const auto role_root_def = role > eoi;

const auto role_universal_def = with_constructor_parentheses(lit(ast::RoleUniversal::keyword) >> x3::attr(ast::RoleUniversal {}));
const auto role_atomic_state_def = with_constructor_parentheses(lit(ast::RoleAtomicState::keyword) > predicate_name_string_parser());
const auto role_atomic_goal_def = with_constructor_parentheses(lit(ast::RoleAtomicGoal::keyword) > predicate_name_string_parser() > bool_string_parser());
const auto role_intersection_def = with_constructor_parentheses(lit(ast::RoleIntersection::keyword) > role_choice > role_choice);
const auto role_union_def = with_constructor_parentheses(lit(ast::RoleUnion::keyword) > role_choice > role_choice);
const auto role_complement_def = with_constructor_parentheses(lit(ast::RoleComplement::keyword) > role_choice);
const auto role_inverse_def = with_constructor_parentheses(lit(ast::RoleInverse::keyword) > role_choice);
const auto role_composition_def = with_constructor_parentheses(lit(ast::RoleComposition::keyword) > role_choice > role_choice);
const auto role_transitive_closure_def = with_constructor_parentheses(lit(ast::RoleTransitiveClosure::keyword) > role_choice);
const auto role_reflexive_transitive_closure_def = with_constructor_parentheses(lit(ast::RoleReflexiveTransitiveClosure::keyword) > role_choice);
const auto role_restriction_def = with_constructor_parentheses(lit(ast::RoleRestriction::keyword) > role_choice > concept_choice);
const auto role_identity_def = with_constructor_parentheses(lit(ast::RoleIdentity::keyword) > concept_choice);
const auto role_non_terminal_def = role_non_terminal_string_parser();
const auto role_choice_def = role_non_terminal | role;
const auto role_derivation_rule_def = (lit("(") >> role_non_terminal) > ((lit("(") > (role_choice % lit("or")) > lit(")")) | x3::repeat(1)[role_choice])
                                      > lit(")");

const auto constructor_or_non_terminal_variant_def = concept_choice | role_choice;

const auto boolean_def = boolean_atomic_state | boolean_nonempty;
const auto boolean_root_def = boolean > eoi;
const auto boolean_atomic_state_def =
    with_constructor_parentheses(lit(ast::BooleanAtomicState::keyword) > predicate_name_string_parser() > bool_string_parser());
const auto boolean_nonempty_def = with_constructor_parentheses(lit(ast::BooleanNonempty::keyword) > constructor_or_non_terminal_variant);
const auto boolean_non_terminal_def = boolean_non_terminal_string_parser();
const auto boolean_choice_def = boolean_non_terminal | boolean;
const auto boolean_derivation_rule_def = (lit("(") >> boolean_non_terminal)
                                         > ((lit("(") > (boolean_choice % lit("or")) > lit(")")) | x3::repeat(1)[boolean_choice]) > lit(")");

const auto numerical_def = numerical_count | numerical_distance;
const auto numerical_root_def = numerical > eoi;
const auto numerical_count_def = with_constructor_parentheses(lit(ast::NumericalCount::keyword) > constructor_or_non_terminal_variant);
const auto numerical_distance_def = with_constructor_parentheses(lit(ast::NumericalDistance::keyword) > concept_choice > role_choice > concept_choice);
const auto numerical_non_terminal_def = numerical_non_terminal_string_parser();
const auto numerical_choice_def = numerical_non_terminal | numerical;
const auto numerical_derivation_rule_def = (lit("(") >> numerical_non_terminal)
                                           > ((lit("(") > (numerical_choice % lit("or")) > lit(")")) | x3::repeat(1)[numerical_choice]) > lit(")");

const auto derivation_rule_variant_def = concept_derivation_rule | role_derivation_rule | boolean_derivation_rule | numerical_derivation_rule;
const auto grammar_body_def = *derivation_rule_variant;
const auto grammar_def = lit("(") > grammar_body > lit(")");
const auto grammar_root_def = grammar > eoi;

BOOST_SPIRIT_DEFINE(concept_,
                    concept_root,
                    concept_bot,
                    concept_top,
                    concept_atomic_state,
                    concept_atomic_goal,
                    concept_intersection,
                    concept_union,
                    concept_negation,
                    concept_value_restriction,
                    concept_existential_quantification,
                    concept_role_value_map_containment,
                    concept_role_value_map_equality,
                    concept_nominal,
                    concept_non_terminal,
                    concept_choice,
                    concept_derivation_rule)

BOOST_SPIRIT_DEFINE(role,
                    role_root,
                    role_universal,
                    role_atomic_state,
                    role_atomic_goal,
                    role_intersection,
                    role_union,
                    role_complement,
                    role_inverse,
                    role_composition,
                    role_transitive_closure,
                    role_reflexive_transitive_closure,
                    role_restriction,
                    role_identity,
                    role_non_terminal,
                    role_choice,
                    role_derivation_rule)

BOOST_SPIRIT_DEFINE(constructor_or_non_terminal_variant)

BOOST_SPIRIT_DEFINE(boolean, boolean_root, boolean_atomic_state, boolean_nonempty, boolean_non_terminal, boolean_choice, boolean_derivation_rule)

BOOST_SPIRIT_DEFINE(numerical, numerical_root, numerical_count, numerical_distance, numerical_non_terminal, numerical_choice, numerical_derivation_rule)

BOOST_SPIRIT_DEFINE(derivation_rule_variant, grammar_body, grammar, grammar_root)

template<>
struct ConstructorClass<runir::kr::dl::ConceptTag> : x3::annotate_on_success
{
};

template<>
struct ConstructorRootClass<runir::kr::dl::ConceptTag> : ErrorHandlerBase
{
};

template<>
struct ConstructorClass<runir::kr::dl::RoleTag> : x3::annotate_on_success
{
};

template<>
struct ConstructorRootClass<runir::kr::dl::RoleTag> : ErrorHandlerBase
{
};

template<>
struct ConstructorClass<runir::kr::dl::BooleanTag> : x3::annotate_on_success
{
};

template<>
struct ConstructorRootClass<runir::kr::dl::BooleanTag> : ErrorHandlerBase
{
};

template<>
struct ConstructorClass<runir::kr::dl::NumericalTag> : x3::annotate_on_success
{
};

template<>
struct ConstructorRootClass<runir::kr::dl::NumericalTag> : ErrorHandlerBase
{
};

template<>
struct NonTerminalClass<runir::kr::dl::ConceptTag> : x3::annotate_on_success
{
};

template<>
struct NonTerminalClass<runir::kr::dl::RoleTag> : x3::annotate_on_success
{
};

template<>
struct NonTerminalClass<runir::kr::dl::BooleanTag> : x3::annotate_on_success
{
};

template<>
struct NonTerminalClass<runir::kr::dl::NumericalTag> : x3::annotate_on_success
{
};

template<>
struct ConstructorOrNonTerminalClass<runir::kr::dl::ConceptTag> : x3::annotate_on_success
{
};

template<>
struct ConstructorOrNonTerminalClass<runir::kr::dl::RoleTag> : x3::annotate_on_success
{
};

template<>
struct ConstructorOrNonTerminalClass<runir::kr::dl::BooleanTag> : x3::annotate_on_success
{
};

template<>
struct ConstructorOrNonTerminalClass<runir::kr::dl::NumericalTag> : x3::annotate_on_success
{
};

template<>
struct DerivationRuleClass<runir::kr::dl::ConceptTag> : x3::annotate_on_success
{
};

template<>
struct DerivationRuleClass<runir::kr::dl::RoleTag> : x3::annotate_on_success
{
};

template<>
struct DerivationRuleClass<runir::kr::dl::BooleanTag> : x3::annotate_on_success
{
};

template<>
struct DerivationRuleClass<runir::kr::dl::NumericalTag> : x3::annotate_on_success
{
};

struct ConceptBotClass : x3::annotate_on_success
{
};

struct ConceptTopClass : x3::annotate_on_success
{
};

struct ConceptAtomicStateClass : x3::annotate_on_success
{
};

struct ConceptAtomicGoalClass : x3::annotate_on_success
{
};

struct ConceptIntersectionClass : x3::annotate_on_success
{
};

struct ConceptUnionClass : x3::annotate_on_success
{
};

struct ConceptNegationClass : x3::annotate_on_success
{
};

struct ConceptValueRestrictionClass : x3::annotate_on_success
{
};

struct ConceptExistentialQuantificationClass : x3::annotate_on_success
{
};

struct ConceptRoleValueMapContainmentClass : x3::annotate_on_success
{
};

struct ConceptRoleValueMapEqualityClass : x3::annotate_on_success
{
};

struct ConceptNominalClass : x3::annotate_on_success
{
};

struct RoleUniversalClass : x3::annotate_on_success
{
};

struct RoleAtomicStateClass : x3::annotate_on_success
{
};

struct RoleAtomicGoalClass : x3::annotate_on_success
{
};

struct RoleIntersectionClass : x3::annotate_on_success
{
};

struct RoleUnionClass : x3::annotate_on_success
{
};

struct RoleComplementClass : x3::annotate_on_success
{
};

struct RoleInverseClass : x3::annotate_on_success
{
};

struct RoleCompositionClass : x3::annotate_on_success
{
};

struct RoleTransitiveClosureClass : x3::annotate_on_success
{
};

struct RoleReflexiveTransitiveClosureClass : x3::annotate_on_success
{
};

struct RoleRestrictionClass : x3::annotate_on_success
{
};

struct RoleIdentityClass : x3::annotate_on_success
{
};

struct ConstructorOrNonTerminalVariantClass : x3::annotate_on_success
{
};

struct BooleanAtomicStateClass : x3::annotate_on_success
{
};

struct BooleanNonemptyClass : x3::annotate_on_success
{
};

struct NumericalCountClass : x3::annotate_on_success
{
};

struct NumericalDistanceClass : x3::annotate_on_success
{
};

struct DerivationRuleVariantClass : x3::annotate_on_success
{
};

struct GrammarBodyClass : x3::annotate_on_success
{
};

struct GrammarClass : x3::annotate_on_success, ErrorHandlerBase
{
};

struct GrammarRootClass : ErrorHandlerBase
{
};

concept_type const& concept_parser() { return concept_; }
concept_root_type const& concept_root_parser() { return concept_root; }
concept_bot_type const& concept_bot_parser() { return concept_bot; }
concept_top_type const& concept_top_parser() { return concept_top; }
concept_atomic_state_type const& concept_atomic_state_parser() { return concept_atomic_state; }
concept_atomic_goal_type const& concept_atomic_goal_parser() { return concept_atomic_goal; }
concept_intersection_type const& concept_intersection_parser() { return concept_intersection; }
concept_union_type const& concept_union_parser() { return concept_union; }
concept_negation_type const& concept_negation_parser() { return concept_negation; }
concept_value_restriction_type const& concept_value_restriction_parser() { return concept_value_restriction; }
concept_existential_quantification_type const& concept_existential_quantification_parser() { return concept_existential_quantification; }
concept_role_value_map_containment_type const& concept_role_value_map_containment_parser() { return concept_role_value_map_containment; }
concept_role_value_map_equality_type const& concept_role_value_map_equality_parser() { return concept_role_value_map_equality; }
concept_nominal_type const& concept_nominal_parser() { return concept_nominal; }
concept_non_terminal_type const& concept_non_terminal_parser() { return concept_non_terminal; }
concept_choice_type const& concept_choice_parser() { return concept_choice; }
concept_derivation_rule_type const& concept_derivation_rule_parser() { return concept_derivation_rule; }

role_type const& role_parser() { return role; }
role_root_type const& role_root_parser() { return role_root; }
role_universal_type const& role_universal_parser() { return role_universal; }
role_atomic_state_type const& role_atomic_state_parser() { return role_atomic_state; }
role_atomic_goal_type const& role_atomic_goal_parser() { return role_atomic_goal; }
role_intersection_type const& role_intersection_parser() { return role_intersection; }
role_union_type const& role_union_parser() { return role_union; }
role_complement_type const& role_complement_parser() { return role_complement; }
role_inverse_type const& role_inverse_parser() { return role_inverse; }
role_composition_type const& role_composition_parser() { return role_composition; }
role_transitive_closure_type const& role_transitive_closure_parser() { return role_transitive_closure; }
role_reflexive_transitive_closure_type const& role_reflexive_transitive_closure_parser() { return role_reflexive_transitive_closure; }
role_restriction_type const& role_restriction_parser() { return role_restriction; }
role_identity_type const& role_identity_parser() { return role_identity; }
role_non_terminal_type const& role_non_terminal_parser() { return role_non_terminal; }
role_choice_type const& role_choice_parser() { return role_choice; }
role_derivation_rule_type const& role_derivation_rule_parser() { return role_derivation_rule; }

constructor_or_non_terminal_variant_type const& constructor_or_non_terminal_variant_parser() { return constructor_or_non_terminal_variant; }

boolean_type const& boolean_parser() { return boolean; }
boolean_root_type const& boolean_root_parser() { return boolean_root; }
boolean_atomic_state_type const& boolean_atomic_state_parser() { return boolean_atomic_state; }
boolean_nonempty_type const& boolean_nonempty_parser() { return boolean_nonempty; }
boolean_non_terminal_type const& boolean_non_terminal_parser() { return boolean_non_terminal; }
boolean_choice_type const& boolean_choice_parser() { return boolean_choice; }
boolean_derivation_rule_type const& boolean_derivation_rule_parser() { return boolean_derivation_rule; }

numerical_type const& numerical_parser() { return numerical; }
numerical_root_type const& numerical_root_parser() { return numerical_root; }
numerical_count_type const& numerical_count_parser() { return numerical_count; }
numerical_distance_type const& numerical_distance_parser() { return numerical_distance; }
numerical_non_terminal_type const& numerical_non_terminal_parser() { return numerical_non_terminal; }
numerical_choice_type const& numerical_choice_parser() { return numerical_choice; }
numerical_derivation_rule_type const& numerical_derivation_rule_parser() { return numerical_derivation_rule; }

derivation_rule_variant_type const& derivation_rule_variant_parser() { return derivation_rule_variant; }
grammar_body_type const& grammar_body_parser() { return grammar_body; }
grammar_type const& grammar_parser() { return grammar; }
grammar_root_type const& grammar_root_parser() { return grammar_root; }

}

#endif
