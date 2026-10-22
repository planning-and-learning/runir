#ifndef RUNIR_CANONICALIZATION_HPP_
#define RUNIR_CANONICALIZATION_HPP_

#include "runir/kr/dl/concept_data.hpp"
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
    requires ConceptConstructorTag<Tag>
bool is_canonical(const ygg::Data<Concept<Tag>>&) noexcept
{
    return true;
}

template<typename Tag>
    requires RoleConstructorTag<Tag>
bool is_canonical(const ygg::Data<Role<Tag>>&) noexcept
{
    return true;
}

template<typename Tag>
    requires BooleanConstructorTag<Tag>
bool is_canonical(const ygg::Data<Boolean<Tag>>&) noexcept
{
    return true;
}

template<typename Tag>
    requires NumericalConstructorTag<Tag>
bool is_canonical(const ygg::Data<Numerical<Tag>>&) noexcept
{
    return true;
}

template<CategoryTag Category>
bool is_canonical(const ygg::Data<Constructor<Category>>&) noexcept
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
    requires ConceptConstructorTag<Tag>
void canonicalize(ygg::Data<Concept<Tag>>&) noexcept
{
    // Trivially canonical
}

template<typename Tag>
    requires RoleConstructorTag<Tag>
void canonicalize(ygg::Data<Role<Tag>>&) noexcept
{
    // Trivially canonical
}

template<typename Tag>
    requires BooleanConstructorTag<Tag>
void canonicalize(ygg::Data<Boolean<Tag>>&) noexcept
{
    // Trivially canonical
}

template<typename Tag>
    requires NumericalConstructorTag<Tag>
void canonicalize(ygg::Data<Numerical<Tag>>&) noexcept
{
    // Trivially canonical
}

template<CategoryTag Category>
void canonicalize(ygg::Data<Constructor<Category>>&) noexcept
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

inline bool is_canonical(const ygg::Data<Concept<IntersectionTag>>& data) noexcept { return data.lhs <= data.rhs; }

inline bool is_canonical(const ygg::Data<Concept<UnionTag>>& data) noexcept { return data.lhs <= data.rhs; }

inline bool is_canonical(const ygg::Data<Concept<AgreementTag>>& data) noexcept { return data.lhs <= data.rhs; }

inline bool is_canonical(const ygg::Data<Role<IntersectionTag>>& data) noexcept { return data.lhs <= data.rhs; }

inline bool is_canonical(const ygg::Data<Role<UnionTag>>& data) noexcept { return data.lhs <= data.rhs; }

inline void canonicalize(ygg::Data<Concept<IntersectionTag>>& data) noexcept { canonicalize_commutative_binary(data); }

inline void canonicalize(ygg::Data<Concept<UnionTag>>& data) noexcept { canonicalize_commutative_binary(data); }

inline void canonicalize(ygg::Data<Concept<AgreementTag>>& data) noexcept { canonicalize_commutative_binary(data); }

inline void canonicalize(ygg::Data<Role<IntersectionTag>>& data) noexcept { canonicalize_commutative_binary(data); }

inline void canonicalize(ygg::Data<Role<UnionTag>>& data) noexcept { canonicalize_commutative_binary(data); }

}  // namespace runir

#endif
