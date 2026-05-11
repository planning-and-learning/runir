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

template<ConceptConstructorTag Tag>
bool is_canonical(const tyr::Data<Concept<Tag>>&) noexcept
{
    return true;
}

template<RoleConstructorTag Tag>
bool is_canonical(const tyr::Data<Role<Tag>>&) noexcept
{
    return true;
}

template<BooleanConstructorTag Tag>
bool is_canonical(const tyr::Data<Boolean<Tag>>&) noexcept
{
    return true;
}

template<NumericalConstructorTag Tag>
bool is_canonical(const tyr::Data<Numerical<Tag>>&) noexcept
{
    return true;
}

template<CategoryTag Category>
bool is_canonical(const tyr::Data<Constructor<Category>>&) noexcept
{
    return true;
}

template<ConceptConstructorTag Tag>
void canonicalize(tyr::Data<Concept<Tag>>&) noexcept
{
    // Trivially canonical
}

template<RoleConstructorTag Tag>
void canonicalize(tyr::Data<Role<Tag>>&) noexcept
{
    // Trivially canonical
}

template<BooleanConstructorTag Tag>
void canonicalize(tyr::Data<Boolean<Tag>>&) noexcept
{
    // Trivially canonical
}

template<NumericalConstructorTag Tag>
void canonicalize(tyr::Data<Numerical<Tag>>&) noexcept
{
    // Trivially canonical
}

template<CategoryTag Category>
void canonicalize(tyr::Data<Constructor<Category>>&) noexcept
{
    // Trivially canonical
}

}  // namespace runir

#endif
