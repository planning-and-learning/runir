#ifndef RUNIR_KR_DL_GRAMMAR_PARSER_PARSER_DEF_HPP_
#define RUNIR_KR_DL_GRAMMAR_PARSER_PARSER_DEF_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/grammar/ast/ast_adapted.hpp"
#include "runir/kr/dl/grammar/parser/parsers.hpp"

#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>
#include <string>

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

struct PredicateNameClass : ErrorHandlerBase
{
};
struct ObjectNameClass : ErrorHandlerBase
{
};
struct IdentifierClass : x3::annotate_on_success
{
};
struct NumericReferenceClass : x3::annotate_on_success
{
};

template<runir::kr::dl::CategoryTag Category>
struct NonTerminalNameClass : ErrorHandlerBase
{
};

struct PredicateNameTextClass
{
};
struct ObjectNameTextClass
{
};
struct IdentifierTextClass
{
};
struct NumericReferenceTextClass
{
};

template<runir::kr::dl::CategoryTag Category>
struct NonTerminalNameTextClass
{
};

predicate_name_type const predicate_name = "predicate_name";
object_name_type const object_name = "object_name";
identifier_type const identifier = "identifier";
numeric_reference_type const numeric_reference = "numeric_reference";

const auto predicate_name_def = x3::rule<PredicateNameTextClass, std::string> { "predicate_name_text" } =
    raw[lexeme[((&alpha >> +(alnum | char_('-') | char_('_'))) | x3::repeat(1)[char_('=')])]];
const auto object_name_def = x3::rule<ObjectNameTextClass, std::string> { "object_name_text" } = raw[lexeme[&alpha >> +(alnum | char_('-') | char_('_'))]];
const auto identifier_def = x3::rule<IdentifierTextClass, std::string> { "identifier_text" } =
    raw[lexeme[(alpha | char_('_')) >> *(alnum | char_('-') | char_('_'))]];
const auto numeric_reference_def = x3::rule<NumericReferenceTextClass, ygg::uint_t> { "numeric_reference_value" } = uint_;

BOOST_SPIRIT_DEFINE(predicate_name, object_name, identifier, numeric_reference)

template<runir::kr::dl::CategoryTag Category>
auto argument_reference_parser()
{
    return argument_reference_type<Category> { "argument_reference" } = numeric_reference | identifier;
}

template<runir::kr::dl::CategoryTag Category>
auto register_reference_parser()
{
    return register_reference_type<Category> { "register_reference" } = numeric_reference | identifier;
}

inline auto predicate_name_string_parser() { return lexeme[omit[lit('"')]] > predicate_name > lexeme[omit[lit('"')]]; }
inline auto object_name_string_parser() { return lexeme[omit[lit('"')]] > object_name > lexeme[omit[lit('"')]]; }

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
    const auto parser = []
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return lexeme[lit("c_") >> +digit];
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return lexeme[lit("r_") >> +digit];
        else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            return lexeme[lit("b_") >> +digit];
        else
            return lexeme[lit("n_") >> +digit];
    }();
    return x3::rule<NonTerminalNameClass<Category>, grammar_ast::Identifier> { "non_terminal_name" } =
               (x3::rule<NonTerminalNameTextClass<Category>, std::string> { "non_terminal_name_text" } = raw[parser]);
}

#define RUNIR_CONCEPT_CONSTRUCTORS(Family, prefix, X)                                                                                 \
    X(Family, prefix, runir::kr::dl::ConceptTag, BotTag, concept_bot)                                                                 \
    X(Family, prefix, runir::kr::dl::ConceptTag, TopTag, concept_top)                                                                 \
    X(Family, prefix, runir::kr::dl::ConceptTag, ConceptAtomicStateSyntaxTag, concept_atomic_state)                                   \
    X(Family, prefix, runir::kr::dl::ConceptTag, ConceptAtomicGoalSyntaxTag, concept_atomic_goal)                                     \
    X(Family, prefix, runir::kr::dl::ConceptTag, ConceptIntersectionSyntaxTag, concept_intersection)                                  \
    X(Family, prefix, runir::kr::dl::ConceptTag, ConceptUnionSyntaxTag, concept_union)                                                \
    X(Family, prefix, runir::kr::dl::ConceptTag, NegationTag, concept_negation)                                                       \
    X(Family, prefix, runir::kr::dl::ConceptTag, ValueRestrictionTag, concept_value_restriction)                                      \
    X(Family, prefix, runir::kr::dl::ConceptTag, ExistentialQuantificationTag, concept_existential_quantification)                    \
    X(Family, prefix, runir::kr::dl::ConceptTag, AtLeastNumberRestrictionTag, concept_at_least_number_restriction)                    \
    X(Family, prefix, runir::kr::dl::ConceptTag, AtMostNumberRestrictionTag, concept_at_most_number_restriction)                      \
    X(Family, prefix, runir::kr::dl::ConceptTag, ExactNumberRestrictionTag, concept_exact_number_restriction)                         \
    X(Family, prefix, runir::kr::dl::ConceptTag, QualifiedAtLeastNumberRestrictionTag, concept_qualified_at_least_number_restriction) \
    X(Family, prefix, runir::kr::dl::ConceptTag, QualifiedAtMostNumberRestrictionTag, concept_qualified_at_most_number_restriction)   \
    X(Family, prefix, runir::kr::dl::ConceptTag, QualifiedExactNumberRestrictionTag, concept_qualified_exact_number_restriction)      \
    X(Family, prefix, runir::kr::dl::ConceptTag, RoleValueMapTag, concept_role_value_map)                                             \
    X(Family, prefix, runir::kr::dl::ConceptTag, AgreementTag, concept_agreement)                                                     \
    X(Family, prefix, runir::kr::dl::ConceptTag, RoleFillersTag, concept_role_fillers)                                                \
    X(Family, prefix, runir::kr::dl::ConceptTag, OneOfTag, concept_one_of)                                                            \
    X(Family, prefix, runir::kr::dl::ConceptTag, NominalTag, concept_nominal)                                                         \
    X(Family, prefix, runir::kr::dl::ConceptTag, RegisterTag, concept_register)                                                       \
    X(Family, prefix, runir::kr::dl::ConceptTag, ArgumentTag<runir::kr::dl::ConceptTag>, concept_argument)

#define RUNIR_ROLE_CONSTRUCTORS(Family, prefix, X)                                                              \
    X(Family, prefix, runir::kr::dl::RoleTag, UniversalTag, role_universal)                                     \
    X(Family, prefix, runir::kr::dl::RoleTag, RoleAtomicStateSyntaxTag, role_atomic_state)                      \
    X(Family, prefix, runir::kr::dl::RoleTag, RoleAtomicGoalSyntaxTag, role_atomic_goal)                        \
    X(Family, prefix, runir::kr::dl::RoleTag, RoleIntersectionSyntaxTag, role_intersection)                     \
    X(Family, prefix, runir::kr::dl::RoleTag, RoleUnionSyntaxTag, role_union)                                   \
    X(Family, prefix, runir::kr::dl::RoleTag, ComplementTag, role_complement)                                   \
    X(Family, prefix, runir::kr::dl::RoleTag, InverseTag, role_inverse)                                         \
    X(Family, prefix, runir::kr::dl::RoleTag, CompositionTag, role_composition)                                 \
    X(Family, prefix, runir::kr::dl::RoleTag, TransitiveClosureTag, role_transitive_closure)                    \
    X(Family, prefix, runir::kr::dl::RoleTag, ReflexiveTransitiveClosureTag, role_reflexive_transitive_closure) \
    X(Family, prefix, runir::kr::dl::RoleTag, RestrictionTag, role_restriction)                                 \
    X(Family, prefix, runir::kr::dl::RoleTag, IdentityTag, role_identity)                                       \
    X(Family, prefix, runir::kr::dl::RoleTag, RegisterTag, role_register)                                       \
    X(Family, prefix, runir::kr::dl::RoleTag, ArgumentTag<runir::kr::dl::RoleTag>, role_argument)

#define RUNIR_BOOLEAN_CONSTRUCTORS(Family, prefix, X)                                               \
    X(Family, prefix, runir::kr::dl::BooleanTag, BooleanAtomicStateSyntaxTag, boolean_atomic_state) \
    X(Family, prefix, runir::kr::dl::BooleanTag, BooleanAtomicGoalSyntaxTag, boolean_atomic_goal)   \
    X(Family, prefix, runir::kr::dl::BooleanTag, NonemptyTag, boolean_nonempty)                     \
    X(Family, prefix, runir::kr::dl::BooleanTag, BooleanEqTag, boolean_eq)                          \
    X(Family, prefix, runir::kr::dl::BooleanTag, BooleanNeTag, boolean_neq)                         \
    X(Family, prefix, runir::kr::dl::BooleanTag, BooleanLtTag, boolean_lt)                          \
    X(Family, prefix, runir::kr::dl::BooleanTag, BooleanLeTag, boolean_le)                          \
    X(Family, prefix, runir::kr::dl::BooleanTag, BooleanGtTag, boolean_gt)                          \
    X(Family, prefix, runir::kr::dl::BooleanTag, BooleanGeTag, boolean_ge)                          \
    X(Family, prefix, runir::kr::dl::BooleanTag, NumericalEqTag, numerical_eq)                      \
    X(Family, prefix, runir::kr::dl::BooleanTag, NumericalNeTag, numerical_neq)                     \
    X(Family, prefix, runir::kr::dl::BooleanTag, NumericalLtTag, numerical_lt)                      \
    X(Family, prefix, runir::kr::dl::BooleanTag, NumericalLeTag, numerical_le)                      \
    X(Family, prefix, runir::kr::dl::BooleanTag, NumericalGtTag, numerical_gt)                      \
    X(Family, prefix, runir::kr::dl::BooleanTag, NumericalGeTag, numerical_ge)                      \
    X(Family, prefix, runir::kr::dl::BooleanTag, BooleanConstantTag, boolean_constant)              \
    X(Family, prefix, runir::kr::dl::BooleanTag, AndTag, boolean_and)                               \
    X(Family, prefix, runir::kr::dl::BooleanTag, OrTag, boolean_or)                                 \
    X(Family, prefix, runir::kr::dl::BooleanTag, NotTag, boolean_not)                               \
    X(Family, prefix, runir::kr::dl::BooleanTag, ArgumentTag<runir::kr::dl::BooleanTag>, boolean_argument)

#define RUNIR_NUMERICAL_CONSTRUCTORS(Family, prefix, X)                                      \
    X(Family, prefix, runir::kr::dl::NumericalTag, CountTag, numerical_count)                \
    X(Family, prefix, runir::kr::dl::NumericalTag, DistanceTag, numerical_distance)          \
    X(Family, prefix, runir::kr::dl::NumericalTag, NumericalConstantTag, numerical_constant) \
    X(Family, prefix, runir::kr::dl::NumericalTag, AddTag, numerical_add)                    \
    X(Family, prefix, runir::kr::dl::NumericalTag, SubTag, numerical_sub)                    \
    X(Family, prefix, runir::kr::dl::NumericalTag, MulTag, numerical_mul)                    \
    X(Family, prefix, runir::kr::dl::NumericalTag, DivTag, numerical_div)                    \
    X(Family, prefix, runir::kr::dl::NumericalTag, MinTag, numerical_min)                    \
    X(Family, prefix, runir::kr::dl::NumericalTag, MaxTag, numerical_max)                    \
    X(Family, prefix, runir::kr::dl::NumericalTag, ArgumentTag<runir::kr::dl::NumericalTag>, numerical_argument)

#define RUNIR_DECLARE_CONSTRUCTOR_RULE(Family, prefix, Category, Tag, name) \
    constructor_tag_type<Family, Category, runir::kr::dl::Tag> const prefix##_##name = #name;

#define RUNIR_CONSTRUCTOR_ALTERNATIVE(Family, prefix, Category, Tag, name) \
    | maybe<has_ast_constructor_tag_v<Family, Category, runir::kr::dl::Tag>>(prefix##_##name)

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
    RUNIR_CONCEPT_CONSTRUCTORS(Family, prefix, RUNIR_DECLARE_CONSTRUCTOR_RULE)                                                                                 \
    RUNIR_ROLE_CONSTRUCTORS(Family, prefix, RUNIR_DECLARE_CONSTRUCTOR_RULE)                                                                                    \
    RUNIR_BOOLEAN_CONSTRUCTORS(Family, prefix, RUNIR_DECLARE_CONSTRUCTOR_RULE)                                                                                 \
    RUNIR_NUMERICAL_CONSTRUCTORS(Family, prefix, RUNIR_DECLARE_CONSTRUCTOR_RULE)                                                                               \
    const auto prefix##_concept_def = x3::eps(false) RUNIR_CONCEPT_CONSTRUCTORS(Family, prefix, RUNIR_CONSTRUCTOR_ALTERNATIVE);                                \
    const auto prefix##_concept_root_def = prefix##_concept > eoi;                                                                                             \
    const auto prefix##_concept_non_terminal_def = non_terminal_string_parser<runir::kr::dl::ConceptTag>();                                                    \
    const auto prefix##_concept_choice_def = prefix##_concept_non_terminal | prefix##_concept;                                                                 \
    const auto prefix##_role_def = x3::eps(false) RUNIR_ROLE_CONSTRUCTORS(Family, prefix, RUNIR_CONSTRUCTOR_ALTERNATIVE);                                      \
    const auto prefix##_role_root_def = prefix##_role > eoi;                                                                                                   \
    const auto prefix##_role_non_terminal_def = non_terminal_string_parser<runir::kr::dl::RoleTag>();                                                          \
    const auto prefix##_role_choice_def = prefix##_role_non_terminal | prefix##_role;                                                                          \
    const auto prefix##_constructor_or_non_terminal_variant_def = prefix##_concept_choice | prefix##_role_choice;                                              \
    const auto prefix##_boolean_def = x3::eps(false) RUNIR_BOOLEAN_CONSTRUCTORS(Family, prefix, RUNIR_CONSTRUCTOR_ALTERNATIVE);                                \
    const auto prefix##_boolean_root_def = prefix##_boolean > eoi;                                                                                             \
    const auto prefix##_boolean_non_terminal_def = non_terminal_string_parser<runir::kr::dl::BooleanTag>();                                                    \
    const auto prefix##_boolean_choice_def = prefix##_boolean_non_terminal | prefix##_boolean;                                                                 \
    const auto prefix##_numerical_def = x3::eps(false) RUNIR_NUMERICAL_CONSTRUCTORS(Family, prefix, RUNIR_CONSTRUCTOR_ALTERNATIVE);                            \
    const auto prefix##_numerical_root_def = prefix##_numerical > eoi;                                                                                         \
    const auto prefix##_numerical_non_terminal_def = non_terminal_string_parser<runir::kr::dl::NumericalTag>();                                                \
    const auto prefix##_numerical_choice_def = prefix##_numerical_non_terminal | prefix##_numerical;                                                           \
    const auto prefix##_concept_bot_def = with_constructor_parentheses(lit(runir::kr::dl::BotTag::keyword) >> x3::attr(grammar_ast::ConceptBot<Family> {}));   \
    const auto prefix##_concept_top_def = with_constructor_parentheses(lit(runir::kr::dl::TopTag::keyword) >> x3::attr(grammar_ast::ConceptTop<Family> {}));   \
    const auto prefix##_concept_atomic_state_def =                                                                                                             \
        with_constructor_parentheses(lit(runir::kr::dl::ConceptAtomicStateSyntaxTag::keyword) > predicate_name_string_parser());                               \
    const auto prefix##_concept_atomic_goal_def =                                                                                                              \
        with_constructor_parentheses(lit(runir::kr::dl::ConceptAtomicGoalSyntaxTag::keyword) > predicate_name_string_parser() > bool_string_parser());         \
    const auto prefix##_concept_intersection_def =                                                                                                             \
        with_constructor_parentheses(lit(runir::kr::dl::ConceptIntersectionSyntaxTag::keyword) > prefix##_concept_choice > prefix##_concept_choice);           \
    const auto prefix##_concept_union_def =                                                                                                                    \
        with_constructor_parentheses(lit(runir::kr::dl::ConceptUnionSyntaxTag::keyword) > prefix##_concept_choice > prefix##_concept_choice);                  \
    const auto prefix##_concept_negation_def = with_constructor_parentheses(lit(runir::kr::dl::NegationTag::keyword) > prefix##_concept_choice);               \
    const auto prefix##_concept_value_restriction_def =                                                                                                        \
        with_constructor_parentheses(lit(runir::kr::dl::ValueRestrictionTag::keyword) > prefix##_role_choice > prefix##_concept_choice);                       \
    const auto prefix##_concept_existential_quantification_def =                                                                                               \
        with_constructor_parentheses(lit(runir::kr::dl::ExistentialQuantificationTag::keyword) > prefix##_role_choice > prefix##_concept_choice);              \
    const auto prefix##_concept_at_least_number_restriction_def =                                                                                              \
        with_constructor_parentheses(lit(runir::kr::dl::AtLeastNumberRestrictionTag::keyword) > uint_ > prefix##_role_choice);                                 \
    const auto prefix##_concept_at_most_number_restriction_def =                                                                                               \
        with_constructor_parentheses(lit(runir::kr::dl::AtMostNumberRestrictionTag::keyword) > uint_ > prefix##_role_choice);                                  \
    const auto prefix##_concept_exact_number_restriction_def =                                                                                                 \
        with_constructor_parentheses(lit(runir::kr::dl::ExactNumberRestrictionTag::keyword) > uint_ > prefix##_role_choice);                                   \
    const auto prefix##_concept_qualified_at_least_number_restriction_def = with_constructor_parentheses(                                                      \
        lit(runir::kr::dl::QualifiedAtLeastNumberRestrictionTag::keyword) > uint_ > prefix##_role_choice > prefix##_concept_choice);                           \
    const auto prefix##_concept_qualified_at_most_number_restriction_def = with_constructor_parentheses(                                                       \
        lit(runir::kr::dl::QualifiedAtMostNumberRestrictionTag::keyword) > uint_ > prefix##_role_choice > prefix##_concept_choice);                            \
    const auto prefix##_concept_qualified_exact_number_restriction_def = with_constructor_parentheses(                                                         \
        lit(runir::kr::dl::QualifiedExactNumberRestrictionTag::keyword) > uint_ > prefix##_role_choice > prefix##_concept_choice);                             \
    const auto prefix##_concept_role_value_map_def =                                                                                                           \
        with_constructor_parentheses(lit(runir::kr::dl::RoleValueMapTag::keyword) > prefix##_role_choice > prefix##_role_choice);                              \
    const auto prefix##_concept_agreement_def =                                                                                                                \
        with_constructor_parentheses(lit(runir::kr::dl::AgreementTag::keyword) > prefix##_role_choice > prefix##_role_choice);                                 \
    const auto prefix##_concept_role_fillers_def =                                                                                                             \
        with_constructor_parentheses(lit(runir::kr::dl::RoleFillersTag::keyword) > prefix##_role_choice > +object_name_string_parser());                       \
    const auto prefix##_concept_one_of_def = with_constructor_parentheses(lit(runir::kr::dl::OneOfTag::keyword) > +object_name_string_parser());               \
    const auto prefix##_concept_nominal_def = with_constructor_parentheses(lit(runir::kr::dl::NominalTag::keyword) > object_name_string_parser());             \
    const auto prefix##_concept_register_def =                                                                                                                 \
        with_constructor_parentheses(lit(runir::kr::dl::ConceptRegisterSyntaxTag::keyword) > register_reference_parser<runir::kr::dl::ConceptTag>());          \
    const auto prefix##_concept_argument_def = with_constructor_parentheses(lit(runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>::keyword)                \
                                                                            > argument_reference_parser<runir::kr::dl::ConceptTag>());                         \
    const auto prefix##_role_universal_def =                                                                                                                   \
        with_constructor_parentheses(lit(runir::kr::dl::UniversalTag::keyword) >> x3::attr(grammar_ast::RoleUniversal<Family> {}));                            \
    const auto prefix##_role_atomic_state_def =                                                                                                                \
        with_constructor_parentheses(lit(runir::kr::dl::RoleAtomicStateSyntaxTag::keyword) > predicate_name_string_parser());                                  \
    const auto prefix##_role_atomic_goal_def =                                                                                                                 \
        with_constructor_parentheses(lit(runir::kr::dl::RoleAtomicGoalSyntaxTag::keyword) > predicate_name_string_parser() > bool_string_parser());            \
    const auto prefix##_role_intersection_def =                                                                                                                \
        with_constructor_parentheses(lit(runir::kr::dl::RoleIntersectionSyntaxTag::keyword) > prefix##_role_choice > prefix##_role_choice);                    \
    const auto prefix##_role_union_def =                                                                                                                       \
        with_constructor_parentheses(lit(runir::kr::dl::RoleUnionSyntaxTag::keyword) > prefix##_role_choice > prefix##_role_choice);                           \
    const auto prefix##_role_complement_def = with_constructor_parentheses(lit(runir::kr::dl::ComplementTag::keyword) > prefix##_role_choice);                 \
    const auto prefix##_role_inverse_def = with_constructor_parentheses(lit(runir::kr::dl::InverseTag::keyword) > prefix##_role_choice);                       \
    const auto prefix##_role_composition_def =                                                                                                                 \
        with_constructor_parentheses(lit(runir::kr::dl::CompositionTag::keyword) > prefix##_role_choice > prefix##_role_choice);                               \
    const auto prefix##_role_transitive_closure_def = with_constructor_parentheses(lit(runir::kr::dl::TransitiveClosureTag::keyword) > prefix##_role_choice);  \
    const auto prefix##_role_reflexive_transitive_closure_def =                                                                                                \
        with_constructor_parentheses(lit(runir::kr::dl::ReflexiveTransitiveClosureTag::keyword) > prefix##_role_choice);                                       \
    const auto prefix##_role_restriction_def =                                                                                                                 \
        with_constructor_parentheses(lit(runir::kr::dl::RestrictionTag::keyword) > prefix##_role_choice > prefix##_concept_choice);                            \
    const auto prefix##_role_identity_def = with_constructor_parentheses(lit(runir::kr::dl::IdentityTag::keyword) > prefix##_concept_choice);                  \
    const auto prefix##_role_register_def =                                                                                                                    \
        with_constructor_parentheses(lit(runir::kr::dl::RoleRegisterSyntaxTag::keyword) > register_reference_parser<runir::kr::dl::RoleTag>());                \
    const auto prefix##_role_argument_def =                                                                                                                    \
        with_constructor_parentheses(lit(runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>::keyword) > argument_reference_parser<runir::kr::dl::RoleTag>());  \
    const auto prefix##_boolean_atomic_state_def =                                                                                                             \
        with_constructor_parentheses(lit(runir::kr::dl::BooleanAtomicStateSyntaxTag::keyword) > predicate_name_string_parser() > bool_string_parser());        \
    const auto prefix##_boolean_atomic_goal_def =                                                                                                              \
        with_constructor_parentheses(lit(runir::kr::dl::BooleanAtomicGoalSyntaxTag::keyword) > predicate_name_string_parser() > bool_string_parser());         \
    const auto prefix##_boolean_nonempty_def =                                                                                                                 \
        with_constructor_parentheses(lit(runir::kr::dl::NonemptyTag::keyword) > prefix##_constructor_or_non_terminal_variant);                                 \
    const auto prefix##_boolean_eq_def =                                                                                                                       \
        with_constructor_parentheses(lit(runir::kr::dl::BooleanEqTag::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                           \
    const auto prefix##_boolean_neq_def =                                                                                                                      \
        with_constructor_parentheses(lit(runir::kr::dl::BooleanNeTag::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                           \
    const auto prefix##_boolean_lt_def =                                                                                                                       \
        with_constructor_parentheses(lit(runir::kr::dl::BooleanLtTag::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                           \
    const auto prefix##_boolean_le_def =                                                                                                                       \
        with_constructor_parentheses(lit(runir::kr::dl::BooleanLeTag::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                           \
    const auto prefix##_boolean_gt_def =                                                                                                                       \
        with_constructor_parentheses(lit(runir::kr::dl::BooleanGtTag::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                           \
    const auto prefix##_boolean_ge_def =                                                                                                                       \
        with_constructor_parentheses(lit(runir::kr::dl::BooleanGeTag::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                           \
    const auto prefix##_numerical_eq_def =                                                                                                                     \
        with_constructor_parentheses(lit(runir::kr::dl::NumericalEqTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                     \
    const auto prefix##_numerical_neq_def =                                                                                                                    \
        with_constructor_parentheses(lit(runir::kr::dl::NumericalNeTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                     \
    const auto prefix##_numerical_lt_def =                                                                                                                     \
        with_constructor_parentheses(lit(runir::kr::dl::NumericalLtTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                     \
    const auto prefix##_numerical_le_def =                                                                                                                     \
        with_constructor_parentheses(lit(runir::kr::dl::NumericalLeTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                     \
    const auto prefix##_numerical_gt_def =                                                                                                                     \
        with_constructor_parentheses(lit(runir::kr::dl::NumericalGtTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                     \
    const auto prefix##_numerical_ge_def =                                                                                                                     \
        with_constructor_parentheses(lit(runir::kr::dl::NumericalGeTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                     \
    const auto prefix##_boolean_constant_def = with_constructor_parentheses(lit(runir::kr::dl::BooleanConstantTag::keyword) > bool_string_parser());           \
    const auto prefix##_boolean_and_def =                                                                                                                      \
        with_constructor_parentheses(lit(runir::kr::dl::AndTag::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                                 \
    const auto prefix##_boolean_or_def = with_constructor_parentheses(lit(runir::kr::dl::OrTag::keyword) > prefix##_boolean_choice > prefix##_boolean_choice); \
    const auto prefix##_boolean_not_def = with_constructor_parentheses(lit(runir::kr::dl::NotTag::keyword) > prefix##_boolean_choice);                         \
    const auto prefix##_boolean_argument_def = with_constructor_parentheses(lit(runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>::keyword)                \
                                                                            > argument_reference_parser<runir::kr::dl::BooleanTag>());                         \
    const auto prefix##_numerical_count_def =                                                                                                                  \
        with_constructor_parentheses(lit(runir::kr::dl::CountTag::keyword) > prefix##_constructor_or_non_terminal_variant);                                    \
    const auto prefix##_numerical_distance_def =                                                                                                               \
        with_constructor_parentheses(lit(runir::kr::dl::DistanceTag::keyword) > prefix##_concept_choice > prefix##_role_choice > prefix##_concept_choice);     \
    const auto prefix##_numerical_constant_def = with_constructor_parentheses(lit(runir::kr::dl::NumericalConstantTag::keyword) > uint_);                      \
    const auto prefix##_numerical_add_def =                                                                                                                    \
        with_constructor_parentheses(lit(runir::kr::dl::AddTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                             \
    const auto prefix##_numerical_sub_def =                                                                                                                    \
        with_constructor_parentheses(lit(runir::kr::dl::SubTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                             \
    const auto prefix##_numerical_mul_def =                                                                                                                    \
        with_constructor_parentheses(lit(runir::kr::dl::MulTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                             \
    const auto prefix##_numerical_div_def =                                                                                                                    \
        with_constructor_parentheses(lit(runir::kr::dl::DivTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                             \
    const auto prefix##_numerical_min_def =                                                                                                                    \
        with_constructor_parentheses(lit(runir::kr::dl::MinTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                             \
    const auto prefix##_numerical_max_def =                                                                                                                    \
        with_constructor_parentheses(lit(runir::kr::dl::MaxTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                             \
    const auto prefix##_numerical_argument_def = with_constructor_parentheses(lit(runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>::keyword)            \
                                                                              > argument_reference_parser<runir::kr::dl::NumericalTag>());                     \
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
#undef RUNIR_CONSTRUCTOR_ALTERNATIVE
#undef RUNIR_DECLARE_CONSTRUCTOR_RULE
#undef RUNIR_NUMERICAL_CONSTRUCTORS
#undef RUNIR_BOOLEAN_CONSTRUCTORS
#undef RUNIR_ROLE_CONSTRUCTORS
#undef RUNIR_CONCEPT_CONSTRUCTORS

}  // namespace runir::kr::dl::grammar::parser

#endif
