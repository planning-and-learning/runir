#ifndef RUNIR_DECLARATIONS_HPP_
#define RUNIR_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"

#include <concepts>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <tyr/common/index_mixins.hpp>
#include <tyr/common/type_list.hpp>
#include <tyr/common/types.hpp>
#include <tyr/formalism/declarations.hpp>

namespace runir::kr::dl
{

using BaseFamilyTag = runir::kr::BaseFamilyTag;
using ExtFamilyTag = runir::kr::ExtFamilyTag;

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

struct RegisterTag
{
};

template<CategoryTag Category>
struct ArgumentTag
{
};

inline constexpr size_t num_registers = 4;

template<CategoryTag Category>
    requires(std::same_as<Category, ConceptTag> || std::same_as<Category, RoleTag>)
struct RegisterIdentifier : tyr::IndexMixin<RegisterIdentifier<Category>>
{
    using Base = tyr::IndexMixin<RegisterIdentifier<Category>>;

    constexpr RegisterIdentifier() noexcept = default;
    explicit RegisterIdentifier(tyr::uint_t value) : Base(check_bounds(value)) {}

private:
    static constexpr tyr::uint_t check_bounds(tyr::uint_t value)
    {
        if (value != Base::MAX && value >= num_registers)
            throw std::out_of_range("Register identifier index is out of range.");
        return value;
    }
};

template<CategoryTag Category>
struct ArgumentIdentifier : tyr::IndexMixin<ArgumentIdentifier<Category>>
{
    using Base = tyr::IndexMixin<ArgumentIdentifier<Category>>;
    using Base::Base;
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
    || std::same_as<T, RoleFillersTag> || std::same_as<T, OneOfTag> || std::same_as<T, NominalTag> || std::same_as<T, RegisterTag>
    || std::same_as<T, ArgumentTag<ConceptTag>>;

template<typename T>
concept RoleConstructorTag = std::same_as<T, UniversalTag> || is_atomic_state_tag_v<T> || is_atomic_goal_tag_v<T> || std::same_as<T, IntersectionTag>
                             || std::same_as<T, UnionTag> || std::same_as<T, ComplementTag> || std::same_as<T, InverseTag> || std::same_as<T, CompositionTag>
                             || std::same_as<T, TransitiveClosureTag> || std::same_as<T, ReflexiveTransitiveClosureTag> || std::same_as<T, RestrictionTag>
                             || std::same_as<T, IdentityTag> || std::same_as<T, RegisterTag> || std::same_as<T, ArgumentTag<RoleTag>>;

template<typename T>
concept BooleanConstructorTag = is_atomic_state_tag_v<T> || is_atomic_goal_tag_v<T> || std::same_as<T, NonemptyTag> || std::same_as<T, ArgumentTag<BooleanTag>>;

template<typename T>
concept NumericalConstructorTag = std::same_as<T, CountTag> || std::same_as<T, DistanceTag> || std::same_as<T, ArgumentTag<NumericalTag>>;

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
                                             NominalTag,
                                             RegisterTag,
                                             ArgumentTag<ConceptTag>>;

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
                                          IdentityTag,
                                          RegisterTag,
                                          ArgumentTag<RoleTag>>;

using BooleanConstructorTags = tyr::TypeList<AtomicStateTag<tyr::formalism::StaticTag>,
                                             AtomicStateTag<tyr::formalism::FluentTag>,
                                             AtomicStateTag<tyr::formalism::DerivedTag>,
                                             AtomicGoalTag<tyr::formalism::StaticTag>,
                                             AtomicGoalTag<tyr::formalism::FluentTag>,
                                             AtomicGoalTag<tyr::formalism::DerivedTag>,
                                             NonemptyTag,
                                             ArgumentTag<BooleanTag>>;

using NumericalConstructorTags = tyr::TypeList<CountTag, DistanceTag, ArgumentTag<NumericalTag>>;

template<FamilyTag Family, ConceptConstructorTag Tag>
struct Concept;

template<FamilyTag Family, RoleConstructorTag Tag>
struct Role;

template<FamilyTag Family, BooleanConstructorTag Tag>
struct Boolean;

template<FamilyTag Family, NumericalConstructorTag Tag>
struct Numerical;

template<FamilyTag Family, CategoryTag Category>
struct Constructor;

template<FamilyTag Family>
struct ConstructorFamily
{
    template<ConceptConstructorTag Tag>
    using Concept = runir::kr::dl::Concept<Family, Tag>;

    template<RoleConstructorTag Tag>
    using Role = runir::kr::dl::Role<Family, Tag>;

    template<BooleanConstructorTag Tag>
    using Boolean = runir::kr::dl::Boolean<Family, Tag>;

    template<NumericalConstructorTag Tag>
    using Numerical = runir::kr::dl::Numerical<Family, Tag>;

    template<CategoryTag Category>
    using Constructor = runir::kr::dl::Constructor<Family, Category>;
};

}

#endif
