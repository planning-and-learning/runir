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
 * Truth values
 */

struct TrueTag
{
    static constexpr auto keyword = "true";
};

struct FalseTag
{
    static constexpr auto keyword = "false";
};

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
    using FactKind = T;
};

template<tyr::formalism::FactKind T>
struct AtomicGoalTag
{
    using FactKind = T;
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

struct AtLeastNumberRestrictionTag
{
};

struct AtMostNumberRestrictionTag
{
};

struct ExactNumberRestrictionTag
{
};

struct QualifiedAtLeastNumberRestrictionTag
{
};

struct QualifiedAtMostNumberRestrictionTag
{
};

struct QualifiedExactNumberRestrictionTag
{
};

struct RoleValueMapTag
{
};

struct AgreementTag
{
};

struct RoleFillersTag
{
};

struct OneOfTag
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

// Numerical constructor: per-object sum of paired distances.
//   (n_sum_pair_distance C R_start R_traverse R_target)
// For each c in [[C]], compute the shortest [[R_traverse]]-distance from
// [[R_start]](c) to [[R_target]](c) (BFS) and sum the per-object values.
// Saturates to UINT_MAX if any c has empty start/target or is unreachable.
// Unlike DistanceTag (min over the full lhs x rhs cross product), this
// keeps the pairing tied through c, so two objects sharing a goal location
// can no longer collapse the global distance to zero.
struct SumPairDistanceTag
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
    || std::same_as<T, AtLeastNumberRestrictionTag> || std::same_as<T, AtMostNumberRestrictionTag> || std::same_as<T, ExactNumberRestrictionTag>
    || std::same_as<T, QualifiedAtLeastNumberRestrictionTag> || std::same_as<T, QualifiedAtMostNumberRestrictionTag>
    || std::same_as<T, QualifiedExactNumberRestrictionTag> || std::same_as<T, RoleValueMapTag> || std::same_as<T, AgreementTag>
    || std::same_as<T, RoleFillersTag> || std::same_as<T, OneOfTag> || std::same_as<T, NominalTag>;

template<typename T>
concept RoleConstructorTag =
    std::same_as<T, UniversalTag> || is_atomic_state_tag_v<T> || is_atomic_goal_tag_v<T> || std::same_as<T, IntersectionTag> || std::same_as<T, UnionTag>
    || std::same_as<T, ComplementTag> || std::same_as<T, InverseTag> || std::same_as<T, CompositionTag> || std::same_as<T, TransitiveClosureTag>
    || std::same_as<T, ReflexiveTransitiveClosureTag> || std::same_as<T, RestrictionTag> || std::same_as<T, IdentityTag>;

template<typename T>
concept BooleanConstructorTag = is_atomic_state_tag_v<T> || is_atomic_goal_tag_v<T> || std::same_as<T, NonemptyTag>;

template<typename T>
concept NumericalConstructorTag = std::same_as<T, CountTag> || std::same_as<T, DistanceTag> || std::same_as<T, SumPairDistanceTag>;

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
                                             AtLeastNumberRestrictionTag,
                                             AtMostNumberRestrictionTag,
                                             ExactNumberRestrictionTag,
                                             QualifiedAtLeastNumberRestrictionTag,
                                             QualifiedAtMostNumberRestrictionTag,
                                             QualifiedExactNumberRestrictionTag,
                                             RoleValueMapTag,
                                             AgreementTag,
                                             RoleFillersTag,
                                             OneOfTag,
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

using BooleanConstructorTags = tyr::TypeList<AtomicStateTag<tyr::formalism::StaticTag>,
                                             AtomicStateTag<tyr::formalism::FluentTag>,
                                             AtomicStateTag<tyr::formalism::DerivedTag>,
                                             AtomicGoalTag<tyr::formalism::StaticTag>,
                                             AtomicGoalTag<tyr::formalism::FluentTag>,
                                             AtomicGoalTag<tyr::formalism::DerivedTag>,
                                             NonemptyTag>;

using NumericalConstructorTags = tyr::TypeList<CountTag, DistanceTag, SumPairDistanceTag>;

}

#endif
