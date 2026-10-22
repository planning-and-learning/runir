#ifndef RUNIR_CANONICALIZATION_HPP_
#define RUNIR_CANONICALIZATION_HPP_

#include "runir/kr/dl/concept_data.hpp"
#include "runir/kr/dl/constructors.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/role_data.hpp"

#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/semantics/comparison.hpp>

namespace runir::kr::dl
{

template<ConceptOrRoleTag Category>
void canonicalize(ygg::Data<QueryProjection<Category>>&) noexcept
{
}
template<ConceptOrRoleTag Category>
bool is_canonical(const ygg::Data<QueryProjection<Category>>&) noexcept
{
    return true;
}

inline void canonicalize(ygg::Data<QueryColumn>&) noexcept {}
inline bool is_canonical(const ygg::Data<QueryColumn>&) noexcept { return true; }
template<typename Tag>
void canonicalize(ygg::Data<Query<Tag>>&) noexcept
{
}
template<typename Tag>
bool is_canonical(const ygg::Data<Query<Tag>>&) noexcept
{
    return true;
}

/// Orders the two operands of a commutative binary constructor so that equal expressions get equal data.
template<typename T>
    requires requires(ygg::Data<T>& data) {
        data.rhs < data.lhs;
        std::swap(data.lhs, data.rhs);
    }
void canonicalize_commutative_binary(ygg::Data<T>& data) noexcept
{
    if (data.rhs < data.lhs)
        std::swap(data.lhs, data.rhs);
}

template<typename Tag>
    requires FamilyConceptConstructorTag<Tag>
bool is_canonical(const ygg::Data<FamilyConcept<Tag>>&) noexcept
{
    return true;
}

template<typename Tag>
    requires FamilyRoleConstructorTag<Tag>
bool is_canonical(const ygg::Data<FamilyRole<Tag>>&) noexcept
{
    return true;
}

template<typename Tag>
    requires FamilyBooleanConstructorTag<Tag>
bool is_canonical(const ygg::Data<FamilyBoolean<Tag>>&) noexcept
{
    return true;
}

template<typename Tag>
    requires FamilyNumericalConstructorTag<Tag>
bool is_canonical(const ygg::Data<FamilyNumerical<Tag>>&) noexcept
{
    return true;
}

template<CategoryTag Category>
bool is_canonical(const ygg::Data<FamilyConstructor<Category>>&) noexcept
{
    return true;
}

template<CategoryTag Category>
bool is_canonical(const ygg::Data<Argument<Category>>&) noexcept
{
    return true;
}

template<ConceptOrRoleTag Category>
bool is_canonical(const ygg::Data<Register<Category>>&) noexcept
{
    return true;
}

template<typename Tag>
    requires FamilyConceptConstructorTag<Tag>
void canonicalize(ygg::Data<FamilyConcept<Tag>>&) noexcept
{
    // Trivially canonical
}

template<typename Tag>
    requires FamilyRoleConstructorTag<Tag>
void canonicalize(ygg::Data<FamilyRole<Tag>>&) noexcept
{
    // Trivially canonical
}

template<typename Tag>
    requires FamilyBooleanConstructorTag<Tag>
void canonicalize(ygg::Data<FamilyBoolean<Tag>>&) noexcept
{
    // Trivially canonical
}

template<typename Tag>
    requires FamilyNumericalConstructorTag<Tag>
void canonicalize(ygg::Data<FamilyNumerical<Tag>>&) noexcept
{
    // Trivially canonical
}

template<CategoryTag Category>
void canonicalize(ygg::Data<FamilyConstructor<Category>>&) noexcept
{
    // Trivially canonical
}

template<CategoryTag Category>
void canonicalize(ygg::Data<Argument<Category>>&) noexcept
{
    // Trivially canonical
}

template<ConceptOrRoleTag Category>
void canonicalize(ygg::Data<Register<Category>>&) noexcept
{
    // Trivially canonical
}

inline bool is_canonical(const ygg::Data<FamilyConcept<IntersectionTag>>& data) noexcept { return data.lhs <= data.rhs; }

inline bool is_canonical(const ygg::Data<FamilyConcept<UnionTag>>& data) noexcept { return data.lhs <= data.rhs; }

inline bool is_canonical(const ygg::Data<FamilyConcept<AgreementTag>>& data) noexcept { return data.lhs <= data.rhs; }

inline bool is_canonical(const ygg::Data<FamilyRole<IntersectionTag>>& data) noexcept { return data.lhs <= data.rhs; }

inline bool is_canonical(const ygg::Data<FamilyRole<UnionTag>>& data) noexcept { return data.lhs <= data.rhs; }

inline void canonicalize(ygg::Data<FamilyConcept<IntersectionTag>>& data) noexcept { canonicalize_commutative_binary(data); }

inline void canonicalize(ygg::Data<FamilyConcept<UnionTag>>& data) noexcept { canonicalize_commutative_binary(data); }

inline void canonicalize(ygg::Data<FamilyConcept<AgreementTag>>& data) noexcept { canonicalize_commutative_binary(data); }

inline void canonicalize(ygg::Data<FamilyRole<IntersectionTag>>& data) noexcept { canonicalize_commutative_binary(data); }

inline void canonicalize(ygg::Data<FamilyRole<UnionTag>>& data) noexcept { canonicalize_commutative_binary(data); }

}  // namespace runir

#endif
