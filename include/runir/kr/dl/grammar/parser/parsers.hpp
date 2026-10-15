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

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename Dialect = FullDialectTag>
struct ConstructorClass : boost::spirit::x3::annotate_on_success
{
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename Dialect = FullDialectTag>
struct ConstructorRootClass : ErrorHandlerBase
{
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename Dialect = FullDialectTag>
struct NonTerminalClass : boost::spirit::x3::annotate_on_success
{
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename Dialect = FullDialectTag>
struct ConstructorOrNonTerminalClass : boost::spirit::x3::annotate_on_success
{
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename Dialect = FullDialectTag>
struct DerivationRuleClass : boost::spirit::x3::annotate_on_success
{
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename Tag, typename Dialect = FullDialectTag>
struct ConstructorTagClass : boost::spirit::x3::annotate_on_success
{
};

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
struct ConstructorOrNonTerminalVariantClass : boost::spirit::x3::annotate_on_success
{
};

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
struct DerivationRuleVariantClass : boost::spirit::x3::annotate_on_success
{
};

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
struct GrammarBodyClass : boost::spirit::x3::annotate_on_success
{
};

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
struct GrammarClass : boost::spirit::x3::annotate_on_success
{
};

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
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

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename Dialect = FullDialectTag>
using constructor_type = boost::spirit::x3::rule<ConstructorClass<Family, Category, Dialect>, ast::Constructor<Family, Category>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename Dialect = FullDialectTag>
using constructor_root_type = boost::spirit::x3::rule<ConstructorRootClass<Family, Category, Dialect>, ast::Constructor<Family, Category>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename Dialect = FullDialectTag>
using non_terminal_type = boost::spirit::x3::rule<NonTerminalClass<Family, Category, Dialect>, ast::NonTerminal<Family, Category>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename Dialect = FullDialectTag>
using choice_type = boost::spirit::x3::rule<ConstructorOrNonTerminalClass<Family, Category, Dialect>, ast::ConstructorOrNonTerminal<Family, Category>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename Dialect = FullDialectTag>
using derivation_rule_type = boost::spirit::x3::rule<DerivationRuleClass<Family, Category, Dialect>, ast::DerivationRule<Family, Category>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename Tag, typename Dialect = FullDialectTag>
using constructor_tag_type = boost::spirit::x3::rule<ConstructorTagClass<Family, Category, Tag, Dialect>, ast::Constructor<Family, Category, Tag>>;

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
using query_atomic_state_type = boost::spirit::x3::rule<QueryClass<ast::QueryAtomicState<Family>, Dialect>, ast::QueryAtomicState<Family>>;

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
using query_atomic_goal_type = boost::spirit::x3::rule<QueryClass<ast::QueryAtomicGoal<Family>, Dialect>, ast::QueryAtomicGoal<Family>>;

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
using query_concept_type = boost::spirit::x3::rule<QueryClass<ast::QueryConcept<Family>, Dialect>, ast::QueryConcept<Family>>;

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
using query_role_type = boost::spirit::x3::rule<QueryClass<ast::QueryRole<Family>, Dialect>, ast::QueryRole<Family>>;

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
using query_join_type = boost::spirit::x3::rule<QueryClass<ast::QueryJoin<Family>, Dialect>, ast::QueryJoin<Family>>;

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
using query_project_type = boost::spirit::x3::rule<QueryClass<ast::QueryProject<Family>, Dialect>, ast::QueryProject<Family>>;

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
using query_rename_type = boost::spirit::x3::rule<QueryClass<ast::QueryRename<Family>, Dialect>, ast::QueryRename<Family>>;

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
using query_select_equal_type = boost::spirit::x3::rule<QueryClass<ast::QuerySelectEqual<Family>, Dialect>, ast::QuerySelectEqual<Family>>;

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
using query_select_value_type = boost::spirit::x3::rule<QueryClass<ast::QuerySelectValue<Family>, Dialect>, ast::QuerySelectValue<Family>>;

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
using query_union_type = boost::spirit::x3::rule<QueryClass<ast::QueryUnion<Family>, Dialect>, ast::QueryUnion<Family>>;

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
using query_difference_type = boost::spirit::x3::rule<QueryClass<ast::QueryDifference<Family>, Dialect>, ast::QueryDifference<Family>>;

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
using query_generic_join_type = boost::spirit::x3::rule<QueryClass<ast::QueryGenericJoin<Family>, Dialect>, ast::QueryGenericJoin<Family>>;

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
using query_empty_type = boost::spirit::x3::rule<QueryClass<ast::QueryEmpty<Family>, Dialect>, ast::QueryEmpty<Family>>;

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
using query_must_pass_type = boost::spirit::x3::rule<QueryClass<ast::QueryMustPass<Family>, Dialect>, ast::QueryMustPass<Family>>;

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
using query_type = boost::spirit::x3::rule<QueryClass<ast::Query<Family>, Dialect>, ast::Query<Family>>;

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
using constructor_or_non_terminal_variant_type =
    boost::spirit::x3::rule<ConstructorOrNonTerminalVariantClass<Family, Dialect>, ast::ConstructorOrNonTerminalVariant<Family>>;

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
using derivation_rule_variant_type = boost::spirit::x3::rule<DerivationRuleVariantClass<Family, Dialect>, ast::DerivationRuleVariant<Family>>;

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
using grammar_body_type = boost::spirit::x3::rule<GrammarBodyClass<Family, Dialect>, std::vector<ast::DerivationRuleVariant<Family>>>;

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
using grammar_type = boost::spirit::x3::rule<GrammarClass<Family, Dialect>, ast::Grammar<Family>>;

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
using grammar_root_type = boost::spirit::x3::rule<GrammarRootClass<Family, Dialect>, ast::Grammar<Family>>;

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
using concept_type = constructor_type<Family, runir::kr::dl::ConceptTag, Dialect>;

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
using concept_root_type = constructor_root_type<Family, runir::kr::dl::ConceptTag, Dialect>;

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
using role_type = constructor_type<Family, runir::kr::dl::RoleTag, Dialect>;

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
using role_root_type = constructor_root_type<Family, runir::kr::dl::RoleTag, Dialect>;

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
using boolean_type = constructor_type<Family, runir::kr::dl::BooleanTag, Dialect>;

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
using boolean_root_type = constructor_root_type<Family, runir::kr::dl::BooleanTag, Dialect>;

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
using numerical_type = constructor_type<Family, runir::kr::dl::NumericalTag, Dialect>;

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
using numerical_root_type = constructor_root_type<Family, runir::kr::dl::NumericalTag, Dialect>;

#define RUNIR_DECLARE_RULE_ALIASES(Prefix, Family, Dialect)                                                   \
    using Prefix##_concept_root_type = constructor_root_type<Family, runir::kr::dl::ConceptTag, Dialect>;     \
    using Prefix##_role_root_type = constructor_root_type<Family, runir::kr::dl::RoleTag, Dialect>;           \
    using Prefix##_boolean_root_type = constructor_root_type<Family, runir::kr::dl::BooleanTag, Dialect>;     \
    using Prefix##_numerical_root_type = constructor_root_type<Family, runir::kr::dl::NumericalTag, Dialect>; \
    using Prefix##_concept_type = constructor_type<Family, runir::kr::dl::ConceptTag, Dialect>;               \
    using Prefix##_role_type = constructor_type<Family, runir::kr::dl::RoleTag, Dialect>;                     \
    using Prefix##_boolean_type = constructor_type<Family, runir::kr::dl::BooleanTag, Dialect>;               \
    using Prefix##_numerical_type = constructor_type<Family, runir::kr::dl::NumericalTag, Dialect>;           \
    using Prefix##_concept_choice_type = choice_type<Family, runir::kr::dl::ConceptTag, Dialect>;             \
    using Prefix##_role_choice_type = choice_type<Family, runir::kr::dl::RoleTag, Dialect>;                   \
    using Prefix##_boolean_choice_type = choice_type<Family, runir::kr::dl::BooleanTag, Dialect>;             \
    using Prefix##_numerical_choice_type = choice_type<Family, runir::kr::dl::NumericalTag, Dialect>;         \
    using Prefix##_query_type = query_type<Family, Dialect>;                                                  \
    using Prefix##_grammar_root_type = grammar_root_type<Family, Dialect>;

RUNIR_DECLARE_RULE_ALIASES(base_family, runir::kr::BaseFamilyTag, FullDialectTag)
RUNIR_DECLARE_RULE_ALIASES(ext_family, runir::kr::ExtFamilyTag, FullDialectTag)
RUNIR_DECLARE_RULE_ALIASES(uns_family, runir::kr::UnsFamilyTag, FullDialectTag)
RUNIR_DECLARE_RULE_ALIASES(base_family_search_free, runir::kr::BaseFamilyTag, SearchFreeDialectTag)
RUNIR_DECLARE_RULE_ALIASES(ext_family_search_free, runir::kr::ExtFamilyTag, SearchFreeDialectTag)
RUNIR_DECLARE_RULE_ALIASES(uns_family_search_free, runir::kr::UnsFamilyTag, SearchFreeDialectTag)

#undef RUNIR_DECLARE_RULE_ALIASES

BOOST_SPIRIT_DECLARE(base_family_concept_root_type,
                     base_family_role_root_type,
                     base_family_boolean_root_type,
                     base_family_numerical_root_type,
                     base_family_concept_type,
                     base_family_role_type,
                     base_family_boolean_type,
                     base_family_numerical_type,
                     base_family_concept_choice_type,
                     base_family_role_choice_type,
                     base_family_boolean_choice_type,
                     base_family_numerical_choice_type,
                     base_family_query_type,
                     base_family_grammar_root_type)

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

BOOST_SPIRIT_DECLARE(uns_family_concept_root_type,
                     uns_family_role_root_type,
                     uns_family_boolean_root_type,
                     uns_family_numerical_root_type,
                     uns_family_concept_type,
                     uns_family_role_type,
                     uns_family_boolean_type,
                     uns_family_numerical_type,
                     uns_family_concept_choice_type,
                     uns_family_role_choice_type,
                     uns_family_boolean_choice_type,
                     uns_family_numerical_choice_type,
                     uns_family_query_type,
                     uns_family_grammar_root_type)

BOOST_SPIRIT_DECLARE(base_family_search_free_concept_root_type,
                     base_family_search_free_role_root_type,
                     base_family_search_free_boolean_root_type,
                     base_family_search_free_numerical_root_type,
                     base_family_search_free_concept_type,
                     base_family_search_free_role_type,
                     base_family_search_free_boolean_type,
                     base_family_search_free_numerical_type,
                     base_family_search_free_concept_choice_type,
                     base_family_search_free_role_choice_type,
                     base_family_search_free_boolean_choice_type,
                     base_family_search_free_numerical_choice_type,
                     base_family_search_free_query_type,
                     base_family_search_free_grammar_root_type)

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

BOOST_SPIRIT_DECLARE(uns_family_search_free_concept_root_type,
                     uns_family_search_free_role_root_type,
                     uns_family_search_free_boolean_root_type,
                     uns_family_search_free_numerical_root_type,
                     uns_family_search_free_concept_type,
                     uns_family_search_free_role_type,
                     uns_family_search_free_boolean_type,
                     uns_family_search_free_numerical_type,
                     uns_family_search_free_concept_choice_type,
                     uns_family_search_free_role_choice_type,
                     uns_family_search_free_boolean_choice_type,
                     uns_family_search_free_numerical_choice_type,
                     uns_family_search_free_query_type,
                     uns_family_search_free_grammar_root_type)

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
concept_type<Family, Dialect> const& concept_parser();

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
concept_root_type<Family, Dialect> const& concept_root_parser();

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
role_type<Family, Dialect> const& role_parser();

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
role_root_type<Family, Dialect> const& role_root_parser();

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
boolean_type<Family, Dialect> const& boolean_parser();

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
boolean_root_type<Family, Dialect> const& boolean_root_parser();

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
numerical_type<Family, Dialect> const& numerical_parser();

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
numerical_root_type<Family, Dialect> const& numerical_root_parser();

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
query_type<Family, Dialect> const& query_parser();

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
grammar_root_type<Family, Dialect> const& grammar_root_parser();

#define RUNIR_DECLARE_PARSER_ACCESSORS(Family, Dialect)                                   \
    template<>                                                                            \
    concept_type<Family, Dialect> const& concept_parser<Family, Dialect>();               \
    template<>                                                                            \
    concept_root_type<Family, Dialect> const& concept_root_parser<Family, Dialect>();     \
    template<>                                                                            \
    role_type<Family, Dialect> const& role_parser<Family, Dialect>();                     \
    template<>                                                                            \
    role_root_type<Family, Dialect> const& role_root_parser<Family, Dialect>();           \
    template<>                                                                            \
    boolean_type<Family, Dialect> const& boolean_parser<Family, Dialect>();               \
    template<>                                                                            \
    boolean_root_type<Family, Dialect> const& boolean_root_parser<Family, Dialect>();     \
    template<>                                                                            \
    numerical_type<Family, Dialect> const& numerical_parser<Family, Dialect>();           \
    template<>                                                                            \
    numerical_root_type<Family, Dialect> const& numerical_root_parser<Family, Dialect>(); \
    template<>                                                                            \
    query_type<Family, Dialect> const& query_parser<Family, Dialect>();                   \
    template<>                                                                            \
    grammar_root_type<Family, Dialect> const& grammar_root_parser<Family, Dialect>();

RUNIR_DECLARE_PARSER_ACCESSORS(runir::kr::BaseFamilyTag, FullDialectTag)
RUNIR_DECLARE_PARSER_ACCESSORS(runir::kr::ExtFamilyTag, FullDialectTag)
RUNIR_DECLARE_PARSER_ACCESSORS(runir::kr::UnsFamilyTag, FullDialectTag)
RUNIR_DECLARE_PARSER_ACCESSORS(runir::kr::BaseFamilyTag, SearchFreeDialectTag)
RUNIR_DECLARE_PARSER_ACCESSORS(runir::kr::ExtFamilyTag, SearchFreeDialectTag)
RUNIR_DECLARE_PARSER_ACCESSORS(runir::kr::UnsFamilyTag, SearchFreeDialectTag)

#undef RUNIR_DECLARE_PARSER_ACCESSORS

}  // namespace runir::kr::dl::grammar::parser

#endif
