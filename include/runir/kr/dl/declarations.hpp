#ifndef RUNIR_DECLARATIONS_HPP_
#define RUNIR_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"

#include <concepts>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <yggdrasil/ids/index_mixins.hpp>
#include <yggdrasil/core/type_list.hpp>
#include <yggdrasil/core/types.hpp>
#include <tyr/formalism/declarations.hpp>

namespace runir::kr::dl
{

template<typename T>
concept FamilyTag = runir::kr::FamilyTag<T>;

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

using CategoryTags = ygg::TypeList<ConceptTag, RoleTag, BooleanTag, NumericalTag>;

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

// Customization point: constructor tags that are grammar placeholders (e.g. the ext family's
// RegisterTag/ArgumentTag) rather than directly-evaluable constructors. The base layer knows of
// none; family layers (see kr/dl/ext/declarations.hpp) specialize this to true. evaluate_impl and
// the formatters use it to dispatch placeholders explicitly while keeping a dependent_false
// catch-all that makes any genuinely-unhandled tag a compile error.
template<typename T>
struct IsPlaceholderConstructorTag : std::false_type
{
};

template<typename T>
inline constexpr bool is_placeholder_constructor_tag_v = IsPlaceholderConstructorTag<T>::value;

template<typename T>
concept BaseConceptConstructorTag =
    std::same_as<T, BotTag> || std::same_as<T, TopTag> || is_atomic_state_tag_v<T> || is_atomic_goal_tag_v<T> || std::same_as<T, IntersectionTag>
    || std::same_as<T, UnionTag> || std::same_as<T, NegationTag> || std::same_as<T, ValueRestrictionTag> || std::same_as<T, ExistentialQuantificationTag>
    || std::same_as<T, AtLeastNumberRestrictionTag> || std::same_as<T, AtMostNumberRestrictionTag> || std::same_as<T, ExactNumberRestrictionTag>
    || std::same_as<T, QualifiedAtLeastNumberRestrictionTag> || std::same_as<T, QualifiedAtMostNumberRestrictionTag>
    || std::same_as<T, QualifiedExactNumberRestrictionTag> || std::same_as<T, RoleValueMapTag> || std::same_as<T, AgreementTag>
    || std::same_as<T, RoleFillersTag> || std::same_as<T, OneOfTag> || std::same_as<T, NominalTag>;

template<typename T>
concept BaseRoleConstructorTag =
    std::same_as<T, UniversalTag> || is_atomic_state_tag_v<T> || is_atomic_goal_tag_v<T> || std::same_as<T, IntersectionTag> || std::same_as<T, UnionTag>
    || std::same_as<T, ComplementTag> || std::same_as<T, InverseTag> || std::same_as<T, CompositionTag> || std::same_as<T, TransitiveClosureTag>
    || std::same_as<T, ReflexiveTransitiveClosureTag> || std::same_as<T, RestrictionTag> || std::same_as<T, IdentityTag>;

template<typename T>
concept BaseBooleanConstructorTag = is_atomic_state_tag_v<T> || is_atomic_goal_tag_v<T> || std::same_as<T, NonemptyTag>;

template<typename T>
concept BaseNumericalConstructorTag = std::same_as<T, CountTag> || std::same_as<T, DistanceTag>;

using BaseConceptConstructorTags = ygg::TypeList<BotTag,
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

using BaseRoleConstructorTags = ygg::TypeList<UniversalTag,
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

using BaseBooleanConstructorTags = ygg::TypeList<AtomicStateTag<tyr::formalism::StaticTag>,
                                                 AtomicStateTag<tyr::formalism::FluentTag>,
                                                 AtomicStateTag<tyr::formalism::DerivedTag>,
                                                 AtomicGoalTag<tyr::formalism::StaticTag>,
                                                 AtomicGoalTag<tyr::formalism::FluentTag>,
                                                 AtomicGoalTag<tyr::formalism::DerivedTag>,
                                                 NonemptyTag>;

using BaseNumericalConstructorTags = ygg::TypeList<CountTag, DistanceTag>;

template<FamilyTag Family, typename T>
struct IsFamilyConceptConstructorTag : std::false_type
{
};

template<FamilyTag Family, typename T>
struct IsFamilyRoleConstructorTag : std::false_type
{
};

template<FamilyTag Family, typename T>
struct IsFamilyBooleanConstructorTag : std::false_type
{
};

template<FamilyTag Family, typename T>
struct IsFamilyNumericalConstructorTag : std::false_type
{
};

template<typename Family, typename T>
concept FamilyConceptConstructorTag = FamilyTag<Family> && IsFamilyConceptConstructorTag<Family, T>::value;

template<typename Family, typename T>
concept FamilyRoleConstructorTag = FamilyTag<Family> && IsFamilyRoleConstructorTag<Family, T>::value;

template<typename Family, typename T>
concept FamilyBooleanConstructorTag = FamilyTag<Family> && IsFamilyBooleanConstructorTag<Family, T>::value;

template<typename Family, typename T>
concept FamilyNumericalConstructorTag = FamilyTag<Family> && IsFamilyNumericalConstructorTag<Family, T>::value;

template<FamilyTag Family>
struct ConstructorTagLists;

template<FamilyTag Family>
using FamilyConceptConstructorTags = typename ConstructorTagLists<Family>::Concept;

template<FamilyTag Family>
using FamilyRoleConstructorTags = typename ConstructorTagLists<Family>::Role;

template<FamilyTag Family>
using FamilyBooleanConstructorTags = typename ConstructorTagLists<Family>::Boolean;

template<FamilyTag Family>
using FamilyNumericalConstructorTags = typename ConstructorTagLists<Family>::Numerical;

template<FamilyTag Family, typename Tag>
    requires FamilyConceptConstructorTag<Family, Tag>
struct Concept;

template<FamilyTag Family, typename Tag>
    requires FamilyRoleConstructorTag<Family, Tag>
struct Role;

template<FamilyTag Family, typename Tag>
    requires FamilyBooleanConstructorTag<Family, Tag>
struct Boolean;

template<FamilyTag Family, typename Tag>
    requires FamilyNumericalConstructorTag<Family, Tag>
struct Numerical;

template<FamilyTag Family, CategoryTag Category>
struct Constructor;

template<FamilyTag Family>
struct ConstructorFamily
{
    template<typename Tag>
        requires FamilyConceptConstructorTag<Family, Tag>
    using Concept = runir::kr::dl::Concept<Family, Tag>;

    template<typename Tag>
        requires FamilyRoleConstructorTag<Family, Tag>
    using Role = runir::kr::dl::Role<Family, Tag>;

    template<typename Tag>
        requires FamilyBooleanConstructorTag<Family, Tag>
    using Boolean = runir::kr::dl::Boolean<Family, Tag>;

    template<typename Tag>
        requires FamilyNumericalConstructorTag<Family, Tag>
    using Numerical = runir::kr::dl::Numerical<Family, Tag>;

    template<CategoryTag Category>
    using Constructor = runir::kr::dl::Constructor<Family, Category>;
};

}

#endif
