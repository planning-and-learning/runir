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

#include <boost/optional.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <string>
#include <vector>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::dl::grammar::ast
{
namespace x3 = boost::spirit::x3;

template<typename... Alternatives>
struct PositionedVariant : x3::position_tagged, x3::variant<Alternatives...>
{
    using Base = x3::variant<Alternatives...>;
    using Base::Base;
    using Base::operator=;
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename Tag = void>
struct Constructor;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct NonTerminal : x3::position_tagged
{
    std::string name;
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct ConstructorOrNonTerminal : PositionedVariant<NonTerminal<Family, Category>, Constructor<Family, Category>>
{
    using Base = PositionedVariant<NonTerminal<Family, Category>, Constructor<Family, Category>>;
    using Base::Base;
    using Base::operator=;
};

template<runir::kr::dl::FamilyTag Family>
using ConceptChoice = ConstructorOrNonTerminal<Family, runir::kr::dl::ConceptTag>;

template<runir::kr::dl::FamilyTag Family>
using RoleChoice = ConstructorOrNonTerminal<Family, runir::kr::dl::RoleTag>;

template<runir::kr::dl::FamilyTag Family>
using BooleanChoice = ConstructorOrNonTerminal<Family, runir::kr::dl::BooleanTag>;

template<runir::kr::dl::FamilyTag Family>
using NumericalChoice = ConstructorOrNonTerminal<Family, runir::kr::dl::NumericalTag>;

template<runir::kr::dl::FamilyTag Family>
struct ConstructorOrNonTerminalVariant : PositionedVariant<ConceptChoice<Family>, RoleChoice<Family>>
{
    using Base = PositionedVariant<ConceptChoice<Family>, RoleChoice<Family>>;
    using Base::Base;
    using Base::operator=;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::BotTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::BotTag::keyword;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::TopTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::TopTag::keyword;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ConceptAtomicStateSyntaxTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::ConceptAtomicStateSyntaxTag::keyword;
    std::string predicate_name;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ConceptAtomicGoalSyntaxTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::ConceptAtomicGoalSyntaxTag::keyword;
    std::string predicate_name;
    bool polarity;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ConceptIntersectionSyntaxTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::ConceptIntersectionSyntaxTag::keyword;
    ConceptChoice<Family> lhs;
    ConceptChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ConceptUnionSyntaxTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::ConceptUnionSyntaxTag::keyword;
    ConceptChoice<Family> lhs;
    ConceptChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::NegationTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::NegationTag::keyword;
    ConceptChoice<Family> arg;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ValueRestrictionTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::ValueRestrictionTag::keyword;
    RoleChoice<Family> lhs;
    ConceptChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ExistentialQuantificationTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::ExistentialQuantificationTag::keyword;
    RoleChoice<Family> lhs;
    ConceptChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::AtLeastNumberRestrictionTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::AtLeastNumberRestrictionTag::keyword;
    ygg::uint_t n;
    RoleChoice<Family> role;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::AtMostNumberRestrictionTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::AtMostNumberRestrictionTag::keyword;
    ygg::uint_t n;
    RoleChoice<Family> role;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ExactNumberRestrictionTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::ExactNumberRestrictionTag::keyword;
    ygg::uint_t n;
    RoleChoice<Family> role;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::QualifiedAtLeastNumberRestrictionTag::keyword;
    ygg::uint_t n;
    RoleChoice<Family> role;
    ConceptChoice<Family> concept_;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::QualifiedAtMostNumberRestrictionTag::keyword;
    ygg::uint_t n;
    RoleChoice<Family> role;
    ConceptChoice<Family> concept_;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::QualifiedExactNumberRestrictionTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::QualifiedExactNumberRestrictionTag::keyword;
    ygg::uint_t n;
    RoleChoice<Family> role;
    ConceptChoice<Family> concept_;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::RoleValueMapTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::RoleValueMapTag::keyword;
    RoleChoice<Family> lhs;
    RoleChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::AgreementTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::AgreementTag::keyword;
    RoleChoice<Family> lhs;
    RoleChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::RoleFillersTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::RoleFillersTag::keyword;
    RoleChoice<Family> role;
    std::vector<std::string> object_names;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::OneOfTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::OneOfTag::keyword;
    std::vector<std::string> object_names;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::NominalTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::NominalTag::keyword;
    std::string object_name;
};

template<runir::kr::dl::FamilyTag Family>
using ConceptBot = Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::BotTag>;
template<runir::kr::dl::FamilyTag Family>
using ConceptTop = Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::TopTag>;
template<runir::kr::dl::FamilyTag Family>
using ConceptAtomicState = Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ConceptAtomicStateSyntaxTag>;
template<runir::kr::dl::FamilyTag Family>
using ConceptAtomicGoal = Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ConceptAtomicGoalSyntaxTag>;
template<runir::kr::dl::FamilyTag Family>
using ConceptIntersection = Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ConceptIntersectionSyntaxTag>;
template<runir::kr::dl::FamilyTag Family>
using ConceptUnion = Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ConceptUnionSyntaxTag>;
template<runir::kr::dl::FamilyTag Family>
using ConceptNegation = Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::NegationTag>;
template<runir::kr::dl::FamilyTag Family>
using ConceptValueRestriction = Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ValueRestrictionTag>;
template<runir::kr::dl::FamilyTag Family>
using ConceptExistentialQuantification = Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ExistentialQuantificationTag>;
template<runir::kr::dl::FamilyTag Family>
using ConceptAtLeastNumberRestriction = Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::AtLeastNumberRestrictionTag>;
template<runir::kr::dl::FamilyTag Family>
using ConceptAtMostNumberRestriction = Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::AtMostNumberRestrictionTag>;
template<runir::kr::dl::FamilyTag Family>
using ConceptExactNumberRestriction = Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ExactNumberRestrictionTag>;
template<runir::kr::dl::FamilyTag Family>
using ConceptQualifiedAtLeastNumberRestriction = Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>;
template<runir::kr::dl::FamilyTag Family>
using ConceptQualifiedAtMostNumberRestriction = Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>;
template<runir::kr::dl::FamilyTag Family>
using ConceptQualifiedExactNumberRestriction = Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::QualifiedExactNumberRestrictionTag>;
template<runir::kr::dl::FamilyTag Family>
using ConceptRoleValueMap = Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::RoleValueMapTag>;
template<runir::kr::dl::FamilyTag Family>
using ConceptAgreement = Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::AgreementTag>;
template<runir::kr::dl::FamilyTag Family>
using ConceptRoleFillers = Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::RoleFillersTag>;
template<runir::kr::dl::FamilyTag Family>
using ConceptOneOf = Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::OneOfTag>;
template<runir::kr::dl::FamilyTag Family>
using ConceptNominal = Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::NominalTag>;

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::ConceptTag, void> :
    PositionedVariant<x3::forward_ast<ConceptBot<Family>>,
                      x3::forward_ast<ConceptTop<Family>>,
                      x3::forward_ast<ConceptAtomicState<Family>>,
                      x3::forward_ast<ConceptAtomicGoal<Family>>,
                      x3::forward_ast<ConceptIntersection<Family>>,
                      x3::forward_ast<ConceptUnion<Family>>,
                      x3::forward_ast<ConceptNegation<Family>>,
                      x3::forward_ast<ConceptValueRestriction<Family>>,
                      x3::forward_ast<ConceptExistentialQuantification<Family>>,
                      x3::forward_ast<ConceptAtLeastNumberRestriction<Family>>,
                      x3::forward_ast<ConceptAtMostNumberRestriction<Family>>,
                      x3::forward_ast<ConceptExactNumberRestriction<Family>>,
                      x3::forward_ast<ConceptQualifiedAtLeastNumberRestriction<Family>>,
                      x3::forward_ast<ConceptQualifiedAtMostNumberRestriction<Family>>,
                      x3::forward_ast<ConceptQualifiedExactNumberRestriction<Family>>,
                      x3::forward_ast<ConceptRoleValueMap<Family>>,
                      x3::forward_ast<ConceptAgreement<Family>>,
                      x3::forward_ast<ConceptRoleFillers<Family>>,
                      x3::forward_ast<ConceptOneOf<Family>>,
                      x3::forward_ast<ConceptNominal<Family>>>
{
    using Base = PositionedVariant<x3::forward_ast<ConceptBot<Family>>,
                                   x3::forward_ast<ConceptTop<Family>>,
                                   x3::forward_ast<ConceptAtomicState<Family>>,
                                   x3::forward_ast<ConceptAtomicGoal<Family>>,
                                   x3::forward_ast<ConceptIntersection<Family>>,
                                   x3::forward_ast<ConceptUnion<Family>>,
                                   x3::forward_ast<ConceptNegation<Family>>,
                                   x3::forward_ast<ConceptValueRestriction<Family>>,
                                   x3::forward_ast<ConceptExistentialQuantification<Family>>,
                                   x3::forward_ast<ConceptAtLeastNumberRestriction<Family>>,
                                   x3::forward_ast<ConceptAtMostNumberRestriction<Family>>,
                                   x3::forward_ast<ConceptExactNumberRestriction<Family>>,
                                   x3::forward_ast<ConceptQualifiedAtLeastNumberRestriction<Family>>,
                                   x3::forward_ast<ConceptQualifiedAtMostNumberRestriction<Family>>,
                                   x3::forward_ast<ConceptQualifiedExactNumberRestriction<Family>>,
                                   x3::forward_ast<ConceptRoleValueMap<Family>>,
                                   x3::forward_ast<ConceptAgreement<Family>>,
                                   x3::forward_ast<ConceptRoleFillers<Family>>,
                                   x3::forward_ast<ConceptOneOf<Family>>,
                                   x3::forward_ast<ConceptNominal<Family>>>;
    using Base::Base;
    using Base::operator=;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::RoleTag, runir::kr::dl::UniversalTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::UniversalTag::keyword;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::RoleTag, runir::kr::dl::RoleAtomicStateSyntaxTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::RoleAtomicStateSyntaxTag::keyword;
    std::string predicate_name;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::RoleTag, runir::kr::dl::RoleAtomicGoalSyntaxTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::RoleAtomicGoalSyntaxTag::keyword;
    std::string predicate_name;
    bool polarity;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::RoleTag, runir::kr::dl::RoleIntersectionSyntaxTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::RoleIntersectionSyntaxTag::keyword;
    RoleChoice<Family> lhs;
    RoleChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::RoleTag, runir::kr::dl::RoleUnionSyntaxTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::RoleUnionSyntaxTag::keyword;
    RoleChoice<Family> lhs;
    RoleChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::RoleTag, runir::kr::dl::ComplementTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::ComplementTag::keyword;
    RoleChoice<Family> arg;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::RoleTag, runir::kr::dl::InverseTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::InverseTag::keyword;
    RoleChoice<Family> arg;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::RoleTag, runir::kr::dl::CompositionTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::CompositionTag::keyword;
    RoleChoice<Family> lhs;
    RoleChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::RoleTag, runir::kr::dl::TransitiveClosureTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::TransitiveClosureTag::keyword;
    RoleChoice<Family> arg;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::RoleTag, runir::kr::dl::ReflexiveTransitiveClosureTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::ReflexiveTransitiveClosureTag::keyword;
    RoleChoice<Family> arg;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::RoleTag, runir::kr::dl::RestrictionTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::RestrictionTag::keyword;
    RoleChoice<Family> lhs;
    ConceptChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::RoleTag, runir::kr::dl::IdentityTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::IdentityTag::keyword;
    ConceptChoice<Family> arg;
};

template<runir::kr::dl::FamilyTag Family>
using RoleUniversal = Constructor<Family, runir::kr::dl::RoleTag, runir::kr::dl::UniversalTag>;
template<runir::kr::dl::FamilyTag Family>
using RoleAtomicState = Constructor<Family, runir::kr::dl::RoleTag, runir::kr::dl::RoleAtomicStateSyntaxTag>;
template<runir::kr::dl::FamilyTag Family>
using RoleAtomicGoal = Constructor<Family, runir::kr::dl::RoleTag, runir::kr::dl::RoleAtomicGoalSyntaxTag>;
template<runir::kr::dl::FamilyTag Family>
using RoleIntersection = Constructor<Family, runir::kr::dl::RoleTag, runir::kr::dl::RoleIntersectionSyntaxTag>;
template<runir::kr::dl::FamilyTag Family>
using RoleUnion = Constructor<Family, runir::kr::dl::RoleTag, runir::kr::dl::RoleUnionSyntaxTag>;
template<runir::kr::dl::FamilyTag Family>
using RoleComplement = Constructor<Family, runir::kr::dl::RoleTag, runir::kr::dl::ComplementTag>;
template<runir::kr::dl::FamilyTag Family>
using RoleInverse = Constructor<Family, runir::kr::dl::RoleTag, runir::kr::dl::InverseTag>;
template<runir::kr::dl::FamilyTag Family>
using RoleComposition = Constructor<Family, runir::kr::dl::RoleTag, runir::kr::dl::CompositionTag>;
template<runir::kr::dl::FamilyTag Family>
using RoleTransitiveClosure = Constructor<Family, runir::kr::dl::RoleTag, runir::kr::dl::TransitiveClosureTag>;
template<runir::kr::dl::FamilyTag Family>
using RoleReflexiveTransitiveClosure = Constructor<Family, runir::kr::dl::RoleTag, runir::kr::dl::ReflexiveTransitiveClosureTag>;
template<runir::kr::dl::FamilyTag Family>
using RoleRestriction = Constructor<Family, runir::kr::dl::RoleTag, runir::kr::dl::RestrictionTag>;
template<runir::kr::dl::FamilyTag Family>
using RoleIdentity = Constructor<Family, runir::kr::dl::RoleTag, runir::kr::dl::IdentityTag>;

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::RoleTag, void> :
    PositionedVariant<x3::forward_ast<RoleUniversal<Family>>,
                      x3::forward_ast<RoleAtomicState<Family>>,
                      x3::forward_ast<RoleAtomicGoal<Family>>,
                      x3::forward_ast<RoleIntersection<Family>>,
                      x3::forward_ast<RoleUnion<Family>>,
                      x3::forward_ast<RoleComplement<Family>>,
                      x3::forward_ast<RoleInverse<Family>>,
                      x3::forward_ast<RoleComposition<Family>>,
                      x3::forward_ast<RoleTransitiveClosure<Family>>,
                      x3::forward_ast<RoleReflexiveTransitiveClosure<Family>>,
                      x3::forward_ast<RoleRestriction<Family>>,
                      x3::forward_ast<RoleIdentity<Family>>>
{
    using Base = PositionedVariant<x3::forward_ast<RoleUniversal<Family>>,
                                   x3::forward_ast<RoleAtomicState<Family>>,
                                   x3::forward_ast<RoleAtomicGoal<Family>>,
                                   x3::forward_ast<RoleIntersection<Family>>,
                                   x3::forward_ast<RoleUnion<Family>>,
                                   x3::forward_ast<RoleComplement<Family>>,
                                   x3::forward_ast<RoleInverse<Family>>,
                                   x3::forward_ast<RoleComposition<Family>>,
                                   x3::forward_ast<RoleTransitiveClosure<Family>>,
                                   x3::forward_ast<RoleReflexiveTransitiveClosure<Family>>,
                                   x3::forward_ast<RoleRestriction<Family>>,
                                   x3::forward_ast<RoleIdentity<Family>>>;
    using Base::Base;
    using Base::operator=;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanAtomicStateSyntaxTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::BooleanAtomicStateSyntaxTag::keyword;
    std::string predicate_name;
    bool polarity;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanAtomicGoalSyntaxTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::BooleanAtomicGoalSyntaxTag::keyword;
    std::string predicate_name;
    bool polarity;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::BooleanTag, runir::kr::dl::NonemptyTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::NonemptyTag::keyword;
    ConstructorOrNonTerminalVariant<Family> arg;
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::ComparisonTag Tag>
struct Constructor<Family, runir::kr::dl::BooleanTag, Tag> : x3::position_tagged
{
    static constexpr auto keyword = Tag::keyword;
    ConstructorOrNonTerminal<Family, typename Tag::OperandCategory> lhs;
    ConstructorOrNonTerminal<Family, typename Tag::OperandCategory> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanConstantTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::BooleanConstantTag::keyword;
    bool value;
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::LogicalBinaryTag Tag>
struct Constructor<Family, runir::kr::dl::BooleanTag, Tag> : x3::position_tagged
{
    static constexpr auto keyword = Tag::keyword;
    BooleanChoice<Family> lhs;
    BooleanChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::BooleanTag, runir::kr::dl::NotTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::NotTag::keyword;
    BooleanChoice<Family> arg;
};

template<runir::kr::dl::FamilyTag Family>
using BooleanAtomicState = Constructor<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanAtomicStateSyntaxTag>;
template<runir::kr::dl::FamilyTag Family>
using BooleanAtomicGoal = Constructor<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanAtomicGoalSyntaxTag>;
template<runir::kr::dl::FamilyTag Family>
using BooleanNonempty = Constructor<Family, runir::kr::dl::BooleanTag, runir::kr::dl::NonemptyTag>;
template<runir::kr::dl::FamilyTag Family>
using BooleanEq = Constructor<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanEqTag>;
template<runir::kr::dl::FamilyTag Family>
using BooleanNeq = Constructor<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanNeTag>;
template<runir::kr::dl::FamilyTag Family>
using BooleanLt = Constructor<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanLtTag>;
template<runir::kr::dl::FamilyTag Family>
using BooleanLe = Constructor<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanLeTag>;
template<runir::kr::dl::FamilyTag Family>
using BooleanGt = Constructor<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanGtTag>;
template<runir::kr::dl::FamilyTag Family>
using BooleanGe = Constructor<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanGeTag>;
template<runir::kr::dl::FamilyTag Family>
using NumericalEq = Constructor<Family, runir::kr::dl::BooleanTag, runir::kr::dl::NumericalEqTag>;
template<runir::kr::dl::FamilyTag Family>
using NumericalNeq = Constructor<Family, runir::kr::dl::BooleanTag, runir::kr::dl::NumericalNeTag>;
template<runir::kr::dl::FamilyTag Family>
using NumericalLt = Constructor<Family, runir::kr::dl::BooleanTag, runir::kr::dl::NumericalLtTag>;
template<runir::kr::dl::FamilyTag Family>
using NumericalLe = Constructor<Family, runir::kr::dl::BooleanTag, runir::kr::dl::NumericalLeTag>;
template<runir::kr::dl::FamilyTag Family>
using NumericalGt = Constructor<Family, runir::kr::dl::BooleanTag, runir::kr::dl::NumericalGtTag>;
template<runir::kr::dl::FamilyTag Family>
using NumericalGe = Constructor<Family, runir::kr::dl::BooleanTag, runir::kr::dl::NumericalGeTag>;
template<runir::kr::dl::FamilyTag Family>
using BooleanConstant = Constructor<Family, runir::kr::dl::BooleanTag, runir::kr::dl::BooleanConstantTag>;
template<runir::kr::dl::FamilyTag Family>
using BooleanAnd = Constructor<Family, runir::kr::dl::BooleanTag, runir::kr::dl::AndTag>;
template<runir::kr::dl::FamilyTag Family>
using BooleanOr = Constructor<Family, runir::kr::dl::BooleanTag, runir::kr::dl::OrTag>;
template<runir::kr::dl::FamilyTag Family>
using BooleanNot = Constructor<Family, runir::kr::dl::BooleanTag, runir::kr::dl::NotTag>;

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::BooleanTag, void> :
    PositionedVariant<x3::forward_ast<BooleanAtomicState<Family>>, x3::forward_ast<BooleanAtomicGoal<Family>>, x3::forward_ast<BooleanNonempty<Family>>>
{
    using Base =
        PositionedVariant<x3::forward_ast<BooleanAtomicState<Family>>, x3::forward_ast<BooleanAtomicGoal<Family>>, x3::forward_ast<BooleanNonempty<Family>>>;
    using Base::Base;
    using Base::operator=;
};

template<>
struct Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag, void> :
    PositionedVariant<x3::forward_ast<BooleanAtomicState<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<BooleanAtomicGoal<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<BooleanNonempty<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<BooleanEq<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<BooleanNeq<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<BooleanLt<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<BooleanLe<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<BooleanGt<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<BooleanGe<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalEq<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalNeq<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalLt<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalLe<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalGt<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalGe<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<BooleanConstant<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<BooleanAnd<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<BooleanOr<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<BooleanNot<runir::kr::UnsFamilyTag>>>
{
    using Base = PositionedVariant<x3::forward_ast<BooleanAtomicState<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<BooleanAtomicGoal<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<BooleanNonempty<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<BooleanEq<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<BooleanNeq<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<BooleanLt<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<BooleanLe<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<BooleanGt<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<BooleanGe<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalEq<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalNeq<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalLt<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalLe<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalGt<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalGe<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<BooleanConstant<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<BooleanAnd<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<BooleanOr<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<BooleanNot<runir::kr::UnsFamilyTag>>>;
    using Base::Base;
    using Base::operator=;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::NumericalTag, runir::kr::dl::CountTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::CountTag::keyword;
    ConstructorOrNonTerminalVariant<Family> arg;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::NumericalTag, runir::kr::dl::DistanceTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::DistanceTag::keyword;
    ConceptChoice<Family> lhs;
    RoleChoice<Family> mid;
    ConceptChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::NumericalTag, runir::kr::dl::NumericalConstantTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::NumericalConstantTag::keyword;
    ygg::uint_t value;
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::NumericalBinaryTag Tag>
struct Constructor<Family, runir::kr::dl::NumericalTag, Tag> : x3::position_tagged
{
    static constexpr auto keyword = Tag::keyword;
    NumericalChoice<Family> lhs;
    NumericalChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
using NumericalCount = Constructor<Family, runir::kr::dl::NumericalTag, runir::kr::dl::CountTag>;
template<runir::kr::dl::FamilyTag Family>
using NumericalDistance = Constructor<Family, runir::kr::dl::NumericalTag, runir::kr::dl::DistanceTag>;
template<runir::kr::dl::FamilyTag Family>
using NumericalConstant = Constructor<Family, runir::kr::dl::NumericalTag, runir::kr::dl::NumericalConstantTag>;
template<runir::kr::dl::FamilyTag Family>
using NumericalAdd = Constructor<Family, runir::kr::dl::NumericalTag, runir::kr::dl::AddTag>;
template<runir::kr::dl::FamilyTag Family>
using NumericalSub = Constructor<Family, runir::kr::dl::NumericalTag, runir::kr::dl::SubTag>;
template<runir::kr::dl::FamilyTag Family>
using NumericalMul = Constructor<Family, runir::kr::dl::NumericalTag, runir::kr::dl::MulTag>;
template<runir::kr::dl::FamilyTag Family>
using NumericalDiv = Constructor<Family, runir::kr::dl::NumericalTag, runir::kr::dl::DivTag>;
template<runir::kr::dl::FamilyTag Family>
using NumericalMin = Constructor<Family, runir::kr::dl::NumericalTag, runir::kr::dl::MinTag>;
template<runir::kr::dl::FamilyTag Family>
using NumericalMax = Constructor<Family, runir::kr::dl::NumericalTag, runir::kr::dl::MaxTag>;

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::NumericalTag, void> : PositionedVariant<x3::forward_ast<NumericalCount<Family>>, x3::forward_ast<NumericalDistance<Family>>>
{
    using Base = PositionedVariant<x3::forward_ast<NumericalCount<Family>>, x3::forward_ast<NumericalDistance<Family>>>;
    using Base::Base;
    using Base::operator=;
};

template<>
struct Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::NumericalTag, void> :
    PositionedVariant<x3::forward_ast<NumericalCount<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalDistance<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalConstant<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalAdd<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalSub<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalMul<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalDiv<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalMin<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalMax<runir::kr::UnsFamilyTag>>>
{
    using Base = PositionedVariant<x3::forward_ast<NumericalCount<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalDistance<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalConstant<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalAdd<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalSub<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalMul<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalDiv<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalMin<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalMax<runir::kr::UnsFamilyTag>>>;
    using Base::Base;
    using Base::operator=;
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct DerivationRule : x3::position_tagged
{
    using CategoryTag = Category;
    using Lhs = NonTerminal<Family, Category>;
    using Rhs = std::vector<ConstructorOrNonTerminal<Family, Category>>;

    Lhs lhs;
    Rhs rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct DerivationRuleVariant :
    PositionedVariant<DerivationRule<Family, runir::kr::dl::ConceptTag>,
                      DerivationRule<Family, runir::kr::dl::RoleTag>,
                      DerivationRule<Family, runir::kr::dl::BooleanTag>,
                      DerivationRule<Family, runir::kr::dl::NumericalTag>>
{
    using Base = PositionedVariant<DerivationRule<Family, runir::kr::dl::ConceptTag>,
                                   DerivationRule<Family, runir::kr::dl::RoleTag>,
                                   DerivationRule<Family, runir::kr::dl::BooleanTag>,
                                   DerivationRule<Family, runir::kr::dl::NumericalTag>>;
    using Base::Base;
    using Base::operator=;
};

template<runir::kr::dl::FamilyTag Family>
struct Grammar : x3::position_tagged
{
    std::vector<DerivationRuleVariant<Family>> rules;
};

template<runir::kr::dl::FamilyTag Family>
using ConceptConstructor = Constructor<Family, runir::kr::dl::ConceptTag>;

template<runir::kr::dl::FamilyTag Family>
using RoleConstructor = Constructor<Family, runir::kr::dl::RoleTag>;

template<runir::kr::dl::FamilyTag Family>
using BooleanConstructor = Constructor<Family, runir::kr::dl::BooleanTag>;

template<runir::kr::dl::FamilyTag Family>
using NumericalConstructor = Constructor<Family, runir::kr::dl::NumericalTag>;

}  // namespace runir::kr::dl::grammar::ast

#endif
