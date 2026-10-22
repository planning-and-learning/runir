#ifndef RUNIR_KR_DL_GRAMMAR_PARSER_PARSER_DEF_HPP_
#define RUNIR_KR_DL_GRAMMAR_PARSER_PARSER_DEF_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/grammar/ast/ast_adapted.hpp"
#include "runir/kr/dl/grammar/parser/parsers.hpp"

#include <boost/spirit/home/x3/support/traits/is_parser.hpp>
#include <string>

namespace runir::kr::dl::grammar::parser
{

using boost::spirit::x3::eoi;
using boost::spirit::x3::lexeme;
using boost::spirit::x3::lit;
using boost::spirit::x3::raw;
using boost::spirit::x3::ascii::alnum;
using boost::spirit::x3::ascii::alpha;
using boost::spirit::x3::ascii::char_;
using boost::spirit::x3::ascii::digit;
using runir::kr::parser::keyword;
using ygg::diagnostics::context;

template<runir::kr::dl::CategoryTag Category, typename Tag>
inline constexpr bool has_ast_constructor_tag_v = ygg::InTypeList<Tag, runir::kr::dl::grammar::ast::AstConstructorTagsT<Category>>;

template<bool Enabled, typename Parser>
    requires boost::spirit::x3::traits::is_parser<Parser>::value
auto maybe(Parser parser)
{
    if constexpr (Enabled)
        return parser;
    else
        return boost::spirit::x3::eps(false);
}

/// Whether a dialect admits a constructor: SearchFreeDialectTag, the dialect of the edges of n_distance and q_must_pass,
/// excludes transitive closures and searches; the searches explore their edges themselves.
template<typename Dialect, typename Tag>
inline constexpr bool dialect_allows_v =
    std::same_as<Dialect, FullDialectTag>
    || !(std::same_as<Tag, runir::kr::dl::TransitiveClosureTag> || std::same_as<Tag, runir::kr::dl::ReflexiveTransitiveClosureTag>
         || std::same_as<Tag, runir::kr::dl::DistanceTag> || std::same_as<Tag, runir::kr::dl::QueryMustPassTag>);

template<typename Tag>
struct ForbiddenClass
{
};

/// Fails with an expectation error at the keyword of a constructor the dialect excludes, instead of a generic one.
template<typename Dialect, typename Tag>
auto forbidden()
{
    if constexpr (dialect_allows_v<Dialect, Tag>)
        return boost::spirit::x3::eps(false);
    else
        return lit("(") >> keyword(Tag::keyword)
               > (boost::spirit::x3::rule<ForbiddenClass<Tag>> { "no transitive closure, n_distance or q_must_pass in the edges of n_distance or q_must_pass" } =
                      boost::spirit::x3::eps(false));
}

predicate_name_type const predicate_name = "predicate name";
object_name_type const object_name = "object name";
identifier_type const identifier = "identifier";
numeric_reference_type const numeric_reference = "numeric_reference";

const auto predicate_name_def = boost::spirit::x3::rule<PredicateNameTextClass, std::string> { "predicate_name_text" } =
    raw[lexeme[((&alpha >> +(alnum | char_('-') | char_('_'))) | boost::spirit::x3::repeat(1)[char_('=')])]];
const auto object_name_def = boost::spirit::x3::rule<ObjectNameTextClass, std::string> { "object_name_text" } =
    raw[lexeme[&alpha >> +(alnum | char_('-') | char_('_'))]];
const auto identifier_def = boost::spirit::x3::rule<IdentifierTextClass, std::string> { "identifier_text" } =
    raw[lexeme[(alpha | char_('_')) >> *(alnum | char_('-') | char_('_'))]];
const auto unsigned_integer = boost::spirit::x3::rule<class UnsignedInteger, unsigned int> { "unsigned integer" } = boost::spirit::x3::uint_;
const auto numeric_reference_def = boost::spirit::x3::rule<NumericReferenceTextClass, ygg::uint_t> { "numeric_reference_value" } = unsigned_integer;

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

inline auto predicate_name_string_parser()
{
    return boost::spirit::x3::rule<class QuotedPredicateName, runir::kr::dl::grammar::ast::Identifier> { "quoted predicate name" } =
               lexeme[context("quoted predicate name")[lit('"') > predicate_name > lit('"')]];
}

inline auto object_name_string_parser()
{
    return boost::spirit::x3::rule<class QuotedObjectName, runir::kr::dl::grammar::ast::Identifier> { "quoted object name" } =
               lexeme[context("quoted object name")[lit('"') > object_name > lit('"')]];
}

inline auto object_names_parser()
{
    return boost::spirit::x3::rule<class ObjectNamesClass, std::vector<runir::kr::dl::grammar::ast::Identifier>> { "object names" } =
               +object_name_string_parser();
}

inline auto query_columns_parser()
{
    return boost::spirit::x3::rule<class QueryColumnsClass, std::vector<runir::kr::dl::grammar::ast::Identifier>> { "query columns" } =
               lit("(") > *identifier > lit(")");
}

inline auto true_string_parser() { return keyword(runir::kr::dl::TrueTag::keyword) >> boost::spirit::x3::attr(true); }
inline auto false_string_parser() { return keyword(runir::kr::dl::FalseTag::keyword) >> boost::spirit::x3::attr(false); }
inline auto bool_string_parser()
{
    return boost::spirit::x3::rule<class BooleanValue, bool> { "true or false" } = boost::spirit::x3::lexeme[true_string_parser() | false_string_parser()];
}

template<typename Choice>
    requires boost::spirit::x3::traits::is_parser<Choice>::value && requires { typename Choice::attribute_type; }
auto derivation_choices_parser(Choice choice)
{
    const auto choices = boost::spirit::x3::rule<class DerivationChoiceList, std::vector<typename Choice::attribute_type>> { "constructor alternatives" } =
        choice % keyword("or");
    return boost::spirit::x3::rule<class DerivationChoices, std::vector<typename Choice::attribute_type>> { "constructor or alternatives" } =
               boost::spirit::x3::repeat(1)[choice] | (lit("(") > choices > lit(")"));
}

template<typename Parser>
    requires boost::spirit::x3::traits::is_parser<Parser>::value
auto with_constructor_parentheses(Parser parser)
{
    return lit("(") >> parser > lit(")");
}

template<typename Tag>
auto constructor_context(const char* rule_name)
{
    if constexpr (requires { Tag::keyword; })
        return context(Tag::keyword);
    else
        return context(rule_name);
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
    return boost::spirit::x3::rule<NonTerminalNameClass<Category>, runir::kr::dl::grammar::ast::Identifier> { "non_terminal_name" } =
               (boost::spirit::x3::rule<NonTerminalNameTextClass<Category>, std::string> { "non_terminal_name_text" } = raw[parser]);
}

// Try longer arity overloads first; their final operand below remains backtrackable.
#define RUNIR_CONCEPT_CONSTRUCTORS(prefix, X)                                                                                 \
    X(prefix, runir::kr::dl::ConceptTag, ProjectTag, concept_project)                                                         \
    X(prefix, runir::kr::dl::ConceptTag, BotTag, concept_bot)                                                                 \
    X(prefix, runir::kr::dl::ConceptTag, TopTag, concept_top)                                                                 \
    X(prefix, runir::kr::dl::ConceptTag, ConceptAtomicStateSyntaxTag, concept_atomic_state)                                   \
    X(prefix, runir::kr::dl::ConceptTag, ConceptAtomicGoalSyntaxTag, concept_atomic_goal)                                     \
    X(prefix, runir::kr::dl::ConceptTag, ConceptIntersectionSyntaxTag, concept_intersection)                                  \
    X(prefix, runir::kr::dl::ConceptTag, ConceptUnionSyntaxTag, concept_union)                                                \
    X(prefix, runir::kr::dl::ConceptTag, NegationTag, concept_negation)                                                       \
    X(prefix, runir::kr::dl::ConceptTag, ValueRestrictionTag, concept_value_restriction)                                      \
    X(prefix, runir::kr::dl::ConceptTag, ExistentialQuantificationTag, concept_existential_quantification)                    \
    X(prefix, runir::kr::dl::ConceptTag, QualifiedAtLeastNumberRestrictionTag, concept_qualified_at_least_number_restriction) \
    X(prefix, runir::kr::dl::ConceptTag, QualifiedAtMostNumberRestrictionTag, concept_qualified_at_most_number_restriction)   \
    X(prefix, runir::kr::dl::ConceptTag, QualifiedExactNumberRestrictionTag, concept_qualified_exact_number_restriction)      \
    X(prefix, runir::kr::dl::ConceptTag, AtLeastNumberRestrictionTag, concept_at_least_number_restriction)                    \
    X(prefix, runir::kr::dl::ConceptTag, AtMostNumberRestrictionTag, concept_at_most_number_restriction)                      \
    X(prefix, runir::kr::dl::ConceptTag, ExactNumberRestrictionTag, concept_exact_number_restriction)                         \
    X(prefix, runir::kr::dl::ConceptTag, RoleValueMapTag, concept_role_value_map)                                             \
    X(prefix, runir::kr::dl::ConceptTag, AgreementTag, concept_agreement)                                                     \
    X(prefix, runir::kr::dl::ConceptTag, RoleFillersTag, concept_role_fillers)                                                \
    X(prefix, runir::kr::dl::ConceptTag, OneOfTag, concept_one_of)                                                            \
    X(prefix, runir::kr::dl::ConceptTag, NominalTag, concept_nominal)                                                         \
    X(prefix, runir::kr::dl::ConceptTag, RegisterTag, concept_register)                                                       \
    X(prefix, runir::kr::dl::ConceptTag, ArgumentTag<runir::kr::dl::ConceptTag>, concept_argument)

#define RUNIR_ROLE_CONSTRUCTORS(prefix, X)                                                              \
    X(prefix, runir::kr::dl::RoleTag, ProjectTag, role_project)                                         \
    X(prefix, runir::kr::dl::RoleTag, UniversalTag, role_universal)                                     \
    X(prefix, runir::kr::dl::RoleTag, RoleAtomicStateSyntaxTag, role_atomic_state)                      \
    X(prefix, runir::kr::dl::RoleTag, RoleAtomicGoalSyntaxTag, role_atomic_goal)                        \
    X(prefix, runir::kr::dl::RoleTag, RoleIntersectionSyntaxTag, role_intersection)                     \
    X(prefix, runir::kr::dl::RoleTag, RoleUnionSyntaxTag, role_union)                                   \
    X(prefix, runir::kr::dl::RoleTag, ComplementTag, role_complement)                                   \
    X(prefix, runir::kr::dl::RoleTag, InverseTag, role_inverse)                                         \
    X(prefix, runir::kr::dl::RoleTag, CompositionTag, role_composition)                                 \
    X(prefix, runir::kr::dl::RoleTag, TransitiveClosureTag, role_transitive_closure)                    \
    X(prefix, runir::kr::dl::RoleTag, ReflexiveTransitiveClosureTag, role_reflexive_transitive_closure) \
    X(prefix, runir::kr::dl::RoleTag, RestrictionTag, role_restriction)                                 \
    X(prefix, runir::kr::dl::RoleTag, IdentityTag, role_identity)                                       \
    X(prefix, runir::kr::dl::RoleTag, RegisterTag, role_register)                                       \
    X(prefix, runir::kr::dl::RoleTag, ArgumentTag<runir::kr::dl::RoleTag>, role_argument)

#define RUNIR_BOOLEAN_CONSTRUCTORS(prefix, X)                                               \
    X(prefix, runir::kr::dl::BooleanTag, BooleanAtomicStateSyntaxTag, boolean_atomic_state) \
    X(prefix, runir::kr::dl::BooleanTag, BooleanAtomicGoalSyntaxTag, boolean_atomic_goal)   \
    X(prefix, runir::kr::dl::BooleanTag, NonemptyTag, boolean_nonempty)                     \
    X(prefix, runir::kr::dl::BooleanTag, BooleanEqTag, boolean_eq)                          \
    X(prefix, runir::kr::dl::BooleanTag, BooleanNeTag, boolean_neq)                         \
    X(prefix, runir::kr::dl::BooleanTag, BooleanLtTag, boolean_lt)                          \
    X(prefix, runir::kr::dl::BooleanTag, BooleanLeTag, boolean_le)                          \
    X(prefix, runir::kr::dl::BooleanTag, BooleanGtTag, boolean_gt)                          \
    X(prefix, runir::kr::dl::BooleanTag, BooleanGeTag, boolean_ge)                          \
    X(prefix, runir::kr::dl::BooleanTag, NumericalEqTag, numerical_eq)                      \
    X(prefix, runir::kr::dl::BooleanTag, NumericalNeTag, numerical_neq)                     \
    X(prefix, runir::kr::dl::BooleanTag, NumericalLtTag, numerical_lt)                      \
    X(prefix, runir::kr::dl::BooleanTag, NumericalLeTag, numerical_le)                      \
    X(prefix, runir::kr::dl::BooleanTag, NumericalGtTag, numerical_gt)                      \
    X(prefix, runir::kr::dl::BooleanTag, NumericalGeTag, numerical_ge)                      \
    X(prefix, runir::kr::dl::BooleanTag, BooleanConstantTag, boolean_constant)              \
    X(prefix, runir::kr::dl::BooleanTag, AndTag, boolean_and)                               \
    X(prefix, runir::kr::dl::BooleanTag, OrTag, boolean_or)                                 \
    X(prefix, runir::kr::dl::BooleanTag, NotTag, boolean_not)                               \
    X(prefix, runir::kr::dl::BooleanTag, ArgumentTag<runir::kr::dl::BooleanTag>, boolean_argument)

#define RUNIR_NUMERICAL_CONSTRUCTORS(prefix, X)                                      \
    X(prefix, runir::kr::dl::NumericalTag, CountTag, numerical_count)                \
    X(prefix, runir::kr::dl::NumericalTag, DistanceTag, numerical_distance)          \
    X(prefix, runir::kr::dl::NumericalTag, NumericalConstantTag, numerical_constant) \
    X(prefix, runir::kr::dl::NumericalTag, AddTag, numerical_add)                    \
    X(prefix, runir::kr::dl::NumericalTag, SubTag, numerical_sub)                    \
    X(prefix, runir::kr::dl::NumericalTag, MulTag, numerical_mul)                    \
    X(prefix, runir::kr::dl::NumericalTag, DivTag, numerical_div)                    \
    X(prefix, runir::kr::dl::NumericalTag, MinTag, numerical_min)                    \
    X(prefix, runir::kr::dl::NumericalTag, MaxTag, numerical_max)                    \
    X(prefix, runir::kr::dl::NumericalTag, ArgumentTag<runir::kr::dl::NumericalTag>, numerical_argument)

#define RUNIR_DECLARE_CONSTRUCTOR_RULE(prefix, Category, Tag, name) \
    constructor_tag_type<Category, runir::kr::dl::Tag, prefix##_dialect> const prefix##_##name = #name;

#define RUNIR_CONSTRUCTOR_ALTERNATIVE(prefix, Category, Tag, name)                                                              \
    | maybe<has_ast_constructor_tag_v<Category, runir::kr::dl::Tag> && dialect_allows_v<prefix##_dialect, runir::kr::dl::Tag>>( \
        constructor_context<runir::kr::dl::Tag>(#name)[prefix##_##name])                                                        \
        | maybe<has_ast_constructor_tag_v<Category, runir::kr::dl::Tag>>(forbidden<prefix##_dialect, runir::kr::dl::Tag>())

#define RUNIR_DEFINE_PARSER(prefix, Dialect, search_free)                                                                                                      \
    using prefix##_dialect = Dialect;                                                                                                                          \
    query_atomic_state_type<Dialect> const prefix##_query_atomic_state = "query_atomic_state";                                                                 \
    query_atomic_goal_type<Dialect> const prefix##_query_atomic_goal = "query_atomic_goal";                                                                    \
    query_concept_type<Dialect> const prefix##_query_concept = "query_concept";                                                                                \
    query_role_type<Dialect> const prefix##_query_role = "query_role";                                                                                         \
    query_join_type<Dialect> const prefix##_query_join = "query_join";                                                                                         \
    query_project_type<Dialect> const prefix##_query_project = "query_project";                                                                                \
    query_rename_type<Dialect> const prefix##_query_rename = "query_rename";                                                                                   \
    query_select_equal_type<Dialect> const prefix##_query_select_equal = "query_select_equal";                                                                 \
    query_select_value_type<Dialect> const prefix##_query_select_value = "query_select_value";                                                                 \
    query_union_type<Dialect> const prefix##_query_union = "query_union";                                                                                      \
    query_difference_type<Dialect> const prefix##_query_difference = "query_difference";                                                                       \
    query_generic_join_type<Dialect> const prefix##_query_generic_join = "query_generic_join";                                                                 \
    query_empty_type<Dialect> const prefix##_query_empty = "query_empty";                                                                                      \
    query_must_pass_type<Dialect> const prefix##_query_must_pass = "query_must_pass";                                                                          \
    query_type<Dialect> const prefix##_query = "query";                                                                                                        \
    constructor_type<runir::kr::dl::ConceptTag, Dialect> const prefix##_concept = "concept";                                                                   \
    constructor_root_type<runir::kr::dl::ConceptTag, Dialect> const prefix##_concept_root = "concept_root";                                                    \
    constructor_type<runir::kr::dl::RoleTag, Dialect> const prefix##_role = "role";                                                                            \
    constructor_root_type<runir::kr::dl::RoleTag, Dialect> const prefix##_role_root = "role_root";                                                             \
    constructor_type<runir::kr::dl::BooleanTag, Dialect> const prefix##_boolean = "boolean";                                                                   \
    constructor_root_type<runir::kr::dl::BooleanTag, Dialect> const prefix##_boolean_root = "boolean_root";                                                    \
    constructor_type<runir::kr::dl::NumericalTag, Dialect> const prefix##_numerical = "numerical";                                                             \
    constructor_root_type<runir::kr::dl::NumericalTag, Dialect> const prefix##_numerical_root = "numerical_root";                                              \
    non_terminal_type<runir::kr::dl::ConceptTag, Dialect> const prefix##_concept_non_terminal = "concept_non_terminal";                                        \
    choice_type<runir::kr::dl::ConceptTag, Dialect> const prefix##_concept_choice = "concept_choice";                                                          \
    non_terminal_type<runir::kr::dl::RoleTag, Dialect> const prefix##_role_non_terminal = "role_non_terminal";                                                 \
    choice_type<runir::kr::dl::RoleTag, Dialect> const prefix##_role_choice = "role_choice";                                                                   \
    non_terminal_type<runir::kr::dl::BooleanTag, Dialect> const prefix##_boolean_non_terminal = "boolean_non_terminal";                                        \
    choice_type<runir::kr::dl::BooleanTag, Dialect> const prefix##_boolean_choice = "boolean_choice";                                                          \
    non_terminal_type<runir::kr::dl::NumericalTag, Dialect> const prefix##_numerical_non_terminal = "numerical_non_terminal";                                  \
    choice_type<runir::kr::dl::NumericalTag, Dialect> const prefix##_numerical_choice = "numerical_choice";                                                    \
    constructor_or_non_terminal_variant_type<Dialect> const prefix##_constructor_or_non_terminal_variant = "constructor_or_non_terminal_variant";              \
    derivation_rule_type<runir::kr::dl::ConceptTag, Dialect> const prefix##_concept_derivation_rule = "concept_derivation_rule";                               \
    derivation_rule_type<runir::kr::dl::RoleTag, Dialect> const prefix##_role_derivation_rule = "role_derivation_rule";                                        \
    derivation_rule_type<runir::kr::dl::BooleanTag, Dialect> const prefix##_boolean_derivation_rule = "boolean_derivation_rule";                               \
    derivation_rule_type<runir::kr::dl::NumericalTag, Dialect> const prefix##_numerical_derivation_rule = "numerical_derivation_rule";                         \
    derivation_rule_variant_type<Dialect> const prefix##_derivation_rule_variant = "derivation_rule_variant";                                                  \
    grammar_body_type<Dialect> const prefix##_grammar_body = "grammar_body";                                                                                   \
    grammar_type<Dialect> const prefix##_grammar = "grammar";                                                                                                  \
    grammar_root_type<Dialect> const prefix##_grammar_root = "grammar_root";                                                                                   \
    RUNIR_CONCEPT_CONSTRUCTORS(prefix, RUNIR_DECLARE_CONSTRUCTOR_RULE)                                                                                         \
    RUNIR_ROLE_CONSTRUCTORS(prefix, RUNIR_DECLARE_CONSTRUCTOR_RULE)                                                                                            \
    RUNIR_BOOLEAN_CONSTRUCTORS(prefix, RUNIR_DECLARE_CONSTRUCTOR_RULE)                                                                                         \
    RUNIR_NUMERICAL_CONSTRUCTORS(prefix, RUNIR_DECLARE_CONSTRUCTOR_RULE)                                                                                       \
    const auto prefix##_concept_def = boost::spirit::x3::eps(false) RUNIR_CONCEPT_CONSTRUCTORS(prefix, RUNIR_CONSTRUCTOR_ALTERNATIVE);                         \
    const auto prefix##_concept_root_def = context("concept expression")[prefix##_concept > eoi];                                                              \
    const auto prefix##_concept_non_terminal_def = non_terminal_string_parser<runir::kr::dl::ConceptTag>();                                                    \
    const auto prefix##_concept_choice_def = prefix##_concept_non_terminal | prefix##_concept;                                                                 \
    const auto prefix##_role_def = boost::spirit::x3::eps(false) RUNIR_ROLE_CONSTRUCTORS(prefix, RUNIR_CONSTRUCTOR_ALTERNATIVE);                               \
    const auto prefix##_role_root_def = context("role expression")[prefix##_role > eoi];                                                                       \
    const auto prefix##_role_non_terminal_def = non_terminal_string_parser<runir::kr::dl::RoleTag>();                                                          \
    const auto prefix##_role_choice_def = prefix##_role_non_terminal | prefix##_role;                                                                          \
    const auto prefix##_constructor_or_non_terminal_variant_def = prefix##_concept_choice | prefix##_role_choice | prefix##_query;                             \
    const auto prefix##_boolean_def = boost::spirit::x3::eps(false) RUNIR_BOOLEAN_CONSTRUCTORS(prefix, RUNIR_CONSTRUCTOR_ALTERNATIVE);                         \
    const auto prefix##_boolean_root_def = context("boolean expression")[prefix##_boolean > eoi];                                                              \
    const auto prefix##_boolean_non_terminal_def = non_terminal_string_parser<runir::kr::dl::BooleanTag>();                                                    \
    const auto prefix##_boolean_choice_def = prefix##_boolean_non_terminal | prefix##_boolean;                                                                 \
    const auto prefix##_numerical_def = boost::spirit::x3::eps(false) RUNIR_NUMERICAL_CONSTRUCTORS(prefix, RUNIR_CONSTRUCTOR_ALTERNATIVE);                     \
    const auto prefix##_numerical_root_def = context("numerical expression")[prefix##_numerical > eoi];                                                        \
    const auto prefix##_numerical_non_terminal_def = non_terminal_string_parser<runir::kr::dl::NumericalTag>();                                                \
    const auto prefix##_numerical_choice_def = prefix##_numerical_non_terminal | prefix##_numerical;                                                           \
    const auto prefix##_query_atomic_state_def =                                                                                                               \
        with_constructor_parentheses(keyword("q_atomic_state") > predicate_name_string_parser() > query_columns_parser());                                     \
    const auto prefix##_query_atomic_goal_def =                                                                                                                \
        with_constructor_parentheses(keyword("q_atomic_goal") > predicate_name_string_parser() > bool_string_parser() > query_columns_parser());               \
    const auto prefix##_query_concept_def =                                                                                                                    \
        with_constructor_parentheses(keyword(runir::kr::dl::QueryConceptTag::keyword) > boost::spirit::x3::repeat(1)[identifier] > prefix##_concept_choice);   \
    const auto prefix##_query_role_def =                                                                                                                       \
        with_constructor_parentheses(keyword(runir::kr::dl::QueryRoleTag::keyword) > query_columns_parser() > prefix##_role_choice);                           \
    const auto prefix##_query_join_def = with_constructor_parentheses(keyword(runir::kr::dl::QueryJoinTag::keyword) > prefix##_query > prefix##_query);        \
    const auto prefix##_query_project_def =                                                                                                                    \
        with_constructor_parentheses(keyword(runir::kr::dl::QueryProjectTag::keyword) > query_columns_parser() > prefix##_query);                              \
    const auto prefix##_query_rename_def =                                                                                                                     \
        with_constructor_parentheses(keyword(runir::kr::dl::QueryRenameTag::keyword) > query_columns_parser() > prefix##_query);                               \
    const auto prefix##_query_select_equal_def =                                                                                                               \
        with_constructor_parentheses(keyword(runir::kr::dl::QuerySelectEqualTag::keyword) > identifier > identifier > prefix##_query);                         \
    const auto prefix##_query_select_value_def =                                                                                                               \
        with_constructor_parentheses(keyword(runir::kr::dl::QuerySelectValueTag::keyword) > identifier > object_name_string_parser() > prefix##_query);        \
    const auto prefix##_query_union_def = with_constructor_parentheses(keyword(runir::kr::dl::QueryUnionTag::keyword) > prefix##_query > prefix##_query);      \
    const auto prefix##_query_difference_def =                                                                                                                 \
        with_constructor_parentheses(keyword(runir::kr::dl::QueryDifferenceTag::keyword) > prefix##_query > prefix##_query);                                   \
    const auto prefix##_query_generic_join_def =                                                                                                               \
        with_constructor_parentheses(keyword(runir::kr::dl::QueryGenericJoinTag::keyword) > query_columns_parser() > query_columns_parser()                    \
                                     > boost::spirit::x3::repeat(2, boost::spirit::x3::inf)[prefix##_query]);                                                  \
    const auto prefix##_query_empty_def = with_constructor_parentheses(keyword(runir::kr::dl::QueryEmptyTag::keyword) > query_columns_parser());               \
    const auto prefix##_query_must_pass_def = with_constructor_parentheses(keyword(runir::kr::dl::QueryMustPassTag::keyword) > query_columns_parser()          \
                                                                           > prefix##_query > search_free##_query > prefix##_query > prefix##_query);          \
    const auto prefix##_concept_project_def =                                                                                                                  \
        with_constructor_parentheses(keyword(runir::kr::dl::ConceptProjectSyntaxTag::keyword) > boost::spirit::x3::repeat(1)[identifier] > prefix##_query);    \
    const auto prefix##_role_project_def =                                                                                                                     \
        with_constructor_parentheses(keyword(runir::kr::dl::RoleProjectSyntaxTag::keyword) > boost::spirit::x3::repeat(2)[identifier] > prefix##_query);       \
    const auto prefix##_query_def = prefix##_query_atomic_state | prefix##_query_atomic_goal | prefix##_query_concept | prefix##_query_role                    \
                                    | prefix##_query_join | prefix##_query_project | prefix##_query_rename | prefix##_query_select_equal                       \
                                    | prefix##_query_select_value | prefix##_query_union | prefix##_query_difference | prefix##_query_generic_join             \
                                    | prefix##_query_empty | maybe<dialect_allows_v<Dialect, runir::kr::dl::QueryMustPassTag>>(prefix##_query_must_pass)       \
                                    | forbidden<Dialect, runir::kr::dl::QueryMustPassTag>();                                                                   \
    BOOST_SPIRIT_DEFINE(prefix##_query_atomic_state,                                                                                                           \
                        prefix##_query_atomic_goal,                                                                                                            \
                        prefix##_query_concept,                                                                                                                \
                        prefix##_query_role,                                                                                                                   \
                        prefix##_query_join,                                                                                                                   \
                        prefix##_query_project,                                                                                                                \
                        prefix##_query_rename,                                                                                                                 \
                        prefix##_query_select_equal,                                                                                                           \
                        prefix##_query_select_value,                                                                                                           \
                        prefix##_query_union,                                                                                                                  \
                        prefix##_query_difference,                                                                                                             \
                        prefix##_query_generic_join,                                                                                                           \
                        prefix##_query_empty,                                                                                                                  \
                        prefix##_query_must_pass,                                                                                                              \
                        prefix##_query,                                                                                                                        \
                        prefix##_concept_project,                                                                                                              \
                        prefix##_role_project)                                                                                                                 \
    const auto prefix##_concept_bot_def =                                                                                                                      \
        with_constructor_parentheses(keyword(runir::kr::dl::BotTag::keyword) >> boost::spirit::x3::attr(runir::kr::dl::grammar::ast::ConceptBot {}));          \
    const auto prefix##_concept_top_def =                                                                                                                      \
        with_constructor_parentheses(keyword(runir::kr::dl::TopTag::keyword) >> boost::spirit::x3::attr(runir::kr::dl::grammar::ast::ConceptTop {}));          \
    const auto prefix##_concept_atomic_state_def =                                                                                                             \
        with_constructor_parentheses(keyword(runir::kr::dl::ConceptAtomicStateSyntaxTag::keyword) > predicate_name_string_parser());                           \
    const auto prefix##_concept_atomic_goal_def =                                                                                                              \
        with_constructor_parentheses(keyword(runir::kr::dl::ConceptAtomicGoalSyntaxTag::keyword) > predicate_name_string_parser() > bool_string_parser());     \
    const auto prefix##_concept_intersection_def =                                                                                                             \
        with_constructor_parentheses(keyword(runir::kr::dl::ConceptIntersectionSyntaxTag::keyword) > prefix##_concept_choice > prefix##_concept_choice);       \
    const auto prefix##_concept_union_def =                                                                                                                    \
        with_constructor_parentheses(keyword(runir::kr::dl::ConceptUnionSyntaxTag::keyword) > prefix##_concept_choice > prefix##_concept_choice);              \
    const auto prefix##_concept_negation_def = with_constructor_parentheses(keyword(runir::kr::dl::NegationTag::keyword) > prefix##_concept_choice);           \
    const auto prefix##_concept_value_restriction_def =                                                                                                        \
        with_constructor_parentheses(keyword(runir::kr::dl::ValueRestrictionTag::keyword) > prefix##_role_choice > prefix##_concept_choice);                   \
    const auto prefix##_concept_existential_quantification_def =                                                                                               \
        with_constructor_parentheses(keyword(runir::kr::dl::ExistentialQuantificationTag::keyword) > prefix##_role_choice > prefix##_concept_choice);          \
    const auto prefix##_concept_at_least_number_restriction_def =                                                                                              \
        with_constructor_parentheses(keyword(runir::kr::dl::AtLeastNumberRestrictionTag::keyword) > unsigned_integer > prefix##_role_choice);                  \
    const auto prefix##_concept_at_most_number_restriction_def =                                                                                               \
        with_constructor_parentheses(keyword(runir::kr::dl::AtMostNumberRestrictionTag::keyword) > unsigned_integer > prefix##_role_choice);                   \
    const auto prefix##_concept_exact_number_restriction_def =                                                                                                 \
        with_constructor_parentheses(keyword(runir::kr::dl::ExactNumberRestrictionTag::keyword) > unsigned_integer > prefix##_role_choice);                    \
    const auto prefix##_concept_qualified_at_least_number_restriction_def = with_constructor_parentheses(                                                      \
        (keyword(runir::kr::dl::QualifiedAtLeastNumberRestrictionTag::keyword) > unsigned_integer > prefix##_role_choice) >> prefix##_concept_choice);         \
    const auto prefix##_concept_qualified_at_most_number_restriction_def = with_constructor_parentheses(                                                       \
        (keyword(runir::kr::dl::QualifiedAtMostNumberRestrictionTag::keyword) > unsigned_integer > prefix##_role_choice) >> prefix##_concept_choice);          \
    const auto prefix##_concept_qualified_exact_number_restriction_def = with_constructor_parentheses(                                                         \
        (keyword(runir::kr::dl::QualifiedExactNumberRestrictionTag::keyword) > unsigned_integer > prefix##_role_choice) >> prefix##_concept_choice);           \
    const auto prefix##_concept_role_value_map_def =                                                                                                           \
        with_constructor_parentheses(keyword(runir::kr::dl::RoleValueMapTag::keyword) > prefix##_role_choice > prefix##_role_choice);                          \
    const auto prefix##_concept_agreement_def =                                                                                                                \
        with_constructor_parentheses(keyword(runir::kr::dl::AgreementTag::keyword) > prefix##_role_choice > prefix##_role_choice);                             \
    const auto prefix##_concept_role_fillers_def =                                                                                                             \
        with_constructor_parentheses(keyword(runir::kr::dl::RoleFillersTag::keyword) > prefix##_role_choice > object_names_parser());                          \
    const auto prefix##_concept_one_of_def = with_constructor_parentheses(keyword(runir::kr::dl::OneOfTag::keyword) > object_names_parser());                  \
    const auto prefix##_concept_nominal_def = with_constructor_parentheses(keyword(runir::kr::dl::NominalTag::keyword) > object_name_string_parser());         \
    const auto prefix##_concept_register_def =                                                                                                                 \
        with_constructor_parentheses(keyword(runir::kr::dl::ConceptRegisterSyntaxTag::keyword) > register_reference_parser<runir::kr::dl::ConceptTag>());      \
    const auto prefix##_concept_argument_def = with_constructor_parentheses(keyword(runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>::keyword)            \
                                                                            > argument_reference_parser<runir::kr::dl::ConceptTag>());                         \
    const auto prefix##_role_universal_def =                                                                                                                   \
        with_constructor_parentheses(keyword(runir::kr::dl::UniversalTag::keyword) >> boost::spirit::x3::attr(runir::kr::dl::grammar::ast::RoleUniversal {})); \
    const auto prefix##_role_atomic_state_def =                                                                                                                \
        with_constructor_parentheses(keyword(runir::kr::dl::RoleAtomicStateSyntaxTag::keyword) > predicate_name_string_parser());                              \
    const auto prefix##_role_atomic_goal_def =                                                                                                                 \
        with_constructor_parentheses(keyword(runir::kr::dl::RoleAtomicGoalSyntaxTag::keyword) > predicate_name_string_parser() > bool_string_parser());        \
    const auto prefix##_role_intersection_def =                                                                                                                \
        with_constructor_parentheses(keyword(runir::kr::dl::RoleIntersectionSyntaxTag::keyword) > prefix##_role_choice > prefix##_role_choice);                \
    const auto prefix##_role_union_def =                                                                                                                       \
        with_constructor_parentheses(keyword(runir::kr::dl::RoleUnionSyntaxTag::keyword) > prefix##_role_choice > prefix##_role_choice);                       \
    const auto prefix##_role_complement_def = with_constructor_parentheses(keyword(runir::kr::dl::ComplementTag::keyword) > prefix##_role_choice);             \
    const auto prefix##_role_inverse_def = with_constructor_parentheses(keyword(runir::kr::dl::InverseTag::keyword) > prefix##_role_choice);                   \
    const auto prefix##_role_composition_def =                                                                                                                 \
        with_constructor_parentheses(keyword(runir::kr::dl::CompositionTag::keyword) > prefix##_role_choice > prefix##_role_choice);                           \
    const auto prefix##_role_transitive_closure_def =                                                                                                          \
        with_constructor_parentheses(keyword(runir::kr::dl::TransitiveClosureTag::keyword) > prefix##_role_choice);                                            \
    const auto prefix##_role_reflexive_transitive_closure_def =                                                                                                \
        with_constructor_parentheses(keyword(runir::kr::dl::ReflexiveTransitiveClosureTag::keyword) > prefix##_role_choice);                                   \
    const auto prefix##_role_restriction_def =                                                                                                                 \
        with_constructor_parentheses(keyword(runir::kr::dl::RestrictionTag::keyword) > prefix##_role_choice > prefix##_concept_choice);                        \
    const auto prefix##_role_identity_def = with_constructor_parentheses(keyword(runir::kr::dl::IdentityTag::keyword) > prefix##_concept_choice);              \
    const auto prefix##_role_register_def =                                                                                                                    \
        with_constructor_parentheses(keyword(runir::kr::dl::RoleRegisterSyntaxTag::keyword) > register_reference_parser<runir::kr::dl::RoleTag>());            \
    const auto prefix##_role_argument_def = with_constructor_parentheses(keyword(runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>::keyword)                  \
                                                                         > argument_reference_parser<runir::kr::dl::RoleTag>());                               \
    const auto prefix##_boolean_atomic_state_def =                                                                                                             \
        with_constructor_parentheses(keyword(runir::kr::dl::BooleanAtomicStateSyntaxTag::keyword) > predicate_name_string_parser() > bool_string_parser());    \
    const auto prefix##_boolean_atomic_goal_def =                                                                                                              \
        with_constructor_parentheses(keyword(runir::kr::dl::BooleanAtomicGoalSyntaxTag::keyword) > predicate_name_string_parser() > bool_string_parser());     \
    const auto prefix##_boolean_nonempty_def =                                                                                                                 \
        with_constructor_parentheses(keyword(runir::kr::dl::NonemptyTag::keyword) > prefix##_constructor_or_non_terminal_variant);                             \
    const auto prefix##_boolean_eq_def =                                                                                                                       \
        with_constructor_parentheses(keyword(runir::kr::dl::BooleanEqTag::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                       \
    const auto prefix##_boolean_neq_def =                                                                                                                      \
        with_constructor_parentheses(keyword(runir::kr::dl::BooleanNeTag::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                       \
    const auto prefix##_boolean_lt_def =                                                                                                                       \
        with_constructor_parentheses(keyword(runir::kr::dl::BooleanLtTag::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                       \
    const auto prefix##_boolean_le_def =                                                                                                                       \
        with_constructor_parentheses(keyword(runir::kr::dl::BooleanLeTag::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                       \
    const auto prefix##_boolean_gt_def =                                                                                                                       \
        with_constructor_parentheses(keyword(runir::kr::dl::BooleanGtTag::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                       \
    const auto prefix##_boolean_ge_def =                                                                                                                       \
        with_constructor_parentheses(keyword(runir::kr::dl::BooleanGeTag::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                       \
    const auto prefix##_numerical_eq_def =                                                                                                                     \
        with_constructor_parentheses(keyword(runir::kr::dl::NumericalEqTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                 \
    const auto prefix##_numerical_neq_def =                                                                                                                    \
        with_constructor_parentheses(keyword(runir::kr::dl::NumericalNeTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                 \
    const auto prefix##_numerical_lt_def =                                                                                                                     \
        with_constructor_parentheses(keyword(runir::kr::dl::NumericalLtTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                 \
    const auto prefix##_numerical_le_def =                                                                                                                     \
        with_constructor_parentheses(keyword(runir::kr::dl::NumericalLeTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                 \
    const auto prefix##_numerical_gt_def =                                                                                                                     \
        with_constructor_parentheses(keyword(runir::kr::dl::NumericalGtTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                 \
    const auto prefix##_numerical_ge_def =                                                                                                                     \
        with_constructor_parentheses(keyword(runir::kr::dl::NumericalGeTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                 \
    const auto prefix##_boolean_constant_def = with_constructor_parentheses(keyword(runir::kr::dl::BooleanConstantTag::keyword) > bool_string_parser());       \
    const auto prefix##_boolean_and_def =                                                                                                                      \
        with_constructor_parentheses(keyword(runir::kr::dl::AndTag::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                             \
    const auto prefix##_boolean_or_def =                                                                                                                       \
        with_constructor_parentheses(keyword(runir::kr::dl::OrTag::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                              \
    const auto prefix##_boolean_not_def = with_constructor_parentheses(keyword(runir::kr::dl::NotTag::keyword) > prefix##_boolean_choice);                     \
    const auto prefix##_boolean_argument_def = with_constructor_parentheses(keyword(runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>::keyword)            \
                                                                            > argument_reference_parser<runir::kr::dl::BooleanTag>());                         \
    const auto prefix##_numerical_count_def =                                                                                                                  \
        with_constructor_parentheses(keyword(runir::kr::dl::CountTag::keyword) > prefix##_constructor_or_non_terminal_variant);                                \
    const auto prefix##_numerical_distance_def =                                                                                                               \
        with_constructor_parentheses(keyword(runir::kr::dl::DistanceTag::keyword) > (prefix##_concept_choice | prefix##_query)                                 \
                                     > (search_free##_role_choice | search_free##_query) > (prefix##_concept_choice | prefix##_query));                        \
    const auto prefix##_numerical_constant_def = with_constructor_parentheses(keyword(runir::kr::dl::NumericalConstantTag::keyword) > unsigned_integer);       \
    const auto prefix##_numerical_add_def =                                                                                                                    \
        with_constructor_parentheses(keyword(runir::kr::dl::AddTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                         \
    const auto prefix##_numerical_sub_def =                                                                                                                    \
        with_constructor_parentheses(keyword(runir::kr::dl::SubTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                         \
    const auto prefix##_numerical_mul_def =                                                                                                                    \
        with_constructor_parentheses(keyword(runir::kr::dl::MulTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                         \
    const auto prefix##_numerical_div_def =                                                                                                                    \
        with_constructor_parentheses(keyword(runir::kr::dl::DivTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                         \
    const auto prefix##_numerical_min_def =                                                                                                                    \
        with_constructor_parentheses(keyword(runir::kr::dl::MinTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                         \
    const auto prefix##_numerical_max_def =                                                                                                                    \
        with_constructor_parentheses(keyword(runir::kr::dl::MaxTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                         \
    const auto prefix##_numerical_argument_def = with_constructor_parentheses(keyword(runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>::keyword)        \
                                                                              > argument_reference_parser<runir::kr::dl::NumericalTag>());                     \
    const auto prefix##_concept_derivation_rule_def =                                                                                                          \
        context("concept derivation")[(lit("(") >> prefix##_concept_non_terminal) > derivation_choices_parser(prefix##_concept_choice) > lit(")")];            \
    const auto prefix##_role_derivation_rule_def =                                                                                                             \
        context("role derivation")[(lit("(") >> prefix##_role_non_terminal) > derivation_choices_parser(prefix##_role_choice) > lit(")")];                     \
    const auto prefix##_boolean_derivation_rule_def =                                                                                                          \
        context("boolean derivation")[(lit("(") >> prefix##_boolean_non_terminal) > derivation_choices_parser(prefix##_boolean_choice) > lit(")")];            \
    const auto prefix##_numerical_derivation_rule_def =                                                                                                        \
        context("numerical derivation")[(lit("(") >> prefix##_numerical_non_terminal) > derivation_choices_parser(prefix##_numerical_choice) > lit(")")];      \
    const auto prefix##_derivation_rule_variant_def =                                                                                                          \
        prefix##_concept_derivation_rule | prefix##_role_derivation_rule | prefix##_boolean_derivation_rule | prefix##_numerical_derivation_rule;              \
    const auto prefix##_grammar_body_def = *prefix##_derivation_rule_variant;                                                                                  \
    const auto prefix##_grammar_def = context("DL grammar")[lit("(") > prefix##_grammar_body > lit(")")];                                                      \
    const auto prefix##_grammar_root_def = context("DL grammar")[prefix##_grammar > eoi];                                                                      \
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

// The search-free instances come first: the full ones parse the edges of n_distance and q_must_pass with them.
RUNIR_DEFINE_PARSER(ext_family_search_free, SearchFreeDialectTag, ext_family_search_free)
RUNIR_DEFINE_PARSER(ext_family, FullDialectTag, ext_family_search_free)

#undef RUNIR_DEFINE_PARSER
#undef RUNIR_CONSTRUCTOR_ALTERNATIVE
#undef RUNIR_DECLARE_CONSTRUCTOR_RULE
#undef RUNIR_NUMERICAL_CONSTRUCTORS
#undef RUNIR_BOOLEAN_CONSTRUCTORS
#undef RUNIR_ROLE_CONSTRUCTORS
#undef RUNIR_CONCEPT_CONSTRUCTORS

}  // namespace runir::kr::dl::grammar::parser

#endif
