#ifndef RUNIR_SRC_KR_DL_GRAMMAR_PARSER_EXT_PARSER_DEF_HPP_
#define RUNIR_SRC_KR_DL_GRAMMAR_PARSER_EXT_PARSER_DEF_HPP_

#include "runir/kr/dl/grammar/parser/base/ast_adapted.hpp"
#include "runir/kr/dl/grammar/parser/error_handler.hpp"
#include "runir/kr/dl/grammar/parser/ext/ast_adapted.hpp"
#include "runir/kr/dl/grammar/parser/ext/parsers.hpp"

#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>

namespace runir::kr::dl::grammar::parser::ext
{
namespace x3 = boost::spirit::x3;
namespace base_ast = runir::kr::dl::grammar::parser::base::ast;

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

ext_concept_type const ext_concept = "ext_concept";
ext_concept_root_type const ext_concept_root = "ext_concept_root";
ext_role_type const ext_role = "ext_role";
ext_role_root_type const ext_role_root = "ext_role_root";
ext_concept_non_terminal_type const ext_concept_non_terminal = "ext_concept_non_terminal";
ext_role_non_terminal_type const ext_role_non_terminal = "ext_role_non_terminal";
ext_concept_choice_type const ext_concept_choice = "ext_concept_choice";
ext_role_choice_type const ext_role_choice = "ext_role_choice";

concept_register_type const concept_register = "concept_register";
role_register_type const role_register = "role_register";
concept_argument_type const concept_argument = "concept_argument";
role_argument_type const role_argument = "role_argument";
boolean_argument_type const boolean_argument = "boolean_argument";
numerical_argument_type const numerical_argument = "numerical_argument";

x3::rule<ConceptBotClass, base_ast::ConceptBot> const concept_bot = "concept_bot";
x3::rule<ConceptTopClass, base_ast::ConceptTop> const concept_top = "concept_top";
x3::rule<ConceptAtomicStateClass, base_ast::ConceptAtomicState> const concept_atomic_state = "concept_atomic_state";
x3::rule<ConceptAtomicGoalClass, base_ast::ConceptAtomicGoal> const concept_atomic_goal = "concept_atomic_goal";
x3::rule<ConceptIntersectionClass, base_ast::ConceptIntersection<runir::kr::ExtFamilyTag>> const concept_intersection = "concept_intersection";
x3::rule<ConceptUnionClass, base_ast::ConceptUnion<runir::kr::ExtFamilyTag>> const concept_union = "concept_union";
x3::rule<ConceptNegationClass, base_ast::ConceptNegation<runir::kr::ExtFamilyTag>> const concept_negation = "concept_negation";
x3::rule<ConceptValueRestrictionClass, base_ast::ConceptValueRestriction<runir::kr::ExtFamilyTag>> const concept_value_restriction =
    "concept_value_restriction";
x3::rule<ConceptExistentialQuantificationClass, base_ast::ConceptExistentialQuantification<runir::kr::ExtFamilyTag>> const concept_existential_quantification =
    "concept_existential_quantification";
x3::rule<ConceptAtLeastNumberRestrictionClass, base_ast::ConceptAtLeastNumberRestriction<runir::kr::ExtFamilyTag>> const concept_at_least_number_restriction =
    "concept_at_least_number_restriction";
x3::rule<ConceptAtMostNumberRestrictionClass, base_ast::ConceptAtMostNumberRestriction<runir::kr::ExtFamilyTag>> const concept_at_most_number_restriction =
    "concept_at_most_number_restriction";
x3::rule<ConceptExactNumberRestrictionClass, base_ast::ConceptExactNumberRestriction<runir::kr::ExtFamilyTag>> const concept_exact_number_restriction =
    "concept_exact_number_restriction";
x3::rule<ConceptQualifiedAtLeastNumberRestrictionClass, base_ast::ConceptQualifiedAtLeastNumberRestriction<runir::kr::ExtFamilyTag>> const
    concept_qualified_at_least_number_restriction = "concept_qualified_at_least_number_restriction";
x3::rule<ConceptQualifiedAtMostNumberRestrictionClass, base_ast::ConceptQualifiedAtMostNumberRestriction<runir::kr::ExtFamilyTag>> const
    concept_qualified_at_most_number_restriction = "concept_qualified_at_most_number_restriction";
x3::rule<ConceptQualifiedExactNumberRestrictionClass, base_ast::ConceptQualifiedExactNumberRestriction<runir::kr::ExtFamilyTag>> const
    concept_qualified_exact_number_restriction = "concept_qualified_exact_number_restriction";
x3::rule<ConceptRoleValueMapClass, base_ast::ConceptRoleValueMap<runir::kr::ExtFamilyTag>> const concept_role_value_map = "concept_role_value_map";
x3::rule<ConceptAgreementClass, base_ast::ConceptAgreement<runir::kr::ExtFamilyTag>> const concept_agreement = "concept_agreement";
x3::rule<ConceptRoleFillersClass, base_ast::ConceptRoleFillers<runir::kr::ExtFamilyTag>> const concept_role_fillers = "concept_role_fillers";
x3::rule<ConceptOneOfClass, base_ast::ConceptOneOf> const concept_one_of = "concept_one_of";
x3::rule<ConceptNominalClass, base_ast::ConceptNominal> const concept_nominal = "concept_nominal";

x3::rule<RoleUniversalClass, base_ast::RoleUniversal> const role_universal = "role_universal";
x3::rule<RoleAtomicStateClass, base_ast::RoleAtomicState> const role_atomic_state = "role_atomic_state";
x3::rule<RoleAtomicGoalClass, base_ast::RoleAtomicGoal> const role_atomic_goal = "role_atomic_goal";
x3::rule<RoleIntersectionClass, base_ast::RoleIntersection<runir::kr::ExtFamilyTag>> const role_intersection = "role_intersection";
x3::rule<RoleUnionClass, base_ast::RoleUnion<runir::kr::ExtFamilyTag>> const role_union = "role_union";
x3::rule<RoleComplementClass, base_ast::RoleComplement<runir::kr::ExtFamilyTag>> const role_complement = "role_complement";
x3::rule<RoleInverseClass, base_ast::RoleInverse<runir::kr::ExtFamilyTag>> const role_inverse = "role_inverse";
x3::rule<RoleCompositionClass, base_ast::RoleComposition<runir::kr::ExtFamilyTag>> const role_composition = "role_composition";
x3::rule<RoleTransitiveClosureClass, base_ast::RoleTransitiveClosure<runir::kr::ExtFamilyTag>> const role_transitive_closure = "role_transitive_closure";
x3::rule<RoleReflexiveTransitiveClosureClass, base_ast::RoleReflexiveTransitiveClosure<runir::kr::ExtFamilyTag>> const role_reflexive_transitive_closure =
    "role_reflexive_transitive_closure";
x3::rule<RoleRestrictionClass, base_ast::RoleRestriction<runir::kr::ExtFamilyTag>> const role_restriction = "role_restriction";
x3::rule<RoleIdentityClass, base_ast::RoleIdentity<runir::kr::ExtFamilyTag>> const role_identity = "role_identity";

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

template<typename Ast, typename Parser>
auto constructor_identifier_parser(Parser parser)
{
    return (((lit("(") >> lit(Ast::keyword)) > parser > lit(")")) | (lit(Ast::keyword) > parser));
}

const auto ext_concept_def = concept_bot | concept_top | concept_atomic_state | concept_atomic_goal | concept_intersection | concept_union | concept_negation
                             | concept_value_restriction | concept_existential_quantification | concept_at_least_number_restriction
                             | concept_at_most_number_restriction | concept_exact_number_restriction | concept_qualified_at_least_number_restriction
                             | concept_qualified_at_most_number_restriction | concept_qualified_exact_number_restriction | concept_role_value_map | concept_agreement
                             | concept_role_fillers | concept_one_of | concept_nominal | concept_register | concept_argument;
const auto ext_concept_root_def = ext_concept > eoi;
const auto ext_concept_non_terminal_def = concept_non_terminal_string_parser();
const auto ext_concept_choice_def = ext_concept_non_terminal | ext_concept;

const auto concept_bot_def = with_constructor_parentheses(lit(base_ast::ConceptBot::keyword) >> x3::attr(base_ast::ConceptBot {}));
const auto concept_top_def = with_constructor_parentheses(lit(base_ast::ConceptTop::keyword) >> x3::attr(base_ast::ConceptTop {}));
const auto concept_atomic_state_def = with_constructor_parentheses(lit(base_ast::ConceptAtomicState::keyword) > predicate_name_string_parser());
const auto concept_atomic_goal_def =
    with_constructor_parentheses(lit(base_ast::ConceptAtomicGoal::keyword) > predicate_name_string_parser() > bool_string_parser());
const auto concept_intersection_def =
    with_constructor_parentheses(lit(base_ast::ConceptIntersection<runir::kr::ExtFamilyTag>::keyword) > ext_concept_choice > ext_concept_choice);
const auto concept_union_def =
    with_constructor_parentheses(lit(base_ast::ConceptUnion<runir::kr::ExtFamilyTag>::keyword) > ext_concept_choice > ext_concept_choice);
const auto concept_negation_def = with_constructor_parentheses(lit(base_ast::ConceptNegation<runir::kr::ExtFamilyTag>::keyword) > ext_concept_choice);
const auto concept_value_restriction_def =
    with_constructor_parentheses(lit(base_ast::ConceptValueRestriction<runir::kr::ExtFamilyTag>::keyword) > ext_role_choice > ext_concept_choice);
const auto concept_existential_quantification_def =
    with_constructor_parentheses(lit(base_ast::ConceptExistentialQuantification<runir::kr::ExtFamilyTag>::keyword) > ext_role_choice > ext_concept_choice);
const auto concept_at_least_number_restriction_def =
    with_constructor_parentheses(lit(base_ast::ConceptAtLeastNumberRestriction<runir::kr::ExtFamilyTag>::keyword) > uint_ > ext_role_choice);
const auto concept_at_most_number_restriction_def =
    with_constructor_parentheses(lit(base_ast::ConceptAtMostNumberRestriction<runir::kr::ExtFamilyTag>::keyword) > uint_ > ext_role_choice);
const auto concept_exact_number_restriction_def =
    with_constructor_parentheses(lit(base_ast::ConceptExactNumberRestriction<runir::kr::ExtFamilyTag>::keyword) > uint_ > ext_role_choice);
const auto concept_qualified_at_least_number_restriction_def = with_constructor_parentheses(
    lit(base_ast::ConceptQualifiedAtLeastNumberRestriction<runir::kr::ExtFamilyTag>::keyword) > uint_ > ext_role_choice > ext_concept_choice);
const auto concept_qualified_at_most_number_restriction_def = with_constructor_parentheses(
    lit(base_ast::ConceptQualifiedAtMostNumberRestriction<runir::kr::ExtFamilyTag>::keyword) > uint_ > ext_role_choice > ext_concept_choice);
const auto concept_qualified_exact_number_restriction_def = with_constructor_parentheses(
    lit(base_ast::ConceptQualifiedExactNumberRestriction<runir::kr::ExtFamilyTag>::keyword) > uint_ > ext_role_choice > ext_concept_choice);
const auto concept_role_value_map_def =
    with_constructor_parentheses(lit(base_ast::ConceptRoleValueMap<runir::kr::ExtFamilyTag>::keyword) > ext_role_choice > ext_role_choice);
const auto concept_agreement_def =
    with_constructor_parentheses(lit(base_ast::ConceptAgreement<runir::kr::ExtFamilyTag>::keyword) > ext_role_choice > ext_role_choice);
const auto concept_role_fillers_def =
    with_constructor_parentheses(lit(base_ast::ConceptRoleFillers<runir::kr::ExtFamilyTag>::keyword) > ext_role_choice > +object_name_string_parser());
const auto concept_one_of_def = with_constructor_parentheses(lit(base_ast::ConceptOneOf::keyword) > +object_name_string_parser());
const auto concept_nominal_def = with_constructor_parentheses(lit(base_ast::ConceptNominal::keyword) > object_name_string_parser());

const auto ext_role_def = role_universal | role_atomic_state | role_atomic_goal | role_intersection | role_union | role_complement | role_inverse
                          | role_composition | role_transitive_closure | role_reflexive_transitive_closure | role_restriction | role_identity | role_register
                          | role_argument;
const auto ext_role_root_def = ext_role > eoi;
const auto ext_role_non_terminal_def = role_non_terminal_string_parser();
const auto ext_role_choice_def = ext_role_non_terminal | ext_role;

const auto role_universal_def = with_constructor_parentheses(lit(base_ast::RoleUniversal::keyword) >> x3::attr(base_ast::RoleUniversal {}));
const auto role_atomic_state_def = with_constructor_parentheses(lit(base_ast::RoleAtomicState::keyword) > predicate_name_string_parser());
const auto role_atomic_goal_def = with_constructor_parentheses(lit(base_ast::RoleAtomicGoal::keyword) > predicate_name_string_parser() > bool_string_parser());
const auto role_intersection_def =
    with_constructor_parentheses(lit(base_ast::RoleIntersection<runir::kr::ExtFamilyTag>::keyword) > ext_role_choice > ext_role_choice);
const auto role_union_def = with_constructor_parentheses(lit(base_ast::RoleUnion<runir::kr::ExtFamilyTag>::keyword) > ext_role_choice > ext_role_choice);
const auto role_complement_def = with_constructor_parentheses(lit(base_ast::RoleComplement<runir::kr::ExtFamilyTag>::keyword) > ext_role_choice);
const auto role_inverse_def = with_constructor_parentheses(lit(base_ast::RoleInverse<runir::kr::ExtFamilyTag>::keyword) > ext_role_choice);
const auto role_composition_def =
    with_constructor_parentheses(lit(base_ast::RoleComposition<runir::kr::ExtFamilyTag>::keyword) > ext_role_choice > ext_role_choice);
const auto role_transitive_closure_def = with_constructor_parentheses(lit(base_ast::RoleTransitiveClosure<runir::kr::ExtFamilyTag>::keyword) > ext_role_choice);
const auto role_reflexive_transitive_closure_def =
    with_constructor_parentheses(lit(base_ast::RoleReflexiveTransitiveClosure<runir::kr::ExtFamilyTag>::keyword) > ext_role_choice);
const auto role_restriction_def =
    with_constructor_parentheses(lit(base_ast::RoleRestriction<runir::kr::ExtFamilyTag>::keyword) > ext_role_choice > ext_concept_choice);
const auto role_identity_def = with_constructor_parentheses(lit(base_ast::RoleIdentity<runir::kr::ExtFamilyTag>::keyword) > ext_concept_choice);

const auto concept_register_def = constructor_identifier_parser<ast::ConceptRegister>(uint_);
const auto role_register_def = constructor_identifier_parser<ast::RoleRegister>(uint_);
const auto concept_argument_def = constructor_identifier_parser<ast::Argument<runir::kr::dl::ConceptTag>>(uint_);
const auto role_argument_def = constructor_identifier_parser<ast::Argument<runir::kr::dl::RoleTag>>(uint_);
const auto boolean_argument_def = constructor_identifier_parser<ast::Argument<runir::kr::dl::BooleanTag>>(uint_);
const auto numerical_argument_def = constructor_identifier_parser<ast::Argument<runir::kr::dl::NumericalTag>>(uint_);

BOOST_SPIRIT_DEFINE(ext_concept,
                    ext_concept_root,
                    ext_role,
                    ext_role_root,
                    ext_concept_non_terminal,
                    ext_role_non_terminal,
                    ext_concept_choice,
                    ext_role_choice)
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
BOOST_SPIRIT_DEFINE(concept_register, role_register, concept_argument, role_argument, boolean_argument, numerical_argument)

struct ConceptRegisterClass : x3::annotate_on_success
{
};

struct RoleRegisterClass : x3::annotate_on_success
{
};

template<runir::kr::dl::CategoryTag Category>
struct ArgumentClass : x3::annotate_on_success
{
};

ext_concept_type const& ext_concept_parser() { return ext_concept; }
ext_concept_root_type const& ext_concept_root_parser() { return ext_concept_root; }
ext_role_type const& ext_role_parser() { return ext_role; }
ext_role_root_type const& ext_role_root_parser() { return ext_role_root; }
ext_concept_non_terminal_type const& ext_concept_non_terminal_parser() { return ext_concept_non_terminal; }
ext_role_non_terminal_type const& ext_role_non_terminal_parser() { return ext_role_non_terminal; }
ext_concept_choice_type const& ext_concept_choice_parser() { return ext_concept_choice; }
ext_role_choice_type const& ext_role_choice_parser() { return ext_role_choice; }

concept_register_type const& concept_register_parser() { return concept_register; }
role_register_type const& role_register_parser() { return role_register; }
concept_argument_type const& concept_argument_parser() { return concept_argument; }
role_argument_type const& role_argument_parser() { return role_argument; }
boolean_argument_type const& boolean_argument_parser() { return boolean_argument; }
numerical_argument_type const& numerical_argument_parser() { return numerical_argument; }

}  // namespace runir::kr::dl::grammar::parser::ext

#endif
