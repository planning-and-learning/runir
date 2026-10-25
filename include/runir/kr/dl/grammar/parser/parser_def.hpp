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
constexpr auto maybe(const Parser& parser)
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
constexpr auto forbidden()
{
    if constexpr (dialect_allows_v<Dialect, Tag>)
        return boost::spirit::x3::eps(false);
    else
        return lit("(") >> keyword(Tag::keyword)
               > (boost::spirit::x3::rule<ForbiddenClass<Tag>> {
                      "no transitive closure, n_distance or q_must_pass in the edges of n_distance or q_must_pass" } = boost::spirit::x3::eps(false));
}

constexpr predicate_name_type predicate_name = "predicate name";
constexpr object_name_type object_name = "object name";
constexpr identifier_type identifier = "identifier";
constexpr numeric_reference_type numeric_reference = "numeric_reference";

constexpr auto predicate_name_def = boost::spirit::x3::rule<PredicateNameTextClass, std::string> { "predicate_name_text" } =
    raw[lexeme[((&alpha >> +(alnum | char_('-') | char_('_'))) | boost::spirit::x3::repeat(1)[char_('=')])]];
constexpr auto object_name_def = boost::spirit::x3::rule<ObjectNameTextClass, std::string> { "object_name_text" } =
    raw[lexeme[&alpha >> +(alnum | char_('-') | char_('_'))]];
constexpr auto identifier_def = boost::spirit::x3::rule<IdentifierTextClass, std::string> { "identifier_text" } =
    raw[lexeme[(alpha | char_('_')) >> *(alnum | char_('-') | char_('_'))]];
constexpr auto unsigned_integer = boost::spirit::x3::rule<class UnsignedInteger, unsigned int> { "unsigned integer" } = boost::spirit::x3::uint_;
constexpr auto numeric_reference_def = boost::spirit::x3::rule<NumericReferenceTextClass, ygg::uint_t> { "numeric_reference_value" } = unsigned_integer;

BOOST_SPIRIT_DEFINE(predicate_name, object_name, identifier, numeric_reference)

template<runir::kr::dl::CategoryTag Category>
constexpr auto argument_reference_parser()
{ return argument_reference_type<Category> { "argument_reference" } = numeric_reference | identifier; }

template<runir::kr::dl::CategoryTag Category>
constexpr auto register_reference_parser()
{ return register_reference_type<Category> { "register_reference" } = numeric_reference | identifier; }

constexpr auto predicate_name_string_parser()
{
    return boost::spirit::x3::rule<class QuotedPredicateName, runir::kr::dl::grammar::ast::Identifier> { "quoted predicate name" } =
               lexeme[context("quoted predicate name")[lit('"') > predicate_name > lit('"')]];
}

constexpr auto object_name_string_parser()
{
    return boost::spirit::x3::rule<class QuotedObjectName, runir::kr::dl::grammar::ast::Identifier> { "quoted object name" } =
               lexeme[context("quoted object name")[lit('"') > object_name > lit('"')]];
}

constexpr auto object_names_parser()
{
    return boost::spirit::x3::rule<class ObjectNamesClass, std::vector<runir::kr::dl::grammar::ast::Identifier>> { "object names" } =
               +object_name_string_parser();
}

constexpr auto query_columns_parser()
{
    return boost::spirit::x3::rule<class QueryColumnsClass, std::vector<runir::kr::dl::grammar::ast::Identifier>> { "query columns" } =
               lit("(") > *identifier > lit(")");
}

constexpr auto true_string_parser() { return keyword(runir::kr::dl::TrueTag::keyword) >> boost::spirit::x3::attr(true); }
constexpr auto false_string_parser() { return keyword(runir::kr::dl::FalseTag::keyword) >> boost::spirit::x3::attr(false); }
constexpr auto bool_string_parser()
{ return boost::spirit::x3::rule<class BooleanValue, bool> { "true or false" } = boost::spirit::x3::lexeme[true_string_parser() | false_string_parser()]; }

template<typename Choice>
    requires boost::spirit::x3::traits::is_parser<Choice>::value && requires { typename Choice::attribute_type; }
constexpr auto derivation_choices_parser(const Choice& choice)
{
    const auto choices = boost::spirit::x3::rule<class DerivationChoiceList, std::vector<typename Choice::attribute_type>> { "constructor alternatives" } =
        choice % keyword("or");
    return boost::spirit::x3::rule<class DerivationChoices, std::vector<typename Choice::attribute_type>> { "constructor or alternatives" } =
               boost::spirit::x3::repeat(1)[choice] | (lit("(") > choices > lit(")"));
}

template<typename Parser>
    requires boost::spirit::x3::traits::is_parser<Parser>::value
constexpr auto with_constructor_parentheses(const Parser& parser)
{ return lit("(") >> parser > lit(")"); }

template<typename Tag>
constexpr auto constructor_context(const char* rule_name)
{
    if constexpr (requires { Tag::keyword; })
        return context(Tag::keyword);
    else
        return context(rule_name);
}

template<runir::kr::dl::CategoryTag Category>
constexpr auto non_terminal_string_parser()
{
    const auto nonterminal = []
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
    // Any other bare name is a feature reference (resolved by the semantic parser). The prefixes of constructor keywords
    // and nonterminals (c_, r_, b_, n_, q_) are reserved, so that a constructor without parentheses or a nonterminal of
    // another category still fails here and backtracks.
    const auto name =
        lexeme[!((lit('c') | lit('r') | lit('b') | lit('n') | lit('q')) >> lit('_')) >> (alpha | char_('_')) >> *(alnum | char_('_') | char_('-'))];
    const auto parser = nonterminal | name;
    return boost::spirit::x3::rule<NonTerminalNameClass<Category>, runir::kr::dl::grammar::ast::Identifier> { "non_terminal_name" } =
               (boost::spirit::x3::rule<NonTerminalNameTextClass<Category>, std::string> { "non_terminal_name_text" } = raw[parser]);
}

template<typename Parser>
    requires boost::spirit::x3::traits::is_parser<Parser>::value
constexpr auto constructor_alternatives(const Parser& parser)
{ return parser; }

/// Appends per constructor rule, in order, the constructor where the AST and the dialect admit it, then the expectation error at
/// the keyword of a constructor the dialect excludes.
template<typename Parser, runir::kr::dl::CategoryTag Category, typename Tag, typename Dialect, typename... Rules>
    requires boost::spirit::x3::traits::is_parser<Parser>::value
constexpr auto constructor_alternatives(const Parser& parser, const constructor_tag_type<Category, Tag, Dialect>& rule, const Rules&... rules)
{
    constexpr bool has_ast = has_ast_constructor_tag_v<Category, Tag>;
    constexpr bool admitted = has_ast && dialect_allows_v<Dialect, Tag>;
    return constructor_alternatives(parser | maybe<admitted>(constructor_context<Tag>(rule.name)[rule]) | maybe<has_ast>(forbidden<Dialect, Tag>()), rules...);
}

// The constructors of a category are tried in the order of their rules below: longer arity overloads first; their final operand
// remains backtrackable.
#define RUNIR_DEFINE_PARSER(prefix, Dialect, search_free)                                                                                                      \
    constexpr query_atomic_state_type<Dialect> prefix##_query_atomic_state = "query_atomic_state";                                                             \
    constexpr query_atomic_goal_type<Dialect> prefix##_query_atomic_goal = "query_atomic_goal";                                                                \
    constexpr query_concept_type<Dialect> prefix##_query_concept = "query_concept";                                                                            \
    constexpr query_role_type<Dialect> prefix##_query_role = "query_role";                                                                                     \
    constexpr query_join_type<Dialect> prefix##_query_join = "query_join";                                                                                     \
    constexpr query_project_type<Dialect> prefix##_query_project = "query_project";                                                                            \
    constexpr query_rename_type<Dialect> prefix##_query_rename = "query_rename";                                                                               \
    constexpr query_select_equal_type<Dialect> prefix##_query_select_equal = "query_select_equal";                                                             \
    constexpr query_select_value_type<Dialect> prefix##_query_select_value = "query_select_value";                                                             \
    constexpr query_union_type<Dialect> prefix##_query_union = "query_union";                                                                                  \
    constexpr query_difference_type<Dialect> prefix##_query_difference = "query_difference";                                                                   \
    constexpr query_generic_join_type<Dialect> prefix##_query_generic_join = "query_generic_join";                                                             \
    constexpr query_empty_type<Dialect> prefix##_query_empty = "query_empty";                                                                                  \
    constexpr query_must_pass_type<Dialect> prefix##_query_must_pass = "query_must_pass";                                                                      \
    constexpr query_head_type<Dialect> prefix##_query_head = "query_head";                                                                                     \
    constexpr constructor_root_type<runir::kr::dl::ConceptTag, Dialect> prefix##_concept_root = "concept_root";                                                \
    constexpr constructor_root_type<runir::kr::dl::RoleTag, Dialect> prefix##_role_root = "role_root";                                                         \
    constexpr constructor_root_type<runir::kr::dl::BooleanTag, Dialect> prefix##_boolean_root = "boolean_root";                                                \
    constexpr constructor_root_type<runir::kr::dl::NumericalTag, Dialect> prefix##_numerical_root = "numerical_root";                                          \
    constexpr non_terminal_type<runir::kr::dl::ConceptTag, Dialect> prefix##_concept_non_terminal = "concept_non_terminal";                                    \
    constexpr choice_type<runir::kr::dl::ConceptTag, Dialect> prefix##_concept_choice = "concept_choice";                                                      \
    constexpr non_terminal_type<runir::kr::dl::RoleTag, Dialect> prefix##_role_non_terminal = "role_non_terminal";                                             \
    constexpr choice_type<runir::kr::dl::RoleTag, Dialect> prefix##_role_choice = "role_choice";                                                               \
    constexpr non_terminal_type<runir::kr::dl::BooleanTag, Dialect> prefix##_boolean_non_terminal = "boolean_non_terminal";                                    \
    constexpr choice_type<runir::kr::dl::BooleanTag, Dialect> prefix##_boolean_choice = "boolean_choice";                                                      \
    constexpr non_terminal_type<runir::kr::dl::NumericalTag, Dialect> prefix##_numerical_non_terminal = "numerical_non_terminal";                              \
    constexpr choice_type<runir::kr::dl::NumericalTag, Dialect> prefix##_numerical_choice = "numerical_choice";                                                \
    constexpr constructor_or_non_terminal_variant_type<Dialect> prefix##_constructor_or_non_terminal_variant = "constructor_or_non_terminal_variant";          \
    constexpr derivation_rule_type<runir::kr::dl::ConceptTag, Dialect> prefix##_concept_derivation_rule = "concept_derivation_rule";                           \
    constexpr derivation_rule_type<runir::kr::dl::RoleTag, Dialect> prefix##_role_derivation_rule = "role_derivation_rule";                                    \
    constexpr derivation_rule_type<runir::kr::dl::BooleanTag, Dialect> prefix##_boolean_derivation_rule = "boolean_derivation_rule";                           \
    constexpr derivation_rule_type<runir::kr::dl::NumericalTag, Dialect> prefix##_numerical_derivation_rule = "numerical_derivation_rule";                     \
    constexpr derivation_rule_variant_type<Dialect> prefix##_derivation_rule_variant = "derivation_rule_variant";                                              \
    constexpr grammar_body_type<Dialect> prefix##_grammar_body = "grammar_body";                                                                               \
    constexpr grammar_type<Dialect> prefix##_grammar = "grammar";                                                                                              \
    constexpr grammar_root_type<Dialect> prefix##_grammar_root = "grammar_root";                                                                               \
    constexpr constructor_tag_type<runir::kr::dl::ConceptTag, runir::kr::dl::ProjectTag, Dialect> prefix##_concept_project = "concept_project";                \
    constexpr constructor_tag_type<runir::kr::dl::ConceptTag, runir::kr::dl::BotTag, Dialect> prefix##_concept_bot = "concept_bot";                            \
    constexpr constructor_tag_type<runir::kr::dl::ConceptTag, runir::kr::dl::TopTag, Dialect> prefix##_concept_top = "concept_top";                            \
    constexpr constructor_tag_type<runir::kr::dl::ConceptTag, runir::kr::dl::ConceptAtomicStateSyntaxTag, Dialect> prefix##_concept_atomic_state =             \
        "concept_atomic_state";                                                                                                                                \
    constexpr constructor_tag_type<runir::kr::dl::ConceptTag, runir::kr::dl::ConceptAtomicGoalSyntaxTag, Dialect> prefix##_concept_atomic_goal =               \
        "concept_atomic_goal";                                                                                                                                 \
    constexpr constructor_tag_type<runir::kr::dl::ConceptTag, runir::kr::dl::ConceptIntersectionSyntaxTag, Dialect> prefix##_concept_intersection =            \
        "concept_intersection";                                                                                                                                \
    constexpr constructor_tag_type<runir::kr::dl::ConceptTag, runir::kr::dl::ConceptUnionSyntaxTag, Dialect> prefix##_concept_union = "concept_union";         \
    constexpr constructor_tag_type<runir::kr::dl::ConceptTag, runir::kr::dl::NegationTag, Dialect> prefix##_concept_negation = "concept_negation";             \
    constexpr constructor_tag_type<runir::kr::dl::ConceptTag, runir::kr::dl::ValueRestrictionTag, Dialect> prefix##_concept_value_restriction =                \
        "concept_value_restriction";                                                                                                                           \
    constexpr constructor_tag_type<runir::kr::dl::ConceptTag, runir::kr::dl::ExistentialQuantificationTag, Dialect>                                            \
        prefix##_concept_existential_quantification = "concept_existential_quantification";                                                                    \
    constexpr constructor_tag_type<runir::kr::dl::ConceptTag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag, Dialect>                                    \
        prefix##_concept_qualified_at_least_number_restriction = "concept_qualified_at_least_number_restriction";                                              \
    constexpr constructor_tag_type<runir::kr::dl::ConceptTag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag, Dialect>                                     \
        prefix##_concept_qualified_at_most_number_restriction = "concept_qualified_at_most_number_restriction";                                                \
    constexpr constructor_tag_type<runir::kr::dl::ConceptTag, runir::kr::dl::QualifiedExactNumberRestrictionTag, Dialect>                                      \
        prefix##_concept_qualified_exact_number_restriction = "concept_qualified_exact_number_restriction";                                                    \
    constexpr constructor_tag_type<runir::kr::dl::ConceptTag, runir::kr::dl::AtLeastNumberRestrictionTag, Dialect>                                             \
        prefix##_concept_at_least_number_restriction = "concept_at_least_number_restriction";                                                                  \
    constexpr constructor_tag_type<runir::kr::dl::ConceptTag, runir::kr::dl::AtMostNumberRestrictionTag, Dialect>                                              \
        prefix##_concept_at_most_number_restriction = "concept_at_most_number_restriction";                                                                    \
    constexpr constructor_tag_type<runir::kr::dl::ConceptTag, runir::kr::dl::ExactNumberRestrictionTag, Dialect> prefix##_concept_exact_number_restriction =   \
        "concept_exact_number_restriction";                                                                                                                    \
    constexpr constructor_tag_type<runir::kr::dl::ConceptTag, runir::kr::dl::RoleValueMapTag, Dialect> prefix##_concept_role_value_map =                       \
        "concept_role_value_map";                                                                                                                              \
    constexpr constructor_tag_type<runir::kr::dl::ConceptTag, runir::kr::dl::AgreementTag, Dialect> prefix##_concept_agreement = "concept_agreement";          \
    constexpr constructor_tag_type<runir::kr::dl::ConceptTag, runir::kr::dl::RoleFillersTag, Dialect> prefix##_concept_role_fillers = "concept_role_fillers";  \
    constexpr constructor_tag_type<runir::kr::dl::ConceptTag, runir::kr::dl::OneOfTag, Dialect> prefix##_concept_one_of = "concept_one_of";                    \
    constexpr constructor_tag_type<runir::kr::dl::ConceptTag, runir::kr::dl::NominalTag, Dialect> prefix##_concept_nominal = "concept_nominal";                \
    constexpr constructor_tag_type<runir::kr::dl::ConceptTag, runir::kr::dl::RegisterTag, Dialect> prefix##_concept_register = "concept_register";             \
    constexpr constructor_tag_type<runir::kr::dl::ConceptTag, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>, Dialect> prefix##_concept_argument =      \
        "concept_argument";                                                                                                                                    \
    constexpr constructor_tag_type<runir::kr::dl::RoleTag, runir::kr::dl::ProjectTag, Dialect> prefix##_role_project = "role_project";                         \
    constexpr constructor_tag_type<runir::kr::dl::RoleTag, runir::kr::dl::UniversalTag, Dialect> prefix##_role_universal = "role_universal";                   \
    constexpr constructor_tag_type<runir::kr::dl::RoleTag, runir::kr::dl::RoleAtomicStateSyntaxTag, Dialect> prefix##_role_atomic_state = "role_atomic_state"; \
    constexpr constructor_tag_type<runir::kr::dl::RoleTag, runir::kr::dl::RoleAtomicGoalSyntaxTag, Dialect> prefix##_role_atomic_goal = "role_atomic_goal";    \
    constexpr constructor_tag_type<runir::kr::dl::RoleTag, runir::kr::dl::RoleIntersectionSyntaxTag, Dialect> prefix##_role_intersection =                     \
        "role_intersection";                                                                                                                                   \
    constexpr constructor_tag_type<runir::kr::dl::RoleTag, runir::kr::dl::RoleUnionSyntaxTag, Dialect> prefix##_role_union = "role_union";                     \
    constexpr constructor_tag_type<runir::kr::dl::RoleTag, runir::kr::dl::ComplementTag, Dialect> prefix##_role_complement = "role_complement";                \
    constexpr constructor_tag_type<runir::kr::dl::RoleTag, runir::kr::dl::InverseTag, Dialect> prefix##_role_inverse = "role_inverse";                         \
    constexpr constructor_tag_type<runir::kr::dl::RoleTag, runir::kr::dl::CompositionTag, Dialect> prefix##_role_composition = "role_composition";             \
    constexpr constructor_tag_type<runir::kr::dl::RoleTag, runir::kr::dl::TransitiveClosureTag, Dialect> prefix##_role_transitive_closure =                    \
        "role_transitive_closure";                                                                                                                             \
    constexpr constructor_tag_type<runir::kr::dl::RoleTag, runir::kr::dl::ReflexiveTransitiveClosureTag, Dialect> prefix##_role_reflexive_transitive_closure = \
        "role_reflexive_transitive_closure";                                                                                                                   \
    constexpr constructor_tag_type<runir::kr::dl::RoleTag, runir::kr::dl::RestrictionTag, Dialect> prefix##_role_restriction = "role_restriction";             \
    constexpr constructor_tag_type<runir::kr::dl::RoleTag, runir::kr::dl::IdentityTag, Dialect> prefix##_role_identity = "role_identity";                      \
    constexpr constructor_tag_type<runir::kr::dl::RoleTag, runir::kr::dl::RegisterTag, Dialect> prefix##_role_register = "role_register";                      \
    constexpr constructor_tag_type<runir::kr::dl::RoleTag, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>, Dialect> prefix##_role_argument =               \
        "role_argument";                                                                                                                                       \
    constexpr constructor_tag_type<runir::kr::dl::BooleanTag, runir::kr::dl::BooleanAtomicStateSyntaxTag, Dialect> prefix##_boolean_atomic_state =             \
        "boolean_atomic_state";                                                                                                                                \
    constexpr constructor_tag_type<runir::kr::dl::BooleanTag, runir::kr::dl::BooleanAtomicGoalSyntaxTag, Dialect> prefix##_boolean_atomic_goal =               \
        "boolean_atomic_goal";                                                                                                                                 \
    constexpr constructor_tag_type<runir::kr::dl::BooleanTag, runir::kr::dl::NonemptyTag, Dialect> prefix##_boolean_nonempty = "boolean_nonempty";             \
    constexpr constructor_tag_type<runir::kr::dl::BooleanTag, runir::kr::dl::BooleanEqTag, Dialect> prefix##_boolean_eq = "boolean_eq";                        \
    constexpr constructor_tag_type<runir::kr::dl::BooleanTag, runir::kr::dl::BooleanNeTag, Dialect> prefix##_boolean_neq = "boolean_neq";                      \
    constexpr constructor_tag_type<runir::kr::dl::BooleanTag, runir::kr::dl::BooleanLtTag, Dialect> prefix##_boolean_lt = "boolean_lt";                        \
    constexpr constructor_tag_type<runir::kr::dl::BooleanTag, runir::kr::dl::BooleanLeTag, Dialect> prefix##_boolean_le = "boolean_le";                        \
    constexpr constructor_tag_type<runir::kr::dl::BooleanTag, runir::kr::dl::BooleanGtTag, Dialect> prefix##_boolean_gt = "boolean_gt";                        \
    constexpr constructor_tag_type<runir::kr::dl::BooleanTag, runir::kr::dl::BooleanGeTag, Dialect> prefix##_boolean_ge = "boolean_ge";                        \
    constexpr constructor_tag_type<runir::kr::dl::BooleanTag, runir::kr::dl::NumericalEqTag, Dialect> prefix##_numerical_eq = "numerical_eq";                  \
    constexpr constructor_tag_type<runir::kr::dl::BooleanTag, runir::kr::dl::NumericalNeTag, Dialect> prefix##_numerical_neq = "numerical_neq";                \
    constexpr constructor_tag_type<runir::kr::dl::BooleanTag, runir::kr::dl::NumericalLtTag, Dialect> prefix##_numerical_lt = "numerical_lt";                  \
    constexpr constructor_tag_type<runir::kr::dl::BooleanTag, runir::kr::dl::NumericalLeTag, Dialect> prefix##_numerical_le = "numerical_le";                  \
    constexpr constructor_tag_type<runir::kr::dl::BooleanTag, runir::kr::dl::NumericalGtTag, Dialect> prefix##_numerical_gt = "numerical_gt";                  \
    constexpr constructor_tag_type<runir::kr::dl::BooleanTag, runir::kr::dl::NumericalGeTag, Dialect> prefix##_numerical_ge = "numerical_ge";                  \
    constexpr constructor_tag_type<runir::kr::dl::BooleanTag, runir::kr::dl::BooleanConstantTag, Dialect> prefix##_boolean_constant = "boolean_constant";      \
    constexpr constructor_tag_type<runir::kr::dl::BooleanTag, runir::kr::dl::AndTag, Dialect> prefix##_boolean_and = "boolean_and";                            \
    constexpr constructor_tag_type<runir::kr::dl::BooleanTag, runir::kr::dl::OrTag, Dialect> prefix##_boolean_or = "boolean_or";                               \
    constexpr constructor_tag_type<runir::kr::dl::BooleanTag, runir::kr::dl::NotTag, Dialect> prefix##_boolean_not = "boolean_not";                            \
    constexpr constructor_tag_type<runir::kr::dl::BooleanTag, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>, Dialect> prefix##_boolean_argument =      \
        "boolean_argument";                                                                                                                                    \
    constexpr constructor_tag_type<runir::kr::dl::NumericalTag, runir::kr::dl::CountTag, Dialect> prefix##_numerical_count = "numerical_count";                \
    constexpr constructor_tag_type<runir::kr::dl::NumericalTag, runir::kr::dl::DistanceTag, Dialect> prefix##_numerical_distance = "numerical_distance";       \
    constexpr constructor_tag_type<runir::kr::dl::NumericalTag, runir::kr::dl::NumericalConstantTag, Dialect> prefix##_numerical_constant =                    \
        "numerical_constant";                                                                                                                                  \
    constexpr constructor_tag_type<runir::kr::dl::NumericalTag, runir::kr::dl::AddTag, Dialect> prefix##_numerical_add = "numerical_add";                      \
    constexpr constructor_tag_type<runir::kr::dl::NumericalTag, runir::kr::dl::SubTag, Dialect> prefix##_numerical_sub = "numerical_sub";                      \
    constexpr constructor_tag_type<runir::kr::dl::NumericalTag, runir::kr::dl::MulTag, Dialect> prefix##_numerical_mul = "numerical_mul";                      \
    constexpr constructor_tag_type<runir::kr::dl::NumericalTag, runir::kr::dl::DivTag, Dialect> prefix##_numerical_div = "numerical_div";                      \
    constexpr constructor_tag_type<runir::kr::dl::NumericalTag, runir::kr::dl::MinTag, Dialect> prefix##_numerical_min = "numerical_min";                      \
    constexpr constructor_tag_type<runir::kr::dl::NumericalTag, runir::kr::dl::MaxTag, Dialect> prefix##_numerical_max = "numerical_max";                      \
    constexpr constructor_tag_type<runir::kr::dl::NumericalTag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>, Dialect>                              \
        prefix##_numerical_argument = "numerical_argument";                                                                                                    \
    constexpr auto prefix##_concept_def = constructor_alternatives(boost::spirit::x3::eps(false),                                                              \
                                                                   prefix##_concept_project,                                                                   \
                                                                   prefix##_concept_bot,                                                                       \
                                                                   prefix##_concept_top,                                                                       \
                                                                   prefix##_concept_atomic_state,                                                              \
                                                                   prefix##_concept_atomic_goal,                                                               \
                                                                   prefix##_concept_intersection,                                                              \
                                                                   prefix##_concept_union,                                                                     \
                                                                   prefix##_concept_negation,                                                                  \
                                                                   prefix##_concept_value_restriction,                                                         \
                                                                   prefix##_concept_existential_quantification,                                                \
                                                                   prefix##_concept_qualified_at_least_number_restriction,                                     \
                                                                   prefix##_concept_qualified_at_most_number_restriction,                                      \
                                                                   prefix##_concept_qualified_exact_number_restriction,                                        \
                                                                   prefix##_concept_at_least_number_restriction,                                               \
                                                                   prefix##_concept_at_most_number_restriction,                                                \
                                                                   prefix##_concept_exact_number_restriction,                                                  \
                                                                   prefix##_concept_role_value_map,                                                            \
                                                                   prefix##_concept_agreement,                                                                 \
                                                                   prefix##_concept_role_fillers,                                                              \
                                                                   prefix##_concept_one_of,                                                                    \
                                                                   prefix##_concept_nominal,                                                                   \
                                                                   prefix##_concept_register,                                                                  \
                                                                   prefix##_concept_argument);                                                                 \
    constexpr auto prefix##_concept_root_def = context("concept expression")[prefix##_concept > eoi];                                                          \
    constexpr auto prefix##_concept_non_terminal_def = non_terminal_string_parser<runir::kr::dl::ConceptTag>();                                                \
    constexpr auto prefix##_concept_choice_def = prefix##_concept_non_terminal | prefix##_concept;                                                             \
    constexpr auto prefix##_role_def = constructor_alternatives(boost::spirit::x3::eps(false),                                                                 \
                                                                prefix##_role_project,                                                                         \
                                                                prefix##_role_universal,                                                                       \
                                                                prefix##_role_atomic_state,                                                                    \
                                                                prefix##_role_atomic_goal,                                                                     \
                                                                prefix##_role_intersection,                                                                    \
                                                                prefix##_role_union,                                                                           \
                                                                prefix##_role_complement,                                                                      \
                                                                prefix##_role_inverse,                                                                         \
                                                                prefix##_role_composition,                                                                     \
                                                                prefix##_role_transitive_closure,                                                              \
                                                                prefix##_role_reflexive_transitive_closure,                                                    \
                                                                prefix##_role_restriction,                                                                     \
                                                                prefix##_role_identity,                                                                        \
                                                                prefix##_role_register,                                                                        \
                                                                prefix##_role_argument);                                                                       \
    constexpr auto prefix##_role_root_def = context("role expression")[prefix##_role > eoi];                                                                   \
    constexpr auto prefix##_role_non_terminal_def = non_terminal_string_parser<runir::kr::dl::RoleTag>();                                                      \
    constexpr auto prefix##_role_choice_def = prefix##_role_non_terminal | prefix##_role;                                                                      \
    constexpr auto prefix##_constructor_or_non_terminal_variant_def = prefix##_concept_choice | prefix##_role_choice | prefix##_query;                         \
    constexpr auto prefix##_boolean_def = constructor_alternatives(boost::spirit::x3::eps(false),                                                              \
                                                                   prefix##_boolean_atomic_state,                                                              \
                                                                   prefix##_boolean_atomic_goal,                                                               \
                                                                   prefix##_boolean_nonempty,                                                                  \
                                                                   prefix##_boolean_eq,                                                                        \
                                                                   prefix##_boolean_neq,                                                                       \
                                                                   prefix##_boolean_lt,                                                                        \
                                                                   prefix##_boolean_le,                                                                        \
                                                                   prefix##_boolean_gt,                                                                        \
                                                                   prefix##_boolean_ge,                                                                        \
                                                                   prefix##_numerical_eq,                                                                      \
                                                                   prefix##_numerical_neq,                                                                     \
                                                                   prefix##_numerical_lt,                                                                      \
                                                                   prefix##_numerical_le,                                                                      \
                                                                   prefix##_numerical_gt,                                                                      \
                                                                   prefix##_numerical_ge,                                                                      \
                                                                   prefix##_boolean_constant,                                                                  \
                                                                   prefix##_boolean_and,                                                                       \
                                                                   prefix##_boolean_or,                                                                        \
                                                                   prefix##_boolean_not,                                                                       \
                                                                   prefix##_boolean_argument);                                                                 \
    constexpr auto prefix##_boolean_root_def = context("boolean expression")[prefix##_boolean > eoi];                                                          \
    constexpr auto prefix##_boolean_non_terminal_def = non_terminal_string_parser<runir::kr::dl::BooleanTag>();                                                \
    constexpr auto prefix##_boolean_choice_def = prefix##_boolean_non_terminal | prefix##_boolean;                                                             \
    constexpr auto prefix##_numerical_def = constructor_alternatives(boost::spirit::x3::eps(false),                                                            \
                                                                     prefix##_numerical_count,                                                                 \
                                                                     prefix##_numerical_distance,                                                              \
                                                                     prefix##_numerical_constant,                                                              \
                                                                     prefix##_numerical_add,                                                                   \
                                                                     prefix##_numerical_sub,                                                                   \
                                                                     prefix##_numerical_mul,                                                                   \
                                                                     prefix##_numerical_div,                                                                   \
                                                                     prefix##_numerical_min,                                                                   \
                                                                     prefix##_numerical_max,                                                                   \
                                                                     prefix##_numerical_argument);                                                             \
    constexpr auto prefix##_numerical_root_def = context("numerical expression")[prefix##_numerical > eoi];                                                    \
    constexpr auto prefix##_numerical_non_terminal_def = non_terminal_string_parser<runir::kr::dl::NumericalTag>();                                            \
    constexpr auto prefix##_numerical_choice_def = prefix##_numerical_non_terminal | prefix##_numerical;                                                       \
    constexpr auto prefix##_query_atomic_state_def =                                                                                                           \
        with_constructor_parentheses(keyword("q_atomic_state") > predicate_name_string_parser() > query_columns_parser());                                     \
    constexpr auto prefix##_query_atomic_goal_def =                                                                                                            \
        with_constructor_parentheses(keyword("q_atomic_goal") > predicate_name_string_parser() > bool_string_parser() > query_columns_parser());               \
    constexpr auto prefix##_query_concept_def =                                                                                                                \
        with_constructor_parentheses(keyword(runir::kr::dl::QueryConceptTag::keyword) > boost::spirit::x3::repeat(1)[identifier] > prefix##_concept_choice);   \
    constexpr auto prefix##_query_role_def =                                                                                                                   \
        with_constructor_parentheses(keyword(runir::kr::dl::QueryRoleTag::keyword) > query_columns_parser() > prefix##_role_choice);                           \
    constexpr auto prefix##_query_join_def = with_constructor_parentheses(keyword(runir::kr::dl::QueryJoinTag::keyword) > prefix##_query > prefix##_query);    \
    constexpr auto prefix##_query_project_def =                                                                                                                \
        with_constructor_parentheses(keyword(runir::kr::dl::QueryProjectTag::keyword) > query_columns_parser() > prefix##_query);                              \
    constexpr auto prefix##_query_rename_def =                                                                                                                 \
        with_constructor_parentheses(keyword(runir::kr::dl::QueryRenameTag::keyword) > query_columns_parser() > prefix##_query);                               \
    constexpr auto prefix##_query_select_equal_def =                                                                                                           \
        with_constructor_parentheses(keyword(runir::kr::dl::QuerySelectEqualTag::keyword) > identifier > identifier > prefix##_query);                         \
    constexpr auto prefix##_query_select_value_def =                                                                                                           \
        with_constructor_parentheses(keyword(runir::kr::dl::QuerySelectValueTag::keyword) > identifier > object_name_string_parser() > prefix##_query);        \
    constexpr auto prefix##_query_union_def = with_constructor_parentheses(keyword(runir::kr::dl::QueryUnionTag::keyword) > prefix##_query > prefix##_query);  \
    constexpr auto prefix##_query_difference_def =                                                                                                             \
        with_constructor_parentheses(keyword(runir::kr::dl::QueryDifferenceTag::keyword) > prefix##_query > prefix##_query);                                   \
    constexpr auto prefix##_query_generic_join_def =                                                                                                           \
        with_constructor_parentheses(keyword(runir::kr::dl::QueryGenericJoinTag::keyword) > query_columns_parser() > query_columns_parser()                    \
                                     > boost::spirit::x3::repeat(2, boost::spirit::x3::inf)[prefix##_query]);                                                  \
    constexpr auto prefix##_query_empty_def = with_constructor_parentheses(keyword(runir::kr::dl::QueryEmptyTag::keyword) > query_columns_parser());           \
    constexpr auto prefix##_query_must_pass_def = with_constructor_parentheses(keyword(runir::kr::dl::QueryMustPassTag::keyword) > query_columns_parser()      \
                                                                               > prefix##_query > search_free##_query > prefix##_query > prefix##_query);      \
    constexpr auto prefix##_query_head_def =                                                                                                                   \
        with_constructor_parentheses(keyword(runir::kr::dl::QueryHeadTag::keyword) > identifier > query_columns_parser());                                     \
    constexpr auto prefix##_concept_project_def =                                                                                                              \
        with_constructor_parentheses(keyword(runir::kr::dl::ConceptProjectSyntaxTag::keyword) > boost::spirit::x3::repeat(1)[identifier] > prefix##_query);    \
    constexpr auto prefix##_role_project_def =                                                                                                                 \
        with_constructor_parentheses(keyword(runir::kr::dl::RoleProjectSyntaxTag::keyword) > boost::spirit::x3::repeat(2)[identifier] > prefix##_query);       \
    constexpr auto prefix##_query_def =                                                                                                                        \
        prefix##_query_atomic_state | prefix##_query_atomic_goal | prefix##_query_concept | prefix##_query_role | prefix##_query_join | prefix##_query_project \
        | prefix##_query_rename | prefix##_query_select_equal | prefix##_query_select_value | prefix##_query_union | prefix##_query_difference                 \
        | prefix##_query_generic_join | prefix##_query_empty | prefix##_query_head                                                                             \
        | maybe<dialect_allows_v<Dialect, runir::kr::dl::QueryMustPassTag>>(prefix##_query_must_pass) | forbidden<Dialect, runir::kr::dl::QueryMustPassTag>(); \
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
                        prefix##_query_head,                                                                                                                   \
                        prefix##_query,                                                                                                                        \
                        prefix##_concept_project,                                                                                                              \
                        prefix##_role_project)                                                                                                                 \
    constexpr auto prefix##_concept_bot_def =                                                                                                                  \
        with_constructor_parentheses(keyword(runir::kr::dl::BotTag::keyword) >> boost::spirit::x3::attr(runir::kr::dl::grammar::ast::ConceptBot {}));          \
    constexpr auto prefix##_concept_top_def =                                                                                                                  \
        with_constructor_parentheses(keyword(runir::kr::dl::TopTag::keyword) >> boost::spirit::x3::attr(runir::kr::dl::grammar::ast::ConceptTop {}));          \
    constexpr auto prefix##_concept_atomic_state_def =                                                                                                         \
        with_constructor_parentheses(keyword(runir::kr::dl::ConceptAtomicStateSyntaxTag::keyword) > predicate_name_string_parser());                           \
    constexpr auto prefix##_concept_atomic_goal_def =                                                                                                          \
        with_constructor_parentheses(keyword(runir::kr::dl::ConceptAtomicGoalSyntaxTag::keyword) > predicate_name_string_parser() > bool_string_parser());     \
    constexpr auto prefix##_concept_intersection_def =                                                                                                         \
        with_constructor_parentheses(keyword(runir::kr::dl::ConceptIntersectionSyntaxTag::keyword) > prefix##_concept_choice > prefix##_concept_choice);       \
    constexpr auto prefix##_concept_union_def =                                                                                                                \
        with_constructor_parentheses(keyword(runir::kr::dl::ConceptUnionSyntaxTag::keyword) > prefix##_concept_choice > prefix##_concept_choice);              \
    constexpr auto prefix##_concept_negation_def = with_constructor_parentheses(keyword(runir::kr::dl::NegationTag::keyword) > prefix##_concept_choice);       \
    constexpr auto prefix##_concept_value_restriction_def =                                                                                                    \
        with_constructor_parentheses(keyword(runir::kr::dl::ValueRestrictionTag::keyword) > prefix##_role_choice > prefix##_concept_choice);                   \
    constexpr auto prefix##_concept_existential_quantification_def =                                                                                           \
        with_constructor_parentheses(keyword(runir::kr::dl::ExistentialQuantificationTag::keyword) > prefix##_role_choice > prefix##_concept_choice);          \
    constexpr auto prefix##_concept_at_least_number_restriction_def =                                                                                          \
        with_constructor_parentheses(keyword(runir::kr::dl::AtLeastNumberRestrictionTag::keyword) > unsigned_integer > prefix##_role_choice);                  \
    constexpr auto prefix##_concept_at_most_number_restriction_def =                                                                                           \
        with_constructor_parentheses(keyword(runir::kr::dl::AtMostNumberRestrictionTag::keyword) > unsigned_integer > prefix##_role_choice);                   \
    constexpr auto prefix##_concept_exact_number_restriction_def =                                                                                             \
        with_constructor_parentheses(keyword(runir::kr::dl::ExactNumberRestrictionTag::keyword) > unsigned_integer > prefix##_role_choice);                    \
    constexpr auto prefix##_concept_qualified_at_least_number_restriction_def = with_constructor_parentheses(                                                  \
        (keyword(runir::kr::dl::QualifiedAtLeastNumberRestrictionTag::keyword) > unsigned_integer > prefix##_role_choice) >> prefix##_concept_choice);         \
    constexpr auto prefix##_concept_qualified_at_most_number_restriction_def = with_constructor_parentheses(                                                   \
        (keyword(runir::kr::dl::QualifiedAtMostNumberRestrictionTag::keyword) > unsigned_integer > prefix##_role_choice) >> prefix##_concept_choice);          \
    constexpr auto prefix##_concept_qualified_exact_number_restriction_def = with_constructor_parentheses(                                                     \
        (keyword(runir::kr::dl::QualifiedExactNumberRestrictionTag::keyword) > unsigned_integer > prefix##_role_choice) >> prefix##_concept_choice);           \
    constexpr auto prefix##_concept_role_value_map_def =                                                                                                       \
        with_constructor_parentheses(keyword(runir::kr::dl::RoleValueMapTag::keyword) > prefix##_role_choice > prefix##_role_choice);                          \
    constexpr auto prefix##_concept_agreement_def =                                                                                                            \
        with_constructor_parentheses(keyword(runir::kr::dl::AgreementTag::keyword) > prefix##_role_choice > prefix##_role_choice);                             \
    constexpr auto prefix##_concept_role_fillers_def =                                                                                                         \
        with_constructor_parentheses(keyword(runir::kr::dl::RoleFillersTag::keyword) > prefix##_role_choice > object_names_parser());                          \
    constexpr auto prefix##_concept_one_of_def = with_constructor_parentheses(keyword(runir::kr::dl::OneOfTag::keyword) > object_names_parser());              \
    constexpr auto prefix##_concept_nominal_def = with_constructor_parentheses(keyword(runir::kr::dl::NominalTag::keyword) > object_name_string_parser());     \
    constexpr auto prefix##_concept_register_def =                                                                                                             \
        with_constructor_parentheses(keyword(runir::kr::dl::ConceptRegisterSyntaxTag::keyword) > register_reference_parser<runir::kr::dl::ConceptTag>());      \
    constexpr auto prefix##_concept_argument_def = with_constructor_parentheses(keyword(runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>::keyword)        \
                                                                                > argument_reference_parser<runir::kr::dl::ConceptTag>());                     \
    constexpr auto prefix##_role_universal_def =                                                                                                               \
        with_constructor_parentheses(keyword(runir::kr::dl::UniversalTag::keyword) >> boost::spirit::x3::attr(runir::kr::dl::grammar::ast::RoleUniversal {})); \
    constexpr auto prefix##_role_atomic_state_def =                                                                                                            \
        with_constructor_parentheses(keyword(runir::kr::dl::RoleAtomicStateSyntaxTag::keyword) > predicate_name_string_parser());                              \
    constexpr auto prefix##_role_atomic_goal_def =                                                                                                             \
        with_constructor_parentheses(keyword(runir::kr::dl::RoleAtomicGoalSyntaxTag::keyword) > predicate_name_string_parser() > bool_string_parser());        \
    constexpr auto prefix##_role_intersection_def =                                                                                                            \
        with_constructor_parentheses(keyword(runir::kr::dl::RoleIntersectionSyntaxTag::keyword) > prefix##_role_choice > prefix##_role_choice);                \
    constexpr auto prefix##_role_union_def =                                                                                                                   \
        with_constructor_parentheses(keyword(runir::kr::dl::RoleUnionSyntaxTag::keyword) > prefix##_role_choice > prefix##_role_choice);                       \
    constexpr auto prefix##_role_complement_def = with_constructor_parentheses(keyword(runir::kr::dl::ComplementTag::keyword) > prefix##_role_choice);         \
    constexpr auto prefix##_role_inverse_def = with_constructor_parentheses(keyword(runir::kr::dl::InverseTag::keyword) > prefix##_role_choice);               \
    constexpr auto prefix##_role_composition_def =                                                                                                             \
        with_constructor_parentheses(keyword(runir::kr::dl::CompositionTag::keyword) > prefix##_role_choice > prefix##_role_choice);                           \
    constexpr auto prefix##_role_transitive_closure_def =                                                                                                      \
        with_constructor_parentheses(keyword(runir::kr::dl::TransitiveClosureTag::keyword) > prefix##_role_choice);                                            \
    constexpr auto prefix##_role_reflexive_transitive_closure_def =                                                                                            \
        with_constructor_parentheses(keyword(runir::kr::dl::ReflexiveTransitiveClosureTag::keyword) > prefix##_role_choice);                                   \
    constexpr auto prefix##_role_restriction_def =                                                                                                             \
        with_constructor_parentheses(keyword(runir::kr::dl::RestrictionTag::keyword) > prefix##_role_choice > prefix##_concept_choice);                        \
    constexpr auto prefix##_role_identity_def = with_constructor_parentheses(keyword(runir::kr::dl::IdentityTag::keyword) > prefix##_concept_choice);          \
    constexpr auto prefix##_role_register_def =                                                                                                                \
        with_constructor_parentheses(keyword(runir::kr::dl::RoleRegisterSyntaxTag::keyword) > register_reference_parser<runir::kr::dl::RoleTag>());            \
    constexpr auto prefix##_role_argument_def = with_constructor_parentheses(keyword(runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>::keyword)              \
                                                                             > argument_reference_parser<runir::kr::dl::RoleTag>());                           \
    constexpr auto prefix##_boolean_atomic_state_def =                                                                                                         \
        with_constructor_parentheses(keyword(runir::kr::dl::BooleanAtomicStateSyntaxTag::keyword) > predicate_name_string_parser() > bool_string_parser());    \
    constexpr auto prefix##_boolean_atomic_goal_def =                                                                                                          \
        with_constructor_parentheses(keyword(runir::kr::dl::BooleanAtomicGoalSyntaxTag::keyword) > predicate_name_string_parser() > bool_string_parser());     \
    constexpr auto prefix##_boolean_nonempty_def =                                                                                                             \
        with_constructor_parentheses(keyword(runir::kr::dl::NonemptyTag::keyword) > prefix##_constructor_or_non_terminal_variant);                             \
    constexpr auto prefix##_boolean_eq_def =                                                                                                                   \
        with_constructor_parentheses(keyword(runir::kr::dl::BooleanEqTag::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                       \
    constexpr auto prefix##_boolean_neq_def =                                                                                                                  \
        with_constructor_parentheses(keyword(runir::kr::dl::BooleanNeTag::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                       \
    constexpr auto prefix##_boolean_lt_def =                                                                                                                   \
        with_constructor_parentheses(keyword(runir::kr::dl::BooleanLtTag::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                       \
    constexpr auto prefix##_boolean_le_def =                                                                                                                   \
        with_constructor_parentheses(keyword(runir::kr::dl::BooleanLeTag::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                       \
    constexpr auto prefix##_boolean_gt_def =                                                                                                                   \
        with_constructor_parentheses(keyword(runir::kr::dl::BooleanGtTag::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                       \
    constexpr auto prefix##_boolean_ge_def =                                                                                                                   \
        with_constructor_parentheses(keyword(runir::kr::dl::BooleanGeTag::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                       \
    constexpr auto prefix##_numerical_eq_def =                                                                                                                 \
        with_constructor_parentheses(keyword(runir::kr::dl::NumericalEqTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                 \
    constexpr auto prefix##_numerical_neq_def =                                                                                                                \
        with_constructor_parentheses(keyword(runir::kr::dl::NumericalNeTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                 \
    constexpr auto prefix##_numerical_lt_def =                                                                                                                 \
        with_constructor_parentheses(keyword(runir::kr::dl::NumericalLtTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                 \
    constexpr auto prefix##_numerical_le_def =                                                                                                                 \
        with_constructor_parentheses(keyword(runir::kr::dl::NumericalLeTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                 \
    constexpr auto prefix##_numerical_gt_def =                                                                                                                 \
        with_constructor_parentheses(keyword(runir::kr::dl::NumericalGtTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                 \
    constexpr auto prefix##_numerical_ge_def =                                                                                                                 \
        with_constructor_parentheses(keyword(runir::kr::dl::NumericalGeTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                 \
    constexpr auto prefix##_boolean_constant_def = with_constructor_parentheses(keyword(runir::kr::dl::BooleanConstantTag::keyword) > bool_string_parser());   \
    constexpr auto prefix##_boolean_and_def =                                                                                                                  \
        with_constructor_parentheses(keyword(runir::kr::dl::AndTag::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                             \
    constexpr auto prefix##_boolean_or_def =                                                                                                                   \
        with_constructor_parentheses(keyword(runir::kr::dl::OrTag::keyword) > prefix##_boolean_choice > prefix##_boolean_choice);                              \
    constexpr auto prefix##_boolean_not_def = with_constructor_parentheses(keyword(runir::kr::dl::NotTag::keyword) > prefix##_boolean_choice);                 \
    constexpr auto prefix##_boolean_argument_def = with_constructor_parentheses(keyword(runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>::keyword)        \
                                                                                > argument_reference_parser<runir::kr::dl::BooleanTag>());                     \
    constexpr auto prefix##_numerical_count_def =                                                                                                              \
        with_constructor_parentheses(keyword(runir::kr::dl::CountTag::keyword) > prefix##_constructor_or_non_terminal_variant);                                \
    constexpr auto prefix##_numerical_distance_def =                                                                                                           \
        with_constructor_parentheses(keyword(runir::kr::dl::DistanceTag::keyword) > (prefix##_concept_choice | prefix##_query)                                 \
                                     > (search_free##_role_choice | search_free##_query) > (prefix##_concept_choice | prefix##_query));                        \
    constexpr auto prefix##_numerical_constant_def = with_constructor_parentheses(keyword(runir::kr::dl::NumericalConstantTag::keyword) > unsigned_integer);   \
    constexpr auto prefix##_numerical_add_def =                                                                                                                \
        with_constructor_parentheses(keyword(runir::kr::dl::AddTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                         \
    constexpr auto prefix##_numerical_sub_def =                                                                                                                \
        with_constructor_parentheses(keyword(runir::kr::dl::SubTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                         \
    constexpr auto prefix##_numerical_mul_def =                                                                                                                \
        with_constructor_parentheses(keyword(runir::kr::dl::MulTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                         \
    constexpr auto prefix##_numerical_div_def =                                                                                                                \
        with_constructor_parentheses(keyword(runir::kr::dl::DivTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                         \
    constexpr auto prefix##_numerical_min_def =                                                                                                                \
        with_constructor_parentheses(keyword(runir::kr::dl::MinTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                         \
    constexpr auto prefix##_numerical_max_def =                                                                                                                \
        with_constructor_parentheses(keyword(runir::kr::dl::MaxTag::keyword) > prefix##_numerical_choice > prefix##_numerical_choice);                         \
    constexpr auto prefix##_numerical_argument_def = with_constructor_parentheses(keyword(runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>::keyword)    \
                                                                                  > argument_reference_parser<runir::kr::dl::NumericalTag>());                 \
    constexpr auto prefix##_concept_derivation_rule_def =                                                                                                      \
        context("concept derivation")[(lit("(") >> prefix##_concept_non_terminal) > derivation_choices_parser(prefix##_concept_choice) > lit(")")];            \
    constexpr auto prefix##_role_derivation_rule_def =                                                                                                         \
        context("role derivation")[(lit("(") >> prefix##_role_non_terminal) > derivation_choices_parser(prefix##_role_choice) > lit(")")];                     \
    constexpr auto prefix##_boolean_derivation_rule_def =                                                                                                      \
        context("boolean derivation")[(lit("(") >> prefix##_boolean_non_terminal) > derivation_choices_parser(prefix##_boolean_choice) > lit(")")];            \
    constexpr auto prefix##_numerical_derivation_rule_def =                                                                                                    \
        context("numerical derivation")[(lit("(") >> prefix##_numerical_non_terminal) > derivation_choices_parser(prefix##_numerical_choice) > lit(")")];      \
    constexpr auto prefix##_derivation_rule_variant_def =                                                                                                      \
        prefix##_concept_derivation_rule | prefix##_role_derivation_rule | prefix##_boolean_derivation_rule | prefix##_numerical_derivation_rule;              \
    constexpr auto prefix##_grammar_body_def = *prefix##_derivation_rule_variant;                                                                              \
    constexpr auto prefix##_grammar_def = context("DL grammar")[lit("(") > prefix##_grammar_body > lit(")")];                                                  \
    constexpr auto prefix##_grammar_root_def = context("DL grammar")[prefix##_grammar > eoi];                                                                  \
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
RUNIR_DEFINE_PARSER(dl_search_free, SearchFreeDialectTag, dl_search_free)
RUNIR_DEFINE_PARSER(dl, FullDialectTag, dl_search_free)

#undef RUNIR_DEFINE_PARSER

}  // namespace runir::kr::dl::grammar::parser

#endif
