#ifndef RUNIR_CANONICALIZATION_HPP_
#define RUNIR_CANONICALIZATION_HPP_

#include "runir/kr/dl/concept_data.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/role_data.hpp"

#include <urd/elements/flat.hpp>
#include <utility>

namespace runir::kr::dl
{

/// Orders the two operands of a commutative binary constructor so that equal expressions get equal data.
template<typename T>
    requires requires(urd::data<T>& data) {
        data.right_operand < data.left_operand;
        std::swap(data.left_operand, data.right_operand);
    }
void canonicalize_commutative_binary(urd::data<T>& data) noexcept
{
    if (data.right_operand < data.left_operand)
        std::swap(data.left_operand, data.right_operand);
}

template<typename F>
bool is_canonical(const urd::data<Concept<F, IntersectionTag>>& data) noexcept
{ return data.left_operand <= data.right_operand; }

template<typename F>
bool is_canonical(const urd::data<Concept<F, UnionTag>>& data) noexcept
{ return data.left_operand <= data.right_operand; }

template<typename F>
bool is_canonical(const urd::data<Concept<F, AgreementTag>>& data) noexcept
{ return data.left_operand <= data.right_operand; }

template<typename F>
bool is_canonical(const urd::data<Role<F, IntersectionTag>>& data) noexcept
{ return data.left_operand <= data.right_operand; }

template<typename F>
bool is_canonical(const urd::data<Role<F, UnionTag>>& data) noexcept
{ return data.left_operand <= data.right_operand; }

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
