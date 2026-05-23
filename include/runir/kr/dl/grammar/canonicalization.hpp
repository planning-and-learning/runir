#ifndef RUNIR_GRAMMAR_CANONICALIZATION_HPP_
#define RUNIR_GRAMMAR_CANONICALIZATION_HPP_

#include "runir/kr/dl/canonicalization.hpp"
#include "runir/kr/dl/grammar/datas.hpp"
#include "runir/kr/dl/grammar/declarations.hpp"

#include <tyr/common/canonicalization.hpp>

namespace runir::kr::dl::grammar::base
{

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyConceptConstructorTag<Family, Tag>
bool is_canonical(const tyr::Data<Concept<Family, Tag>>&) noexcept
{
    return true;
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::RoleConstructorTag Tag>
bool is_canonical(const tyr::Data<Role<Family, Tag>>&) noexcept
{
    return true;
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::BooleanConstructorTag Tag>
bool is_canonical(const tyr::Data<Boolean<Family, Tag>>&) noexcept
{
    return true;
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::NumericalConstructorTag Tag>
bool is_canonical(const tyr::Data<Numerical<Family, Tag>>&) noexcept
{
    return true;
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
bool is_canonical(const tyr::Data<Constructor<Family, Category>>&) noexcept
{
    return true;
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
bool is_canonical(const tyr::Data<ConstructorOrNonTerminal<Family, Category>>&) noexcept
{
    return true;
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
bool is_canonical(const tyr::Data<NonTerminal<Family, Category>>&) noexcept
{
    return true;
}

template<runir::kr::dl::FamilyTag Family>
inline bool is_canonical(const tyr::Data<Concept<Family, runir::kr::dl::IntersectionTag>>& data) noexcept
{
    return !tyr::Less<decltype(data.lhs)> {}(data.rhs, data.lhs);
}

template<runir::kr::dl::FamilyTag Family>
inline bool is_canonical(const tyr::Data<Concept<Family, runir::kr::dl::UnionTag>>& data) noexcept
{
    return !tyr::Less<decltype(data.lhs)> {}(data.rhs, data.lhs);
}

template<runir::kr::dl::FamilyTag Family>
inline bool is_canonical(const tyr::Data<Concept<Family, runir::kr::dl::AgreementTag>>& data) noexcept
{
    return !tyr::Less<decltype(data.lhs)> {}(data.rhs, data.lhs);
}

template<runir::kr::dl::FamilyTag Family>
inline bool is_canonical(const tyr::Data<Role<Family, runir::kr::dl::IntersectionTag>>& data) noexcept
{
    return !tyr::Less<decltype(data.lhs)> {}(data.rhs, data.lhs);
}

template<runir::kr::dl::FamilyTag Family>
inline bool is_canonical(const tyr::Data<Role<Family, runir::kr::dl::UnionTag>>& data) noexcept
{
    return !tyr::Less<decltype(data.lhs)> {}(data.rhs, data.lhs);
}

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyConceptConstructorTag<Family, Tag>
void canonicalize(tyr::Data<Concept<Family, Tag>>&) noexcept
{
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::RoleConstructorTag Tag>
void canonicalize(tyr::Data<Role<Family, Tag>>&) noexcept
{
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::BooleanConstructorTag Tag>
void canonicalize(tyr::Data<Boolean<Family, Tag>>&) noexcept
{
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::NumericalConstructorTag Tag>
void canonicalize(tyr::Data<Numerical<Family, Tag>>&) noexcept
{
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
void canonicalize(tyr::Data<Constructor<Family, Category>>&) noexcept
{
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
void canonicalize(tyr::Data<ConstructorOrNonTerminal<Family, Category>>&) noexcept
{
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
void canonicalize(tyr::Data<NonTerminal<Family, Category>>&) noexcept
{
}

template<runir::kr::dl::FamilyTag Family>
inline void canonicalize(tyr::Data<Concept<Family, runir::kr::dl::IntersectionTag>>& data) noexcept
{
    runir::kr::dl::canonicalize_commutative_binary(data);
}

template<runir::kr::dl::FamilyTag Family>
inline void canonicalize(tyr::Data<Concept<Family, runir::kr::dl::UnionTag>>& data) noexcept
{
    runir::kr::dl::canonicalize_commutative_binary(data);
}

template<runir::kr::dl::FamilyTag Family>
inline void canonicalize(tyr::Data<Concept<Family, runir::kr::dl::AgreementTag>>& data) noexcept
{
    runir::kr::dl::canonicalize_commutative_binary(data);
}

template<runir::kr::dl::FamilyTag Family>
inline void canonicalize(tyr::Data<Role<Family, runir::kr::dl::IntersectionTag>>& data) noexcept
{
    runir::kr::dl::canonicalize_commutative_binary(data);
}

template<runir::kr::dl::FamilyTag Family>
inline void canonicalize(tyr::Data<Role<Family, runir::kr::dl::UnionTag>>& data) noexcept
{
    runir::kr::dl::canonicalize_commutative_binary(data);
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
inline bool is_canonical(const tyr::Data<DerivationRule<Family, Category>>& data)
{
    return tyr::is_canonical(data.rhs);
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
inline void canonicalize(tyr::Data<DerivationRule<Family, Category>>& data)
{
    tyr::canonicalize(data.rhs);
}

template<runir::kr::dl::FamilyTag Family>
inline bool is_canonical(const tyr::Data<GrammarTag<Family>>& data)
{
    return tyr::is_canonical(data.concept_derivation_rules) && tyr::is_canonical(data.role_derivation_rules) && tyr::is_canonical(data.boolean_derivation_rules)
           && tyr::is_canonical(data.numerical_derivation_rules);
}

template<runir::kr::dl::FamilyTag Family>
inline void canonicalize(tyr::Data<GrammarTag<Family>>& data)
{
    tyr::canonicalize(data.concept_derivation_rules);
    tyr::canonicalize(data.role_derivation_rules);
    tyr::canonicalize(data.boolean_derivation_rules);
    tyr::canonicalize(data.numerical_derivation_rules);
}

}  // namespace runir::kr::dl::grammar::base

namespace runir::kr::dl::grammar
{

using base::canonicalize;
using base::is_canonical;

}  // namespace runir::kr::dl::grammar

#endif
