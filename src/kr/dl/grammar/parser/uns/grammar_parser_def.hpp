#ifndef RUNIR_SRC_GRAMMAR_PARSER_UNS_GRAMMAR_PARSER_DEF_HPP_
#define RUNIR_SRC_GRAMMAR_PARSER_UNS_GRAMMAR_PARSER_DEF_HPP_

#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/dl/grammar/ast/ast_adapted.hpp"
#include "runir/kr/dl/grammar/ast/uns_ast.hpp"
#include "runir/kr/dl/grammar/ast/uns_ast_adapted.hpp"
#include "runir/kr/dl/grammar/parser/uns/grammar_parsers.hpp"
#include "runir/kr/dl/grammar/parser/error_handler.hpp"

#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>

namespace runir::kr::dl::grammar::parser::uns::grammar
{
namespace x3 = boost::spirit::x3;
namespace grammar_ast = runir::kr::dl::grammar::ast;

using x3::eoi;
using x3::eps;
using x3::lexeme;
using x3::lit;
using x3::omit;
using x3::raw;
using x3::uint_;

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
concept_at_least_number_restriction_type const concept_at_least_number_restriction = "concept_at_least_number_restriction";
concept_at_most_number_restriction_type const concept_at_most_number_restriction = "concept_at_most_number_restriction";
concept_exact_number_restriction_type const concept_exact_number_restriction = "concept_exact_number_restriction";
concept_qualified_at_least_number_restriction_type const concept_qualified_at_least_number_restriction = "concept_qualified_at_least_number_restriction";
concept_qualified_at_most_number_restriction_type const concept_qualified_at_most_number_restriction = "concept_qualified_at_most_number_restriction";
concept_qualified_exact_number_restriction_type const concept_qualified_exact_number_restriction = "concept_qualified_exact_number_restriction";
concept_role_value_map_type const concept_role_value_map = "concept_role_value_map";
concept_agreement_type const concept_agreement = "concept_agreement";
concept_role_fillers_type const concept_role_fillers = "concept_role_fillers";
concept_one_of_type const concept_one_of = "concept_one_of";
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
boolean_atomic_goal_type const boolean_atomic_goal = "boolean_atomic_goal";
boolean_nonempty_type const boolean_nonempty = "boolean_nonempty";
boolean_eq_type const boolean_eq = "boolean_eq";
boolean_neq_type const boolean_neq = "boolean_neq";
boolean_lt_type const boolean_lt = "boolean_lt";
boolean_le_type const boolean_le = "boolean_le";
boolean_gt_type const boolean_gt = "boolean_gt";
boolean_ge_type const boolean_ge = "boolean_ge";
boolean_constant_type const boolean_constant = "boolean_constant";
boolean_non_terminal_type const boolean_non_terminal = "boolean_non_terminal";
boolean_choice_type const boolean_choice = "boolean_choice";
boolean_derivation_rule_type const boolean_derivation_rule = "boolean_derivation_rule";

numerical_type const numerical = "numerical";
numerical_root_type const numerical_root = "numerical_root";
numerical_count_type const numerical_count = "numerical_count";
numerical_distance_type const numerical_distance = "numerical_distance";
numerical_eq_type const numerical_eq = "numerical_eq";
numerical_neq_type const numerical_neq = "numerical_neq";
numerical_lt_type const numerical_lt = "numerical_lt";
numerical_le_type const numerical_le = "numerical_le";
numerical_gt_type const numerical_gt = "numerical_gt";
numerical_ge_type const numerical_ge = "numerical_ge";
numerical_constant_type const numerical_constant = "numerical_constant";
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

inline auto true_string_parser() { return x3::lit(runir::kr::dl::TrueTag::keyword) >> x3::attr(true); }

inline auto false_string_parser() { return x3::lit(runir::kr::dl::FalseTag::keyword) >> x3::attr(false); }

inline auto bool_string_parser() { return x3::lexeme[true_string_parser() | false_string_parser()]; }

template<typename Parser>
auto with_constructor_parentheses(Parser parser)
{
    return (lit("(") >> parser >> lit(")")) | parser;
}

const auto concept__def = concept_bot | concept_top | concept_atomic_state | concept_atomic_goal | concept_intersection | concept_union | concept_negation
                          | concept_value_restriction | concept_existential_quantification | concept_qualified_at_least_number_restriction
                          | concept_qualified_at_most_number_restriction | concept_qualified_exact_number_restriction | concept_at_least_number_restriction
                          | concept_at_most_number_restriction | concept_exact_number_restriction | concept_role_value_map | concept_agreement
                          | concept_role_fillers | concept_one_of | concept_nominal;
const auto concept_root_def = concept_ > eoi;

const auto concept_bot_def = with_constructor_parentheses(lit(grammar_ast::ConceptBot<runir::kr::UnsFamilyTag>::keyword)
                                                          >> x3::attr(grammar_ast::ConceptBot<runir::kr::UnsFamilyTag> {}));
const auto concept_top_def = with_constructor_parentheses(lit(grammar_ast::ConceptTop<runir::kr::UnsFamilyTag>::keyword)
                                                          >> x3::attr(grammar_ast::ConceptTop<runir::kr::UnsFamilyTag> {}));
const auto concept_atomic_state_def =
    with_constructor_parentheses(lit(grammar_ast::ConceptAtomicState<runir::kr::UnsFamilyTag>::keyword) > predicate_name_string_parser());
const auto concept_atomic_goal_def = with_constructor_parentheses(lit(grammar_ast::ConceptAtomicGoal<runir::kr::UnsFamilyTag>::keyword)
                                                                  > predicate_name_string_parser() > bool_string_parser());
const auto concept_intersection_def =
    with_constructor_parentheses(lit(grammar_ast::ConceptIntersection<runir::kr::UnsFamilyTag>::keyword) > concept_choice > concept_choice);
const auto concept_union_def =
    with_constructor_parentheses(lit(grammar_ast::ConceptUnion<runir::kr::UnsFamilyTag>::keyword) > concept_choice > concept_choice);
const auto concept_negation_def = with_constructor_parentheses(lit(grammar_ast::ConceptNegation<runir::kr::UnsFamilyTag>::keyword) > concept_choice);
const auto concept_value_restriction_def =
    with_constructor_parentheses(lit(grammar_ast::ConceptValueRestriction<runir::kr::UnsFamilyTag>::keyword) > role_choice > concept_choice);
const auto concept_existential_quantification_def =
    with_constructor_parentheses(lit(grammar_ast::ConceptExistentialQuantification<runir::kr::UnsFamilyTag>::keyword) > role_choice > concept_choice);
const auto concept_at_least_number_restriction_def =
    with_constructor_parentheses(lit(grammar_ast::ConceptAtLeastNumberRestriction<runir::kr::UnsFamilyTag>::keyword) > uint_ > role_choice);
const auto concept_at_most_number_restriction_def =
    with_constructor_parentheses(lit(grammar_ast::ConceptAtMostNumberRestriction<runir::kr::UnsFamilyTag>::keyword) > uint_ > role_choice);
const auto concept_exact_number_restriction_def =
    with_constructor_parentheses(lit(grammar_ast::ConceptExactNumberRestriction<runir::kr::UnsFamilyTag>::keyword) > uint_ > role_choice);
const auto concept_qualified_at_least_number_restriction_def = with_constructor_parentheses(
    lit(grammar_ast::ConceptQualifiedAtLeastNumberRestriction<runir::kr::UnsFamilyTag>::keyword) > uint_ > role_choice > concept_choice);
const auto concept_qualified_at_most_number_restriction_def = with_constructor_parentheses(
    lit(grammar_ast::ConceptQualifiedAtMostNumberRestriction<runir::kr::UnsFamilyTag>::keyword) > uint_ > role_choice > concept_choice);
const auto concept_qualified_exact_number_restriction_def = with_constructor_parentheses(
    lit(grammar_ast::ConceptQualifiedExactNumberRestriction<runir::kr::UnsFamilyTag>::keyword) > uint_ > role_choice > concept_choice);
const auto concept_role_value_map_def =
    with_constructor_parentheses(lit(grammar_ast::ConceptRoleValueMap<runir::kr::UnsFamilyTag>::keyword) > role_choice > role_choice);
const auto concept_agreement_def =
    with_constructor_parentheses(lit(grammar_ast::ConceptAgreement<runir::kr::UnsFamilyTag>::keyword) > role_choice > role_choice);
const auto concept_role_fillers_def =
    with_constructor_parentheses(lit(grammar_ast::ConceptRoleFillers<runir::kr::UnsFamilyTag>::keyword) > role_choice > +object_name_string_parser());
const auto concept_one_of_def = with_constructor_parentheses(lit(grammar_ast::ConceptOneOf<runir::kr::UnsFamilyTag>::keyword) > +object_name_string_parser());
const auto concept_nominal_def =
    with_constructor_parentheses(lit(grammar_ast::ConceptNominal<runir::kr::UnsFamilyTag>::keyword) > object_name_string_parser());
const auto concept_non_terminal_def = concept_non_terminal_string_parser();
const auto concept_choice_def = concept_non_terminal | concept_;
const auto concept_derivation_rule_def = (lit("(") >> concept_non_terminal)
                                         > ((lit("(") > (concept_choice % lit("or")) > lit(")")) | x3::repeat(1)[concept_choice]) > lit(")");

const auto role_def = role_universal | role_atomic_state | role_atomic_goal | role_intersection | role_union | role_complement | role_inverse | role_composition
                      | role_transitive_closure | role_reflexive_transitive_closure | role_restriction | role_identity;
const auto role_root_def = role > eoi;

const auto role_universal_def = with_constructor_parentheses(lit(grammar_ast::RoleUniversal<runir::kr::UnsFamilyTag>::keyword)
                                                             >> x3::attr(grammar_ast::RoleUniversal<runir::kr::UnsFamilyTag> {}));
const auto role_atomic_state_def =
    with_constructor_parentheses(lit(grammar_ast::RoleAtomicState<runir::kr::UnsFamilyTag>::keyword) > predicate_name_string_parser());
const auto role_atomic_goal_def =
    with_constructor_parentheses(lit(grammar_ast::RoleAtomicGoal<runir::kr::UnsFamilyTag>::keyword) > predicate_name_string_parser() > bool_string_parser());
const auto role_intersection_def =
    with_constructor_parentheses(lit(grammar_ast::RoleIntersection<runir::kr::UnsFamilyTag>::keyword) > role_choice > role_choice);
const auto role_union_def = with_constructor_parentheses(lit(grammar_ast::RoleUnion<runir::kr::UnsFamilyTag>::keyword) > role_choice > role_choice);
const auto role_complement_def = with_constructor_parentheses(lit(grammar_ast::RoleComplement<runir::kr::UnsFamilyTag>::keyword) > role_choice);
const auto role_inverse_def = with_constructor_parentheses(lit(grammar_ast::RoleInverse<runir::kr::UnsFamilyTag>::keyword) > role_choice);
const auto role_composition_def =
    with_constructor_parentheses(lit(grammar_ast::RoleComposition<runir::kr::UnsFamilyTag>::keyword) > role_choice > role_choice);
const auto role_transitive_closure_def = with_constructor_parentheses(lit(grammar_ast::RoleTransitiveClosure<runir::kr::UnsFamilyTag>::keyword) > role_choice);
const auto role_reflexive_transitive_closure_def =
    with_constructor_parentheses(lit(grammar_ast::RoleReflexiveTransitiveClosure<runir::kr::UnsFamilyTag>::keyword) > role_choice);
const auto role_restriction_def =
    with_constructor_parentheses(lit(grammar_ast::RoleRestriction<runir::kr::UnsFamilyTag>::keyword) > role_choice > concept_choice);
const auto role_identity_def = with_constructor_parentheses(lit(grammar_ast::RoleIdentity<runir::kr::UnsFamilyTag>::keyword) > concept_choice);
const auto role_non_terminal_def = role_non_terminal_string_parser();
const auto role_choice_def = role_non_terminal | role;
const auto role_derivation_rule_def = (lit("(") >> role_non_terminal) > ((lit("(") > (role_choice % lit("or")) > lit(")")) | x3::repeat(1)[role_choice])
                                      > lit(")");

const auto constructor_or_non_terminal_variant_def = concept_choice | role_choice;

const auto boolean_def = boolean_atomic_state | boolean_atomic_goal | boolean_nonempty | boolean_eq | boolean_neq | boolean_lt | boolean_le | boolean_gt | boolean_ge | numerical_eq | numerical_neq | numerical_lt | numerical_le | numerical_gt | numerical_ge | boolean_constant;
const auto boolean_root_def = boolean > eoi;
const auto boolean_atomic_state_def = with_constructor_parentheses(lit(grammar_ast::BooleanAtomicState<runir::kr::UnsFamilyTag>::keyword)
                                                                   > predicate_name_string_parser() > bool_string_parser());
const auto boolean_atomic_goal_def = with_constructor_parentheses(lit(grammar_ast::BooleanAtomicGoal<runir::kr::UnsFamilyTag>::keyword)
                                                                  > predicate_name_string_parser() > bool_string_parser());
const auto boolean_nonempty_def =
    with_constructor_parentheses(lit(grammar_ast::BooleanNonempty<runir::kr::UnsFamilyTag>::keyword) > constructor_or_non_terminal_variant);
const auto boolean_eq_def =
    with_constructor_parentheses(lit(grammar_ast::BooleanEq<runir::kr::UnsFamilyTag>::keyword) > boolean_choice > boolean_choice);
const auto boolean_neq_def =
    with_constructor_parentheses(lit(grammar_ast::BooleanNeq<runir::kr::UnsFamilyTag>::keyword) > boolean_choice > boolean_choice);
const auto boolean_lt_def =
    with_constructor_parentheses(lit(grammar_ast::BooleanLt<runir::kr::UnsFamilyTag>::keyword) > boolean_choice > boolean_choice);
const auto boolean_le_def =
    with_constructor_parentheses(lit(grammar_ast::BooleanLe<runir::kr::UnsFamilyTag>::keyword) > boolean_choice > boolean_choice);
const auto boolean_gt_def =
    with_constructor_parentheses(lit(grammar_ast::BooleanGt<runir::kr::UnsFamilyTag>::keyword) > boolean_choice > boolean_choice);
const auto boolean_ge_def =
    with_constructor_parentheses(lit(grammar_ast::BooleanGe<runir::kr::UnsFamilyTag>::keyword) > boolean_choice > boolean_choice);
const auto numerical_eq_def =
    with_constructor_parentheses(lit(grammar_ast::NumericalEq<runir::kr::UnsFamilyTag>::keyword) > numerical_choice > numerical_choice);
const auto numerical_neq_def =
    with_constructor_parentheses(lit(grammar_ast::NumericalNeq<runir::kr::UnsFamilyTag>::keyword) > numerical_choice > numerical_choice);
const auto numerical_lt_def =
    with_constructor_parentheses(lit(grammar_ast::NumericalLt<runir::kr::UnsFamilyTag>::keyword) > numerical_choice > numerical_choice);
const auto numerical_le_def =
    with_constructor_parentheses(lit(grammar_ast::NumericalLe<runir::kr::UnsFamilyTag>::keyword) > numerical_choice > numerical_choice);
const auto numerical_gt_def =
    with_constructor_parentheses(lit(grammar_ast::NumericalGt<runir::kr::UnsFamilyTag>::keyword) > numerical_choice > numerical_choice);
const auto numerical_ge_def =
    with_constructor_parentheses(lit(grammar_ast::NumericalGe<runir::kr::UnsFamilyTag>::keyword) > numerical_choice > numerical_choice);
const auto boolean_constant_def =
    with_constructor_parentheses(lit(grammar_ast::BooleanConstant<runir::kr::UnsFamilyTag>::keyword) > bool_string_parser());
const auto boolean_non_terminal_def = boolean_non_terminal_string_parser();
const auto boolean_choice_def = boolean_non_terminal | boolean;
const auto boolean_derivation_rule_def = (lit("(") >> boolean_non_terminal)
                                         > ((lit("(") > (boolean_choice % lit("or")) > lit(")")) | x3::repeat(1)[boolean_choice]) > lit(")");

const auto numerical_def = numerical_count | numerical_distance | numerical_constant;
const auto numerical_root_def = numerical > eoi;
const auto numerical_count_def =
    with_constructor_parentheses(lit(grammar_ast::NumericalCount<runir::kr::UnsFamilyTag>::keyword) > constructor_or_non_terminal_variant);
const auto numerical_distance_def =
    with_constructor_parentheses(lit(grammar_ast::NumericalDistance<runir::kr::UnsFamilyTag>::keyword) > concept_choice > role_choice > concept_choice);
const auto numerical_constant_def =
    with_constructor_parentheses(lit(grammar_ast::NumericalConstant<runir::kr::UnsFamilyTag>::keyword) > uint_);
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
                    concept_at_least_number_restriction,
                    concept_at_most_number_restriction,
                    concept_exact_number_restriction,
                    concept_qualified_at_least_number_restriction,
                    concept_qualified_at_most_number_restriction,
                    concept_qualified_exact_number_restriction,
                    concept_role_value_map,
                    concept_agreement,
                    concept_role_fillers,
                    concept_one_of,
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

BOOST_SPIRIT_DEFINE(boolean,
                    boolean_root,
                    boolean_atomic_state,
                    boolean_atomic_goal,
                    boolean_nonempty,
                    boolean_eq,
                    boolean_neq,
                    boolean_lt,
                    boolean_le,
                    boolean_gt,
                    boolean_ge,
                    boolean_constant,
                    boolean_non_terminal,
                    boolean_choice,
                    boolean_derivation_rule)

BOOST_SPIRIT_DEFINE(numerical,
                    numerical_root,
                    numerical_count,
                    numerical_distance,
                    numerical_eq,
                    numerical_neq,
                    numerical_lt,
                    numerical_le,
                    numerical_gt,
                    numerical_ge,
                    numerical_constant,
                    numerical_non_terminal,
                    numerical_choice,
                    numerical_derivation_rule)

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

struct ConceptAtLeastNumberRestrictionClass : x3::annotate_on_success
{
};

struct ConceptAtMostNumberRestrictionClass : x3::annotate_on_success
{
};

struct ConceptExactNumberRestrictionClass : x3::annotate_on_success
{
};

struct ConceptQualifiedAtLeastNumberRestrictionClass : x3::annotate_on_success
{
};

struct ConceptQualifiedAtMostNumberRestrictionClass : x3::annotate_on_success
{
};

struct ConceptQualifiedExactNumberRestrictionClass : x3::annotate_on_success
{
};

struct ConceptRoleValueMapClass : x3::annotate_on_success
{
};

struct ConceptAgreementClass : x3::annotate_on_success
{
};

struct ConceptRoleFillersClass : x3::annotate_on_success
{
};

struct ConceptOneOfClass : x3::annotate_on_success
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

struct BooleanAtomicGoalClass : x3::annotate_on_success
{
};

struct BooleanNonemptyClass : x3::annotate_on_success
{
};

struct BooleanEqClass : x3::annotate_on_success
{
};

struct BooleanNeqClass : x3::annotate_on_success
{
};

struct BooleanLtClass : x3::annotate_on_success
{
};

struct BooleanLeClass : x3::annotate_on_success
{
};

struct BooleanGtClass : x3::annotate_on_success
{
};

struct BooleanGeClass : x3::annotate_on_success
{
};

struct BooleanConstantClass : x3::annotate_on_success
{
};

struct NumericalEqClass : x3::annotate_on_success
{
};

struct NumericalNeqClass : x3::annotate_on_success
{
};

struct NumericalLtClass : x3::annotate_on_success
{
};

struct NumericalLeClass : x3::annotate_on_success
{
};

struct NumericalGtClass : x3::annotate_on_success
{
};

struct NumericalGeClass : x3::annotate_on_success
{
};

struct NumericalConstantClass : x3::annotate_on_success
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
concept_at_least_number_restriction_type const& concept_at_least_number_restriction_parser() { return concept_at_least_number_restriction; }
concept_at_most_number_restriction_type const& concept_at_most_number_restriction_parser() { return concept_at_most_number_restriction; }
concept_exact_number_restriction_type const& concept_exact_number_restriction_parser() { return concept_exact_number_restriction; }
concept_qualified_at_least_number_restriction_type const& concept_qualified_at_least_number_restriction_parser()
{
    return concept_qualified_at_least_number_restriction;
}
concept_qualified_at_most_number_restriction_type const& concept_qualified_at_most_number_restriction_parser()
{
    return concept_qualified_at_most_number_restriction;
}
concept_qualified_exact_number_restriction_type const& concept_qualified_exact_number_restriction_parser()
{
    return concept_qualified_exact_number_restriction;
}
concept_role_value_map_type const& concept_role_value_map_parser() { return concept_role_value_map; }
concept_agreement_type const& concept_agreement_parser() { return concept_agreement; }
concept_role_fillers_type const& concept_role_fillers_parser() { return concept_role_fillers; }
concept_one_of_type const& concept_one_of_parser() { return concept_one_of; }
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
boolean_atomic_goal_type const& boolean_atomic_goal_parser() { return boolean_atomic_goal; }
boolean_nonempty_type const& boolean_nonempty_parser() { return boolean_nonempty; }
boolean_eq_type const& boolean_eq_parser() { return boolean_eq; }
boolean_neq_type const& boolean_neq_parser() { return boolean_neq; }
boolean_lt_type const& boolean_lt_parser() { return boolean_lt; }
boolean_le_type const& boolean_le_parser() { return boolean_le; }
boolean_gt_type const& boolean_gt_parser() { return boolean_gt; }
boolean_ge_type const& boolean_ge_parser() { return boolean_ge; }
boolean_constant_type const& boolean_constant_parser() { return boolean_constant; }
numerical_eq_type const& numerical_eq_parser() { return numerical_eq; }
numerical_neq_type const& numerical_neq_parser() { return numerical_neq; }
numerical_lt_type const& numerical_lt_parser() { return numerical_lt; }
numerical_le_type const& numerical_le_parser() { return numerical_le; }
numerical_gt_type const& numerical_gt_parser() { return numerical_gt; }
numerical_ge_type const& numerical_ge_parser() { return numerical_ge; }
numerical_constant_type const& numerical_constant_parser() { return numerical_constant; }
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
