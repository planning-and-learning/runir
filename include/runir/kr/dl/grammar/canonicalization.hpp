#ifndef RUNIR_GRAMMAR_CANONICALIZATION_HPP_
#define RUNIR_GRAMMAR_CANONICALIZATION_HPP_

#include "runir/kr/dl/canonicalization.hpp"
#include "runir/kr/dl/grammar/datas.hpp"
#include "runir/kr/dl/grammar/declarations.hpp"

#include <yggdrasil/semantics/canonicalization.hpp>
#include <yggdrasil/semantics/comparison.hpp>

namespace runir::kr::dl::grammar
{

template<typename Tag>
    requires runir::kr::dl::ConceptConstructorTag<Tag>
bool is_canonical(const ygg::Data<Concept<Tag>>&) noexcept
{
    return true;
}

template<typename Tag>
    requires runir::kr::dl::RoleConstructorTag<Tag>
bool is_canonical(const ygg::Data<Role<Tag>>&) noexcept
{
    return true;
}

template<typename Tag>
    requires runir::kr::dl::BooleanConstructorTag<Tag>
bool is_canonical(const ygg::Data<Boolean<Tag>>&) noexcept
{
    return true;
}

template<typename Tag>
    requires runir::kr::dl::NumericalConstructorTag<Tag>
bool is_canonical(const ygg::Data<Numerical<Tag>>&) noexcept
{
    return true;
}

template<runir::kr::dl::CategoryTag Category>
bool is_canonical(const ygg::Data<Constructor<Category>>&) noexcept
{
    return true;
}

template<runir::kr::dl::CategoryTag Category>
bool is_canonical(const ygg::Data<ConstructorOrNonTerminal<Category>>&) noexcept
{
    return true;
}

template<runir::kr::dl::CategoryTag Category>
bool is_canonical(const ygg::Data<NonTerminal<Category>>&) noexcept
{
    return true;
}

inline bool is_canonical(const ygg::Data<Concept<runir::kr::dl::IntersectionTag>>& data) noexcept { return data.lhs <= data.rhs; }

inline bool is_canonical(const ygg::Data<Concept<runir::kr::dl::UnionTag>>& data) noexcept { return data.lhs <= data.rhs; }

inline bool is_canonical(const ygg::Data<Concept<runir::kr::dl::AgreementTag>>& data) noexcept { return data.lhs <= data.rhs; }

inline bool is_canonical(const ygg::Data<Role<runir::kr::dl::IntersectionTag>>& data) noexcept { return data.lhs <= data.rhs; }

inline bool is_canonical(const ygg::Data<Role<runir::kr::dl::UnionTag>>& data) noexcept { return data.lhs <= data.rhs; }

template<typename Tag>
    requires runir::kr::dl::ConceptConstructorTag<Tag>
void canonicalize(ygg::Data<Concept<Tag>>&) noexcept
{
}

template<typename Tag>
    requires runir::kr::dl::RoleConstructorTag<Tag>
void canonicalize(ygg::Data<Role<Tag>>&) noexcept
{
}

template<typename Tag>
    requires runir::kr::dl::BooleanConstructorTag<Tag>
void canonicalize(ygg::Data<Boolean<Tag>>&) noexcept
{
}

template<typename Tag>
    requires runir::kr::dl::NumericalConstructorTag<Tag>
void canonicalize(ygg::Data<Numerical<Tag>>&) noexcept
{
}

template<runir::kr::dl::CategoryTag Category>
void canonicalize(ygg::Data<Constructor<Category>>&) noexcept
{
}

template<runir::kr::dl::CategoryTag Category>
void canonicalize(ygg::Data<ConstructorOrNonTerminal<Category>>&) noexcept
{
}

template<runir::kr::dl::CategoryTag Category>
void canonicalize(ygg::Data<NonTerminal<Category>>&) noexcept
{
}

inline void canonicalize(ygg::Data<Concept<runir::kr::dl::IntersectionTag>>& data) noexcept { runir::kr::dl::canonicalize_commutative_binary(data); }

inline void canonicalize(ygg::Data<Concept<runir::kr::dl::UnionTag>>& data) noexcept { runir::kr::dl::canonicalize_commutative_binary(data); }

inline void canonicalize(ygg::Data<Concept<runir::kr::dl::AgreementTag>>& data) noexcept { runir::kr::dl::canonicalize_commutative_binary(data); }

inline void canonicalize(ygg::Data<Role<runir::kr::dl::IntersectionTag>>& data) noexcept { runir::kr::dl::canonicalize_commutative_binary(data); }

inline void canonicalize(ygg::Data<Role<runir::kr::dl::UnionTag>>& data) noexcept { runir::kr::dl::canonicalize_commutative_binary(data); }

template<runir::kr::dl::CategoryTag Category>
inline bool is_canonical(const ygg::Data<DerivationRule<Category>>& data)
{
    return ygg::is_canonical(data.rhs);
}

template<runir::kr::dl::CategoryTag Category>
inline void canonicalize(ygg::Data<DerivationRule<Category>>& data)
{
    ygg::canonicalize(data.rhs);
}

inline bool is_canonical(const ygg::Data<GrammarTag>& data)
{
    return ygg::is_canonical(data.concept_derivation_rules) && ygg::is_canonical(data.role_derivation_rules) && ygg::is_canonical(data.boolean_derivation_rules)
           && ygg::is_canonical(data.numerical_derivation_rules);
}

inline void canonicalize(ygg::Data<GrammarTag>& data)
{
    ygg::canonicalize(data.concept_derivation_rules);
    ygg::canonicalize(data.role_derivation_rules);
    ygg::canonicalize(data.boolean_derivation_rules);
    ygg::canonicalize(data.numerical_derivation_rules);
}

}  // namespace runir::kr::dl::grammar

#endif
