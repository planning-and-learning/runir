#ifndef RUNIR_KR_DL_EXT_DECLARATIONS_HPP_
#define RUNIR_KR_DL_EXT_DECLARATIONS_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <stdexcept>
#include <yggdrasil/ids/index_mixins.hpp>
#include <yggdrasil/core/type_list.hpp>
#include <yggdrasil/core/types.hpp>

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

using ExtConceptConstructorTags = ygg::ConcatTypeListsT<BaseConceptConstructorTags, ygg::TypeList<RegisterTag, ArgumentTag<ConceptTag>>>;
using ExtRoleConstructorTags = ygg::ConcatTypeListsT<BaseRoleConstructorTags, ygg::TypeList<RegisterTag, ArgumentTag<RoleTag>>>;
using ExtBooleanConstructorTags = ygg::ConcatTypeListsT<BaseBooleanConstructorTags, ygg::TypeList<ArgumentTag<BooleanTag>>>;
using ExtNumericalConstructorTags = ygg::ConcatTypeListsT<BaseNumericalConstructorTags, ygg::TypeList<ArgumentTag<NumericalTag>>>;

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
