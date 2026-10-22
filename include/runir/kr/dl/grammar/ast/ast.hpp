#ifndef RUNIR_GRAMMAR_AST_AST_HPP_
#define RUNIR_GRAMMAR_AST_AST_HPP_

#ifndef BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#endif
#ifndef BOOST_MPL_LIMIT_LIST_SIZE
#define BOOST_MPL_LIMIT_LIST_SIZE 30
#endif
#ifndef BOOST_VARIANT_LIMIT_TYPES
#define BOOST_VARIANT_LIMIT_TYPES 30
#endif

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/family_traits.hpp"
#include "runir/kr/parser/ast.hpp"

#include <boost/optional.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <string>
#include <utility>
#include <vector>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::dl::grammar::ast
{
struct Identifier : runir::kr::parser::ast::Identifier
{
    Identifier& operator=(std::string value)
    {
        text = std::move(value);
        return *this;
    }
};

struct NumericReference : boost::spirit::x3::position_tagged
{
    ygg::uint_t value {};

    NumericReference& operator=(ygg::uint_t value_)
    {
        value = value_;
        return *this;
    }
};

template<typename... Alternatives>
struct PositionedVariant : boost::spirit::x3::position_tagged, boost::spirit::x3::variant<Alternatives...>
{
    using Base = boost::spirit::x3::variant<Alternatives...>;
    using Base::Base;
    using Base::operator=;
};

template<runir::kr::dl::CategoryTag Category>
struct ArgumentReference : PositionedVariant<NumericReference, Identifier>
{
    using Base = PositionedVariant<NumericReference, Identifier>;
    using Base::Base;
    using Base::operator=;
};

template<runir::kr::dl::CategoryTag Category>
struct RegisterReference : PositionedVariant<NumericReference, Identifier>
{
    using Base = PositionedVariant<NumericReference, Identifier>;
    using Base::Base;
    using Base::operator=;
};

template<runir::kr::dl::CategoryTag Category, typename Tag = void>
struct Constructor;

using BaseAstConceptConstructorTags = ygg::TypeList<runir::kr::dl::BotTag,
                                                    runir::kr::dl::TopTag,
                                                    runir::kr::dl::ConceptAtomicStateSyntaxTag,
                                                    runir::kr::dl::ConceptAtomicGoalSyntaxTag,
                                                    runir::kr::dl::ConceptIntersectionSyntaxTag,
                                                    runir::kr::dl::ConceptUnionSyntaxTag,
                                                    runir::kr::dl::NegationTag,
                                                    runir::kr::dl::ValueRestrictionTag,
                                                    runir::kr::dl::ExistentialQuantificationTag,
                                                    runir::kr::dl::AtLeastNumberRestrictionTag,
                                                    runir::kr::dl::AtMostNumberRestrictionTag,
                                                    runir::kr::dl::ExactNumberRestrictionTag,
                                                    runir::kr::dl::QualifiedAtLeastNumberRestrictionTag,
                                                    runir::kr::dl::QualifiedAtMostNumberRestrictionTag,
                                                    runir::kr::dl::QualifiedExactNumberRestrictionTag,
                                                    runir::kr::dl::RoleValueMapTag,
                                                    runir::kr::dl::AgreementTag,
                                                    runir::kr::dl::RoleFillersTag,
                                                    runir::kr::dl::OneOfTag,
                                                    runir::kr::dl::NominalTag,
                                                    runir::kr::dl::ProjectTag>;

using BaseAstRoleConstructorTags = ygg::TypeList<runir::kr::dl::UniversalTag,
                                                 runir::kr::dl::RoleAtomicStateSyntaxTag,
                                                 runir::kr::dl::RoleAtomicGoalSyntaxTag,
                                                 runir::kr::dl::RoleIntersectionSyntaxTag,
                                                 runir::kr::dl::RoleUnionSyntaxTag,
                                                 runir::kr::dl::ComplementTag,
                                                 runir::kr::dl::InverseTag,
                                                 runir::kr::dl::CompositionTag,
                                                 runir::kr::dl::TransitiveClosureTag,
                                                 runir::kr::dl::ReflexiveTransitiveClosureTag,
                                                 runir::kr::dl::RestrictionTag,
                                                 runir::kr::dl::IdentityTag,
                                                 runir::kr::dl::ProjectTag>;

using BaseAstBooleanConstructorTags =
    ygg::TypeList<runir::kr::dl::BooleanAtomicStateSyntaxTag, runir::kr::dl::BooleanAtomicGoalSyntaxTag, runir::kr::dl::NonemptyTag>;

using BaseAstNumericalConstructorTags = ygg::TypeList<runir::kr::dl::CountTag, runir::kr::dl::DistanceTag>;

using AstConceptConstructorTags =
    ygg::ConcatTypeListsT<BaseAstConceptConstructorTags, ygg::TypeList<runir::kr::dl::RegisterTag, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>>;
using AstRoleConstructorTags =
    ygg::ConcatTypeListsT<BaseAstRoleConstructorTags, ygg::TypeList<runir::kr::dl::RegisterTag, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>>;
using AstBooleanConstructorTags = ygg::ConcatTypeListsT<BaseAstBooleanConstructorTags,
                                                        runir::kr::dl::ComparisonConstructorTags,
                                                        ygg::TypeList<runir::kr::dl::BooleanConstantTag>,
                                                        runir::kr::dl::LogicalConstructorTags,
                                                        ygg::TypeList<runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>>;
using AstNumericalConstructorTags = ygg::ConcatTypeListsT<BaseAstNumericalConstructorTags,
                                                          runir::kr::dl::ArithmeticConstructorTags,
                                                          ygg::TypeList<runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>>;

template<runir::kr::dl::CategoryTag Category>
struct AstConstructorTags;

template<>
struct AstConstructorTags<runir::kr::dl::ConceptTag>
{
    using Type = AstConceptConstructorTags;
};

template<>
struct AstConstructorTags<runir::kr::dl::RoleTag>
{
    using Type = AstRoleConstructorTags;
};

template<>
struct AstConstructorTags<runir::kr::dl::BooleanTag>
{
    using Type = AstBooleanConstructorTags;
};

template<>
struct AstConstructorTags<runir::kr::dl::NumericalTag>
{
    using Type = AstNumericalConstructorTags;
};

template<runir::kr::dl::CategoryTag Category>
using AstConstructorTagsT = typename AstConstructorTags<Category>::Type;

template<runir::kr::dl::CategoryTag Category>
struct AstConstructorAlternative
{
    template<typename Tag>
    using Type = boost::spirit::x3::forward_ast<Constructor<Category, Tag>>;
};

template<runir::kr::dl::CategoryTag Category>
using ConstructorVariantBase =
    ygg::ApplyTypeListT<PositionedVariant, ygg::MapTypeListT<AstConstructorAlternative<Category>::template Type, AstConstructorTagsT<Category>>>;

/// Defined before the constructors so that they can hold choices by value.
template<runir::kr::dl::CategoryTag Category>
struct Constructor<Category, void> : ConstructorVariantBase<Category>
{
    using Base = ConstructorVariantBase<Category>;
    using Base::Base;
    using Base::operator=;
};

template<runir::kr::dl::CategoryTag Category>
struct NonTerminal : boost::spirit::x3::position_tagged
{
    Identifier name;
};

template<runir::kr::dl::CategoryTag Category>
struct ConstructorOrNonTerminal : PositionedVariant<NonTerminal<Category>, Constructor<Category>>
{
    using Base = PositionedVariant<NonTerminal<Category>, Constructor<Category>>;
    using Base::Base;
    using Base::operator=;
};

using ConceptChoice = ConstructorOrNonTerminal<runir::kr::dl::ConceptTag>;

using RoleChoice = ConstructorOrNonTerminal<runir::kr::dl::RoleTag>;

using BooleanChoice = ConstructorOrNonTerminal<runir::kr::dl::BooleanTag>;

using NumericalChoice = ConstructorOrNonTerminal<runir::kr::dl::NumericalTag>;

struct Query;

struct QueryAtomicState : boost::spirit::x3::position_tagged
{
    Identifier predicate_name;
    std::vector<Identifier> columns;
};

struct QueryAtomicGoal : boost::spirit::x3::position_tagged
{
    Identifier predicate_name;
    bool polarity;
    std::vector<Identifier> columns;
};

struct QueryConcept : boost::spirit::x3::position_tagged
{
    std::vector<Identifier> columns;
    ConceptChoice arg;
};

struct QueryRole : boost::spirit::x3::position_tagged
{
    std::vector<Identifier> columns;
    RoleChoice arg;
};

template<typename Tag>
struct QueryBinary : boost::spirit::x3::position_tagged
{
    boost::spirit::x3::forward_ast<Query> lhs;
    boost::spirit::x3::forward_ast<Query> rhs;
};

using QueryJoin = QueryBinary<runir::kr::dl::QueryJoinTag>;

using QueryUnion = QueryBinary<runir::kr::dl::QueryUnionTag>;

using QueryDifference = QueryBinary<runir::kr::dl::QueryDifferenceTag>;

template<typename Tag>
struct QueryColumns : boost::spirit::x3::position_tagged
{
    std::vector<Identifier> columns;
    boost::spirit::x3::forward_ast<Query> arg;
};

using QueryProject = QueryColumns<runir::kr::dl::QueryProjectTag>;

using QueryRename = QueryColumns<runir::kr::dl::QueryRenameTag>;

struct QuerySelectEqual : boost::spirit::x3::position_tagged
{
    Identifier lhs_column;
    Identifier rhs_column;
    boost::spirit::x3::forward_ast<Query> arg;
};

struct QuerySelectValue : boost::spirit::x3::position_tagged
{
    Identifier column;
    Identifier object_name;
    boost::spirit::x3::forward_ast<Query> arg;
};

struct QueryGenericJoin : boost::spirit::x3::position_tagged
{
    std::vector<Identifier> variable_order;
    std::vector<Identifier> output_order;
    std::vector<boost::spirit::x3::forward_ast<Query>> args;
};

struct QueryEmpty : boost::spirit::x3::position_tagged
{
    std::vector<Identifier> columns;
};

struct QueryMustPass : boost::spirit::x3::position_tagged
{
    std::vector<Identifier> keys;
    boost::spirit::x3::forward_ast<Query> sources;
    boost::spirit::x3::forward_ast<Query> edges;
    boost::spirit::x3::forward_ast<Query> targets;
    boost::spirit::x3::forward_ast<Query> probes;
};

struct Query :
    PositionedVariant<QueryAtomicState,
                      QueryAtomicGoal,
                      boost::spirit::x3::forward_ast<QueryConcept>,
                      boost::spirit::x3::forward_ast<QueryRole>,
                      QueryJoin,
                      QueryProject,
                      QueryRename,
                      QuerySelectEqual,
                      QuerySelectValue,
                      QueryUnion,
                      QueryDifference,
                      QueryGenericJoin,
                      QueryEmpty,
                      QueryMustPass>
{
    using Base = PositionedVariant<QueryAtomicState,
                                   QueryAtomicGoal,
                                   boost::spirit::x3::forward_ast<QueryConcept>,
                                   boost::spirit::x3::forward_ast<QueryRole>,
                                   QueryJoin,
                                   QueryProject,
                                   QueryRename,
                                   QuerySelectEqual,
                                   QuerySelectValue,
                                   QueryUnion,
                                   QueryDifference,
                                   QueryGenericJoin,
                                   QueryEmpty,
                                   QueryMustPass>;
    using Base::Base;
    using Base::operator=;
};

template<runir::kr::dl::CategoryTag Category>
struct Constructor<Category, runir::kr::dl::ProjectTag> : boost::spirit::x3::position_tagged
{
    std::vector<Identifier> columns;
    Query arg;
};

using ConceptProject = Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::ProjectTag>;

using RoleProject = Constructor<runir::kr::dl::RoleTag, runir::kr::dl::ProjectTag>;

struct ConstructorOrNonTerminalVariant : PositionedVariant<ConceptChoice, RoleChoice, Query>
{
    using Base = PositionedVariant<ConceptChoice, RoleChoice, Query>;
    using Base::Base;
    using Base::operator=;
};

/// Distance vertices (sources/targets): a concept or a query.
struct ConceptOrQueryVariant : PositionedVariant<ConceptChoice, Query>
{
    using Base = PositionedVariant<ConceptChoice, Query>;
    using Base::Base;
    using Base::operator=;
};

/// Distance edges: a role or a query.
struct RoleOrQueryVariant : PositionedVariant<RoleChoice, Query>
{
    using Base = PositionedVariant<RoleChoice, Query>;
    using Base::Base;
    using Base::operator=;
};

template<>
struct Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::BotTag> : boost::spirit::x3::position_tagged
{
};

template<>
struct Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::TopTag> : boost::spirit::x3::position_tagged
{
};

template<>
struct Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::ConceptAtomicStateSyntaxTag> : boost::spirit::x3::position_tagged
{
    Identifier predicate_name;
};

template<>
struct Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::ConceptAtomicGoalSyntaxTag> : boost::spirit::x3::position_tagged
{
    Identifier predicate_name;
    bool polarity;
};

template<>
struct Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::ConceptIntersectionSyntaxTag> : boost::spirit::x3::position_tagged
{
    ConceptChoice lhs;
    ConceptChoice rhs;
};

template<>
struct Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::ConceptUnionSyntaxTag> : boost::spirit::x3::position_tagged
{
    ConceptChoice lhs;
    ConceptChoice rhs;
};

template<>
struct Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::NegationTag> : boost::spirit::x3::position_tagged
{
    ConceptChoice arg;
};

template<>
struct Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::ValueRestrictionTag> : boost::spirit::x3::position_tagged
{
    RoleChoice lhs;
    ConceptChoice rhs;
};

template<>
struct Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::ExistentialQuantificationTag> : boost::spirit::x3::position_tagged
{
    RoleChoice lhs;
    ConceptChoice rhs;
};

template<>
struct Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::AtLeastNumberRestrictionTag> : boost::spirit::x3::position_tagged
{
    ygg::uint_t n;
    RoleChoice role;
};

template<>
struct Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::AtMostNumberRestrictionTag> : boost::spirit::x3::position_tagged
{
    ygg::uint_t n;
    RoleChoice role;
};

template<>
struct Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::ExactNumberRestrictionTag> : boost::spirit::x3::position_tagged
{
    ygg::uint_t n;
    RoleChoice role;
};

template<>
struct Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag> : boost::spirit::x3::position_tagged
{
    ygg::uint_t n;
    RoleChoice role;
    ConceptChoice concept_;
};

template<>
struct Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag> : boost::spirit::x3::position_tagged
{
    ygg::uint_t n;
    RoleChoice role;
    ConceptChoice concept_;
};

template<>
struct Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::QualifiedExactNumberRestrictionTag> : boost::spirit::x3::position_tagged
{
    ygg::uint_t n;
    RoleChoice role;
    ConceptChoice concept_;
};

template<>
struct Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::RoleValueMapTag> : boost::spirit::x3::position_tagged
{
    RoleChoice lhs;
    RoleChoice rhs;
};

template<>
struct Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::AgreementTag> : boost::spirit::x3::position_tagged
{
    RoleChoice lhs;
    RoleChoice rhs;
};

template<>
struct Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::RoleFillersTag> : boost::spirit::x3::position_tagged
{
    RoleChoice role;
    std::vector<Identifier> object_names;
};

template<>
struct Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::OneOfTag> : boost::spirit::x3::position_tagged
{
    std::vector<Identifier> object_names;
};

template<>
struct Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::NominalTag> : boost::spirit::x3::position_tagged
{
    Identifier object_name;
};

template<runir::kr::dl::CategoryTag Category>
struct Constructor<Category, runir::kr::dl::ArgumentTag<Category>> : boost::spirit::x3::position_tagged
{
    ArgumentReference<Category> reference;
};

template<>
struct Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::RegisterTag> : boost::spirit::x3::position_tagged
{
    RegisterReference<runir::kr::dl::ConceptTag> reference;
};

template<>
struct Constructor<runir::kr::dl::RoleTag, runir::kr::dl::RegisterTag> : boost::spirit::x3::position_tagged
{
    RegisterReference<runir::kr::dl::RoleTag> reference;
};

using ConceptBot = Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::BotTag>;
using ConceptTop = Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::TopTag>;
using ConceptAtomicState = Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::ConceptAtomicStateSyntaxTag>;
using ConceptAtomicGoal = Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::ConceptAtomicGoalSyntaxTag>;
using ConceptIntersection = Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::ConceptIntersectionSyntaxTag>;
using ConceptUnion = Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::ConceptUnionSyntaxTag>;
using ConceptNegation = Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::NegationTag>;
using ConceptValueRestriction = Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::ValueRestrictionTag>;
using ConceptExistentialQuantification = Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::ExistentialQuantificationTag>;
using ConceptAtLeastNumberRestriction = Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::AtLeastNumberRestrictionTag>;
using ConceptAtMostNumberRestriction = Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::AtMostNumberRestrictionTag>;
using ConceptExactNumberRestriction = Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::ExactNumberRestrictionTag>;
using ConceptQualifiedAtLeastNumberRestriction = Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>;
using ConceptQualifiedAtMostNumberRestriction = Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>;
using ConceptQualifiedExactNumberRestriction = Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::QualifiedExactNumberRestrictionTag>;
using ConceptRoleValueMap = Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::RoleValueMapTag>;
using ConceptAgreement = Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::AgreementTag>;
using ConceptRoleFillers = Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::RoleFillersTag>;
using ConceptOneOf = Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::OneOfTag>;
using ConceptNominal = Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::NominalTag>;
using ConceptRegister = Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::RegisterTag>;
using ConceptArgument = Constructor<runir::kr::dl::ConceptTag, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>;

template<>
struct Constructor<runir::kr::dl::RoleTag, runir::kr::dl::UniversalTag> : boost::spirit::x3::position_tagged
{
};

template<>
struct Constructor<runir::kr::dl::RoleTag, runir::kr::dl::RoleAtomicStateSyntaxTag> : boost::spirit::x3::position_tagged
{
    Identifier predicate_name;
};

template<>
struct Constructor<runir::kr::dl::RoleTag, runir::kr::dl::RoleAtomicGoalSyntaxTag> : boost::spirit::x3::position_tagged
{
    Identifier predicate_name;
    bool polarity;
};

template<>
struct Constructor<runir::kr::dl::RoleTag, runir::kr::dl::RoleIntersectionSyntaxTag> : boost::spirit::x3::position_tagged
{
    RoleChoice lhs;
    RoleChoice rhs;
};

template<>
struct Constructor<runir::kr::dl::RoleTag, runir::kr::dl::RoleUnionSyntaxTag> : boost::spirit::x3::position_tagged
{
    RoleChoice lhs;
    RoleChoice rhs;
};

template<>
struct Constructor<runir::kr::dl::RoleTag, runir::kr::dl::ComplementTag> : boost::spirit::x3::position_tagged
{
    RoleChoice arg;
};

template<>
struct Constructor<runir::kr::dl::RoleTag, runir::kr::dl::InverseTag> : boost::spirit::x3::position_tagged
{
    RoleChoice arg;
};

template<>
struct Constructor<runir::kr::dl::RoleTag, runir::kr::dl::CompositionTag> : boost::spirit::x3::position_tagged
{
    RoleChoice lhs;
    RoleChoice rhs;
};

template<>
struct Constructor<runir::kr::dl::RoleTag, runir::kr::dl::TransitiveClosureTag> : boost::spirit::x3::position_tagged
{
    RoleChoice arg;
};

template<>
struct Constructor<runir::kr::dl::RoleTag, runir::kr::dl::ReflexiveTransitiveClosureTag> : boost::spirit::x3::position_tagged
{
    RoleChoice arg;
};

template<>
struct Constructor<runir::kr::dl::RoleTag, runir::kr::dl::RestrictionTag> : boost::spirit::x3::position_tagged
{
    RoleChoice lhs;
    ConceptChoice rhs;
};

template<>
struct Constructor<runir::kr::dl::RoleTag, runir::kr::dl::IdentityTag> : boost::spirit::x3::position_tagged
{
    ConceptChoice arg;
};

using RoleUniversal = Constructor<runir::kr::dl::RoleTag, runir::kr::dl::UniversalTag>;
using RoleAtomicState = Constructor<runir::kr::dl::RoleTag, runir::kr::dl::RoleAtomicStateSyntaxTag>;
using RoleAtomicGoal = Constructor<runir::kr::dl::RoleTag, runir::kr::dl::RoleAtomicGoalSyntaxTag>;
using RoleIntersection = Constructor<runir::kr::dl::RoleTag, runir::kr::dl::RoleIntersectionSyntaxTag>;
using RoleUnion = Constructor<runir::kr::dl::RoleTag, runir::kr::dl::RoleUnionSyntaxTag>;
using RoleComplement = Constructor<runir::kr::dl::RoleTag, runir::kr::dl::ComplementTag>;
using RoleInverse = Constructor<runir::kr::dl::RoleTag, runir::kr::dl::InverseTag>;
using RoleComposition = Constructor<runir::kr::dl::RoleTag, runir::kr::dl::CompositionTag>;
using RoleTransitiveClosure = Constructor<runir::kr::dl::RoleTag, runir::kr::dl::TransitiveClosureTag>;
using RoleReflexiveTransitiveClosure = Constructor<runir::kr::dl::RoleTag, runir::kr::dl::ReflexiveTransitiveClosureTag>;
using RoleRestriction = Constructor<runir::kr::dl::RoleTag, runir::kr::dl::RestrictionTag>;
using RoleIdentity = Constructor<runir::kr::dl::RoleTag, runir::kr::dl::IdentityTag>;
using RoleRegister = Constructor<runir::kr::dl::RoleTag, runir::kr::dl::RegisterTag>;
using RoleArgument = Constructor<runir::kr::dl::RoleTag, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>;

template<>
struct Constructor<runir::kr::dl::BooleanTag, runir::kr::dl::BooleanAtomicStateSyntaxTag> : boost::spirit::x3::position_tagged
{
    Identifier predicate_name;
    bool polarity;
};

template<>
struct Constructor<runir::kr::dl::BooleanTag, runir::kr::dl::BooleanAtomicGoalSyntaxTag> : boost::spirit::x3::position_tagged
{
    Identifier predicate_name;
    bool polarity;
};

template<>
struct Constructor<runir::kr::dl::BooleanTag, runir::kr::dl::NonemptyTag> : boost::spirit::x3::position_tagged
{
    ConstructorOrNonTerminalVariant arg;
};

template<runir::kr::dl::ComparisonTag Tag>
struct Constructor<runir::kr::dl::BooleanTag, Tag> : boost::spirit::x3::position_tagged
{
    ConstructorOrNonTerminal<typename Tag::OperandCategory> lhs;
    ConstructorOrNonTerminal<typename Tag::OperandCategory> rhs;
};

template<>
struct Constructor<runir::kr::dl::BooleanTag, runir::kr::dl::BooleanConstantTag> : boost::spirit::x3::position_tagged
{
    bool value;
};

template<runir::kr::dl::LogicalBinaryTag Tag>
struct Constructor<runir::kr::dl::BooleanTag, Tag> : boost::spirit::x3::position_tagged
{
    BooleanChoice lhs;
    BooleanChoice rhs;
};

template<>
struct Constructor<runir::kr::dl::BooleanTag, runir::kr::dl::NotTag> : boost::spirit::x3::position_tagged
{
    BooleanChoice arg;
};

using BooleanAtomicState = Constructor<runir::kr::dl::BooleanTag, runir::kr::dl::BooleanAtomicStateSyntaxTag>;
using BooleanAtomicGoal = Constructor<runir::kr::dl::BooleanTag, runir::kr::dl::BooleanAtomicGoalSyntaxTag>;
using BooleanNonempty = Constructor<runir::kr::dl::BooleanTag, runir::kr::dl::NonemptyTag>;
using BooleanEq = Constructor<runir::kr::dl::BooleanTag, runir::kr::dl::BooleanEqTag>;
using BooleanNeq = Constructor<runir::kr::dl::BooleanTag, runir::kr::dl::BooleanNeTag>;
using BooleanLt = Constructor<runir::kr::dl::BooleanTag, runir::kr::dl::BooleanLtTag>;
using BooleanLe = Constructor<runir::kr::dl::BooleanTag, runir::kr::dl::BooleanLeTag>;
using BooleanGt = Constructor<runir::kr::dl::BooleanTag, runir::kr::dl::BooleanGtTag>;
using BooleanGe = Constructor<runir::kr::dl::BooleanTag, runir::kr::dl::BooleanGeTag>;
using NumericalEq = Constructor<runir::kr::dl::BooleanTag, runir::kr::dl::NumericalEqTag>;
using NumericalNeq = Constructor<runir::kr::dl::BooleanTag, runir::kr::dl::NumericalNeTag>;
using NumericalLt = Constructor<runir::kr::dl::BooleanTag, runir::kr::dl::NumericalLtTag>;
using NumericalLe = Constructor<runir::kr::dl::BooleanTag, runir::kr::dl::NumericalLeTag>;
using NumericalGt = Constructor<runir::kr::dl::BooleanTag, runir::kr::dl::NumericalGtTag>;
using NumericalGe = Constructor<runir::kr::dl::BooleanTag, runir::kr::dl::NumericalGeTag>;
using BooleanConstant = Constructor<runir::kr::dl::BooleanTag, runir::kr::dl::BooleanConstantTag>;
using BooleanAnd = Constructor<runir::kr::dl::BooleanTag, runir::kr::dl::AndTag>;
using BooleanOr = Constructor<runir::kr::dl::BooleanTag, runir::kr::dl::OrTag>;
using BooleanNot = Constructor<runir::kr::dl::BooleanTag, runir::kr::dl::NotTag>;
using BooleanArgument = Constructor<runir::kr::dl::BooleanTag, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>;

template<>
struct Constructor<runir::kr::dl::NumericalTag, runir::kr::dl::CountTag> : boost::spirit::x3::position_tagged
{
    ConstructorOrNonTerminalVariant arg;
};

template<>
struct Constructor<runir::kr::dl::NumericalTag, runir::kr::dl::DistanceTag> : boost::spirit::x3::position_tagged
{
    ConceptOrQueryVariant lhs;
    RoleOrQueryVariant mid;
    ConceptOrQueryVariant rhs;
};

template<>
struct Constructor<runir::kr::dl::NumericalTag, runir::kr::dl::NumericalConstantTag> : boost::spirit::x3::position_tagged
{
    ygg::uint_t value;
};

template<runir::kr::dl::NumericalBinaryTag Tag>
struct Constructor<runir::kr::dl::NumericalTag, Tag> : boost::spirit::x3::position_tagged
{
    NumericalChoice lhs;
    NumericalChoice rhs;
};

using NumericalCount = Constructor<runir::kr::dl::NumericalTag, runir::kr::dl::CountTag>;
using NumericalDistance = Constructor<runir::kr::dl::NumericalTag, runir::kr::dl::DistanceTag>;
using NumericalConstant = Constructor<runir::kr::dl::NumericalTag, runir::kr::dl::NumericalConstantTag>;
using NumericalAdd = Constructor<runir::kr::dl::NumericalTag, runir::kr::dl::AddTag>;
using NumericalSub = Constructor<runir::kr::dl::NumericalTag, runir::kr::dl::SubTag>;
using NumericalMul = Constructor<runir::kr::dl::NumericalTag, runir::kr::dl::MulTag>;
using NumericalDiv = Constructor<runir::kr::dl::NumericalTag, runir::kr::dl::DivTag>;
using NumericalMin = Constructor<runir::kr::dl::NumericalTag, runir::kr::dl::MinTag>;
using NumericalMax = Constructor<runir::kr::dl::NumericalTag, runir::kr::dl::MaxTag>;
using NumericalArgument = Constructor<runir::kr::dl::NumericalTag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>;

template<runir::kr::dl::CategoryTag Category>
struct DerivationRule : boost::spirit::x3::position_tagged
{
    using CategoryTag = Category;

    NonTerminal<Category> lhs;
    std::vector<ConstructorOrNonTerminal<Category>> rhs;
};

struct DerivationRuleVariant :
    PositionedVariant<DerivationRule<runir::kr::dl::ConceptTag>,
                      DerivationRule<runir::kr::dl::RoleTag>,
                      DerivationRule<runir::kr::dl::BooleanTag>,
                      DerivationRule<runir::kr::dl::NumericalTag>>
{
    using Base = PositionedVariant<DerivationRule<runir::kr::dl::ConceptTag>,
                                   DerivationRule<runir::kr::dl::RoleTag>,
                                   DerivationRule<runir::kr::dl::BooleanTag>,
                                   DerivationRule<runir::kr::dl::NumericalTag>>;
    using Base::Base;
    using Base::operator=;
};

struct Grammar : boost::spirit::x3::position_tagged
{
    std::vector<DerivationRuleVariant> rules;
};

using ConceptConstructor = Constructor<runir::kr::dl::ConceptTag>;

using RoleConstructor = Constructor<runir::kr::dl::RoleTag>;

using BooleanConstructor = Constructor<runir::kr::dl::BooleanTag>;

using NumericalConstructor = Constructor<runir::kr::dl::NumericalTag>;

}  // namespace runir::kr::dl::grammar::ast

#endif
