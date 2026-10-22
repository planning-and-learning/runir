#ifndef RUNIR_CNF_GRAMMAR_CANONICALIZATION_HPP_
#define RUNIR_CNF_GRAMMAR_CANONICALIZATION_HPP_

#include "runir/kr/dl/canonicalization.hpp"
#include "runir/kr/dl/cnf_grammar/concept_data.hpp"
#include "runir/kr/dl/cnf_grammar/derivation_rule_data.hpp"
#include "runir/kr/dl/cnf_grammar/grammar_data.hpp"
#include "runir/kr/dl/cnf_grammar/role_data.hpp"
#include "runir/kr/dl/cnf_grammar/substitution_rule_data.hpp"

#include <yggdrasil/semantics/canonicalization.hpp>
#include <yggdrasil/semantics/comparison.hpp>

namespace runir::kr::dl::cnf_grammar
{

template<typename Tag>
    requires runir::kr::dl::FamilyConceptConstructorTag<Tag>
bool is_canonical(const ygg::Data<Concept<Tag>>&) noexcept
{
    return true;
}

template<typename Tag>
    requires runir::kr::dl::FamilyRoleConstructorTag<Tag>
bool is_canonical(const ygg::Data<Role<Tag>>&) noexcept
{
    return true;
}

template<typename Tag>
    requires runir::kr::dl::FamilyBooleanConstructorTag<Tag>
bool is_canonical(const ygg::Data<Boolean<Tag>>&) noexcept
{
    return true;
}

template<typename Tag>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Tag>
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
    requires runir::kr::dl::FamilyConceptConstructorTag<Tag>
void canonicalize(ygg::Data<Concept<Tag>>&) noexcept
{
    // Trivially canonical
}

template<typename Tag>
    requires runir::kr::dl::FamilyRoleConstructorTag<Tag>
void canonicalize(ygg::Data<Role<Tag>>&) noexcept
{
    // Trivially canonical
}

template<typename Tag>
    requires runir::kr::dl::FamilyBooleanConstructorTag<Tag>
void canonicalize(ygg::Data<Boolean<Tag>>&) noexcept
{
    // Trivially canonical
}

template<typename Tag>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Tag>
void canonicalize(ygg::Data<Numerical<Tag>>&) noexcept
{
    // Trivially canonical
}

template<runir::kr::dl::CategoryTag Category>
void canonicalize(ygg::Data<Constructor<Category>>&) noexcept
{
    // Trivially canonical
}

template<runir::kr::dl::CategoryTag Category>
void canonicalize(ygg::Data<NonTerminal<Category>>&) noexcept
{
    // Trivially canonical
}

inline void canonicalize(ygg::Data<Concept<runir::kr::dl::IntersectionTag>>& data) noexcept { runir::kr::dl::canonicalize_commutative_binary(data); }

inline void canonicalize(ygg::Data<Concept<runir::kr::dl::UnionTag>>& data) noexcept { runir::kr::dl::canonicalize_commutative_binary(data); }

inline void canonicalize(ygg::Data<Concept<runir::kr::dl::AgreementTag>>& data) noexcept { runir::kr::dl::canonicalize_commutative_binary(data); }

inline void canonicalize(ygg::Data<Role<runir::kr::dl::IntersectionTag>>& data) noexcept { runir::kr::dl::canonicalize_commutative_binary(data); }

inline void canonicalize(ygg::Data<Role<runir::kr::dl::UnionTag>>& data) noexcept { runir::kr::dl::canonicalize_commutative_binary(data); }

template<runir::kr::dl::CategoryTag Category>
inline bool is_canonical(const ygg::Data<DerivationRule<Category>>&) noexcept
{
    return true;
}

template<runir::kr::dl::CategoryTag Category>
inline void canonicalize(ygg::Data<DerivationRule<Category>>&) noexcept
{
}

template<runir::kr::dl::CategoryTag Category>
inline bool is_canonical(const ygg::Data<SubstitutionRule<Category>>&) noexcept
{
    return true;
}

template<runir::kr::dl::CategoryTag Category>
inline void canonicalize(ygg::Data<SubstitutionRule<Category>>&) noexcept
{
}

inline bool is_canonical(const ygg::Data<Grammar>& data)
{
    return ygg::is_canonical(data.concept_derivation_rules) && ygg::is_canonical(data.role_derivation_rules) && ygg::is_canonical(data.boolean_derivation_rules)
           && ygg::is_canonical(data.numerical_derivation_rules) && ygg::is_canonical(data.concept_substitution_rules)
           && ygg::is_canonical(data.role_substitution_rules) && ygg::is_canonical(data.boolean_substitution_rules)
           && ygg::is_canonical(data.numerical_substitution_rules);
}

inline void canonicalize(ygg::Data<Grammar>& data)
{
    ygg::canonicalize(data.concept_derivation_rules);
    ygg::canonicalize(data.role_derivation_rules);
    ygg::canonicalize(data.boolean_derivation_rules);
    ygg::canonicalize(data.numerical_derivation_rules);
    ygg::canonicalize(data.concept_substitution_rules);
    ygg::canonicalize(data.role_substitution_rules);
    ygg::canonicalize(data.boolean_substitution_rules);
    ygg::canonicalize(data.numerical_substitution_rules);
}

}  // namespace runir::kr::dl::cnf_grammar

#endif
