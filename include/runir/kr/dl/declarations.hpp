#ifndef RUNIR_DECLARATIONS_HPP_
#define RUNIR_DECLARATIONS_HPP_

#include <concepts>
#include <type_traits>
#include <tyr/common/type_list.hpp>
#include <tyr/formalism/declarations.hpp>

namespace runir::kr::dl
{

/**
 * Categories
 */

struct ConceptTag
{
    static constexpr auto name = "concept";
};

struct RoleTag
{
    static constexpr auto name = "role";
};

struct BooleanTag
{
    static constexpr auto name = "boolean";
};

struct NumericalTag
{
    static constexpr auto name = "numerical";
};

template<typename T>
concept CategoryTag = std::same_as<T, ConceptTag> || std::same_as<T, RoleTag> || std::same_as<T, BooleanTag> || std::same_as<T, NumericalTag>;

using CategoryTags = tyr::TypeList<ConceptTag, RoleTag, BooleanTag, NumericalTag>;

/**
 * Constructor tags
 */

struct BotTag
{
    static constexpr auto name = "bot";
};

struct TopTag
{
    static constexpr auto name = "top";
};

template<tyr::formalism::FactKind T>
struct AtomicStateTag
{
    using FactKind = T;

    static constexpr auto name = "atomic_state";
};

template<tyr::formalism::FactKind T>
struct AtomicGoalTag
{
    using FactKind = T;

    static constexpr auto name = "atomic_goal";
};

struct IntersectionTag
{
    static constexpr auto name = "intersection";
};

struct UnionTag
{
    static constexpr auto name = "union";
};

struct NegationTag
{
    static constexpr auto name = "negation";
};

struct ValueRestrictionTag
{
    static constexpr auto name = "value_restriction";
};

struct ExistentialQuantificationTag
{
    static constexpr auto name = "existential_quantification";
};

struct RoleValueMapContainmentTag
{
    static constexpr auto name = "role_value_map_containment";
};

struct RoleValueMapEqualityTag
{
    static constexpr auto name = "role_value_map_equality";
};

struct NominalTag
{
    static constexpr auto name = "nominal";
};

struct UniversalTag
{
    static constexpr auto name = "universal";
};

struct ComplementTag
{
    static constexpr auto name = "complement";
};

struct InverseTag
{
    static constexpr auto name = "inverse";
};

struct CompositionTag
{
    static constexpr auto name = "composition";
};

struct TransitiveClosureTag
{
    static constexpr auto name = "transitive_closure";
};

struct ReflexiveTransitiveClosureTag
{
    static constexpr auto name = "reflexive_transitive_closure";
};

struct RestrictionTag
{
    static constexpr auto name = "restriction";
};

struct IdentityTag
{
    static constexpr auto name = "identity";
};

struct NonemptyTag
{
    static constexpr auto name = "nonempty";
};

struct CountTag
{
    static constexpr auto name = "count";
};

struct DistanceTag
{
    static constexpr auto name = "distance";
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
