#ifndef RUNIR_CANONICALIZATION_HPP_
#define RUNIR_CANONICALIZATION_HPP_

#include "runir/kr/dl/constructors.hpp"
#include "runir/kr/dl/declarations.hpp"

#include <type_traits>
#include <tyr/common/comparators.hpp>
#include <tyr/common/types.hpp>
#include <utility>

namespace runir::kr::dl
{

template<typename Data>
void canonicalize_commutative_binary(Data& data) noexcept
{
    using Lhs = std::remove_cvref_t<decltype(data.lhs)>;
    using Rhs = std::remove_cvref_t<decltype(data.rhs)>;
    static_assert(std::same_as<Lhs, Rhs>);

    if (tyr::Less<Lhs> {}(data.rhs, data.lhs))
        std::swap(data.lhs, data.rhs);
}

template<FamilyTag Family, typename Tag>
    requires ConceptConstructorTag<Tag>
bool is_canonical(const tyr::Data<FamilyConcept<Family, Tag>>&) noexcept
{
    return true;
}

template<FamilyTag Family, RoleConstructorTag Tag>
bool is_canonical(const tyr::Data<FamilyRole<Family, Tag>>&) noexcept
{
    return true;
}

template<FamilyTag Family, BooleanConstructorTag Tag>
bool is_canonical(const tyr::Data<FamilyBoolean<Family, Tag>>&) noexcept
{
    return true;
}

template<FamilyTag Family, NumericalConstructorTag Tag>
bool is_canonical(const tyr::Data<FamilyNumerical<Family, Tag>>&) noexcept
{
    return true;
}

template<FamilyTag Family, CategoryTag Category>
bool is_canonical(const tyr::Data<FamilyConstructor<Family, Category>>&) noexcept
{
    return true;
}

template<FamilyTag Family, typename Tag>
    requires ConceptConstructorTag<Tag>
void canonicalize(tyr::Data<FamilyConcept<Family, Tag>>&) noexcept
{
    // Trivially canonical
}

template<FamilyTag Family, RoleConstructorTag Tag>
void canonicalize(tyr::Data<FamilyRole<Family, Tag>>&) noexcept
{
    // Trivially canonical
}

template<FamilyTag Family, BooleanConstructorTag Tag>
void canonicalize(tyr::Data<FamilyBoolean<Family, Tag>>&) noexcept
{
    // Trivially canonical
}

template<FamilyTag Family, NumericalConstructorTag Tag>
void canonicalize(tyr::Data<FamilyNumerical<Family, Tag>>&) noexcept
{
    // Trivially canonical
}

template<FamilyTag Family, CategoryTag Category>
void canonicalize(tyr::Data<FamilyConstructor<Family, Category>>&) noexcept
{
    // Trivially canonical
}

}  // namespace runir

#endif
