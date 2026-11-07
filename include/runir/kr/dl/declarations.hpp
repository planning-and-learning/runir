#ifndef RUNIR_DECLARATIONS_HPP_
#define RUNIR_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"

#include <concepts>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <tyr/formalism/declarations.hpp>
#include <boost/mp11.hpp>
#include <urd/elements/flat.hpp>

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

using CategoryTags = boost::mp11::mp_list<ConceptTag, RoleTag, BooleanTag, NumericalTag>;
using ConceptOrRoleTags = boost::mp11::mp_list<ConceptTag, RoleTag>;
using BooleanOrNumericalTags = boost::mp11::mp_list<BooleanTag, NumericalTag>;

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
using RegisterIdentifier = urd::index<RegisterSlot<Category>>;

template<CategoryTag Category>
using ArgumentIdentifier = urd::index<ArgumentSlot<Category>>;

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

using QueryConstructorTags = boost::mp11::mp_list<AtomicStateTag<tyr::formalism::StaticTag>,
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

using BaseConceptConstructorTags = boost::mp11::mp_list<BotTag,
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

using BaseRoleConstructorTags = boost::mp11::mp_list<UniversalTag,
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

using BaseBooleanConstructorTags = boost::mp11::mp_list<AtomicStateTag<tyr::formalism::StaticTag>,
                                                 AtomicStateTag<tyr::formalism::FluentTag>,
                                                 AtomicStateTag<tyr::formalism::DerivedTag>,
                                                 AtomicGoalTag<tyr::formalism::StaticTag>,
                                                 AtomicGoalTag<tyr::formalism::FluentTag>,
                                                 AtomicGoalTag<tyr::formalism::DerivedTag>,
                                                 NonemptyTag>;

using BaseNumericalConstructorTags = boost::mp11::mp_list<CountTag, DistanceTag>;

using NumericalBinaryConstructorTags = boost::mp11::mp_list<AddTag, SubTag, MulTag, DivTag, MinTag, MaxTag>;
using ArithmeticConstructorTags = boost::mp11::mp_append<boost::mp11::mp_list<NumericalConstantTag>, NumericalBinaryConstructorTags>;

using ComparisonConstructorTags = boost::mp11::mp_list<EqTag<BooleanTag>,
                                                NeqTag<BooleanTag>,
                                                LtTag<BooleanTag>,
                                                LeTag<BooleanTag>,
                                                GtTag<BooleanTag>,
                                                GeTag<BooleanTag>,
                                                EqTag<NumericalTag>,
                                                NeqTag<NumericalTag>,
                                                LtTag<NumericalTag>,
                                                LeTag<NumericalTag>,
                                                GtTag<NumericalTag>,
                                                GeTag<NumericalTag>>;

using LogicalConstructorTags = boost::mp11::mp_list<AndTag, OrTag, NotTag>;

/// Every language shares one constructor set; a language's parser rejects what it does not support.
using ConceptConstructorTags = boost::mp11::mp_append<BaseConceptConstructorTags, boost::mp11::mp_list<RegisterTag, ArgumentTag<ConceptTag>>>;
using RoleConstructorTags = boost::mp11::mp_append<BaseRoleConstructorTags, boost::mp11::mp_list<RegisterTag, ArgumentTag<RoleTag>>>;
using BooleanConstructorTags = boost::mp11::mp_append<BaseBooleanConstructorTags,
                                                     ComparisonConstructorTags,
                                                     boost::mp11::mp_list<BooleanConstantTag>,
                                                     LogicalConstructorTags,
                                                     boost::mp11::mp_list<ArgumentTag<BooleanTag>>>;
using NumericalConstructorTags = boost::mp11::mp_append<BaseNumericalConstructorTags, ArithmeticConstructorTags, boost::mp11::mp_list<ArgumentTag<NumericalTag>>>;

template<typename T>
concept ConceptConstructorTag = boost::mp11::mp_contains<ConceptConstructorTags, T>::value;

template<typename T>
concept RoleConstructorTag = boost::mp11::mp_contains<RoleConstructorTags, T>::value;

template<typename T>
concept BooleanConstructorTag = boost::mp11::mp_contains<BooleanConstructorTags, T>::value;

template<typename T>
concept NumericalConstructorTag = boost::mp11::mp_contains<NumericalConstructorTags, T>::value;

template<typename T>
concept BaseConceptConstructorTag = boost::mp11::mp_contains<BaseConceptConstructorTags, T>::value;

template<typename T>
concept BaseRoleConstructorTag = boost::mp11::mp_contains<BaseRoleConstructorTags, T>::value;

template<typename T>
concept BaseBooleanConstructorTag = boost::mp11::mp_contains<BaseBooleanConstructorTags, T>::value;

template<typename T>
concept BaseNumericalConstructorTag = boost::mp11::mp_contains<BaseNumericalConstructorTags, T>::value;

template<typename Tag>
    requires ConceptConstructorTag<Tag>
struct Concept;

template<typename Tag>
    requires RoleConstructorTag<Tag>
struct Role;

template<typename Tag>
    requires BooleanConstructorTag<Tag>
struct Boolean;

template<typename Tag>
    requires NumericalConstructorTag<Tag>
struct Numerical;

template<CategoryTag Category>
struct Constructor;

namespace detail
{

template<typename RepositoryTypes>
class BasicConstructorRepository;

template<typename RepositoryTypes>
class BasicConstructorRepositoryFactory;

}  // namespace detail

using ConceptTypes = boost::mp11::mp_transform<Concept, ConceptConstructorTags>;

using RoleTypes = boost::mp11::mp_transform<Role, RoleConstructorTags>;

using BooleanTypes = boost::mp11::mp_transform<Boolean, BooleanConstructorTags>;

using NumericalTypes = boost::mp11::mp_transform<Numerical, NumericalConstructorTags>;

using ConstructorTypes = boost::mp11::mp_transform<Constructor, CategoryTags>;

using QueryTypes = boost::mp11::mp_transform<Query, QueryConstructorTags>;

using ReferenceTypes =
    boost::mp11::mp_list<Argument<ConceptTag>, Argument<RoleTag>, Argument<BooleanTag>, Argument<NumericalTag>, Register<ConceptTag>, Register<RoleTag>>;

using ConstructorRepositoryTypes = boost::mp11::mp_append<ConceptTypes,
                                                         RoleTypes,
                                                         BooleanTypes,
                                                         NumericalTypes,
                                                         ConstructorTypes,
                                                         ReferenceTypes,
                                                         QueryTypes,
                                                         boost::mp11::mp_list<Query<>, QueryColumn, QueryProjection<ConceptTag>, QueryProjection<RoleTag>>>;

using ConstructorRepository = runir::kr::dl::detail::BasicConstructorRepository<ConstructorRepositoryTypes>;

using ConstructorRepositoryPtr = std::shared_ptr<ConstructorRepository>;

using ConstructorRepositoryFactory = runir::kr::dl::detail::BasicConstructorRepositoryFactory<ConstructorRepositoryTypes>;

template<typename Tag>
    requires ConceptConstructorTag<Tag>
using ConceptView = urd::index_view<Concept<Tag>, ConstructorRepository>;

template<typename Tag>
    requires RoleConstructorTag<Tag>
using RoleView = urd::index_view<Role<Tag>, ConstructorRepository>;

template<typename Tag>
    requires BooleanConstructorTag<Tag>
using BooleanView = urd::index_view<Boolean<Tag>, ConstructorRepository>;

template<typename Tag>
    requires NumericalConstructorTag<Tag>
using NumericalView = urd::index_view<Numerical<Tag>, ConstructorRepository>;

template<CategoryTag Category>
using ConstructorView = urd::index_view<Constructor<Category>, ConstructorRepository>;

template<typename Tag = void>
using QueryView = urd::index_view<Query<Tag>, ConstructorRepository>;

template<CategoryTag Category>
using ArgumentView = urd::index_view<Argument<Category>, ConstructorRepository>;

template<ConceptOrRoleTag Category>
using RegisterView = urd::index_view<Register<Category>, ConstructorRepository>;
}

#endif
