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

RUNIR_DECLARE_RULE_ALIASES(ext_family, FullDialectTag)
RUNIR_DECLARE_RULE_ALIASES(ext_family_search_free, SearchFreeDialectTag)

#undef RUNIR_DECLARE_RULE_ALIASES

BOOST_SPIRIT_DECLARE(ext_family_concept_root_type,
                     ext_family_role_root_type,
                     ext_family_boolean_root_type,
                     ext_family_numerical_root_type,
                     ext_family_concept_type,
                     ext_family_role_type,
                     ext_family_boolean_type,
                     ext_family_numerical_type,
                     ext_family_concept_choice_type,
                     ext_family_role_choice_type,
                     ext_family_boolean_choice_type,
                     ext_family_numerical_choice_type,
                     ext_family_query_type,
                     ext_family_grammar_root_type)

BOOST_SPIRIT_DECLARE(ext_family_search_free_concept_root_type,
                     ext_family_search_free_role_root_type,
                     ext_family_search_free_boolean_root_type,
                     ext_family_search_free_numerical_root_type,
                     ext_family_search_free_concept_type,
                     ext_family_search_free_role_type,
                     ext_family_search_free_boolean_type,
                     ext_family_search_free_numerical_type,
                     ext_family_search_free_concept_choice_type,
                     ext_family_search_free_role_choice_type,
                     ext_family_search_free_boolean_choice_type,
                     ext_family_search_free_numerical_choice_type,
                     ext_family_search_free_query_type,
                     ext_family_search_free_grammar_root_type)

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

#define RUNIR_DECLARE_PARSER_ACCESSORS(Dialect)                           \
    template<>                                                            \
    concept_type<Dialect> const& concept_parser<Dialect>();               \
    template<>                                                            \
    concept_root_type<Dialect> const& concept_root_parser<Dialect>();     \
    template<>                                                            \
    role_type<Dialect> const& role_parser<Dialect>();                     \
    template<>                                                            \
    role_root_type<Dialect> const& role_root_parser<Dialect>();           \
    template<>                                                            \
    boolean_type<Dialect> const& boolean_parser<Dialect>();               \
    template<>                                                            \
    boolean_root_type<Dialect> const& boolean_root_parser<Dialect>();     \
    template<>                                                            \
    numerical_type<Dialect> const& numerical_parser<Dialect>();           \
    template<>                                                            \
    numerical_root_type<Dialect> const& numerical_root_parser<Dialect>(); \
    template<>                                                            \
    query_type<Dialect> const& query_parser<Dialect>();                   \
    template<>                                                            \
    grammar_root_type<Dialect> const& grammar_root_parser<Dialect>();

RUNIR_DECLARE_PARSER_ACCESSORS(FullDialectTag)
RUNIR_DECLARE_PARSER_ACCESSORS(SearchFreeDialectTag)

#undef RUNIR_DECLARE_PARSER_ACCESSORS

}  // namespace runir::kr::dl::grammar::parser

#endif
