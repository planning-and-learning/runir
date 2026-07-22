#ifndef RUNIR_SEMANTICS_CANONICALIZATION_HPP_
#define RUNIR_SEMANTICS_CANONICALIZATION_HPP_

#include "runir/kr/dl/canonicalization.hpp"
#include "runir/kr/dl/concept_data.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/role_data.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/denotation_data.hpp"

#include <yggdrasil/core/types.hpp>
#include <yggdrasil/semantics/comparison.hpp>

namespace runir::kr::dl
{

template<FamilyTag Family>
inline bool is_canonical(const ygg::Data<FamilyConcept<Family, IntersectionTag>>& data) noexcept
{
    return data.lhs <= data.rhs;
}

template<FamilyTag Family>
inline bool is_canonical(const ygg::Data<FamilyConcept<Family, UnionTag>>& data) noexcept
{
    return data.lhs <= data.rhs;
}

template<FamilyTag Family>
inline bool is_canonical(const ygg::Data<FamilyConcept<Family, AgreementTag>>& data) noexcept
{
    return data.lhs <= data.rhs;
}

template<FamilyTag Family>
inline bool is_canonical(const ygg::Data<FamilyRole<Family, IntersectionTag>>& data) noexcept
{
    return data.lhs <= data.rhs;
}

template<FamilyTag Family>
inline bool is_canonical(const ygg::Data<FamilyRole<Family, UnionTag>>& data) noexcept
{
    return data.lhs <= data.rhs;
}

template<FamilyTag Family>
inline void canonicalize(ygg::Data<FamilyConcept<Family, IntersectionTag>>& data) noexcept
{
    canonicalize_commutative_binary(data);
}

template<FamilyTag Family>
inline void canonicalize(ygg::Data<FamilyConcept<Family, UnionTag>>& data) noexcept
{
    canonicalize_commutative_binary(data);
}

template<FamilyTag Family>
inline void canonicalize(ygg::Data<FamilyConcept<Family, AgreementTag>>& data) noexcept
{
    canonicalize_commutative_binary(data);
}

template<FamilyTag Family>
inline void canonicalize(ygg::Data<FamilyRole<Family, IntersectionTag>>& data) noexcept
{
    canonicalize_commutative_binary(data);
}

template<FamilyTag Family>
inline void canonicalize(ygg::Data<FamilyRole<Family, UnionTag>>& data) noexcept
{
    canonicalize_commutative_binary(data);
}

}  // namespace runir

namespace runir::kr::dl::semantics
{

template<CategoryTag Category>
bool is_canonical(const ygg::Data<Denotation<Category>>&) noexcept
{
    return true;
}

template<CategoryTag Category>
void canonicalize(ygg::Data<Denotation<Category>>&) noexcept
{
    // Trivially canonical
}

}  // namespace runir::kr::dl::semantics

#endif
