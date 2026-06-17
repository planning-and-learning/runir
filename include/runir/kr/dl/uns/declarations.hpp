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
 * Numerical binary operators
 *
 * A numerical binary operator takes two Numerical operands and produces a Numerical. A single
 * NumericalBinaryOperatorTag template with named alias templates per operator.
 */

enum class NumericalBinaryOperator
{
    Add,
    Subtract,
    Multiply,
    Divide,
    Min,
    Max
};

struct NumericalBinaryTagMarker
{
};

template<NumericalBinaryOperator Op>
struct NumericalBinaryOperatorTag : NumericalBinaryTagMarker
{
    static constexpr NumericalBinaryOperator op = Op;
};

using AddTag = NumericalBinaryOperatorTag<NumericalBinaryOperator::Add>;
using SubtractTag = NumericalBinaryOperatorTag<NumericalBinaryOperator::Subtract>;
using MultiplyTag = NumericalBinaryOperatorTag<NumericalBinaryOperator::Multiply>;
using DivideTag = NumericalBinaryOperatorTag<NumericalBinaryOperator::Divide>;
using MinTag = NumericalBinaryOperatorTag<NumericalBinaryOperator::Min>;
using MaxTag = NumericalBinaryOperatorTag<NumericalBinaryOperator::Max>;

template<typename T>
concept NumericalBinaryTag = std::derived_from<T, NumericalBinaryTagMarker>;

/**
 * Logical operators
 *
 * Binary logical operators (And, Or) take two Boolean operands; the unary Not takes one. All produce
 * a Boolean. (Boolean equivalence/xor are already available as b_eq / b_neq comparisons.)
 */

enum class LogicalBinaryOperator
{
    And,
    Or
};

struct LogicalBinaryTagMarker
{
};

template<LogicalBinaryOperator Op>
struct LogicalBinaryOperatorTag : LogicalBinaryTagMarker
{
    static constexpr LogicalBinaryOperator op = Op;
};

using AndTag = LogicalBinaryOperatorTag<LogicalBinaryOperator::And>;
using OrTag = LogicalBinaryOperatorTag<LogicalBinaryOperator::Or>;

template<typename T>
concept LogicalBinaryTag = std::derived_from<T, LogicalBinaryTagMarker>;

struct NotTag
{
};

/**
 * Canonical keywords (single source of truth for parsing and formatting).
 */

inline constexpr const char* boolean_constant_keyword = "b_const";
inline constexpr const char* numerical_constant_keyword = "n_const";
inline constexpr const char* logical_not_keyword = "b_not";

template<NumericalBinaryTag Tag>
constexpr const char* numerical_binary_keyword() noexcept
{
    constexpr auto op = Tag::op;
    if constexpr (op == NumericalBinaryOperator::Add)
        return "n_add";
    else if constexpr (op == NumericalBinaryOperator::Subtract)
        return "n_sub";
    else if constexpr (op == NumericalBinaryOperator::Multiply)
        return "n_mul";
    else if constexpr (op == NumericalBinaryOperator::Divide)
        return "n_div";
    else if constexpr (op == NumericalBinaryOperator::Min)
        return "n_min";
    else
        return "n_max";
}

template<LogicalBinaryTag Tag>
constexpr const char* logical_binary_keyword() noexcept
{
    constexpr auto op = Tag::op;
    if constexpr (op == LogicalBinaryOperator::And)
        return "b_and";
    else
        return "b_or";
}

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
concept UnsBooleanConstructorTag =
    BaseBooleanConstructorTag<T> || ComparisonTag<T> || std::same_as<T, BooleanConstantTag> || LogicalBinaryTag<T> || std::same_as<T, NotTag>;

template<typename T>
concept UnsNumericalConstructorTag = BaseNumericalConstructorTag<T> || std::same_as<T, NumericalConstantTag> || NumericalBinaryTag<T>;

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

using UnsLogicalConstructorTags = ygg::TypeList<AndTag, OrTag, NotTag>;
using UnsNumericalBinaryConstructorTags = ygg::TypeList<AddTag, SubtractTag, MultiplyTag, DivideTag, MinTag, MaxTag>;

using UnsConceptConstructorTags = BaseConceptConstructorTags;
using UnsRoleConstructorTags = BaseRoleConstructorTags;
using UnsBooleanConstructorTags = ygg::ConcatTypeListsT<
    BaseBooleanConstructorTags,
    ygg::ConcatTypeListsT<UnsComparisonConstructorTags, ygg::ConcatTypeListsT<ygg::TypeList<BooleanConstantTag>, UnsLogicalConstructorTags>>>;
using UnsNumericalConstructorTags =
    ygg::ConcatTypeListsT<BaseNumericalConstructorTags, ygg::ConcatTypeListsT<ygg::TypeList<NumericalConstantTag>, UnsNumericalBinaryConstructorTags>>;

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
