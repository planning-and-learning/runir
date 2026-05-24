#ifndef RUNIR_SEMANTICS_CANONICALIZATION_HPP_
#define RUNIR_SEMANTICS_CANONICALIZATION_HPP_

#include "runir/kr/dl/canonicalization.hpp"
#include "runir/kr/dl/concept_data.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/role_data.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/denotation_data.hpp"

#include <tyr/common/comparators.hpp>
#include <tyr/common/types.hpp>

namespace runir::kr::dl
{

template<FamilyTag Family>
inline bool is_canonical(const tyr::Data<FamilyConcept<Family, IntersectionTag>>& data) noexcept
{
    return !tyr::Less<decltype(data.lhs)> {}(data.rhs, data.lhs);
}

template<FamilyTag Family>
inline bool is_canonical(const tyr::Data<FamilyConcept<Family, UnionTag>>& data) noexcept
{
    return !tyr::Less<decltype(data.lhs)> {}(data.rhs, data.lhs);
}

template<FamilyTag Family>
inline bool is_canonical(const tyr::Data<FamilyConcept<Family, AgreementTag>>& data) noexcept
{
    return !tyr::Less<decltype(data.lhs)> {}(data.rhs, data.lhs);
}

template<FamilyTag Family>
inline bool is_canonical(const tyr::Data<FamilyRole<Family, IntersectionTag>>& data) noexcept
{
    return !tyr::Less<decltype(data.lhs)> {}(data.rhs, data.lhs);
}

template<FamilyTag Family>
inline bool is_canonical(const tyr::Data<FamilyRole<Family, UnionTag>>& data) noexcept
{
    return !tyr::Less<decltype(data.lhs)> {}(data.rhs, data.lhs);
}

template<FamilyTag Family>
inline void canonicalize(tyr::Data<FamilyConcept<Family, IntersectionTag>>& data) noexcept
{
    canonicalize_commutative_binary(data);
}

template<FamilyTag Family>
inline void canonicalize(tyr::Data<FamilyConcept<Family, UnionTag>>& data) noexcept
{
    canonicalize_commutative_binary(data);
}

template<FamilyTag Family>
inline void canonicalize(tyr::Data<FamilyConcept<Family, AgreementTag>>& data) noexcept
{
    canonicalize_commutative_binary(data);
}

template<FamilyTag Family>
inline void canonicalize(tyr::Data<FamilyRole<Family, IntersectionTag>>& data) noexcept
{
    canonicalize_commutative_binary(data);
}

template<FamilyTag Family>
inline void canonicalize(tyr::Data<FamilyRole<Family, UnionTag>>& data) noexcept
{
    canonicalize_commutative_binary(data);
}

}  // namespace runir

namespace runir::kr::dl::semantics
{

template<CategoryTag Category>
bool is_canonical(const tyr::Data<Denotation<Category>>&) noexcept
{
    return true;
}

template<CategoryTag Category>
void canonicalize(tyr::Data<Denotation<Category>>&) noexcept
{
    // Trivially canonical
}

}  // namespace runir::kr::dl::semantics

#endif
