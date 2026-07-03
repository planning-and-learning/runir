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


template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct Constructor<Family, Category, runir::kr::dl::ArgumentTag<Category>> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::ArgumentTag<Category>::keyword;
    ygg::uint_t identifier;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::RegisterTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::ConceptRegisterSyntaxTag::keyword;
    ygg::uint_t identifier;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::RoleTag, runir::kr::dl::RegisterTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::RoleRegisterSyntaxTag::keyword;
    ygg::uint_t identifier;
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
using ConceptRegister = Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::RegisterTag>;
template<runir::kr::dl::FamilyTag Family>
using ConceptArgument = Constructor<Family, runir::kr::dl::ConceptTag, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>;

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
                                                   runir::kr::dl::NominalTag>;

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
                                                runir::kr::dl::IdentityTag>;

using BaseAstBooleanConstructorTags = ygg::TypeList<runir::kr::dl::BooleanAtomicStateSyntaxTag,
                                                   runir::kr::dl::BooleanAtomicGoalSyntaxTag,
                                                   runir::kr::dl::NonemptyTag>;

using BaseAstNumericalConstructorTags = ygg::TypeList<runir::kr::dl::CountTag, runir::kr::dl::DistanceTag>;

using ExtAstConceptConstructorTags = ygg::ConcatTypeListsT<BaseAstConceptConstructorTags,
                                                          ygg::TypeList<runir::kr::dl::RegisterTag,
                                                                        runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>>;
using ExtAstRoleConstructorTags = ygg::ConcatTypeListsT<BaseAstRoleConstructorTags,
                                                       ygg::TypeList<runir::kr::dl::RegisterTag,
                                                                     runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>>;
using ExtAstBooleanConstructorTags = ygg::ConcatTypeListsT<BaseAstBooleanConstructorTags,
                                                          ygg::TypeList<runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>>;
using ExtAstNumericalConstructorTags = ygg::ConcatTypeListsT<BaseAstNumericalConstructorTags,
                                                            ygg::TypeList<runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>>;

using UnsAstBooleanConstructorTags = ygg::ConcatTypeListsT<BaseAstBooleanConstructorTags,
                                                          ygg::ConcatTypeListsT<runir::kr::dl::UnsComparisonConstructorTags,
                                                                                ygg::ConcatTypeListsT<ygg::TypeList<runir::kr::dl::BooleanConstantTag>,
                                                                                                      runir::kr::dl::UnsLogicalConstructorTags>>>;
using UnsAstNumericalConstructorTags = ygg::ConcatTypeListsT<BaseAstNumericalConstructorTags,
                                                            ygg::ConcatTypeListsT<ygg::TypeList<runir::kr::dl::NumericalConstantTag>,
                                                                                  runir::kr::dl::UnsNumericalBinaryConstructorTags>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct AstConstructorTags;

template<>
struct AstConstructorTags<runir::kr::BaseFamilyTag, runir::kr::dl::ConceptTag>
{
    using Type = BaseAstConceptConstructorTags;
};

template<>
struct AstConstructorTags<runir::kr::BaseFamilyTag, runir::kr::dl::RoleTag>
{
    using Type = BaseAstRoleConstructorTags;
};

template<>
struct AstConstructorTags<runir::kr::BaseFamilyTag, runir::kr::dl::BooleanTag>
{
    using Type = BaseAstBooleanConstructorTags;
};

template<>
struct AstConstructorTags<runir::kr::BaseFamilyTag, runir::kr::dl::NumericalTag>
{
    using Type = BaseAstNumericalConstructorTags;
};

template<>
struct AstConstructorTags<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>
{
    using Type = ExtAstConceptConstructorTags;
};

template<>
struct AstConstructorTags<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>
{
    using Type = ExtAstRoleConstructorTags;
};

template<>
struct AstConstructorTags<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanTag>
{
    using Type = ExtAstBooleanConstructorTags;
};

template<>
struct AstConstructorTags<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalTag>
{
    using Type = ExtAstNumericalConstructorTags;
};

template<>
struct AstConstructorTags<runir::kr::UnsFamilyTag, runir::kr::dl::ConceptTag>
{
    using Type = BaseAstConceptConstructorTags;
};

template<>
struct AstConstructorTags<runir::kr::UnsFamilyTag, runir::kr::dl::RoleTag>
{
    using Type = BaseAstRoleConstructorTags;
};

template<>
struct AstConstructorTags<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>
{
    using Type = UnsAstBooleanConstructorTags;
};

template<>
struct AstConstructorTags<runir::kr::UnsFamilyTag, runir::kr::dl::NumericalTag>
{
    using Type = UnsAstNumericalConstructorTags;
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
using AstConstructorTagsT = typename AstConstructorTags<Family, Category>::Type;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct AstConstructorAlternative
{
    template<typename Tag>
    using Type = x3::forward_ast<Constructor<Family, Category, Tag>>;
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
using ConstructorVariantBase = ygg::ApplyTypeListT<PositionedVariant,
                                                  ygg::MapTypeListT<AstConstructorAlternative<Family, Category>::template Type,
                                                                    AstConstructorTagsT<Family, Category>>>;

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
using RoleRegister = Constructor<Family, runir::kr::dl::RoleTag, runir::kr::dl::RegisterTag>;
template<runir::kr::dl::FamilyTag Family>
using RoleArgument = Constructor<Family, runir::kr::dl::RoleTag, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>;

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
using BooleanArgument = Constructor<Family, runir::kr::dl::BooleanTag, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>;

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
using NumericalArgument = Constructor<Family, runir::kr::dl::NumericalTag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>;


template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct Constructor<Family, Category, void> : ConstructorVariantBase<Family, Category>
{
    using Base = ConstructorVariantBase<Family, Category>;
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
