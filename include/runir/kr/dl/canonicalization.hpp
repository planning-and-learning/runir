#ifndef RUNIR_CANONICALIZATION_HPP_
#define RUNIR_CANONICALIZATION_HPP_

#include "runir/kr/dl/constructors.hpp"
#include "runir/kr/dl/declarations.hpp"

#include <type_traits>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/semantics/comparison.hpp>

namespace runir::kr::dl
{

template<typename Data>
void canonicalize_commutative_binary(Data& data) noexcept
{
    using Lhs = std::remove_cvref_t<decltype(data.lhs)>;
    using Rhs = std::remove_cvref_t<decltype(data.rhs)>;
    static_assert(std::same_as<Lhs, Rhs>);

    if (data.rhs < data.lhs)
        std::swap(data.lhs, data.rhs);
}

template<FamilyTag Family, typename Tag>
    requires FamilyConceptConstructorTag<Family, Tag>
bool is_canonical(const ygg::Data<FamilyConcept<Family, Tag>>&) noexcept
{
    return true;
}

template<FamilyTag Family, typename Tag>
    requires FamilyRoleConstructorTag<Family, Tag>
bool is_canonical(const ygg::Data<FamilyRole<Family, Tag>>&) noexcept
{
    return true;
}

template<FamilyTag Family, typename Tag>
    requires FamilyBooleanConstructorTag<Family, Tag>
bool is_canonical(const ygg::Data<FamilyBoolean<Family, Tag>>&) noexcept
{
    return true;
}

template<FamilyTag Family, typename Tag>
    requires FamilyNumericalConstructorTag<Family, Tag>
bool is_canonical(const ygg::Data<FamilyNumerical<Family, Tag>>&) noexcept
{
    return true;
}

template<FamilyTag Family, CategoryTag Category>
bool is_canonical(const ygg::Data<FamilyConstructor<Family, Category>>&) noexcept
{
    return true;
}

template<CategoryTag Category>
bool is_canonical(const ygg::Data<Argument<Category>>&) noexcept
{
    return true;
}

template<CategoryTag Category>
bool is_canonical(const ygg::Data<Register<Category>>&) noexcept
{
    return true;
}

template<FamilyTag Family, typename Tag>
    requires FamilyConceptConstructorTag<Family, Tag>
void canonicalize(ygg::Data<FamilyConcept<Family, Tag>>&) noexcept
{
    // Trivially canonical
}

template<FamilyTag Family, typename Tag>
    requires FamilyRoleConstructorTag<Family, Tag>
void canonicalize(ygg::Data<FamilyRole<Family, Tag>>&) noexcept
{
    // Trivially canonical
}

template<FamilyTag Family, typename Tag>
    requires FamilyBooleanConstructorTag<Family, Tag>
void canonicalize(ygg::Data<FamilyBoolean<Family, Tag>>&) noexcept
{
    // Trivially canonical
}

template<FamilyTag Family, typename Tag>
    requires FamilyNumericalConstructorTag<Family, Tag>
void canonicalize(ygg::Data<FamilyNumerical<Family, Tag>>&) noexcept
{
    // Trivially canonical
}

template<FamilyTag Family, CategoryTag Category>
void canonicalize(ygg::Data<FamilyConstructor<Family, Category>>&) noexcept
{
    // Trivially canonical
}

template<CategoryTag Category>
void canonicalize(ygg::Data<Argument<Category>>&) noexcept
{
    // Trivially canonical
}

template<CategoryTag Category>
void canonicalize(ygg::Data<Register<Category>>&) noexcept
{
    // Trivially canonical
}

}  // namespace runir

#endif
