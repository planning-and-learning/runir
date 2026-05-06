#ifndef RUNIR_DECLARATIONS_HPP_
#define RUNIR_DECLARATIONS_HPP_

#include <concepts>
#include <type_traits>
#include <tyr/common/type_list.hpp>
#include <tyr/formalism/declarations.hpp>

namespace runir
{

/**
 * Categories
 */

struct ConceptTag
{
};

struct RoleTag
{
};

struct BooleanTag
{
};

struct NumericalTag
{
};

template<typename T>
concept CategoryTag = std::same_as<T, ConceptTag> || std::same_as<T, RoleTag> || std::same_as<T, BooleanTag> || std::same_as<T, NumericalTag>;

using CategoryTags = tyr::TypeList<ConceptTag, RoleTag, BooleanTag, NumericalTag>;

/**
 * Constructor tags
 */

struct BotTag
{
};

struct TopTag
{
};

template<tyr::formalism::FactKind T>
struct AtomicStateTag
{
};

template<tyr::formalism::FactKind T>
struct AtomicGoalTag
{
};

struct IntersectionTag
{
};

struct UnionTag
{
};

struct NegationTag
{
};

struct ValueRestrictionTag
{
};

struct ExistentialQuantificationTag
{
};

struct RoleValueMapContainmentTag
{
};

struct RoleValueMapEqualityTag
{
};

struct NominalTag
{
};

struct UniversalTag
{
};

struct ComplementTag
{
};

struct InverseTag
{
};

struct CompositionTag
{
};

struct TransitiveClosureTag
{
};

struct ReflexiveTransitiveClosureTag
{
};

struct RestrictionTag
{
};

struct IdentityTag
{
};

struct NonemptyTag
{
};

struct CountTag
{
};

struct DistanceTag
{
};

template<typename T>
struct IsAtomicStateTag : std::false_type
{
};

template<tyr::formalism::FactKind T>
struct IsAtomicStateTag<AtomicStateTag<T>> : std::true_type
{
};

template<typename T>
inline constexpr bool is_atomic_state_tag_v = IsAtomicStateTag<T>::value;

template<typename T>
struct IsAtomicGoalTag : std::false_type
{
};

template<tyr::formalism::FactKind T>
struct IsAtomicGoalTag<AtomicGoalTag<T>> : std::true_type
{
};

template<typename T>
inline constexpr bool is_atomic_goal_tag_v = IsAtomicGoalTag<T>::value;

template<typename T>
concept ConceptConstructorTag =
    std::same_as<T, BotTag> || std::same_as<T, TopTag> || is_atomic_state_tag_v<T> || is_atomic_goal_tag_v<T> || std::same_as<T, IntersectionTag>
    || std::same_as<T, UnionTag> || std::same_as<T, NegationTag> || std::same_as<T, ValueRestrictionTag> || std::same_as<T, ExistentialQuantificationTag>
    || std::same_as<T, RoleValueMapContainmentTag> || std::same_as<T, RoleValueMapEqualityTag> || std::same_as<T, NominalTag>;

template<typename T>
concept RoleConstructorTag =
    std::same_as<T, UniversalTag> || is_atomic_state_tag_v<T> || is_atomic_goal_tag_v<T> || std::same_as<T, IntersectionTag> || std::same_as<T, UnionTag>
    || std::same_as<T, ComplementTag> || std::same_as<T, InverseTag> || std::same_as<T, CompositionTag> || std::same_as<T, TransitiveClosureTag>
    || std::same_as<T, ReflexiveTransitiveClosureTag> || std::same_as<T, RestrictionTag> || std::same_as<T, IdentityTag>;

template<typename T>
concept BooleanConstructorTag = is_atomic_state_tag_v<T> || std::same_as<T, NonemptyTag>;

template<typename T>
concept NumericalConstructorTag = std::same_as<T, CountTag> || std::same_as<T, DistanceTag>;

using ConceptConstructorTags = tyr::TypeList<BotTag,
                                             TopTag,
                                             AtomicStateTag<tyr::formalism::StaticTag>,
                                             AtomicStateTag<tyr::formalism::FluentTag>,
                                             AtomicStateTag<tyr::formalism::DerivedTag>,
                                             AtomicGoalTag<tyr::formalism::StaticTag>,
                                             AtomicGoalTag<tyr::formalism::FluentTag>,
                                             AtomicGoalTag<tyr::formalism::DerivedTag>,
                                             IntersectionTag,
                                             UnionTag,
                                             NegationTag,
                                             ValueRestrictionTag,
                                             ExistentialQuantificationTag,
                                             RoleValueMapContainmentTag,
                                             RoleValueMapEqualityTag,
                                             NominalTag>;

using RoleConstructorTags = tyr::TypeList<UniversalTag,
                                          AtomicStateTag<tyr::formalism::StaticTag>,
                                          AtomicStateTag<tyr::formalism::FluentTag>,
                                          AtomicStateTag<tyr::formalism::DerivedTag>,
                                          AtomicGoalTag<tyr::formalism::StaticTag>,
                                          AtomicGoalTag<tyr::formalism::FluentTag>,
                                          AtomicGoalTag<tyr::formalism::DerivedTag>,
                                          IntersectionTag,
                                          UnionTag,
                                          ComplementTag,
                                          InverseTag,
                                          CompositionTag,
                                          TransitiveClosureTag,
                                          ReflexiveTransitiveClosureTag,
                                          RestrictionTag,
                                          IdentityTag>;

using BooleanConstructorTags = tyr::
    TypeList<AtomicStateTag<tyr::formalism::StaticTag>, AtomicStateTag<tyr::formalism::FluentTag>, AtomicStateTag<tyr::formalism::DerivedTag>, NonemptyTag>;

using NumericalConstructorTags = tyr::TypeList<CountTag, DistanceTag>;

}

#endif
