#ifndef RUNIR_CANONICALIZATION_HPP_
#define RUNIR_CANONICALIZATION_HPP_

#include "runir/kr/dl/concept_data.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/role_data.hpp"

#include <urd/elements/flat.hpp>
#include <utility>

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
/// Input slots are positional, so a program has no canonical reordering.
inline void canonicalize(urd::data<DatalogProgram>&) noexcept {}
inline bool is_canonical(const urd::data<DatalogProgram>&) noexcept { return true; }
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

template<typename F, typename Tag>
bool is_canonical(const urd::data<Concept<F, Tag>>&) noexcept
{ return true; }

template<typename F, typename Tag>
bool is_canonical(const urd::data<Role<F, Tag>>&) noexcept
{ return true; }

template<typename F, typename Tag>
bool is_canonical(const urd::data<Boolean<F, Tag>>&) noexcept
{ return true; }

template<typename F, typename Tag>
bool is_canonical(const urd::data<Numerical<F, Tag>>&) noexcept
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

template<typename F, typename Tag>
void canonicalize(urd::data<Concept<F, Tag>>&) noexcept
{
    // Trivially canonical
}

template<typename F, typename Tag>
void canonicalize(urd::data<Role<F, Tag>>&) noexcept
{
    // Trivially canonical
}

template<typename F, typename Tag>
void canonicalize(urd::data<Boolean<F, Tag>>&) noexcept
{
    // Trivially canonical
}

template<typename F, typename Tag>
void canonicalize(urd::data<Numerical<F, Tag>>&) noexcept
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

template<typename F>
bool is_canonical(const urd::data<Concept<F, IntersectionTag>>& data) noexcept
{ return data.lhs <= data.rhs; }

template<typename F>
bool is_canonical(const urd::data<Concept<F, UnionTag>>& data) noexcept
{ return data.lhs <= data.rhs; }

template<typename F>
bool is_canonical(const urd::data<Concept<F, AgreementTag>>& data) noexcept
{ return data.lhs <= data.rhs; }

template<typename F>
bool is_canonical(const urd::data<Role<F, IntersectionTag>>& data) noexcept
{ return data.lhs <= data.rhs; }

template<typename F>
bool is_canonical(const urd::data<Role<F, UnionTag>>& data) noexcept
{ return data.lhs <= data.rhs; }

template<typename F>
void canonicalize(urd::data<Concept<F, IntersectionTag>>& data) noexcept
{ canonicalize_commutative_binary(data); }

template<typename F>
void canonicalize(urd::data<Concept<F, UnionTag>>& data) noexcept
{ canonicalize_commutative_binary(data); }

template<typename F>
void canonicalize(urd::data<Concept<F, AgreementTag>>& data) noexcept
{ canonicalize_commutative_binary(data); }

template<typename F>
void canonicalize(urd::data<Role<F, IntersectionTag>>& data) noexcept
{ canonicalize_commutative_binary(data); }

template<typename F>
void canonicalize(urd::data<Role<F, UnionTag>>& data) noexcept
{ canonicalize_commutative_binary(data); }

}  // namespace runir

#endif
