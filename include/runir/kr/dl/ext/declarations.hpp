#ifndef RUNIR_KR_DL_EXT_DECLARATIONS_HPP_
#define RUNIR_KR_DL_EXT_DECLARATIONS_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <stdexcept>
#include <tyr/common/index_mixins.hpp>
#include <tyr/common/type_list.hpp>
#include <tyr/common/types.hpp>

namespace runir::kr::dl
{

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
concept ExtConceptConstructorTag = BaseConceptConstructorTag<T> || std::same_as<T, RegisterTag> || std::same_as<T, ArgumentTag<ConceptTag>>;

template<typename T>
concept ExtRoleConstructorTag = BaseRoleConstructorTag<T> || std::same_as<T, RegisterTag> || std::same_as<T, ArgumentTag<RoleTag>>;

template<typename T>
concept ExtBooleanConstructorTag = BaseBooleanConstructorTag<T> || std::same_as<T, ArgumentTag<BooleanTag>>;

template<typename T>
concept ExtNumericalConstructorTag = BaseNumericalConstructorTag<T> || std::same_as<T, ArgumentTag<NumericalTag>>;

template<typename T>
struct IsFamilyConceptConstructorTag<runir::kr::ExtFamilyTag, T> : std::bool_constant<ExtConceptConstructorTag<T>>
{
};

template<typename T>
struct IsFamilyRoleConstructorTag<runir::kr::ExtFamilyTag, T> : std::bool_constant<ExtRoleConstructorTag<T>>
{
};

template<typename T>
struct IsFamilyBooleanConstructorTag<runir::kr::ExtFamilyTag, T> : std::bool_constant<ExtBooleanConstructorTag<T>>
{
};

template<typename T>
struct IsFamilyNumericalConstructorTag<runir::kr::ExtFamilyTag, T> : std::bool_constant<ExtNumericalConstructorTag<T>>
{
};

using ExtConceptConstructorTags = tyr::ConcatTypeListsT<BaseConceptConstructorTags, tyr::TypeList<RegisterTag, ArgumentTag<ConceptTag>>>;
using ExtRoleConstructorTags = tyr::ConcatTypeListsT<BaseRoleConstructorTags, tyr::TypeList<RegisterTag, ArgumentTag<RoleTag>>>;
using ExtBooleanConstructorTags = tyr::ConcatTypeListsT<BaseBooleanConstructorTags, tyr::TypeList<ArgumentTag<BooleanTag>>>;
using ExtNumericalConstructorTags = tyr::ConcatTypeListsT<BaseNumericalConstructorTags, tyr::TypeList<ArgumentTag<NumericalTag>>>;

template<>
struct ConstructorTagLists<runir::kr::ExtFamilyTag>
{
    using Concept = ExtConceptConstructorTags;
    using Role = ExtRoleConstructorTags;
    using Boolean = ExtBooleanConstructorTags;
    using Numerical = ExtNumericalConstructorTags;
};

}  // namespace runir::kr::dl

#endif
