#ifndef RUNIR_KR_DL_UNS_DECLARATIONS_HPP_
#define RUNIR_KR_DL_UNS_DECLARATIONS_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <concepts>
#include <string_view>
#include <yggdrasil/core/type_list.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::dl
{

/**
 * Comparison operators
 *
 * A comparison constructor always produces a Boolean. Its two operands are constructors of a fixed
 * operand category (BooleanTag or NumericalTag). The operator itself is shared between operand
 * categories: a single ComparisonOperatorTag template, with named alias templates per operator.
 */

enum class Comparator
{
    Eq,
    Neq,
    Lt,
    Le,
    Gt,
    Ge
};

struct ComparisonTagMarker
{
};

template<Comparator Op, CategoryTag Operand>
    requires(std::same_as<Operand, BooleanTag> || std::same_as<Operand, NumericalTag>)
struct ComparisonOperatorTag : ComparisonTagMarker
{
    static constexpr Comparator comparator = Op;
    using OperandCategory = Operand;
};

template<CategoryTag Operand>
using EqTag = ComparisonOperatorTag<Comparator::Eq, Operand>;
template<CategoryTag Operand>
using NeqTag = ComparisonOperatorTag<Comparator::Neq, Operand>;
template<CategoryTag Operand>
using LtTag = ComparisonOperatorTag<Comparator::Lt, Operand>;
template<CategoryTag Operand>
using LeTag = ComparisonOperatorTag<Comparator::Le, Operand>;
template<CategoryTag Operand>
using GtTag = ComparisonOperatorTag<Comparator::Gt, Operand>;
template<CategoryTag Operand>
using GeTag = ComparisonOperatorTag<Comparator::Ge, Operand>;

template<typename T>
concept ComparisonTag = std::derived_from<T, ComparisonTagMarker>;

template<ComparisonTag T>
using comparison_operand_t = typename T::OperandCategory;

/**
 * Constants
 */

struct BooleanConstantTag
{
};

struct NumericalConstantTag
{
};

/**
 * Canonical keywords (single source of truth for parsing and formatting).
 */

inline constexpr const char* boolean_constant_keyword = "b_const";
inline constexpr const char* numerical_constant_keyword = "n_const";

template<ComparisonTag Tag>
constexpr const char* comparison_keyword() noexcept
{
    constexpr auto op = Tag::comparator;
    if constexpr (std::same_as<comparison_operand_t<Tag>, BooleanTag>)
    {
        if constexpr (op == Comparator::Eq)
            return "b_eq";
        else if constexpr (op == Comparator::Neq)
            return "b_neq";
        else if constexpr (op == Comparator::Lt)
            return "b_lt";
        else if constexpr (op == Comparator::Le)
            return "b_le";
        else if constexpr (op == Comparator::Gt)
            return "b_gt";
        else
            return "b_ge";
    }
    else
    {
        if constexpr (op == Comparator::Eq)
            return "n_eq";
        else if constexpr (op == Comparator::Neq)
            return "n_neq";
        else if constexpr (op == Comparator::Lt)
            return "n_lt";
        else if constexpr (op == Comparator::Le)
            return "n_le";
        else if constexpr (op == Comparator::Gt)
            return "n_gt";
        else
            return "n_ge";
    }
}

/**
 * Constructor tag concepts for the Uns family (Base + comparisons + constants).
 */

template<typename T>
concept UnsConceptConstructorTag = BaseConceptConstructorTag<T>;

template<typename T>
concept UnsRoleConstructorTag = BaseRoleConstructorTag<T>;

template<typename T>
concept UnsBooleanConstructorTag = BaseBooleanConstructorTag<T> || ComparisonTag<T> || std::same_as<T, BooleanConstantTag>;

template<typename T>
concept UnsNumericalConstructorTag = BaseNumericalConstructorTag<T> || std::same_as<T, NumericalConstantTag>;

template<typename T>
struct IsFamilyConceptConstructorTag<runir::kr::UnsFamilyTag, T> : std::bool_constant<UnsConceptConstructorTag<T>>
{
};

template<typename T>
struct IsFamilyRoleConstructorTag<runir::kr::UnsFamilyTag, T> : std::bool_constant<UnsRoleConstructorTag<T>>
{
};

template<typename T>
struct IsFamilyBooleanConstructorTag<runir::kr::UnsFamilyTag, T> : std::bool_constant<UnsBooleanConstructorTag<T>>
{
};

template<typename T>
struct IsFamilyNumericalConstructorTag<runir::kr::UnsFamilyTag, T> : std::bool_constant<UnsNumericalConstructorTag<T>>
{
};

/**
 * Constructor tag lists for the Uns family.
 */

using UnsComparisonConstructorTags = ygg::TypeList<EqTag<BooleanTag>,
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

using UnsConceptConstructorTags = BaseConceptConstructorTags;
using UnsRoleConstructorTags = BaseRoleConstructorTags;
using UnsBooleanConstructorTags =
    ygg::ConcatTypeListsT<BaseBooleanConstructorTags, ygg::ConcatTypeListsT<UnsComparisonConstructorTags, ygg::TypeList<BooleanConstantTag>>>;
using UnsNumericalConstructorTags = ygg::ConcatTypeListsT<BaseNumericalConstructorTags, ygg::TypeList<NumericalConstantTag>>;

template<>
struct ConstructorTagLists<runir::kr::UnsFamilyTag>
{
    using Concept = UnsConceptConstructorTags;
    using Role = UnsRoleConstructorTags;
    using Boolean = UnsBooleanConstructorTags;
    using Numerical = UnsNumericalConstructorTags;
};

}  // namespace runir::kr::dl

#endif
