#ifndef RUNIR_DECLARATIONS_HPP_
#define RUNIR_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"

#include <concepts>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <tyr/formalism/declarations.hpp>
#include <yggdrasil/core/type_list.hpp>
#include <yggdrasil/core/types.hpp>

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

template<typename T>
concept ConceptOrRoleTag = std::same_as<T, ConceptTag> || std::same_as<T, RoleTag>;

template<typename T>
concept BooleanOrNumericalTag = std::same_as<T, BooleanTag> || std::same_as<T, NumericalTag>;

using CategoryTags = ygg::TypeList<ConceptTag, RoleTag, BooleanTag, NumericalTag>;
using ConceptOrRoleTags = ygg::TypeList<ConceptTag, RoleTag>;
using BooleanOrNumericalTags = ygg::TypeList<BooleanTag, NumericalTag>;

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

/// Fact kind of an atomic state or goal tag.
template<typename Tag>
using TagFactKind = typename Tag::FactKind;

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

template<CategoryTag Category>
struct Argument
{
};

template<ConceptOrRoleTag Category>
struct Register
{
};

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

/// Register and argument positions; `Register` and `Argument` name their constructors.
template<ConceptOrRoleTag Category>
struct RegisterSlot
{
};

template<CategoryTag Category>
struct ArgumentSlot
{
};

template<ConceptOrRoleTag Category>
using RegisterIdentifier = ygg::Index<RegisterSlot<Category>>;

template<CategoryTag Category>
using ArgumentIdentifier = ygg::Index<ArgumentSlot<Category>>;

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

// Relational expressions are intermediate queries, not a new feature category.
struct QueryColumn;

template<typename Tag = void>
struct Query;

template<ConceptOrRoleTag Category>
struct QueryProjection;

struct ProjectTag
{
};

struct ConceptProjectSyntaxTag
{
    static constexpr auto keyword = "c_project";
};

struct RoleProjectSyntaxTag
{
    static constexpr auto keyword = "r_project";
};

struct QueryConceptTag
{
    static constexpr auto keyword = "q_concept";
};

struct QueryRoleTag
{
    static constexpr auto keyword = "q_role";
};

struct QueryJoinTag
{
    static constexpr auto keyword = "q_join";
};

struct QueryProjectTag
{
    static constexpr auto keyword = "q_project";
};

struct QueryRenameTag
{
    static constexpr auto keyword = "q_rename";
};

struct QuerySelectEqualTag
{
    static constexpr auto keyword = "q_select_equal";
};

struct QuerySelectValueTag
{
    static constexpr auto keyword = "q_select_value";
};

struct QueryUnionTag
{
    static constexpr auto keyword = "q_union";
};

struct QueryDifferenceTag
{
    static constexpr auto keyword = "q_difference";
};

struct QueryGenericJoinTag
{
    static constexpr auto keyword = "q_generic_join";
};

struct QueryEmptyTag
{
    static constexpr auto keyword = "q_empty";
};

struct QueryMustPassTag
{
    static constexpr auto keyword = "q_must_pass";
};

using QueryConstructorTags = ygg::TypeList<AtomicStateTag<tyr::formalism::StaticTag>,
                                           AtomicStateTag<tyr::formalism::FluentTag>,
                                           AtomicStateTag<tyr::formalism::DerivedTag>,
                                           AtomicGoalTag<tyr::formalism::StaticTag>,
                                           AtomicGoalTag<tyr::formalism::FluentTag>,
                                           AtomicGoalTag<tyr::formalism::DerivedTag>,
                                           QueryConceptTag,
                                           QueryRoleTag,
                                           QueryJoinTag,
                                           QueryProjectTag,
                                           QueryRenameTag,
                                           QuerySelectEqualTag,
                                           QuerySelectValueTag,
                                           QueryUnionTag,
                                           QueryDifferenceTag,
                                           QueryGenericJoinTag,
                                           QueryEmptyTag,
                                           QueryMustPassTag>;

}  // namespace runir::kr::dl

#include "runir/kr/dl/family_traits.hpp"

namespace runir::kr::dl
{

using FamilyConceptConstructorTags = ExtConceptConstructorTags;

using FamilyRoleConstructorTags = ExtRoleConstructorTags;

using FamilyBooleanConstructorTags = ExtBooleanConstructorTags;

using FamilyNumericalConstructorTags = ExtNumericalConstructorTags;

template<typename T>
concept FamilyConceptConstructorTag = ygg::InTypeList<T, FamilyConceptConstructorTags>;

template<typename T>
concept FamilyRoleConstructorTag = ygg::InTypeList<T, FamilyRoleConstructorTags>;

template<typename T>
concept FamilyBooleanConstructorTag = ygg::InTypeList<T, FamilyBooleanConstructorTags>;

template<typename T>
concept FamilyNumericalConstructorTag = ygg::InTypeList<T, FamilyNumericalConstructorTags>;

template<typename T>
concept BaseConceptConstructorTag = FamilyConceptConstructorTag<T>;

template<typename T>
concept BaseRoleConstructorTag = FamilyRoleConstructorTag<T>;

template<typename T>
concept BaseBooleanConstructorTag = FamilyBooleanConstructorTag<T>;

template<typename T>
concept BaseNumericalConstructorTag = FamilyNumericalConstructorTag<T>;

template<typename T>
concept ExtConceptConstructorTag = FamilyConceptConstructorTag<T>;

template<typename T>
concept ExtRoleConstructorTag = FamilyRoleConstructorTag<T>;

template<typename T>
concept ExtBooleanConstructorTag = FamilyBooleanConstructorTag<T>;

template<typename T>
concept ExtNumericalConstructorTag = FamilyNumericalConstructorTag<T>;

template<typename T>
concept UnsConceptConstructorTag = FamilyConceptConstructorTag<T>;

template<typename T>
concept UnsRoleConstructorTag = FamilyRoleConstructorTag<T>;

template<typename T>
concept UnsBooleanConstructorTag = FamilyBooleanConstructorTag<T>;

template<typename T>
concept UnsNumericalConstructorTag = FamilyNumericalConstructorTag<T>;

template<typename Tag>
    requires FamilyConceptConstructorTag<Tag>
struct Concept;

template<typename Tag>
    requires FamilyRoleConstructorTag<Tag>
struct Role;

template<typename Tag>
    requires FamilyBooleanConstructorTag<Tag>
struct Boolean;

template<typename Tag>
    requires FamilyNumericalConstructorTag<Tag>
struct Numerical;

template<CategoryTag Category>
struct Constructor;

namespace detail
{

template<typename RepositoryTypes>
class ConstructorRepository;

template<typename RepositoryTypes>
class ConstructorRepositoryFactory;

}  // namespace detail

using FamilyConceptTypes = ygg::MapTypeListT<Concept, FamilyConceptConstructorTags>;

using FamilyRoleTypes = ygg::MapTypeListT<Role, FamilyRoleConstructorTags>;

using FamilyBooleanTypes = ygg::MapTypeListT<Boolean, FamilyBooleanConstructorTags>;

using FamilyNumericalTypes = ygg::MapTypeListT<Numerical, FamilyNumericalConstructorTags>;

using FamilyConstructorTypes = ygg::MapTypeListT<Constructor, CategoryTags>;

using FamilyQueryTypes = ygg::MapTypeListT<Query, QueryConstructorTags>;

using FamilyReferenceTypes = std::conditional_t<
    true,
    ygg::TypeList<Argument<ConceptTag>, Argument<RoleTag>, Argument<BooleanTag>, Argument<NumericalTag>, Register<ConceptTag>, Register<RoleTag>>,
    ygg::TypeList<>>;

using FamilyConstructorRepositoryTypes = ygg::ConcatTypeListsT<FamilyConceptTypes,
                                                               FamilyRoleTypes,
                                                               FamilyBooleanTypes,
                                                               FamilyNumericalTypes,
                                                               FamilyConstructorTypes,
                                                               FamilyReferenceTypes,
                                                               FamilyQueryTypes,
                                                               ygg::TypeList<Query<>, QueryColumn, QueryProjection<ConceptTag>, QueryProjection<RoleTag>>>;

using BasicConstructorRepository = runir::kr::dl::detail::ConstructorRepository<FamilyConstructorRepositoryTypes>;

using BasicConstructorRepositoryFactory = runir::kr::dl::detail::ConstructorRepositoryFactory<FamilyConstructorRepositoryTypes>;

using ConstructorRepositoryFor = BasicConstructorRepository;

using ConstructorRepositoryPtrFor = std::shared_ptr<ConstructorRepositoryFor>;

using ConstructorRepositoryFactoryFor = BasicConstructorRepositoryFactory;

template<typename Tag>
    requires FamilyConceptConstructorTag<Tag>
using FamilyConceptView = ygg::View<ygg::Index<Concept<Tag>>, ConstructorRepositoryFor>;

template<typename Tag>
    requires FamilyRoleConstructorTag<Tag>
using FamilyRoleView = ygg::View<ygg::Index<Role<Tag>>, ConstructorRepositoryFor>;

template<typename Tag>
    requires FamilyBooleanConstructorTag<Tag>
using FamilyBooleanView = ygg::View<ygg::Index<Boolean<Tag>>, ConstructorRepositoryFor>;

template<typename Tag>
    requires FamilyNumericalConstructorTag<Tag>
using FamilyNumericalView = ygg::View<ygg::Index<Numerical<Tag>>, ConstructorRepositoryFor>;

template<CategoryTag Category>
using FamilyConstructorView = ygg::View<ygg::Index<Constructor<Category>>, ConstructorRepositoryFor>;

template<typename Tag = void>
using FamilyQueryView = ygg::View<ygg::Index<Query<Tag>>, ConstructorRepositoryFor>;

using BaseConstructorRepository = ConstructorRepositoryFor;
using BaseConstructorRepositoryPtr = ConstructorRepositoryPtrFor;
using BaseConstructorRepositoryFactory = ConstructorRepositoryFactoryFor;

using ExtConstructorRepository = ConstructorRepositoryFor;
using ExtConstructorRepositoryPtr = ConstructorRepositoryPtrFor;
using ExtConstructorRepositoryFactory = ConstructorRepositoryFactoryFor;

template<CategoryTag Category>
using ArgumentView = ygg::View<ygg::Index<Argument<Category>>, ExtConstructorRepository>;

template<ConceptOrRoleTag Category>
using RegisterView = ygg::View<ygg::Index<Register<Category>>, ExtConstructorRepository>;

using UnsConstructorRepository = ConstructorRepositoryFor;
using UnsConstructorRepositoryPtr = ConstructorRepositoryPtrFor;
using UnsConstructorRepositoryFactory = ConstructorRepositoryFactoryFor;

template<typename Tag>
    requires FamilyConceptConstructorTag<Tag>
using BaseConceptView = FamilyConceptView<Tag>;

template<typename Tag>
    requires FamilyRoleConstructorTag<Tag>
using BaseRoleView = FamilyRoleView<Tag>;

template<typename Tag>
    requires FamilyBooleanConstructorTag<Tag>
using BaseBooleanView = FamilyBooleanView<Tag>;

template<typename Tag>
    requires FamilyNumericalConstructorTag<Tag>
using BaseNumericalView = FamilyNumericalView<Tag>;

template<CategoryTag Category>
using BaseConstructorView = FamilyConstructorView<Category>;

template<typename Tag>
    requires FamilyConceptConstructorTag<Tag>
using ExtConceptView = FamilyConceptView<Tag>;

template<typename Tag>
    requires FamilyRoleConstructorTag<Tag>
using ExtRoleView = FamilyRoleView<Tag>;

template<typename Tag>
    requires FamilyBooleanConstructorTag<Tag>
using ExtBooleanView = FamilyBooleanView<Tag>;

template<typename Tag>
    requires FamilyNumericalConstructorTag<Tag>
using ExtNumericalView = FamilyNumericalView<Tag>;

template<CategoryTag Category>
using ExtConstructorView = FamilyConstructorView<Category>;

template<typename Tag>
    requires FamilyConceptConstructorTag<Tag>
using UnsConceptView = FamilyConceptView<Tag>;

template<typename Tag>
    requires FamilyRoleConstructorTag<Tag>
using UnsRoleView = FamilyRoleView<Tag>;

template<typename Tag>
    requires FamilyBooleanConstructorTag<Tag>
using UnsBooleanView = FamilyBooleanView<Tag>;

template<typename Tag>
    requires FamilyNumericalConstructorTag<Tag>
using UnsNumericalView = FamilyNumericalView<Tag>;

template<CategoryTag Category>
using UnsConstructorView = FamilyConstructorView<Category>;
}

#endif
