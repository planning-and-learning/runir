#ifndef RUNIR_SRC_KR_DL_GRAMMAR_PARSER_BASE_PARSER_DEF_HPP_
#define RUNIR_SRC_KR_DL_GRAMMAR_PARSER_BASE_PARSER_DEF_HPP_

#include "runir/kr/dl/grammar/parser/base/ast_adapted.hpp"
#include "runir/kr/dl/grammar/parser/base/parsers.hpp"
#include "runir/kr/dl/grammar/parser/error_handler.hpp"

#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>

namespace runir::kr::dl::grammar::parser::base
{
namespace x3 = boost::spirit::x3;

using x3::eoi;
using x3::lexeme;
using x3::lit;
using x3::omit;
using x3::raw;
using x3::uint_;
using x3::ascii::alnum;
using x3::ascii::alpha;
using x3::ascii::char_;
using x3::ascii::digit;

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

base_concept_type const base_concept = "base_concept";
base_concept_root_type const base_concept_root = "base_concept_root";
base_role_type const base_role = "base_role";
base_role_root_type const base_role_root = "base_role_root";
base_concept_non_terminal_type const base_concept_non_terminal = "base_concept_non_terminal";
base_role_non_terminal_type const base_role_non_terminal = "base_role_non_terminal";
base_concept_choice_type const base_concept_choice = "base_concept_choice";
base_role_choice_type const base_role_choice = "base_role_choice";

x3::rule<ConceptBotClass, ast::ConceptBot> const concept_bot = "concept_bot";
x3::rule<ConceptTopClass, ast::ConceptTop> const concept_top = "concept_top";
x3::rule<ConceptAtomicStateClass, ast::ConceptAtomicState> const concept_atomic_state = "concept_atomic_state";
x3::rule<ConceptAtomicGoalClass, ast::ConceptAtomicGoal> const concept_atomic_goal = "concept_atomic_goal";
x3::rule<ConceptIntersectionClass, ast::ConceptIntersection<runir::kr::BaseFamilyTag>> const concept_intersection = "concept_intersection";
x3::rule<ConceptUnionClass, ast::ConceptUnion<runir::kr::BaseFamilyTag>> const concept_union = "concept_union";
x3::rule<ConceptNegationClass, ast::ConceptNegation<runir::kr::BaseFamilyTag>> const concept_negation = "concept_negation";
x3::rule<ConceptValueRestrictionClass, ast::ConceptValueRestriction<runir::kr::BaseFamilyTag>> const concept_value_restriction = "concept_value_restriction";
x3::rule<ConceptExistentialQuantificationClass, ast::ConceptExistentialQuantification<runir::kr::BaseFamilyTag>> const concept_existential_quantification =
    "concept_existential_quantification";
x3::rule<ConceptAtLeastNumberRestrictionClass, ast::ConceptAtLeastNumberRestriction<runir::kr::BaseFamilyTag>> const concept_at_least_number_restriction =
    "concept_at_least_number_restriction";
x3::rule<ConceptAtMostNumberRestrictionClass, ast::ConceptAtMostNumberRestriction<runir::kr::BaseFamilyTag>> const concept_at_most_number_restriction =
    "concept_at_most_number_restriction";
x3::rule<ConceptExactNumberRestrictionClass, ast::ConceptExactNumberRestriction<runir::kr::BaseFamilyTag>> const concept_exact_number_restriction =
    "concept_exact_number_restriction";
x3::rule<ConceptQualifiedAtLeastNumberRestrictionClass, ast::ConceptQualifiedAtLeastNumberRestriction<runir::kr::BaseFamilyTag>> const
    concept_qualified_at_least_number_restriction = "concept_qualified_at_least_number_restriction";
x3::rule<ConceptQualifiedAtMostNumberRestrictionClass, ast::ConceptQualifiedAtMostNumberRestriction<runir::kr::BaseFamilyTag>> const
    concept_qualified_at_most_number_restriction = "concept_qualified_at_most_number_restriction";
x3::rule<ConceptQualifiedExactNumberRestrictionClass, ast::ConceptQualifiedExactNumberRestriction<runir::kr::BaseFamilyTag>> const
    concept_qualified_exact_number_restriction = "concept_qualified_exact_number_restriction";
x3::rule<ConceptRoleValueMapClass, ast::ConceptRoleValueMap<runir::kr::BaseFamilyTag>> const concept_role_value_map = "concept_role_value_map";
x3::rule<ConceptAgreementClass, ast::ConceptAgreement<runir::kr::BaseFamilyTag>> const concept_agreement = "concept_agreement";
x3::rule<ConceptRoleFillersClass, ast::ConceptRoleFillers<runir::kr::BaseFamilyTag>> const concept_role_fillers = "concept_role_fillers";
x3::rule<ConceptOneOfClass, ast::ConceptOneOf> const concept_one_of = "concept_one_of";
x3::rule<ConceptNominalClass, ast::ConceptNominal> const concept_nominal = "concept_nominal";

x3::rule<RoleUniversalClass, ast::RoleUniversal> const role_universal = "role_universal";
x3::rule<RoleAtomicStateClass, ast::RoleAtomicState> const role_atomic_state = "role_atomic_state";
x3::rule<RoleAtomicGoalClass, ast::RoleAtomicGoal> const role_atomic_goal = "role_atomic_goal";
x3::rule<RoleIntersectionClass, ast::RoleIntersection<runir::kr::BaseFamilyTag>> const role_intersection = "role_intersection";
x3::rule<RoleUnionClass, ast::RoleUnion<runir::kr::BaseFamilyTag>> const role_union = "role_union";
x3::rule<RoleComplementClass, ast::RoleComplement<runir::kr::BaseFamilyTag>> const role_complement = "role_complement";
x3::rule<RoleInverseClass, ast::RoleInverse<runir::kr::BaseFamilyTag>> const role_inverse = "role_inverse";
x3::rule<RoleCompositionClass, ast::RoleComposition<runir::kr::BaseFamilyTag>> const role_composition = "role_composition";
x3::rule<RoleTransitiveClosureClass, ast::RoleTransitiveClosure<runir::kr::BaseFamilyTag>> const role_transitive_closure = "role_transitive_closure";
x3::rule<RoleReflexiveTransitiveClosureClass, ast::RoleReflexiveTransitiveClosure<runir::kr::BaseFamilyTag>> const role_reflexive_transitive_closure =
    "role_reflexive_transitive_closure";
x3::rule<RoleRestrictionClass, ast::RoleRestriction<runir::kr::BaseFamilyTag>> const role_restriction = "role_restriction";
x3::rule<RoleIdentityClass, ast::RoleIdentity<runir::kr::BaseFamilyTag>> const role_identity = "role_identity";

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

const auto base_concept_def = concept_bot | concept_top | concept_atomic_state | concept_atomic_goal | concept_intersection | concept_union | concept_negation
                              | concept_value_restriction | concept_existential_quantification | concept_qualified_at_least_number_restriction
                              | concept_qualified_at_most_number_restriction | concept_qualified_exact_number_restriction | concept_at_least_number_restriction
                              | concept_at_most_number_restriction | concept_exact_number_restriction | concept_role_value_map | concept_agreement
                              | concept_role_fillers | concept_one_of | concept_nominal;
const auto base_concept_root_def = base_concept > eoi;
const auto base_concept_non_terminal_def = concept_non_terminal_string_parser();
const auto base_concept_choice_def = base_concept_non_terminal | base_concept;

const auto concept_bot_def = with_constructor_parentheses(lit(ast::ConceptBot::keyword) >> x3::attr(ast::ConceptBot {}));
const auto concept_top_def = with_constructor_parentheses(lit(ast::ConceptTop::keyword) >> x3::attr(ast::ConceptTop {}));
const auto concept_atomic_state_def = with_constructor_parentheses(lit(ast::ConceptAtomicState::keyword) > predicate_name_string_parser());
const auto concept_atomic_goal_def = with_constructor_parentheses(lit(ast::ConceptAtomicGoal::keyword) > predicate_name_string_parser() > bool_string_parser());
const auto concept_intersection_def =
    with_constructor_parentheses(lit(ast::ConceptIntersection<runir::kr::BaseFamilyTag>::keyword) > base_concept_choice > base_concept_choice);
const auto concept_union_def =
    with_constructor_parentheses(lit(ast::ConceptUnion<runir::kr::BaseFamilyTag>::keyword) > base_concept_choice > base_concept_choice);
const auto concept_negation_def = with_constructor_parentheses(lit(ast::ConceptNegation<runir::kr::BaseFamilyTag>::keyword) > base_concept_choice);
const auto concept_value_restriction_def =
    with_constructor_parentheses(lit(ast::ConceptValueRestriction<runir::kr::BaseFamilyTag>::keyword) > base_role_choice > base_concept_choice);
const auto concept_existential_quantification_def =
    with_constructor_parentheses(lit(ast::ConceptExistentialQuantification<runir::kr::BaseFamilyTag>::keyword) > base_role_choice > base_concept_choice);
const auto concept_at_least_number_restriction_def =
    with_constructor_parentheses(lit(ast::ConceptAtLeastNumberRestriction<runir::kr::BaseFamilyTag>::keyword) > uint_ > base_role_choice);
const auto concept_at_most_number_restriction_def =
    with_constructor_parentheses(lit(ast::ConceptAtMostNumberRestriction<runir::kr::BaseFamilyTag>::keyword) > uint_ > base_role_choice);
const auto concept_exact_number_restriction_def =
    with_constructor_parentheses(lit(ast::ConceptExactNumberRestriction<runir::kr::BaseFamilyTag>::keyword) > uint_ > base_role_choice);
const auto concept_qualified_at_least_number_restriction_def = with_constructor_parentheses(
    lit(ast::ConceptQualifiedAtLeastNumberRestriction<runir::kr::BaseFamilyTag>::keyword) > uint_ > base_role_choice > base_concept_choice);
const auto concept_qualified_at_most_number_restriction_def = with_constructor_parentheses(
    lit(ast::ConceptQualifiedAtMostNumberRestriction<runir::kr::BaseFamilyTag>::keyword) > uint_ > base_role_choice > base_concept_choice);
const auto concept_qualified_exact_number_restriction_def = with_constructor_parentheses(
    lit(ast::ConceptQualifiedExactNumberRestriction<runir::kr::BaseFamilyTag>::keyword) > uint_ > base_role_choice > base_concept_choice);
const auto concept_role_value_map_def =
    with_constructor_parentheses(lit(ast::ConceptRoleValueMap<runir::kr::BaseFamilyTag>::keyword) > base_role_choice > base_role_choice);
const auto concept_agreement_def =
    with_constructor_parentheses(lit(ast::ConceptAgreement<runir::kr::BaseFamilyTag>::keyword) > base_role_choice > base_role_choice);
const auto concept_role_fillers_def =
    with_constructor_parentheses(lit(ast::ConceptRoleFillers<runir::kr::BaseFamilyTag>::keyword) > base_role_choice > +object_name_string_parser());
const auto concept_one_of_def = with_constructor_parentheses(lit(ast::ConceptOneOf::keyword) > +object_name_string_parser());
const auto concept_nominal_def = with_constructor_parentheses(lit(ast::ConceptNominal::keyword) > object_name_string_parser());

const auto base_role_def = role_universal | role_atomic_state | role_atomic_goal | role_intersection | role_union | role_complement | role_inverse
                           | role_composition | role_transitive_closure | role_reflexive_transitive_closure | role_restriction | role_identity;
const auto base_role_root_def = base_role > eoi;
const auto base_role_non_terminal_def = role_non_terminal_string_parser();
const auto base_role_choice_def = base_role_non_terminal | base_role;

const auto role_universal_def = with_constructor_parentheses(lit(ast::RoleUniversal::keyword) >> x3::attr(ast::RoleUniversal {}));
const auto role_atomic_state_def = with_constructor_parentheses(lit(ast::RoleAtomicState::keyword) > predicate_name_string_parser());
const auto role_atomic_goal_def = with_constructor_parentheses(lit(ast::RoleAtomicGoal::keyword) > predicate_name_string_parser() > bool_string_parser());
const auto role_intersection_def =
    with_constructor_parentheses(lit(ast::RoleIntersection<runir::kr::BaseFamilyTag>::keyword) > base_role_choice > base_role_choice);
const auto role_union_def = with_constructor_parentheses(lit(ast::RoleUnion<runir::kr::BaseFamilyTag>::keyword) > base_role_choice > base_role_choice);
const auto role_complement_def = with_constructor_parentheses(lit(ast::RoleComplement<runir::kr::BaseFamilyTag>::keyword) > base_role_choice);
const auto role_inverse_def = with_constructor_parentheses(lit(ast::RoleInverse<runir::kr::BaseFamilyTag>::keyword) > base_role_choice);
const auto role_composition_def =
    with_constructor_parentheses(lit(ast::RoleComposition<runir::kr::BaseFamilyTag>::keyword) > base_role_choice > base_role_choice);
const auto role_transitive_closure_def = with_constructor_parentheses(lit(ast::RoleTransitiveClosure<runir::kr::BaseFamilyTag>::keyword) > base_role_choice);
const auto role_reflexive_transitive_closure_def =
    with_constructor_parentheses(lit(ast::RoleReflexiveTransitiveClosure<runir::kr::BaseFamilyTag>::keyword) > base_role_choice);
const auto role_restriction_def =
    with_constructor_parentheses(lit(ast::RoleRestriction<runir::kr::BaseFamilyTag>::keyword) > base_role_choice > base_concept_choice);
const auto role_identity_def = with_constructor_parentheses(lit(ast::RoleIdentity<runir::kr::BaseFamilyTag>::keyword) > base_concept_choice);

BOOST_SPIRIT_DEFINE(base_concept,
                    base_concept_root,
                    base_role,
                    base_role_root,
                    base_concept_non_terminal,
                    base_role_non_terminal,
                    base_concept_choice,
                    base_role_choice)
BOOST_SPIRIT_DEFINE(concept_bot,
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
                    concept_nominal)
BOOST_SPIRIT_DEFINE(role_universal,
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
                    role_identity)

base_concept_type const& base_concept_parser() { return base_concept; }
base_concept_root_type const& base_concept_root_parser() { return base_concept_root; }
base_role_type const& base_role_parser() { return base_role; }
base_role_root_type const& base_role_root_parser() { return base_role_root; }
base_concept_non_terminal_type const& base_concept_non_terminal_parser() { return base_concept_non_terminal; }
base_role_non_terminal_type const& base_role_non_terminal_parser() { return base_role_non_terminal; }
base_concept_choice_type const& base_concept_choice_parser() { return base_concept_choice; }
base_role_choice_type const& base_role_choice_parser() { return base_role_choice; }

}  // namespace runir::kr::dl::grammar::parser::base

#endif
