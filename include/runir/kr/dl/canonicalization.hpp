#ifndef RUNIR_CANONICALIZATION_HPP_
#define RUNIR_CANONICALIZATION_HPP_

#include "runir/kr/dl/concept_data.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/role_data.hpp"

#include <utility>
#include <urd/elements/flat.hpp>

namespace runir::kr::dl
{

template<ConceptOrRoleTag Category>
void canonicalize(urd::data<QueryProjection<Category>>&) noexcept
{
}
template<ConceptOrRoleTag Category>
bool is_canonical(const urd::data<QueryProjection<Category>>&) noexcept
{ return true; }

inline void canonicalize(urd::data<QueryColumn>&) noexcept {}
inline bool is_canonical(const urd::data<QueryColumn>&) noexcept { return true; }
template<typename Tag>
void canonicalize(urd::data<Query<Tag>>&) noexcept
{
}
template<typename Tag>
bool is_canonical(const urd::data<Query<Tag>>&) noexcept
{ return true; }

/// Orders the two operands of a commutative binary constructor so that equal expressions get equal data.
template<typename T>
    requires requires(urd::data<T>& data) {
        data.rhs < data.lhs;
        std::swap(data.lhs, data.rhs);
    }
void canonicalize_commutative_binary(urd::data<T>& data) noexcept
{
    if (data.rhs < data.lhs)
        std::swap(data.lhs, data.rhs);
}

template<typename Tag>
    requires ConceptConstructorTag<Tag>
bool is_canonical(const urd::data<Concept<Tag>>&) noexcept
{ return true; }

template<typename Tag>
    requires RoleConstructorTag<Tag>
bool is_canonical(const urd::data<Role<Tag>>&) noexcept
{ return true; }

template<typename Tag>
    requires BooleanConstructorTag<Tag>
bool is_canonical(const urd::data<Boolean<Tag>>&) noexcept
{ return true; }

template<typename Tag>
    requires NumericalConstructorTag<Tag>
bool is_canonical(const urd::data<Numerical<Tag>>&) noexcept
{ return true; }

template<CategoryTag Category>
bool is_canonical(const urd::data<Constructor<Category>>&) noexcept
{ return true; }

template<CategoryTag Category>
bool is_canonical(const urd::data<Argument<Category>>&) noexcept
{ return true; }

template<ConceptOrRoleTag Category>
bool is_canonical(const urd::data<Register<Category>>&) noexcept
{ return true; }

template<typename Tag>
    requires ConceptConstructorTag<Tag>
void canonicalize(urd::data<Concept<Tag>>&) noexcept
{
    // Trivially canonical
}

template<typename Tag>
    requires RoleConstructorTag<Tag>
void canonicalize(urd::data<Role<Tag>>&) noexcept
{
    // Trivially canonical
}

template<typename Tag>
    requires BooleanConstructorTag<Tag>
void canonicalize(urd::data<Boolean<Tag>>&) noexcept
{
    // Trivially canonical
}

template<typename Tag>
    requires NumericalConstructorTag<Tag>
void canonicalize(urd::data<Numerical<Tag>>&) noexcept
{
    // Trivially canonical
}

template<CategoryTag Category>
void canonicalize(urd::data<Constructor<Category>>&) noexcept
{
    // Trivially canonical
}

template<CategoryTag Category>
void canonicalize(urd::data<Argument<Category>>&) noexcept
{
    // Trivially canonical
}

template<ConceptOrRoleTag Category>
void canonicalize(urd::data<Register<Category>>&) noexcept
{
    // Trivially canonical
}

inline bool is_canonical(const urd::data<Concept<IntersectionTag>>& data) noexcept { return data.lhs <= data.rhs; }

inline bool is_canonical(const urd::data<Concept<UnionTag>>& data) noexcept { return data.lhs <= data.rhs; }

inline bool is_canonical(const urd::data<Concept<AgreementTag>>& data) noexcept { return data.lhs <= data.rhs; }

inline bool is_canonical(const urd::data<Role<IntersectionTag>>& data) noexcept { return data.lhs <= data.rhs; }

inline bool is_canonical(const urd::data<Role<UnionTag>>& data) noexcept { return data.lhs <= data.rhs; }

inline void canonicalize(urd::data<Concept<IntersectionTag>>& data) noexcept { canonicalize_commutative_binary(data); }

inline void canonicalize(urd::data<Concept<UnionTag>>& data) noexcept { canonicalize_commutative_binary(data); }

inline void canonicalize(urd::data<Concept<AgreementTag>>& data) noexcept { canonicalize_commutative_binary(data); }

inline void canonicalize(urd::data<Role<IntersectionTag>>& data) noexcept { canonicalize_commutative_binary(data); }

inline void canonicalize(urd::data<Role<UnionTag>>& data) noexcept { canonicalize_commutative_binary(data); }

}  // namespace runir

#endif
