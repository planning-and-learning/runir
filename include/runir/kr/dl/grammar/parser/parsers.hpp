#ifndef RUNIR_KR_DL_GRAMMAR_PARSER_PARSERS_HPP_
#define RUNIR_KR_DL_GRAMMAR_PARSER_PARSERS_HPP_

#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/dl/grammar/parser/parser.hpp"
#include "runir/kr/parser/error_handler.hpp"

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>
#include <vector>

namespace runir::kr::dl::grammar::parser
{

using iterator_type = runir::kr::parser::Iterator;
using context_type = runir::kr::parser::Context;
using ErrorHandlerBase = runir::kr::parser::ErrorHandlerBase;

template<typename Ast, typename Dialect = FullDialectTag>
struct QueryClass : boost::spirit::x3::annotate_on_success
{
};

struct PredicateNameClass : ErrorHandlerBase
{
};

struct ObjectNameClass : ErrorHandlerBase
{
};

struct IdentifierClass : boost::spirit::x3::annotate_on_success
{
};

struct FeatureNameClass : boost::spirit::x3::annotate_on_success
{
};

struct NumericReferenceClass : boost::spirit::x3::annotate_on_success
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

struct FeatureNameTextClass
{
};

struct NumericReferenceTextClass
{
};

template<runir::kr::dl::CategoryTag Category>
struct NonTerminalNameTextClass
{
};

template<runir::kr::dl::CategoryTag Category>
struct ArgumentReferenceClass : boost::spirit::x3::annotate_on_success
{
};

template<runir::kr::dl::CategoryTag Category>
struct RegisterReferenceClass : boost::spirit::x3::annotate_on_success
{
};

template<runir::kr::dl::CategoryTag Category>
struct NonTerminalNameClass : ErrorHandlerBase
{
};

template<runir::kr::dl::CategoryTag Category, typename Dialect = FullDialectTag>
struct ConstructorClass : boost::spirit::x3::annotate_on_success
{
};

template<runir::kr::dl::CategoryTag Category, typename Dialect = FullDialectTag>
struct ConstructorRootClass : ErrorHandlerBase
{
};

template<runir::kr::dl::CategoryTag Category, typename Dialect = FullDialectTag>
struct NonTerminalClass : boost::spirit::x3::annotate_on_success
{
};

template<runir::kr::dl::CategoryTag Category, typename Dialect = FullDialectTag>
struct ConstructorOrNonTerminalClass : boost::spirit::x3::annotate_on_success
{
};

template<runir::kr::dl::CategoryTag Category, typename Dialect = FullDialectTag>
struct DerivationRuleClass : boost::spirit::x3::annotate_on_success
{
};

template<runir::kr::dl::CategoryTag Category, typename Tag, typename Dialect = FullDialectTag>
struct ConstructorTagClass : boost::spirit::x3::annotate_on_success
{
};

template<typename Dialect = FullDialectTag>
struct ConstructorOrNonTerminalVariantClass : boost::spirit::x3::annotate_on_success
{
};

template<typename Dialect = FullDialectTag>
struct DerivationRuleVariantClass : boost::spirit::x3::annotate_on_success
{
};

template<typename Dialect = FullDialectTag>
struct GrammarBodyClass : boost::spirit::x3::annotate_on_success
{
};

template<typename Dialect = FullDialectTag>
struct GrammarClass : boost::spirit::x3::annotate_on_success
{
};

template<typename Dialect = FullDialectTag>
struct GrammarRootClass : ErrorHandlerBase
{
};

using predicate_name_type = boost::spirit::x3::rule<PredicateNameClass, ast::Identifier>;
using object_name_type = boost::spirit::x3::rule<ObjectNameClass, ast::Identifier>;
using identifier_type = boost::spirit::x3::rule<IdentifierClass, ast::Identifier>;
using feature_name_type = boost::spirit::x3::rule<FeatureNameClass, ast::FeatureName>;
using numeric_reference_type = boost::spirit::x3::rule<NumericReferenceClass, ast::NumericReference>;

template<runir::kr::dl::CategoryTag Category>
using argument_reference_type = boost::spirit::x3::rule<ArgumentReferenceClass<Category>, ast::ArgumentReference<Category>>;

template<runir::kr::dl::CategoryTag Category>
using register_reference_type = boost::spirit::x3::rule<RegisterReferenceClass<Category>, ast::RegisterReference<Category>>;

template<runir::kr::dl::CategoryTag Category, typename Dialect = FullDialectTag>
using constructor_type = boost::spirit::x3::rule<ConstructorClass<Category, Dialect>, ast::Constructor<Category>>;

template<runir::kr::dl::CategoryTag Category, typename Dialect = FullDialectTag>
using constructor_root_type = boost::spirit::x3::rule<ConstructorRootClass<Category, Dialect>, ast::Constructor<Category>>;

template<runir::kr::dl::CategoryTag Category, typename Dialect = FullDialectTag>
using non_terminal_type = boost::spirit::x3::rule<NonTerminalClass<Category, Dialect>, ast::NonTerminal<Category>>;

template<runir::kr::dl::CategoryTag Category, typename Dialect = FullDialectTag>
using choice_type = boost::spirit::x3::rule<ConstructorOrNonTerminalClass<Category, Dialect>, ast::ConstructorOrNonTerminal<Category>>;

template<runir::kr::dl::CategoryTag Category, typename Dialect = FullDialectTag>
using derivation_rule_type = boost::spirit::x3::rule<DerivationRuleClass<Category, Dialect>, ast::DerivationRule<Category>>;

template<runir::kr::dl::CategoryTag Category, typename Tag, typename Dialect = FullDialectTag>
using constructor_tag_type = boost::spirit::x3::rule<ConstructorTagClass<Category, Tag, Dialect>, ast::Constructor<Category, Tag>>;

template<typename Dialect = FullDialectTag>
using query_atomic_state_type = boost::spirit::x3::rule<QueryClass<ast::QueryAtomicState, Dialect>, ast::QueryAtomicState>;

template<typename Dialect = FullDialectTag>
using query_atomic_goal_type = boost::spirit::x3::rule<QueryClass<ast::QueryAtomicGoal, Dialect>, ast::QueryAtomicGoal>;

template<typename Dialect = FullDialectTag>
using query_concept_type = boost::spirit::x3::rule<QueryClass<ast::QueryConcept, Dialect>, ast::QueryConcept>;

template<typename Dialect = FullDialectTag>
using query_role_type = boost::spirit::x3::rule<QueryClass<ast::QueryRole, Dialect>, ast::QueryRole>;

template<typename Dialect = FullDialectTag>
using query_join_type = boost::spirit::x3::rule<QueryClass<ast::QueryJoin, Dialect>, ast::QueryJoin>;

template<typename Dialect = FullDialectTag>
using query_project_type = boost::spirit::x3::rule<QueryClass<ast::QueryProject, Dialect>, ast::QueryProject>;

template<typename Dialect = FullDialectTag>
using query_rename_type = boost::spirit::x3::rule<QueryClass<ast::QueryRename, Dialect>, ast::QueryRename>;

template<typename Dialect = FullDialectTag>
using query_select_equal_type = boost::spirit::x3::rule<QueryClass<ast::QuerySelectEqual, Dialect>, ast::QuerySelectEqual>;

template<typename Dialect = FullDialectTag>
using query_select_value_type = boost::spirit::x3::rule<QueryClass<ast::QuerySelectValue, Dialect>, ast::QuerySelectValue>;

template<typename Dialect = FullDialectTag>
using query_union_type = boost::spirit::x3::rule<QueryClass<ast::QueryUnion, Dialect>, ast::QueryUnion>;

template<typename Dialect = FullDialectTag>
using query_difference_type = boost::spirit::x3::rule<QueryClass<ast::QueryDifference, Dialect>, ast::QueryDifference>;

template<typename Dialect = FullDialectTag>
using query_generic_join_type = boost::spirit::x3::rule<QueryClass<ast::QueryGenericJoin, Dialect>, ast::QueryGenericJoin>;

template<typename Dialect = FullDialectTag>
using query_empty_type = boost::spirit::x3::rule<QueryClass<ast::QueryEmpty, Dialect>, ast::QueryEmpty>;

template<typename Dialect = FullDialectTag>
using query_must_pass_type = boost::spirit::x3::rule<QueryClass<ast::QueryMustPass, Dialect>, ast::QueryMustPass>;

template<typename Dialect = FullDialectTag>
using query_head_type = boost::spirit::x3::rule<QueryClass<ast::QueryHead, Dialect>, ast::QueryHead>;

template<typename Dialect = FullDialectTag>
using query_type = boost::spirit::x3::rule<QueryClass<ast::Query, Dialect>, ast::Query>;

template<typename Dialect = FullDialectTag>
using constructor_or_non_terminal_variant_type = boost::spirit::x3::rule<ConstructorOrNonTerminalVariantClass<Dialect>, ast::ConstructorOrNonTerminalVariant>;

template<typename Dialect = FullDialectTag>
using derivation_rule_variant_type = boost::spirit::x3::rule<DerivationRuleVariantClass<Dialect>, ast::DerivationRuleVariant>;

template<typename Dialect = FullDialectTag>
using grammar_body_type = boost::spirit::x3::rule<GrammarBodyClass<Dialect>, std::vector<ast::DerivationRuleVariant>>;

template<typename Dialect = FullDialectTag>
using grammar_type = boost::spirit::x3::rule<GrammarClass<Dialect>, ast::Grammar>;

template<typename Dialect = FullDialectTag>
using grammar_root_type = boost::spirit::x3::rule<GrammarRootClass<Dialect>, ast::Grammar>;

template<typename Dialect = FullDialectTag>
using concept_type = constructor_type<runir::kr::dl::ConceptTag, Dialect>;

template<typename Dialect = FullDialectTag>
using concept_root_type = constructor_root_type<runir::kr::dl::ConceptTag, Dialect>;

template<typename Dialect = FullDialectTag>
using role_type = constructor_type<runir::kr::dl::RoleTag, Dialect>;

template<typename Dialect = FullDialectTag>
using role_root_type = constructor_root_type<runir::kr::dl::RoleTag, Dialect>;

template<typename Dialect = FullDialectTag>
using boolean_type = constructor_type<runir::kr::dl::BooleanTag, Dialect>;

template<typename Dialect = FullDialectTag>
using boolean_root_type = constructor_root_type<runir::kr::dl::BooleanTag, Dialect>;

template<typename Dialect = FullDialectTag>
using numerical_type = constructor_type<runir::kr::dl::NumericalTag, Dialect>;

template<typename Dialect = FullDialectTag>
using numerical_root_type = constructor_root_type<runir::kr::dl::NumericalTag, Dialect>;

#define RUNIR_DECLARE_RULE_ALIASES(Prefix, Dialect)                                                   \
    using Prefix##_concept_root_type = constructor_root_type<runir::kr::dl::ConceptTag, Dialect>;     \
    using Prefix##_role_root_type = constructor_root_type<runir::kr::dl::RoleTag, Dialect>;           \
    using Prefix##_boolean_root_type = constructor_root_type<runir::kr::dl::BooleanTag, Dialect>;     \
    using Prefix##_numerical_root_type = constructor_root_type<runir::kr::dl::NumericalTag, Dialect>; \
    using Prefix##_concept_type = constructor_type<runir::kr::dl::ConceptTag, Dialect>;               \
    using Prefix##_role_type = constructor_type<runir::kr::dl::RoleTag, Dialect>;                     \
    using Prefix##_boolean_type = constructor_type<runir::kr::dl::BooleanTag, Dialect>;               \
    using Prefix##_numerical_type = constructor_type<runir::kr::dl::NumericalTag, Dialect>;           \
    using Prefix##_concept_choice_type = choice_type<runir::kr::dl::ConceptTag, Dialect>;             \
    using Prefix##_role_choice_type = choice_type<runir::kr::dl::RoleTag, Dialect>;                   \
    using Prefix##_boolean_choice_type = choice_type<runir::kr::dl::BooleanTag, Dialect>;             \
    using Prefix##_numerical_choice_type = choice_type<runir::kr::dl::NumericalTag, Dialect>;         \
    using Prefix##_query_type = query_type<Dialect>;                                                  \
    using Prefix##_grammar_root_type = grammar_root_type<Dialect>;

RUNIR_DECLARE_RULE_ALIASES(dl, FullDialectTag)
RUNIR_DECLARE_RULE_ALIASES(dl_search_free, SearchFreeDialectTag)

#undef RUNIR_DECLARE_RULE_ALIASES

BOOST_SPIRIT_DECLARE(dl_concept_root_type,
                     dl_role_root_type,
                     dl_boolean_root_type,
                     dl_numerical_root_type,
                     dl_concept_type,
                     dl_role_type,
                     dl_boolean_type,
                     dl_numerical_type,
                     dl_concept_choice_type,
                     dl_role_choice_type,
                     dl_boolean_choice_type,
                     dl_numerical_choice_type,
                     dl_query_type,
                     dl_grammar_root_type)

BOOST_SPIRIT_DECLARE(dl_search_free_concept_root_type,
                     dl_search_free_role_root_type,
                     dl_search_free_boolean_root_type,
                     dl_search_free_numerical_root_type,
                     dl_search_free_concept_type,
                     dl_search_free_role_type,
                     dl_search_free_boolean_type,
                     dl_search_free_numerical_type,
                     dl_search_free_concept_choice_type,
                     dl_search_free_role_choice_type,
                     dl_search_free_boolean_choice_type,
                     dl_search_free_numerical_choice_type,
                     dl_search_free_query_type,
                     dl_search_free_grammar_root_type)

/// The rules other grammars embed; X3 parses a rule with the definition of its type, which the declarations above make visible.
inline constexpr dl_concept_type dl_concept = "concept";
inline constexpr dl_role_type dl_role = "role";
inline constexpr dl_boolean_type dl_boolean = "boolean";
inline constexpr dl_numerical_type dl_numerical = "numerical";
inline constexpr dl_query_type dl_query = "query";

inline constexpr dl_search_free_concept_type dl_search_free_concept = "concept";
inline constexpr dl_search_free_role_type dl_search_free_role = "role";
inline constexpr dl_search_free_boolean_type dl_search_free_boolean = "boolean";
inline constexpr dl_search_free_numerical_type dl_search_free_numerical = "numerical";
inline constexpr dl_search_free_query_type dl_search_free_query = "query";

template<typename Dialect = FullDialectTag>
concept_type<Dialect> const& concept_parser();

template<typename Dialect = FullDialectTag>
concept_root_type<Dialect> const& concept_root_parser();

template<typename Dialect = FullDialectTag>
role_type<Dialect> const& role_parser();

template<typename Dialect = FullDialectTag>
role_root_type<Dialect> const& role_root_parser();

template<typename Dialect = FullDialectTag>
boolean_type<Dialect> const& boolean_parser();

template<typename Dialect = FullDialectTag>
boolean_root_type<Dialect> const& boolean_root_parser();

template<typename Dialect = FullDialectTag>
numerical_type<Dialect> const& numerical_parser();

template<typename Dialect = FullDialectTag>
numerical_root_type<Dialect> const& numerical_root_parser();

template<typename Dialect = FullDialectTag>
query_type<Dialect> const& query_parser();

template<typename Dialect = FullDialectTag>
grammar_root_type<Dialect> const& grammar_root_parser();

template<>
concept_type<FullDialectTag> const& concept_parser<FullDialectTag>();
template<>
concept_root_type<FullDialectTag> const& concept_root_parser<FullDialectTag>();
template<>
role_type<FullDialectTag> const& role_parser<FullDialectTag>();
template<>
role_root_type<FullDialectTag> const& role_root_parser<FullDialectTag>();
template<>
boolean_type<FullDialectTag> const& boolean_parser<FullDialectTag>();
template<>
boolean_root_type<FullDialectTag> const& boolean_root_parser<FullDialectTag>();
template<>
numerical_type<FullDialectTag> const& numerical_parser<FullDialectTag>();
template<>
numerical_root_type<FullDialectTag> const& numerical_root_parser<FullDialectTag>();
template<>
query_type<FullDialectTag> const& query_parser<FullDialectTag>();
template<>
grammar_root_type<FullDialectTag> const& grammar_root_parser<FullDialectTag>();

template<>
concept_type<SearchFreeDialectTag> const& concept_parser<SearchFreeDialectTag>();
template<>
concept_root_type<SearchFreeDialectTag> const& concept_root_parser<SearchFreeDialectTag>();
template<>
role_type<SearchFreeDialectTag> const& role_parser<SearchFreeDialectTag>();
template<>
role_root_type<SearchFreeDialectTag> const& role_root_parser<SearchFreeDialectTag>();
template<>
boolean_type<SearchFreeDialectTag> const& boolean_parser<SearchFreeDialectTag>();
template<>
boolean_root_type<SearchFreeDialectTag> const& boolean_root_parser<SearchFreeDialectTag>();
template<>
numerical_type<SearchFreeDialectTag> const& numerical_parser<SearchFreeDialectTag>();
template<>
numerical_root_type<SearchFreeDialectTag> const& numerical_root_parser<SearchFreeDialectTag>();
template<>
query_type<SearchFreeDialectTag> const& query_parser<SearchFreeDialectTag>();
template<>
grammar_root_type<SearchFreeDialectTag> const& grammar_root_parser<SearchFreeDialectTag>();

}  // namespace runir::kr::dl::grammar::parser

#endif
