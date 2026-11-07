#ifndef RUNIR_GRAMMAR_CANONICALIZATION_HPP_
#define RUNIR_GRAMMAR_CANONICALIZATION_HPP_

#include "runir/kr/dl/canonicalization.hpp"
#include "runir/kr/dl/grammar/boolean_data.hpp"
#include "runir/kr/dl/grammar/concept_data.hpp"
#include "runir/kr/dl/grammar/derivation_rule_data.hpp"
#include "runir/kr/dl/grammar/grammar_data.hpp"
#include "runir/kr/dl/grammar/non_terminal_data.hpp"
#include "runir/kr/dl/grammar/numerical_data.hpp"
#include "runir/kr/dl/grammar/role_data.hpp"
#include "runir/kr/dl/grammar/declarations.hpp"

#include <algorithm>
#include <yggdrasil/semantics/canonicalization.hpp>

namespace runir::kr::dl::grammar
{

template<typename Tag>
    requires runir::kr::dl::BaseConceptConstructorTag<Tag>
bool is_canonical(const urd::data<Concept<Tag>>&) noexcept
{ return true; }

template<typename Tag>
    requires runir::kr::dl::BaseRoleConstructorTag<Tag>
bool is_canonical(const urd::data<Role<Tag>>&) noexcept
{ return true; }

template<typename Tag>
    requires runir::kr::dl::BaseBooleanConstructorTag<Tag>
bool is_canonical(const urd::data<Boolean<Tag>>&) noexcept
{ return true; }

template<typename Tag>
    requires runir::kr::dl::BaseNumericalConstructorTag<Tag>
bool is_canonical(const urd::data<Numerical<Tag>>&) noexcept
{ return true; }

template<runir::kr::dl::CategoryTag Category>
bool is_canonical(const urd::data<NonTerminal<Category>>&) noexcept
{ return true; }

inline bool is_canonical(const urd::data<Concept<runir::kr::dl::IntersectionTag>>& data) noexcept { return data.lhs <= data.rhs; }

inline bool is_canonical(const urd::data<Concept<runir::kr::dl::UnionTag>>& data) noexcept { return data.lhs <= data.rhs; }

inline bool is_canonical(const urd::data<Concept<runir::kr::dl::AgreementTag>>& data) noexcept { return data.lhs <= data.rhs; }

inline bool is_canonical(const urd::data<Role<runir::kr::dl::IntersectionTag>>& data) noexcept { return data.lhs <= data.rhs; }

inline bool is_canonical(const urd::data<Role<runir::kr::dl::UnionTag>>& data) noexcept { return data.lhs <= data.rhs; }

template<typename Tag>
    requires runir::kr::dl::BaseConceptConstructorTag<Tag>
void canonicalize(urd::data<Concept<Tag>>&) noexcept
{
}

template<typename Tag>
    requires runir::kr::dl::BaseRoleConstructorTag<Tag>
void canonicalize(urd::data<Role<Tag>>&) noexcept
{
}

template<typename Tag>
    requires runir::kr::dl::BaseBooleanConstructorTag<Tag>
void canonicalize(urd::data<Boolean<Tag>>&) noexcept
{
}

template<typename Tag>
    requires runir::kr::dl::BaseNumericalConstructorTag<Tag>
void canonicalize(urd::data<Numerical<Tag>>&) noexcept
{
}

template<runir::kr::dl::CategoryTag Category>
void canonicalize(urd::data<NonTerminal<Category>>&) noexcept
{
}

inline void canonicalize(urd::data<Concept<runir::kr::dl::IntersectionTag>>& data) noexcept { runir::kr::dl::canonicalize_commutative_binary(data); }

inline void canonicalize(urd::data<Concept<runir::kr::dl::UnionTag>>& data) noexcept { runir::kr::dl::canonicalize_commutative_binary(data); }

inline void canonicalize(urd::data<Concept<runir::kr::dl::AgreementTag>>& data) noexcept { runir::kr::dl::canonicalize_commutative_binary(data); }

inline void canonicalize(urd::data<Role<runir::kr::dl::IntersectionTag>>& data) noexcept { runir::kr::dl::canonicalize_commutative_binary(data); }

inline void canonicalize(urd::data<Role<runir::kr::dl::UnionTag>>& data) noexcept { runir::kr::dl::canonicalize_commutative_binary(data); }

template<runir::kr::dl::CategoryTag Category>
inline bool is_canonical(const urd::data<DerivationRule<Category>>& data)
{ return std::ranges::is_sorted(data.rhs) && std::ranges::adjacent_find(data.rhs) == data.rhs.end(); }

template<runir::kr::dl::CategoryTag Category>
inline void canonicalize(urd::data<DerivationRule<Category>>& data)
{ ygg::canonicalize(data.rhs); }

inline bool is_canonical(const urd::data<GrammarTag>& data)
{
    return ygg::is_canonical(data.concept_derivation_rules) && ygg::is_canonical(data.role_derivation_rules) && ygg::is_canonical(data.boolean_derivation_rules)
           && ygg::is_canonical(data.numerical_derivation_rules);
}

inline void canonicalize(urd::data<GrammarTag>& data)
{
    ygg::canonicalize(data.concept_derivation_rules);
    ygg::canonicalize(data.role_derivation_rules);
    ygg::canonicalize(data.boolean_derivation_rules);
    ygg::canonicalize(data.numerical_derivation_rules);
}

}  // namespace runir::kr::dl::grammar

#endif
