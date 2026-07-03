#ifndef RUNIR_SRC_KR_DL_GRAMMAR_PARSER_PARSER_DEF_HPP_
#define RUNIR_SRC_KR_DL_GRAMMAR_PARSER_PARSER_DEF_HPP_

#include "runir/kr/dl/grammar/ast/ast_adapted.hpp"
#include "runir/kr/dl/grammar/parser/parsers.hpp"

#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>

namespace runir::kr::dl::grammar::parser
{
namespace x3 = boost::spirit::x3;
namespace grammar_ast = runir::kr::dl::grammar::ast;

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

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename Tag>
inline constexpr bool has_ast_constructor_tag_v = runir::kr::dl::TypeListContains<Tag, grammar_ast::AstConstructorTagsT<Family, Category>>::value;

template<bool Enabled, typename Parser>
auto maybe(Parser parser)
{
    if constexpr (Enabled)
        return parser;
    else
        return x3::eps(false);
}

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

template<runir::kr::dl::CategoryTag Category>
auto non_terminal_string_parser()
{
    if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
        return raw[lexeme[lit("c_") >> +digit]];
    else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
        return raw[lexeme[lit("r_") >> +digit]];
    else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
        return raw[lexeme[lit("b_") >> +digit]];
    else
        return raw[lexeme[lit("n_") >> +digit]];
}

#define RUNIR_DEFINE_FAMILY_PARSER(Family, prefix)                                                                                                             \
    constructor_type<Family, runir::kr::dl::ConceptTag> const prefix##_concept = "concept";                                                                    \
    constructor_root_type<Family, runir::kr::dl::ConceptTag> const prefix##_concept_root = "concept_root";                                                     \
    constructor_type<Family, runir::kr::dl::RoleTag> const prefix##_role = "role";                                                                             \
    constructor_root_type<Family, runir::kr::dl::RoleTag> const prefix##_role_root = "role_root";                                                              \
    constructor_type<Family, runir::kr::dl::BooleanTag> const prefix##_boolean = "boolean";                                                                    \
    constructor_root_type<Family, runir::kr::dl::BooleanTag> const prefix##_boolean_root = "boolean_root";                                                     \
    constructor_type<Family, runir::kr::dl::NumericalTag> const prefix##_numerical = "numerical";                                                              \
    constructor_root_type<Family, runir::kr::dl::NumericalTag> const prefix##_numerical_root = "numerical_root";                                               \
    non_terminal_type<Family, runir::kr::dl::ConceptTag> const prefix##_concept_non_terminal = "concept_non_terminal";                                         \
    choice_type<Family, runir::kr::dl::ConceptTag> const prefix##_concept_choice = "concept_choice";                                                           \
    non_terminal_type<Family, runir::kr::dl::RoleTag> const prefix##_role_non_terminal = "role_non_terminal";                                                  \
    choice_type<Family, runir::kr::dl::RoleTag> const prefix##_role_choice = "role_choice";                                                                    \
    non_terminal_type<Family, runir::kr::dl::BooleanTag> const prefix##_boolean_non_terminal = "boolean_non_terminal";                                         \
    choice_type<Family, runir::kr::dl::BooleanTag> const prefix##_boolean_choice = "boolean_choice";                                                           \
    non_terminal_type<Family, runir::kr::dl::NumericalTag> const prefix##_numerical_non_terminal = "numerical_non_terminal";                                   \
    choice_type<Family, runir::kr::dl::NumericalTag> const prefix##_numerical_choice = "numerical_choice";                                                     \
    constructor_or_non_terminal_variant_type<Family> const prefix##_constructor_or_non_terminal_variant = "constructor_or_non_terminal_variant";               \
    derivation_rule_type<Family, runir::kr::dl::ConceptTag> const prefix##_concept_derivation_rule = "concept_derivation_rule";                                \
    derivation_rule_type<Family, runir::kr::dl::RoleTag> const prefix##_role_derivation_rule = "role_derivation_rule";                                         \
    derivation_rule_type<Family, runir::kr::dl::BooleanTag> const prefix##_boolean_derivation_rule = "boolean_derivation_rule";                                \
    derivation_rule_type<Family, runir::kr::dl::NumericalTag> const prefix##_numerical_derivation_rule = "numerical_derivation_rule";                          \
    derivation_rule_variant_type<Family> const prefix##_derivation_rule_variant = "derivation_rule_variant";                                                   \
    grammar_body_type<Family> const prefix##_grammar_body = "grammar_body";                                                                                    \
    grammar_type<Family> const prefix##_grammar = "grammar";                                                                                                   \
    grammar_root_type<Family> const prefix##_grammar_root = "grammar_root";                                                                                    \
    constructor_tag_type<Family, runir::kr::dl::ConceptTag, runir::kr::dl::BotTag> const prefix##_concept_bot = "concept_bot";                                 \
    constructor_tag_type<Family, runir::kr::dl::ConceptTag, runir::kr::dl::TopTag> const prefix##_concept_top = "concept_top";                                 \
    constructor_tag_type<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ConceptAtomicStateSyntaxTag> const prefix##_concept_atomic_state =                  \
        "concept_atomic_state";                                                                                                                                \
    constructor_tag_type<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ConceptAtomicGoalSyntaxTag> const prefix##_concept_atomic_goal =                    \
        "concept_atomic_goal";                                                                                                                                 \
    constructor_tag_type<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ConceptIntersectionSyntaxTag> const prefix##_concept_intersection =                 \
        "concept_intersection";                                                                                                                                \
    constructor_tag_type<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ConceptUnionSyntaxTag> const prefix##_concept_union = "concept_union";              \
    constructor_tag_type<Family, runir::kr::dl::ConceptTag, runir::kr::dl::NegationTag> const prefix##_concept_negation = "concept_negation";                  \
    constructor_tag_type<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ValueRestrictionTag> const prefix##_concept_value_restriction =                     \
        "concept_value_restriction";                                                                                                                           \
    constructor_tag_type<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ExistentialQuantificationTag> const prefix##_concept_existential_quantification =   \
        "concept_existential_quantification";                                                                                                                  \
    constructor_tag_type<Family, runir::kr::dl::ConceptTag, runir::kr::dl::AtLeastNumberRestrictionTag> const prefix##_concept_at_least_number_restriction =   \
        "concept_at_least_number_restriction";                                                                                                                 \
    constructor_tag_type<Family, runir::kr::dl::ConceptTag, runir::kr::dl::AtMostNumberRestrictionTag> const prefix##_concept_at_most_number_restriction =     \
        "concept_at_most_number_restriction";                                                                                                                  \
    constructor_tag_type<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ExactNumberRestrictionTag> const prefix##_concept_exact_number_restriction =        \
        "concept_exact_number_restriction";                                                                                                                    \
    constructor_tag_type<Family, runir::kr::dl::ConceptTag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag> const                                         \
        prefix##_concept_qualified_at_least_number_restriction = "concept_qualified_at_least_number_restriction";                                              \
    constructor_tag_type<Family, runir::kr::dl::ConceptTag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag> const                                          \
        prefix##_concept_qualified_at_most_number_restriction = "concept_qualified_at_most_number_restriction";                                                \
    constructor_tag_type<Family, runir::kr::dl::ConceptTag, runir::kr::dl::QualifiedExactNumberRestrictionTag> const                                           \
        prefix##_concept_qualified_exact_number_restriction = "concept_qualified_exact_number_restriction";                                                    \
    constructor_tag_type<Family, runir::kr::dl::ConceptTag, runir::kr::dl::RoleValueMapTag> const prefix##_concept_role_value_map = "concept_role_value_map";  \
    constructor_tag_type<Family, runir::kr::dl::ConceptTag, runir::kr::dl::AgreementTag> const prefix##_concept_agreement = "concept_agreement";               \
    constructor_tag_type<Family, runir::kr::dl::ConceptTag, runir::kr::dl::RoleFillersTag> const prefix##_concept_role_fillers = "concept_role_fillers";       \
    constructor_tag_type<Family, runir::kr::dl::ConceptTag, runir::kr::dl::OneOfTag> const prefix##_concept_one_of = "concept_one_of";                         \
    constructor_tag_type<Family, runir::kr::dl::ConceptTag, runir::kr::dl::NominalTag> const prefix##_concept_nominal = "concept_nominal";                     \
    constructor_tag_type<Family, runir::kr::dl::ConceptTag, runir::kr::dl::RegisterTag> const prefix##_concept_register = "concept_register";                  \
    constructor_tag_type<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>> const prefix##_concept_argument =           \
        "concept_argument";                                                                                                                                    \
    constructor_tag_type<Family, runir::kr::dl::RoleTag, runir::kr::dl::UniversalTag> const prefix##_role_universal = "role_universal";                        \
    constructor_tag_type<Family, runir::kr::dl::RoleTag, runir::kr::dl::RoleAtomicStateSyntaxTag> const prefix##_role_atomic_state = "role_atomic_state";      \
    constructor_tag_type<Family, runir::kr::dl::RoleTag, runir::kr::dl::RoleAtomicGoalSyntaxTag> const prefix##_role_atomic_goal = "role_atomic_goal";         \
    constructor_tag_type<Family, runir::kr::dl::RoleTag, runir::kr::dl::RoleIntersectionSyntaxTag> const prefix##_role_intersection = "role_intersection";     \
    constructor_tag_type<Family, runir::kr::dl::RoleTag, runir::kr::dl::RoleUnionSyntaxTag> const prefix##_role_union = "role_union";                          \
    constructor_tag_type<Family, runir::kr::dl::RoleTag, runir::kr::dl::ComplementTag> const prefix##_role_complement = "role_complement";                     \
    constructor_tag_type<Family, runir::kr::dl::RoleTag, runir::kr::dl::InverseTag> const prefix##_role_inverse = "role_inverse";                              \
    constructor_tag_type<Family, runir::kr::dl::RoleTag, runir::kr::dl::CompositionTag> const prefix##_role_composition = "role_composition";                  \
    constructor_tag_type<Family, runir::kr::dl::RoleTag, runir::kr::dl::TransitiveClosureTag> const prefix##_role_transitive_closure =                         \
        "role_transitive_closure";                                                                                                                             \
    constructor_tag_type<Family, runir::kr::dl::RoleTag, runir::kr::dl::ReflexiveTransitiveClosureTag> const prefix##_role_reflexive_transitive_closure =      \
        "role_reflexive_transitive_closure";                                                                                                                   \
    constructor_tag_type<Family, runir::kr::dl::RoleTag, runir::kr::dl::RestrictionTag> const prefix##_role_restriction = "role_restriction";                  \
    constructor_tag_type<Family, runir::kr::dl::RoleTag, runir::kr::dl::IdentityTag> const prefix##_role_identity = "role_identity";                           \
    constructor_tag_type<Family, runir::kr::dl::RoleTag, runir::kr::dl::RegisterTag> const prefix##_role_register = "role_register";                           \
    constructor_tag_type<Family, runir::kr::dl::RoleTag, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>> const prefix##_role_argument = "role_argument";   \
    constructor_tag_type<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanAtomicStateSyntaxTag> const prefix##_boolean_atomic_state =                  \
        "boolean_atomic_state";                                                                                                                                \
    constructor_tag_type<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanAtomicGoalSyntaxTag> const prefix##_boolean_atomic_goal =                    \
        "boolean_atomic_goal";                                                                                                                                 \
    constructor_tag_type<Family, runir::kr::dl::BooleanTag, runir::kr::dl::NonemptyTag> const prefix##_boolean_nonempty = "boolean_nonempty";                  \
    constructor_tag_type<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanEqTag> const prefix##_boolean_eq = "boolean_eq";                             \
    constructor_tag_type<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanNeTag> const prefix##_boolean_neq = "boolean_neq";                           \
    constructor_tag_type<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanLtTag> const prefix##_boolean_lt = "boolean_lt";                             \
    constructor_tag_type<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanLeTag> const prefix##_boolean_le = "boolean_le";                             \
    constructor_tag_type<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanGtTag> const prefix##_boolean_gt = "boolean_gt";                             \
    constructor_tag_type<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanGeTag> const prefix##_boolean_ge = "boolean_ge";                             \
    constructor_tag_type<Family, runir::kr::dl::BooleanTag, runir::kr::dl::NumericalEqTag> const prefix##_numerical_eq = "numerical_eq";                       \
    constructor_tag_type<Family, runir::kr::dl::BooleanTag, runir::kr::dl::NumericalNeTag> const prefix##_numerical_neq = "numerical_neq";                     \
    constructor_tag_type<Family, runir::kr::dl::BooleanTag, runir::kr::dl::NumericalLtTag> const prefix##_numerical_lt = "numerical_lt";                       \
    constructor_tag_type<Family, runir::kr::dl::BooleanTag, runir::kr::dl::NumericalLeTag> const prefix##_numerical_le = "numerical_le";                       \
    constructor_tag_type<Family, runir::kr::dl::BooleanTag, runir::kr::dl::NumericalGtTag> const prefix##_numerical_gt = "numerical_gt";                       \
    constructor_tag_type<Family, runir::kr::dl::BooleanTag, runir::kr::dl::NumericalGeTag> const prefix##_numerical_ge = "numerical_ge";                       \
    constructor_tag_type<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanConstantTag> const prefix##_boolean_constant = "boolean_constant";           \
    constructor_tag_type<Family, runir::kr::dl::BooleanTag, runir::kr::dl::AndTag> const prefix##_boolean_and = "boolean_and";                                 \
    constructor_tag_type<Family, runir::kr::dl::BooleanTag, runir::kr::dl::OrTag> const prefix##_boolean_or = "boolean_or";                                    \
    constructor_tag_type<Family, runir::kr::dl::BooleanTag, runir::kr::dl::NotTag> const prefix##_boolean_not = "boolean_not";                                 \
    constructor_tag_type<Family, runir::kr::dl::BooleanTag, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>> const prefix##_boolean_argument =           \
        "boolean_argument";                                                                                                                                    \
    constructor_tag_type<Family, runir::kr::dl::NumericalTag, runir::kr::dl::CountTag> const prefix##_numerical_count = "numerical_count";                     \
    constructor_tag_type<Family, runir::kr::dl::NumericalTag, runir::kr::dl::DistanceTag> const prefix##_numerical_distance = "numerical_distance";            \
    constructor_tag_type<Family, runir::kr::dl::NumericalTag, runir::kr::dl::NumericalConstantTag> const prefix##_numerical_constant = "numerical_constant";   \
    constructor_tag_type<Family, runir::kr::dl::NumericalTag, runir::kr::dl::AddTag> const prefix##_numerical_add = "numerical_add";                           \
    constructor_tag_type<Family, runir::kr::dl::NumericalTag, runir::kr::dl::SubTag> const prefix##_numerical_sub = "numerical_sub";                           \
    constructor_tag_type<Family, runir::kr::dl::NumericalTag, runir::kr::dl::MulTag> const prefix##_numerical_mul = "numerical_mul";                           \
    constructor_tag_type<Family, runir::kr::dl::NumericalTag, runir::kr::dl::DivTag> const prefix##_numerical_div = "numerical_div";                           \
    constructor_tag_type<Family, runir::kr::dl::NumericalTag, runir::kr::dl::MinTag> const prefix##_numerical_min = "numerical_min";                           \
    constructor_tag_type<Family, runir::kr::dl::NumericalTag, runir::kr::dl::MaxTag> const prefix##_numerical_max = "numerical_max";                           \
    constructor_tag_type<Family, runir::kr::dl::NumericalTag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>> const prefix##_numerical_argument =     \
        "numerical_argument";                                                                                                                                  \
    const auto prefix##_concept_def =                                                                                                                          \
        maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::ConceptTag, runir::kr::dl::BotTag>>(prefix##_concept_bot)                                       \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::ConceptTag, runir::kr::dl::TopTag>>(prefix##_concept_top)                                     \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ConceptAtomicStateSyntaxTag>>(prefix##_concept_atomic_state)       \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ConceptAtomicGoalSyntaxTag>>(prefix##_concept_atomic_goal)         \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ConceptIntersectionSyntaxTag>>(prefix##_concept_intersection)      \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ConceptUnionSyntaxTag>>(prefix##_concept_union)                    \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::ConceptTag, runir::kr::dl::NegationTag>>(prefix##_concept_negation)                           \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ValueRestrictionTag>>(prefix##_concept_value_restriction)          \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ExistentialQuantificationTag>>(                                    \
            prefix##_concept_existential_quantification)                                                                                                       \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::ConceptTag, runir::kr::dl::AtLeastNumberRestrictionTag>>(                                     \
            prefix##_concept_at_least_number_restriction)                                                                                                      \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::ConceptTag, runir::kr::dl::AtMostNumberRestrictionTag>>(                                      \
            prefix##_concept_at_most_number_restriction)                                                                                                       \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ExactNumberRestrictionTag>>(                                       \
            prefix##_concept_exact_number_restriction)                                                                                                         \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::ConceptTag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>>(                            \
            prefix##_concept_qualified_at_least_number_restriction)                                                                                            \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::ConceptTag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>>(                             \
            prefix##_concept_qualified_at_most_number_restriction)                                                                                             \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::ConceptTag, runir::kr::dl::QualifiedExactNumberRestrictionTag>>(                              \
            prefix##_concept_qualified_exact_number_restriction)                                                                                               \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::ConceptTag, runir::kr::dl::RoleValueMapTag>>(prefix##_concept_role_value_map)                 \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::ConceptTag, runir::kr::dl::AgreementTag>>(prefix##_concept_agreement)                         \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::ConceptTag, runir::kr::dl::RoleFillersTag>>(prefix##_concept_role_fillers)                    \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::ConceptTag, runir::kr::dl::OneOfTag>>(prefix##_concept_one_of)                                \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::ConceptTag, runir::kr::dl::NominalTag>>(prefix##_concept_nominal)                             \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::ConceptTag, runir::kr::dl::RegisterTag>>(prefix##_concept_register)                           \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>>(                          \
            prefix##_concept_argument);                                                                                                                        \
    const auto prefix##_concept_root_def = prefix##_concept > eoi;                                                                                             \
    const auto prefix##_concept_non_terminal_def = non_terminal_string_parser<runir::kr::dl::ConceptTag>();                                                    \
    const auto prefix##_concept_choice_def = prefix##_concept_non_terminal | prefix##_concept;                                                                 \
    const auto prefix##_role_def =                                                                                                                             \
        maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::RoleTag, runir::kr::dl::UniversalTag>>(prefix##_role_universal)                                 \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::RoleTag, runir::kr::dl::RoleAtomicStateSyntaxTag>>(prefix##_role_atomic_state)                \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::RoleTag, runir::kr::dl::RoleAtomicGoalSyntaxTag>>(prefix##_role_atomic_goal)                  \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::RoleTag, runir::kr::dl::RoleIntersectionSyntaxTag>>(prefix##_role_intersection)               \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::RoleTag, runir::kr::dl::RoleUnionSyntaxTag>>(prefix##_role_union)                             \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::RoleTag, runir::kr::dl::ComplementTag>>(prefix##_role_complement)                             \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::RoleTag, runir::kr::dl::InverseTag>>(prefix##_role_inverse)                                   \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::RoleTag, runir::kr::dl::CompositionTag>>(prefix##_role_composition)                           \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::RoleTag, runir::kr::dl::TransitiveClosureTag>>(prefix##_role_transitive_closure)              \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::RoleTag, runir::kr::dl::ReflexiveTransitiveClosureTag>>(                                      \
            prefix##_role_reflexive_transitive_closure)                                                                                                        \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::RoleTag, runir::kr::dl::RestrictionTag>>(prefix##_role_restriction)                           \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::RoleTag, runir::kr::dl::IdentityTag>>(prefix##_role_identity)                                 \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::RoleTag, runir::kr::dl::RegisterTag>>(prefix##_role_register)                                 \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::RoleTag, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>>(prefix##_role_argument);        \
    const auto prefix##_role_root_def = prefix##_role > eoi;                                                                                                   \
    const auto prefix##_role_non_terminal_def = non_terminal_string_parser<runir::kr::dl::RoleTag>();                                                          \
    const auto prefix##_role_choice_def = prefix##_role_non_terminal | prefix##_role;                                                                          \
    const auto prefix##_constructor_or_non_terminal_variant_def = prefix##_concept_choice | prefix##_role_choice;                                              \
    const auto prefix##_boolean_def =                                                                                                                          \
        maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanAtomicStateSyntaxTag>>(prefix##_boolean_atomic_state)         \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanAtomicGoalSyntaxTag>>(prefix##_boolean_atomic_goal)         \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::BooleanTag, runir::kr::dl::NonemptyTag>>(prefix##_boolean_nonempty)                           \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanEqTag>>(prefix##_boolean_eq)                                \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanNeTag>>(prefix##_boolean_neq)                               \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanLtTag>>(prefix##_boolean_lt)                                \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanLeTag>>(prefix##_boolean_le)                                \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanGtTag>>(prefix##_boolean_gt)                                \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanGeTag>>(prefix##_boolean_ge)                                \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::BooleanTag, runir::kr::dl::NumericalEqTag>>(prefix##_numerical_eq)                            \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::BooleanTag, runir::kr::dl::NumericalNeTag>>(prefix##_numerical_neq)                           \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::BooleanTag, runir::kr::dl::NumericalLtTag>>(prefix##_numerical_lt)                            \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::BooleanTag, runir::kr::dl::NumericalLeTag>>(prefix##_numerical_le)                            \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::BooleanTag, runir::kr::dl::NumericalGtTag>>(prefix##_numerical_gt)                            \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::BooleanTag, runir::kr::dl::NumericalGeTag>>(prefix##_numerical_ge)                            \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanConstantTag>>(prefix##_boolean_constant)                    \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::BooleanTag, runir::kr::dl::AndTag>>(prefix##_boolean_and)                                     \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::BooleanTag, runir::kr::dl::OrTag>>(prefix##_boolean_or)                                       \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::BooleanTag, runir::kr::dl::NotTag>>(prefix##_boolean_not)                                     \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::BooleanTag, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>>(                          \
            prefix##_boolean_argument);                                                                                                                        \
    const auto prefix##_boolean_root_def = prefix##_boolean > eoi;                                                                                             \
    const auto prefix##_boolean_non_terminal_def = non_terminal_string_parser<runir::kr::dl::BooleanTag>();                                                    \
    const auto prefix##_boolean_choice_def = prefix##_boolean_non_terminal | prefix##_boolean;                                                                 \
    const auto prefix##_numerical_def =                                                                                                                        \
        maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::NumericalTag, runir::kr::dl::CountTag>>(prefix##_numerical_count)                               \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::NumericalTag, runir::kr::dl::DistanceTag>>(prefix##_numerical_distance)                       \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::NumericalTag, runir::kr::dl::NumericalConstantTag>>(prefix##_numerical_constant)              \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::NumericalTag, runir::kr::dl::AddTag>>(prefix##_numerical_add)                                 \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::NumericalTag, runir::kr::dl::SubTag>>(prefix##_numerical_sub)                                 \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::NumericalTag, runir::kr::dl::MulTag>>(prefix##_numerical_mul)                                 \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::NumericalTag, runir::kr::dl::DivTag>>(prefix##_numerical_div)                                 \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::NumericalTag, runir::kr::dl::MinTag>>(prefix##_numerical_min)                                 \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::NumericalTag, runir::kr::dl::MaxTag>>(prefix##_numerical_max)                                 \
        | maybe<has_ast_constructor_tag_v<Family, runir::kr::dl::NumericalTag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>>(                      \
            prefix##_numerical_argument);                                                                                                                      \
    const auto prefix##_numerical_root_def = prefix##_numerical > eoi;                                                                                         \
    const auto prefix##_numerical_non_terminal_def = non_terminal_string_parser<runir::kr::dl::NumericalTag>();                                                \
    const auto prefix##_numerical_choice_def = prefix##_numerical_non_terminal | prefix##_numerical;                                                           \
    const auto prefix##_concept_bot_def =                                                                                                                      \
        with_constructor_parentheses(lit(grammar_ast::ConceptBot<Family>::keyword) >> x3::attr(grammar_ast::ConceptBot<Family> {}));                           \
    const auto prefix##_concept_top_def =                                                                                                                      \
        with_constructor_parentheses(lit(grammar_ast::ConceptTop<Family>::keyword) >> x3::attr(grammar_ast::ConceptTop<Family> {}));                           \
    const auto prefix##_concept_atomic_state_def =                                                                                                             \
        with_constructor_parentheses(lit(grammar_ast::ConceptAtomicState<Family>::keyword) > predicate_name_string_parser());                                  \
    const auto prefix##_concept_atomic_goal_def =                                                                                                              \
        with_constructor_parentheses(lit(grammar_ast::ConceptAtomicGoal<Family>::keyword) > predicate_name_string_parser() > bool_string_parser());            \
    const auto prefix##_concept_intersection_def =                                                                                                             \
        with_constructor_parentheses(lit(grammar_ast::ConceptIntersection<Family>::keyword) > prefix##_concept_choice > prefix##_concept_choice);              \
    const auto prefix##_concept_union_def =                                                                                                                    \
        with_constructor_parentheses(lit(grammar_ast::ConceptUnion<Family>::keyword) > prefix##_concept_choice > prefix##_concept_choice);                     \
    const auto prefix##_concept_negation_def = with_constructor_parentheses(lit(grammar_ast::ConceptNegation<Family>::keyword) > prefix##_concept_choice);     \
    const auto prefix##_concept_value_restriction_def =                                                                                                        \
        with_constructor_parentheses(lit(grammar_ast::ConceptValueRestriction<Family>::keyword) > prefix##_role_choice > prefix##_concept_choice);             \
    const auto prefix##_concept_existential_quantification_def =                                                                                               \
        with_constructor_parentheses(lit(grammar_ast::ConceptExistentialQuantification<Family>::keyword) > prefix##_role_choice > prefix##_concept_choice);    \
    const auto prefix##_concept_at_least_number_restriction_def =                                                                                              \
        with_constructor_parentheses(lit(grammar_ast::ConceptAtLeastNumberRestriction<Family>::keyword) > uint_ > prefix##_role_choice);                       \
    const auto prefix##_concept_at_most_number_restriction_def =                                                                                               \
        with_constructor_parentheses(lit(grammar_ast::ConceptAtMostNumberRestriction<Family>::keyword) > uint_ > prefix##_role_choice);                        \
    const auto prefix##_concept_exact_number_restriction_def =                                                                                                 \
        with_constructor_parentheses(lit(grammar_ast::ConceptExactNumberRestriction<Family>::keyword) > uint_ > prefix##_role_choice);                         \
    const auto prefix##_concept_qualified_at_least_number_restriction_def = with_constructor_parentheses(                                                      \
        lit(grammar_ast::ConceptQualifiedAtLeastNumberRestriction<Family>::keyword) > uint_ > prefix##_role_choice > prefix##_concept_choice);                 \
    const auto prefix##_concept_qualified_at_most_number_restriction_def = with_constructor_parentheses(                                                       \
        lit(grammar_ast::ConceptQualifiedAtMostNumberRestriction<Family>::keyword) > uint_ > prefix##_role_choice > prefix##_concept_choice);                  \
    const auto prefix##_concept_qualified_exact_number_restriction_def = with_constructor_parentheses(                                                         \
        lit(grammar_ast::ConceptQualifiedExactNumberRestriction<Family>::keyword) > uint_ > prefix##_role_choice > prefix##_concept_choice);                   \
    const auto prefix##_concept_role_value_map_def =                                                                                                           \
        with_constructor_parentheses(lit(grammar_ast::ConceptRoleValueMap<Family>::keyword) > prefix##_role_choice > prefix##_role_choice);                    \
    const auto prefix##_concept_agreement_def =                                                                                                                \
        with_constructor_parentheses(lit(grammar_ast::ConceptAgreement<Family>::keyword) > prefix##_role_choice > prefix##_role_choice);                       \
    const auto prefix##_concept_role_fillers_def =                                                                                                             \
        with_constructor_parentheses(lit(grammar_ast::ConceptRoleFillers<Family>::keyword) > prefix##_role_choice > +object_name_string_parser());             \
    const auto prefix##_concept_one_of_def = with_constructor_parentheses(lit(grammar_ast::ConceptOneOf<Family>::keyword) > +object_name_string_parser());     \
    const auto prefix##_concept_nominal_def = with_constructor_parentheses(lit(grammar_ast::ConceptNominal<Family>::keyword) > object_name_string_parser());   \
    const auto prefix##_concept_register_def = with_constructor_parentheses(lit(grammar_ast::ConceptRegister<Family>::keyword) > uint_);                       \
    const auto prefix##_concept_argument_def = with_constructor_parentheses(lit(grammar_ast::ConceptArgument<Family>::keyword) > uint_);                       \
    const auto prefix##_role_universal_def =                                                                                                                   \
        with_constructor_parentheses(lit(grammar_ast::RoleUniversal<Family>::keyword) >> x3::attr(grammar_ast::RoleUniversal<Family> {}));                     \
    const auto prefix##_role_atomic_state_def =                                                                                                                \
        with_constructor_parentheses(lit(grammar_ast::RoleAtomicState<Family>::keyword) > predicate_name_string_parser());                                     \
    const auto prefix##_role_atomic_goal_def =                                                                                                                 \
        with_constructor_parentheses(lit(grammar_ast::RoleAtomicGoal<Family>::keyword) > predicate_name_string_parser() > bool_string_parser());               \
    const auto prefix##_role_intersection_def =                                                                                                                \
        with_constructor_parentheses(lit(grammar_ast::RoleIntersection<Family>::keyword) > prefix##_role_choice > prefix##_role_choice);                       \
    const auto prefix##_role_union_def =                                                                                                                       \
        with_constructor_parentheses(lit(grammar_ast::RoleUnion<Family>::keyword) > prefix##_role_choice > prefix##_role_choice);                              \
    const auto prefix##_role_complement_def = with_constructor_parentheses(lit(grammar_ast::RoleComplement<Family>::keyword) > prefix##_role_choice);          \
    const auto prefix##_role_inverse_def = with_constructor_parentheses(lit(grammar_ast::RoleInverse<Family>::keyword) > prefix##_role_choice);                \
    const auto prefix##_role_composition_def =                                                                                                                 \
        with_constructor_parentheses(lit(grammar_ast::RoleComposition<Family>::keyword) > prefix##_role_choice > prefix##_role_choice);                        \
    const auto prefix##_role_transitive_closure_def =                                                                                                          \
        with_constructor_parentheses(lit(grammar_ast::RoleTransitiveClosure<Family>::keyword) > prefix##_role_choice);                                         \
    const auto prefix##_role_reflexive_transitive_closure_def =                                                                                                \
        with_constructor_parentheses(lit(grammar_ast::RoleReflexiveTransitiveClosure<Family>::keyword) > prefix##_role_choice);                                \
    const auto prefix##_role_restriction_def =                                                                                                                 \
        with_constructor_parentheses(lit(grammar_ast::RoleRestriction<Family>::keyword) > prefix##_role_choice > prefix##_concept_choice);                     \
    const auto prefix##_role_identity_def = with_constructor_parentheses(lit(grammar_ast::RoleIdentity<Family>::keyword) > prefix##_concept_choice);           \
    const auto prefix##_role_register_def = with_constructor_parentheses(lit(grammar_ast::RoleRegister<Family>::keyword) > uint_);                             \
    const auto prefix##_role_argument_def = with_constructor_parentheses(lit(grammar_ast::RoleArgument<Family>::keyword) > uint_);                             \
    const auto prefix##_boolean_atomic_state_def =                                                                                                             \
        with_constructor_parentheses(lit(grammar_ast::BooleanAtomicState<Family>::keyword) > predicate_name_string_parser() > bool_string_parser());           \
    const auto prefix##_boolean_atomic_goal_def =                                                                                                              \
        with_constructor_parentheses(lit(grammar_ast::BooleanAtomicGoal<Family>::keyword) > predicate_name_string_parser() > bool_string_parser());            \
    const auto prefix##_boolean_nonempty_def =                                                                                                                 \
        with_constructor_parentheses(lit(grammar_ast::BooleanNonempty<Family>::keyword) > prefix##_constructor_or_non_terminal_variant);                       \
    const auto prefix##_boolean_eq_def =                                                                                                                       \
        with_constructor_parentheses(lit(grammar_ast::BooleanEq<Family>::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                        \
    const auto prefix##_boolean_neq_def =                                                                                                                      \
        with_constructor_parentheses(lit(grammar_ast::BooleanNeq<Family>::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                       \
    const auto prefix##_boolean_lt_def =                                                                                                                       \
        with_constructor_parentheses(lit(grammar_ast::BooleanLt<Family>::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                        \
    const auto prefix##_boolean_le_def =                                                                                                                       \
        with_constructor_parentheses(lit(grammar_ast::BooleanLe<Family>::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                        \
    const auto prefix##_boolean_gt_def =                                                                                                                       \
        with_constructor_parentheses(lit(grammar_ast::BooleanGt<Family>::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                        \
    const auto prefix##_boolean_ge_def =                                                                                                                       \
        with_constructor_parentheses(lit(grammar_ast::BooleanGe<Family>::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                        \
    const auto prefix##_numerical_eq_def =                                                                                                                     \
        with_constructor_parentheses(lit(grammar_ast::NumericalEq<Family>::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                  \
    const auto prefix##_numerical_neq_def =                                                                                                                    \
        with_constructor_parentheses(lit(grammar_ast::NumericalNeq<Family>::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                 \
    const auto prefix##_numerical_lt_def =                                                                                                                     \
        with_constructor_parentheses(lit(grammar_ast::NumericalLt<Family>::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                  \
    const auto prefix##_numerical_le_def =                                                                                                                     \
        with_constructor_parentheses(lit(grammar_ast::NumericalLe<Family>::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                  \
    const auto prefix##_numerical_gt_def =                                                                                                                     \
        with_constructor_parentheses(lit(grammar_ast::NumericalGt<Family>::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                  \
    const auto prefix##_numerical_ge_def =                                                                                                                     \
        with_constructor_parentheses(lit(grammar_ast::NumericalGe<Family>::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                  \
    const auto prefix##_boolean_constant_def = with_constructor_parentheses(lit(grammar_ast::BooleanConstant<Family>::keyword) > bool_string_parser());        \
    const auto prefix##_boolean_and_def =                                                                                                                      \
        with_constructor_parentheses(lit(grammar_ast::BooleanAnd<Family>::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                       \
    const auto prefix##_boolean_or_def =                                                                                                                       \
        with_constructor_parentheses(lit(grammar_ast::BooleanOr<Family>::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                        \
    const auto prefix##_boolean_not_def = with_constructor_parentheses(lit(grammar_ast::BooleanNot<Family>::keyword) > prefix##_boolean_choice);               \
    const auto prefix##_boolean_argument_def = with_constructor_parentheses(lit(grammar_ast::BooleanArgument<Family>::keyword) > uint_);                       \
    const auto prefix##_numerical_count_def =                                                                                                                  \
        with_constructor_parentheses(lit(grammar_ast::NumericalCount<Family>::keyword) > prefix##_constructor_or_non_terminal_variant);                        \
    const auto prefix##_numerical_distance_def = with_constructor_parentheses(lit(grammar_ast::NumericalDistance<Family>::keyword) > prefix##_concept_choice   \
                                                                              > prefix##_role_choice > prefix##_concept_choice);                               \
    const auto prefix##_numerical_constant_def = with_constructor_parentheses(lit(grammar_ast::NumericalConstant<Family>::keyword) > uint_);                   \
    const auto prefix##_numerical_add_def =                                                                                                                    \
        with_constructor_parentheses(lit(grammar_ast::NumericalAdd<Family>::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                 \
    const auto prefix##_numerical_sub_def =                                                                                                                    \
        with_constructor_parentheses(lit(grammar_ast::NumericalSub<Family>::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                 \
    const auto prefix##_numerical_mul_def =                                                                                                                    \
        with_constructor_parentheses(lit(grammar_ast::NumericalMul<Family>::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                 \
    const auto prefix##_numerical_div_def =                                                                                                                    \
        with_constructor_parentheses(lit(grammar_ast::NumericalDiv<Family>::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                 \
    const auto prefix##_numerical_min_def =                                                                                                                    \
        with_constructor_parentheses(lit(grammar_ast::NumericalMin<Family>::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                 \
    const auto prefix##_numerical_max_def =                                                                                                                    \
        with_constructor_parentheses(lit(grammar_ast::NumericalMax<Family>::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                 \
    const auto prefix##_numerical_argument_def = with_constructor_parentheses(lit(grammar_ast::NumericalArgument<Family>::keyword) > uint_);                   \
    const auto prefix##_concept_derivation_rule_def =                                                                                                          \
        (lit("(") >> prefix##_concept_non_terminal) > ((lit("(") > (prefix##_concept_choice % lit("or")) > lit(")")) | x3::repeat(1)[prefix##_concept_choice]) \
        > lit(")");                                                                                                                                            \
    const auto prefix##_role_derivation_rule_def = (lit("(") >> prefix##_role_non_terminal)                                                                    \
                                                   > ((lit("(") > (prefix##_role_choice % lit("or")) > lit(")")) | x3::repeat(1)[prefix##_role_choice])        \
                                                   > lit(")");                                                                                                 \
    const auto prefix##_boolean_derivation_rule_def =                                                                                                          \
        (lit("(") >> prefix##_boolean_non_terminal) > ((lit("(") > (prefix##_boolean_choice % lit("or")) > lit(")")) | x3::repeat(1)[prefix##_boolean_choice]) \
        > lit(")");                                                                                                                                            \
    const auto prefix##_numerical_derivation_rule_def =                                                                                                        \
        (lit("(") >> prefix##_numerical_non_terminal)                                                                                                          \
        > ((lit("(") > (prefix##_numerical_choice % lit("or")) > lit(")")) | x3::repeat(1)[prefix##_numerical_choice]) > lit(")");                             \
    const auto prefix##_derivation_rule_variant_def =                                                                                                          \
        prefix##_concept_derivation_rule | prefix##_role_derivation_rule | prefix##_boolean_derivation_rule | prefix##_numerical_derivation_rule;              \
    const auto prefix##_grammar_body_def = *prefix##_derivation_rule_variant;                                                                                  \
    const auto prefix##_grammar_def = lit("(") > prefix##_grammar_body > lit(")");                                                                             \
    const auto prefix##_grammar_root_def = prefix##_grammar > eoi;                                                                                             \
    BOOST_SPIRIT_DEFINE(prefix##_concept,                                                                                                                      \
                        prefix##_concept_root,                                                                                                                 \
                        prefix##_role,                                                                                                                         \
                        prefix##_role_root,                                                                                                                    \
                        prefix##_boolean,                                                                                                                      \
                        prefix##_boolean_root,                                                                                                                 \
                        prefix##_numerical,                                                                                                                    \
                        prefix##_numerical_root,                                                                                                               \
                        prefix##_concept_non_terminal,                                                                                                         \
                        prefix##_concept_choice,                                                                                                               \
                        prefix##_role_non_terminal,                                                                                                            \
                        prefix##_role_choice,                                                                                                                  \
                        prefix##_boolean_non_terminal,                                                                                                         \
                        prefix##_boolean_choice,                                                                                                               \
                        prefix##_numerical_non_terminal,                                                                                                       \
                        prefix##_numerical_choice,                                                                                                             \
                        prefix##_constructor_or_non_terminal_variant,                                                                                          \
                        prefix##_concept_derivation_rule,                                                                                                      \
                        prefix##_role_derivation_rule,                                                                                                         \
                        prefix##_boolean_derivation_rule,                                                                                                      \
                        prefix##_numerical_derivation_rule,                                                                                                    \
                        prefix##_derivation_rule_variant,                                                                                                      \
                        prefix##_grammar_body,                                                                                                                 \
                        prefix##_grammar,                                                                                                                      \
                        prefix##_grammar_root)                                                                                                                 \
    BOOST_SPIRIT_DEFINE(prefix##_concept_bot,                                                                                                                  \
                        prefix##_concept_top,                                                                                                                  \
                        prefix##_concept_atomic_state,                                                                                                         \
                        prefix##_concept_atomic_goal,                                                                                                          \
                        prefix##_concept_intersection,                                                                                                         \
                        prefix##_concept_union,                                                                                                                \
                        prefix##_concept_negation,                                                                                                             \
                        prefix##_concept_value_restriction,                                                                                                    \
                        prefix##_concept_existential_quantification,                                                                                           \
                        prefix##_concept_at_least_number_restriction,                                                                                          \
                        prefix##_concept_at_most_number_restriction,                                                                                           \
                        prefix##_concept_exact_number_restriction,                                                                                             \
                        prefix##_concept_qualified_at_least_number_restriction,                                                                                \
                        prefix##_concept_qualified_at_most_number_restriction,                                                                                 \
                        prefix##_concept_qualified_exact_number_restriction,                                                                                   \
                        prefix##_concept_role_value_map,                                                                                                       \
                        prefix##_concept_agreement,                                                                                                            \
                        prefix##_concept_role_fillers,                                                                                                         \
                        prefix##_concept_one_of,                                                                                                               \
                        prefix##_concept_nominal,                                                                                                              \
                        prefix##_concept_register,                                                                                                             \
                        prefix##_concept_argument)                                                                                                             \
    BOOST_SPIRIT_DEFINE(prefix##_role_universal,                                                                                                               \
                        prefix##_role_atomic_state,                                                                                                            \
                        prefix##_role_atomic_goal,                                                                                                             \
                        prefix##_role_intersection,                                                                                                            \
                        prefix##_role_union,                                                                                                                   \
                        prefix##_role_complement,                                                                                                              \
                        prefix##_role_inverse,                                                                                                                 \
                        prefix##_role_composition,                                                                                                             \
                        prefix##_role_transitive_closure,                                                                                                      \
                        prefix##_role_reflexive_transitive_closure,                                                                                            \
                        prefix##_role_restriction,                                                                                                             \
                        prefix##_role_identity,                                                                                                                \
                        prefix##_role_register,                                                                                                                \
                        prefix##_role_argument)                                                                                                                \
    BOOST_SPIRIT_DEFINE(prefix##_boolean_atomic_state,                                                                                                         \
                        prefix##_boolean_atomic_goal,                                                                                                          \
                        prefix##_boolean_nonempty,                                                                                                             \
                        prefix##_boolean_eq,                                                                                                                   \
                        prefix##_boolean_neq,                                                                                                                  \
                        prefix##_boolean_lt,                                                                                                                   \
                        prefix##_boolean_le,                                                                                                                   \
                        prefix##_boolean_gt,                                                                                                                   \
                        prefix##_boolean_ge,                                                                                                                   \
                        prefix##_numerical_eq,                                                                                                                 \
                        prefix##_numerical_neq,                                                                                                                \
                        prefix##_numerical_lt,                                                                                                                 \
                        prefix##_numerical_le,                                                                                                                 \
                        prefix##_numerical_gt,                                                                                                                 \
                        prefix##_numerical_ge,                                                                                                                 \
                        prefix##_boolean_constant,                                                                                                             \
                        prefix##_boolean_and,                                                                                                                  \
                        prefix##_boolean_or,                                                                                                                   \
                        prefix##_boolean_not,                                                                                                                  \
                        prefix##_boolean_argument)                                                                                                             \
    BOOST_SPIRIT_DEFINE(prefix##_numerical_count,                                                                                                              \
                        prefix##_numerical_distance,                                                                                                           \
                        prefix##_numerical_constant,                                                                                                           \
                        prefix##_numerical_add,                                                                                                                \
                        prefix##_numerical_sub,                                                                                                                \
                        prefix##_numerical_mul,                                                                                                                \
                        prefix##_numerical_div,                                                                                                                \
                        prefix##_numerical_min,                                                                                                                \
                        prefix##_numerical_max,                                                                                                                \
                        prefix##_numerical_argument)

RUNIR_DEFINE_FAMILY_PARSER(runir::kr::BaseFamilyTag, base_family)
RUNIR_DEFINE_FAMILY_PARSER(runir::kr::ExtFamilyTag, ext_family)
RUNIR_DEFINE_FAMILY_PARSER(runir::kr::UnsFamilyTag, uns_family)

#undef RUNIR_DEFINE_FAMILY_PARSER

template<>
concept_type<runir::kr::BaseFamilyTag> const& concept_parser<runir::kr::BaseFamilyTag>()
{
    return base_family_concept;
}
template<>
concept_root_type<runir::kr::BaseFamilyTag> const& concept_root_parser<runir::kr::BaseFamilyTag>()
{
    return base_family_concept_root;
}
template<>
role_type<runir::kr::BaseFamilyTag> const& role_parser<runir::kr::BaseFamilyTag>()
{
    return base_family_role;
}
template<>
role_root_type<runir::kr::BaseFamilyTag> const& role_root_parser<runir::kr::BaseFamilyTag>()
{
    return base_family_role_root;
}
template<>
boolean_type<runir::kr::BaseFamilyTag> const& boolean_parser<runir::kr::BaseFamilyTag>()
{
    return base_family_boolean;
}
template<>
boolean_root_type<runir::kr::BaseFamilyTag> const& boolean_root_parser<runir::kr::BaseFamilyTag>()
{
    return base_family_boolean_root;
}
template<>
numerical_type<runir::kr::BaseFamilyTag> const& numerical_parser<runir::kr::BaseFamilyTag>()
{
    return base_family_numerical;
}
template<>
numerical_root_type<runir::kr::BaseFamilyTag> const& numerical_root_parser<runir::kr::BaseFamilyTag>()
{
    return base_family_numerical_root;
}
template<>
grammar_root_type<runir::kr::BaseFamilyTag> const& grammar_root_parser<runir::kr::BaseFamilyTag>()
{
    return base_family_grammar_root;
}

template<>
concept_type<runir::kr::ExtFamilyTag> const& concept_parser<runir::kr::ExtFamilyTag>()
{
    return ext_family_concept;
}
template<>
concept_root_type<runir::kr::ExtFamilyTag> const& concept_root_parser<runir::kr::ExtFamilyTag>()
{
    return ext_family_concept_root;
}
template<>
role_type<runir::kr::ExtFamilyTag> const& role_parser<runir::kr::ExtFamilyTag>()
{
    return ext_family_role;
}
template<>
role_root_type<runir::kr::ExtFamilyTag> const& role_root_parser<runir::kr::ExtFamilyTag>()
{
    return ext_family_role_root;
}
template<>
boolean_type<runir::kr::ExtFamilyTag> const& boolean_parser<runir::kr::ExtFamilyTag>()
{
    return ext_family_boolean;
}
template<>
boolean_root_type<runir::kr::ExtFamilyTag> const& boolean_root_parser<runir::kr::ExtFamilyTag>()
{
    return ext_family_boolean_root;
}
template<>
numerical_type<runir::kr::ExtFamilyTag> const& numerical_parser<runir::kr::ExtFamilyTag>()
{
    return ext_family_numerical;
}
template<>
numerical_root_type<runir::kr::ExtFamilyTag> const& numerical_root_parser<runir::kr::ExtFamilyTag>()
{
    return ext_family_numerical_root;
}
template<>
grammar_root_type<runir::kr::ExtFamilyTag> const& grammar_root_parser<runir::kr::ExtFamilyTag>()
{
    return ext_family_grammar_root;
}

template<>
concept_type<runir::kr::UnsFamilyTag> const& concept_parser<runir::kr::UnsFamilyTag>()
{
    return uns_family_concept;
}
template<>
concept_root_type<runir::kr::UnsFamilyTag> const& concept_root_parser<runir::kr::UnsFamilyTag>()
{
    return uns_family_concept_root;
}
template<>
role_type<runir::kr::UnsFamilyTag> const& role_parser<runir::kr::UnsFamilyTag>()
{
    return uns_family_role;
}
template<>
role_root_type<runir::kr::UnsFamilyTag> const& role_root_parser<runir::kr::UnsFamilyTag>()
{
    return uns_family_role_root;
}
template<>
boolean_type<runir::kr::UnsFamilyTag> const& boolean_parser<runir::kr::UnsFamilyTag>()
{
    return uns_family_boolean;
}
template<>
boolean_root_type<runir::kr::UnsFamilyTag> const& boolean_root_parser<runir::kr::UnsFamilyTag>()
{
    return uns_family_boolean_root;
}
template<>
numerical_type<runir::kr::UnsFamilyTag> const& numerical_parser<runir::kr::UnsFamilyTag>()
{
    return uns_family_numerical;
}
template<>
numerical_root_type<runir::kr::UnsFamilyTag> const& numerical_root_parser<runir::kr::UnsFamilyTag>()
{
    return uns_family_numerical_root;
}
template<>
grammar_root_type<runir::kr::UnsFamilyTag> const& grammar_root_parser<runir::kr::UnsFamilyTag>()
{
    return uns_family_grammar_root;
}

}  // namespace runir::kr::dl::grammar::parser

#endif
