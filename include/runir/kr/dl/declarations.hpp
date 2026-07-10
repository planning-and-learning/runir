#ifndef RUNIR_DECLARATIONS_HPP_
#define RUNIR_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"

#include <concepts>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <tyr/formalism/declarations.hpp>
#include <yggdrasil/core/type_list.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/ids/index_mixins.hpp>

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
    static constexpr auto keyword = "c_bot";
};

struct TopTag
{
    static constexpr auto keyword = "c_top";
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
    static constexpr auto keyword = "c_and";
};

struct UnionTag
{
    static constexpr auto keyword = "c_or";
};

struct NegationTag
{
    static constexpr auto keyword = "c_not";
};

struct ValueRestrictionTag
{
    static constexpr auto keyword = "c_all";
};

struct ExistentialQuantificationTag
{
    static constexpr auto keyword = "c_some";
};

struct AtLeastNumberRestrictionTag
{
    static constexpr auto keyword = "c_at_least";
};

struct AtMostNumberRestrictionTag
{
    static constexpr auto keyword = "c_at_most";
};

struct ExactNumberRestrictionTag
{
    static constexpr auto keyword = "c_exactly";
};

struct QualifiedAtLeastNumberRestrictionTag
{
    static constexpr auto keyword = AtLeastNumberRestrictionTag::keyword;
};

struct QualifiedAtMostNumberRestrictionTag
{
    static constexpr auto keyword = AtMostNumberRestrictionTag::keyword;
};

struct QualifiedExactNumberRestrictionTag
{
    static constexpr auto keyword = ExactNumberRestrictionTag::keyword;
};

struct RoleValueMapTag
{
    static constexpr auto keyword = "c_subset";
};

struct AgreementTag
{
    static constexpr auto keyword = "c_same_as";
};

struct RoleFillersTag
{
    static constexpr auto keyword = "c_fillers";
};

struct OneOfTag
{
    static constexpr auto keyword = "c_one_of";
};

struct NominalTag
{
    static constexpr auto keyword = "c_nominal";
};

struct UniversalTag
{
    static constexpr auto keyword = "r_universal";
};

struct ComplementTag
{
    static constexpr auto keyword = "r_complement";
};

struct InverseTag
{
    static constexpr auto keyword = "r_inverse";
};

struct CompositionTag
{
    static constexpr auto keyword = "r_composition";
};

struct TransitiveClosureTag
{
    static constexpr auto keyword = "r_transitive_closure";
};

struct ReflexiveTransitiveClosureTag
{
    static constexpr auto keyword = "r_reflexive_transitive_closure";
};

struct RestrictionTag
{
    static constexpr auto keyword = "r_restriction";
};

struct IdentityTag
{
    static constexpr auto keyword = "r_identity";
};

struct NonemptyTag
{
    static constexpr auto keyword = "b_nonempty";
};

struct CountTag
{
    static constexpr auto keyword = "n_count";
};

struct DistanceTag
{
    static constexpr auto keyword = "n_distance";
};

struct ConceptAtomicStateSyntaxTag
{
    static constexpr auto keyword = "c_atomic_state";
};

struct ConceptAtomicGoalSyntaxTag
{
    static constexpr auto keyword = "c_atomic_goal";
};

struct RoleAtomicStateSyntaxTag
{
    static constexpr auto keyword = "r_atomic_state";
};

struct RoleAtomicGoalSyntaxTag
{
    static constexpr auto keyword = "r_atomic_goal";
};

struct BooleanAtomicStateSyntaxTag
{
    static constexpr auto keyword = "b_atomic_state";
};

struct BooleanAtomicGoalSyntaxTag
{
    static constexpr auto keyword = "b_atomic_goal";
};

struct ConceptIntersectionSyntaxTag
{
    static constexpr auto keyword = "c_and";
};

struct ConceptUnionSyntaxTag
{
    static constexpr auto keyword = "c_or";
};

struct RoleIntersectionSyntaxTag
{
    static constexpr auto keyword = "r_and";
};

struct RoleUnionSyntaxTag
{
    static constexpr auto keyword = "r_or";
};

struct ConceptRegisterSyntaxTag
{
    static constexpr auto keyword = "c_register";
};

struct RoleRegisterSyntaxTag
{
    static constexpr auto keyword = "r_register";
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

/**
 * Ext family constructors
 */

struct RegisterTag
{
};

template<CategoryTag Category>
struct ArgumentTag
{
    static constexpr auto keyword = []
    {
        if constexpr (std::same_as<Category, ConceptTag>)
            return "c_argument";
        else if constexpr (std::same_as<Category, RoleTag>)
            return "r_argument";
        else if constexpr (std::same_as<Category, BooleanTag>)
            return "b_argument";
        else
            return "n_argument";
    }();
};

inline constexpr size_t num_registers = 4;

template<CategoryTag Category>
struct RegisterIdentifier : ygg::IndexMixin<RegisterIdentifier<Category>>
{
    using Base = ygg::IndexMixin<RegisterIdentifier<Category>>;

    constexpr RegisterIdentifier() noexcept = default;
    explicit RegisterIdentifier(ygg::uint_t value) : Base(check_bounds(value)) {}

private:
    static constexpr ygg::uint_t check_bounds(ygg::uint_t value)
    {
        if (value != Base::MAX && value >= num_registers)
            throw std::out_of_range("Register identifier index is out of range.");
        return value;
    }
};

template<CategoryTag Category>
struct ArgumentIdentifier : ygg::IndexMixin<ArgumentIdentifier<Category>>
{
    using Base = ygg::IndexMixin<ArgumentIdentifier<Category>>;
    using Base::Base;
};

/**
 * Uns family constructors
 */

struct ComparisonTagMarker
{
};

struct BooleanEqTag : ComparisonTagMarker
{
    using OperandCategory = BooleanTag;
    static constexpr auto keyword = "b_eq";
};

struct BooleanNeTag : ComparisonTagMarker
{
    using OperandCategory = BooleanTag;
    static constexpr auto keyword = "b_neq";
};

struct BooleanLtTag : ComparisonTagMarker
{
    using OperandCategory = BooleanTag;
    static constexpr auto keyword = "b_lt";
};

struct BooleanLeTag : ComparisonTagMarker
{
    using OperandCategory = BooleanTag;
    static constexpr auto keyword = "b_le";
};

struct BooleanGtTag : ComparisonTagMarker
{
    using OperandCategory = BooleanTag;
    static constexpr auto keyword = "b_gt";
};

struct BooleanGeTag : ComparisonTagMarker
{
    using OperandCategory = BooleanTag;
    static constexpr auto keyword = "b_ge";
};

struct NumericalEqTag : ComparisonTagMarker
{
    using OperandCategory = NumericalTag;
    static constexpr auto keyword = "n_eq";
};

struct NumericalNeTag : ComparisonTagMarker
{
    using OperandCategory = NumericalTag;
    static constexpr auto keyword = "n_neq";
};

struct NumericalLtTag : ComparisonTagMarker
{
    using OperandCategory = NumericalTag;
    static constexpr auto keyword = "n_lt";
};

struct NumericalLeTag : ComparisonTagMarker
{
    using OperandCategory = NumericalTag;
    static constexpr auto keyword = "n_le";
};

struct NumericalGtTag : ComparisonTagMarker
{
    using OperandCategory = NumericalTag;
    static constexpr auto keyword = "n_gt";
};

struct NumericalGeTag : ComparisonTagMarker
{
    using OperandCategory = NumericalTag;
    static constexpr auto keyword = "n_ge";
};

template<CategoryTag Operand>
struct EqTagFor;

template<>
struct EqTagFor<BooleanTag>
{
    using Type = BooleanEqTag;
};

template<>
struct EqTagFor<NumericalTag>
{
    using Type = NumericalEqTag;
};

template<CategoryTag Operand>
using EqTag = typename EqTagFor<Operand>::Type;

template<CategoryTag Operand>
struct NeqTagFor;

template<>
struct NeqTagFor<BooleanTag>
{
    using Type = BooleanNeTag;
};

template<>
struct NeqTagFor<NumericalTag>
{
    using Type = NumericalNeTag;
};

template<CategoryTag Operand>
using NeqTag = typename NeqTagFor<Operand>::Type;

template<CategoryTag Operand>
struct LtTagFor;

template<>
struct LtTagFor<BooleanTag>
{
    using Type = BooleanLtTag;
};

template<>
struct LtTagFor<NumericalTag>
{
    using Type = NumericalLtTag;
};

template<CategoryTag Operand>
using LtTag = typename LtTagFor<Operand>::Type;

template<CategoryTag Operand>
struct LeTagFor;

template<>
struct LeTagFor<BooleanTag>
{
    using Type = BooleanLeTag;
};

template<>
struct LeTagFor<NumericalTag>
{
    using Type = NumericalLeTag;
};

template<CategoryTag Operand>
using LeTag = typename LeTagFor<Operand>::Type;

template<CategoryTag Operand>
struct GtTagFor;

template<>
struct GtTagFor<BooleanTag>
{
    using Type = BooleanGtTag;
};

template<>
struct GtTagFor<NumericalTag>
{
    using Type = NumericalGtTag;
};

template<CategoryTag Operand>
using GtTag = typename GtTagFor<Operand>::Type;

template<CategoryTag Operand>
struct GeTagFor;

template<>
struct GeTagFor<BooleanTag>
{
    using Type = BooleanGeTag;
};

template<>
struct GeTagFor<NumericalTag>
{
    using Type = NumericalGeTag;
};

template<CategoryTag Operand>
using GeTag = typename GeTagFor<Operand>::Type;

template<typename T>
concept ComparisonTag = std::derived_from<T, ComparisonTagMarker>;

template<ComparisonTag T>
using comparison_operand_t = typename T::OperandCategory;

struct BooleanConstantTag
{
    static constexpr auto keyword = "b_const";
};

struct NumericalConstantTag
{
    static constexpr auto keyword = "n_const";
};

struct NumericalBinaryTagMarker
{
};

struct AddTag : NumericalBinaryTagMarker
{
    static constexpr auto keyword = "n_add";
};

struct SubTag : NumericalBinaryTagMarker
{
    static constexpr auto keyword = "n_sub";
};

struct MulTag : NumericalBinaryTagMarker
{
    static constexpr auto keyword = "n_mul";
};

struct DivTag : NumericalBinaryTagMarker
{
    static constexpr auto keyword = "n_div";
};

struct MinTag : NumericalBinaryTagMarker
{
    static constexpr auto keyword = "n_min";
};

struct MaxTag : NumericalBinaryTagMarker
{
    static constexpr auto keyword = "n_max";
};

template<typename T>
concept NumericalBinaryTag = std::derived_from<T, NumericalBinaryTagMarker>;

struct LogicalBinaryTagMarker
{
};

struct AndTag : LogicalBinaryTagMarker
{
    static constexpr auto keyword = "b_and";
};

struct OrTag : LogicalBinaryTagMarker
{
    static constexpr auto keyword = "b_or";
};

template<typename T>
concept LogicalBinaryTag = std::derived_from<T, LogicalBinaryTagMarker>;

struct NotTag
{
    static constexpr auto keyword = "b_not";
};

template<typename T, typename List>
struct TypeListContains : std::false_type
{
};

template<typename T, typename... Ts>
struct TypeListContains<T, ygg::TypeList<Ts...>> : std::bool_constant<(std::same_as<T, Ts> || ...)>
{
};

template<FamilyTag Family>
struct DlFamilyTraits;

}  // namespace runir::kr::dl

#include "runir/kr/dl/family_traits.hpp"

namespace runir::kr::dl
{

template<FamilyTag Family>
using FamilyConceptConstructorTags = typename DlFamilyTraits<Family>::ConceptConstructorTags;

template<FamilyTag Family>
using FamilyRoleConstructorTags = typename DlFamilyTraits<Family>::RoleConstructorTags;

template<FamilyTag Family>
using FamilyBooleanConstructorTags = typename DlFamilyTraits<Family>::BooleanConstructorTags;

template<FamilyTag Family>
using FamilyNumericalConstructorTags = typename DlFamilyTraits<Family>::NumericalConstructorTags;

template<typename Family, typename T>
concept FamilyConceptConstructorTag = FamilyTag<Family> && TypeListContains<T, FamilyConceptConstructorTags<Family>>::value;

template<typename Family, typename T>
concept FamilyRoleConstructorTag = FamilyTag<Family> && TypeListContains<T, FamilyRoleConstructorTags<Family>>::value;

template<typename Family, typename T>
concept FamilyBooleanConstructorTag = FamilyTag<Family> && TypeListContains<T, FamilyBooleanConstructorTags<Family>>::value;

template<typename Family, typename T>
concept FamilyNumericalConstructorTag = FamilyTag<Family> && TypeListContains<T, FamilyNumericalConstructorTags<Family>>::value;

template<typename T>
concept BaseConceptConstructorTag = FamilyConceptConstructorTag<runir::kr::BaseFamilyTag, T>;

template<typename T>
concept BaseRoleConstructorTag = FamilyRoleConstructorTag<runir::kr::BaseFamilyTag, T>;

template<typename T>
concept BaseBooleanConstructorTag = FamilyBooleanConstructorTag<runir::kr::BaseFamilyTag, T>;

template<typename T>
concept BaseNumericalConstructorTag = FamilyNumericalConstructorTag<runir::kr::BaseFamilyTag, T>;

template<typename T>
concept ExtConceptConstructorTag = FamilyConceptConstructorTag<runir::kr::ExtFamilyTag, T>;

template<typename T>
concept ExtRoleConstructorTag = FamilyRoleConstructorTag<runir::kr::ExtFamilyTag, T>;

template<typename T>
concept ExtBooleanConstructorTag = FamilyBooleanConstructorTag<runir::kr::ExtFamilyTag, T>;

template<typename T>
concept ExtNumericalConstructorTag = FamilyNumericalConstructorTag<runir::kr::ExtFamilyTag, T>;

template<typename T>
concept UnsConceptConstructorTag = FamilyConceptConstructorTag<runir::kr::UnsFamilyTag, T>;

template<typename T>
concept UnsRoleConstructorTag = FamilyRoleConstructorTag<runir::kr::UnsFamilyTag, T>;

template<typename T>
concept UnsBooleanConstructorTag = FamilyBooleanConstructorTag<runir::kr::UnsFamilyTag, T>;

template<typename T>
concept UnsNumericalConstructorTag = FamilyNumericalConstructorTag<runir::kr::UnsFamilyTag, T>;

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

}

#endif
