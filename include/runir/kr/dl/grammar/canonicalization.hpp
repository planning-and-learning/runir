#ifndef RUNIR_GRAMMAR_CANONICALIZATION_HPP_
#define RUNIR_GRAMMAR_CANONICALIZATION_HPP_

#include "runir/kr/dl/canonicalization.hpp"
#include "runir/kr/dl/grammar/datas.hpp"
#include "runir/kr/dl/grammar/declarations.hpp"

#include <yggdrasil/semantics/canonicalization.hpp>

namespace runir::kr::dl::grammar
{

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyConceptConstructorTag<Family, Tag>
bool is_canonical(const ygg::Data<Concept<Family, Tag>>&) noexcept
{
    return true;
}

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyRoleConstructorTag<Family, Tag>
bool is_canonical(const ygg::Data<Role<Family, Tag>>&) noexcept
{
    return true;
}

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyBooleanConstructorTag<Family, Tag>
bool is_canonical(const ygg::Data<Boolean<Family, Tag>>&) noexcept
{
    return true;
}

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Family, Tag>
bool is_canonical(const ygg::Data<Numerical<Family, Tag>>&) noexcept
{
    return true;
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
bool is_canonical(const ygg::Data<Constructor<Family, Category>>&) noexcept
{
    return true;
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
bool is_canonical(const ygg::Data<ConstructorOrNonTerminal<Family, Category>>&) noexcept
{
    return true;
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
bool is_canonical(const ygg::Data<NonTerminal<Family, Category>>&) noexcept
{
    return true;
}

template<runir::kr::dl::FamilyTag Family>
inline bool is_canonical(const ygg::Data<Concept<Family, runir::kr::dl::IntersectionTag>>& data) noexcept
{
    return !ygg::Less<decltype(data.lhs)> {}(data.rhs, data.lhs);
}

template<runir::kr::dl::FamilyTag Family>
inline bool is_canonical(const ygg::Data<Concept<Family, runir::kr::dl::UnionTag>>& data) noexcept
{
    return !ygg::Less<decltype(data.lhs)> {}(data.rhs, data.lhs);
}

template<runir::kr::dl::FamilyTag Family>
inline bool is_canonical(const ygg::Data<Concept<Family, runir::kr::dl::AgreementTag>>& data) noexcept
{
    return !ygg::Less<decltype(data.lhs)> {}(data.rhs, data.lhs);
}

template<runir::kr::dl::FamilyTag Family>
inline bool is_canonical(const ygg::Data<Role<Family, runir::kr::dl::IntersectionTag>>& data) noexcept
{
    return !ygg::Less<decltype(data.lhs)> {}(data.rhs, data.lhs);
}

template<runir::kr::dl::FamilyTag Family>
inline bool is_canonical(const ygg::Data<Role<Family, runir::kr::dl::UnionTag>>& data) noexcept
{
    return !ygg::Less<decltype(data.lhs)> {}(data.rhs, data.lhs);
}

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyConceptConstructorTag<Family, Tag>
void canonicalize(ygg::Data<Concept<Family, Tag>>&) noexcept
{
}

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyRoleConstructorTag<Family, Tag>
void canonicalize(ygg::Data<Role<Family, Tag>>&) noexcept
{
}

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyBooleanConstructorTag<Family, Tag>
void canonicalize(ygg::Data<Boolean<Family, Tag>>&) noexcept
{
}

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Family, Tag>
void canonicalize(ygg::Data<Numerical<Family, Tag>>&) noexcept
{
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
void canonicalize(ygg::Data<Constructor<Family, Category>>&) noexcept
{
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
void canonicalize(ygg::Data<ConstructorOrNonTerminal<Family, Category>>&) noexcept
{
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
void canonicalize(ygg::Data<NonTerminal<Family, Category>>&) noexcept
{
}

template<runir::kr::dl::FamilyTag Family>
inline void canonicalize(ygg::Data<Concept<Family, runir::kr::dl::IntersectionTag>>& data) noexcept
{
    runir::kr::dl::canonicalize_commutative_binary(data);
}

template<runir::kr::dl::FamilyTag Family>
inline void canonicalize(ygg::Data<Concept<Family, runir::kr::dl::UnionTag>>& data) noexcept
{
    runir::kr::dl::canonicalize_commutative_binary(data);
}

template<runir::kr::dl::FamilyTag Family>
inline void canonicalize(ygg::Data<Concept<Family, runir::kr::dl::AgreementTag>>& data) noexcept
{
    runir::kr::dl::canonicalize_commutative_binary(data);
}

template<runir::kr::dl::FamilyTag Family>
inline void canonicalize(ygg::Data<Role<Family, runir::kr::dl::IntersectionTag>>& data) noexcept
{
    runir::kr::dl::canonicalize_commutative_binary(data);
}

template<runir::kr::dl::FamilyTag Family>
inline void canonicalize(ygg::Data<Role<Family, runir::kr::dl::UnionTag>>& data) noexcept
{
    runir::kr::dl::canonicalize_commutative_binary(data);
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
inline bool is_canonical(const ygg::Data<DerivationRule<Family, Category>>& data)
{
    return ygg::is_canonical(data.rhs);
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
inline void canonicalize(ygg::Data<DerivationRule<Family, Category>>& data)
{
    ygg::canonicalize(data.rhs);
}

template<runir::kr::dl::FamilyTag Family>
inline bool is_canonical(const ygg::Data<GrammarTag<Family>>& data)
{
    return ygg::is_canonical(data.concept_derivation_rules) && ygg::is_canonical(data.role_derivation_rules) && ygg::is_canonical(data.boolean_derivation_rules)
           && ygg::is_canonical(data.numerical_derivation_rules);
}

template<runir::kr::dl::FamilyTag Family>
inline void canonicalize(ygg::Data<GrammarTag<Family>>& data)
{
    ygg::canonicalize(data.concept_derivation_rules);
    ygg::canonicalize(data.role_derivation_rules);
    ygg::canonicalize(data.boolean_derivation_rules);
    ygg::canonicalize(data.numerical_derivation_rules);
}

}  // namespace runir::kr::dl::grammar

#endif
