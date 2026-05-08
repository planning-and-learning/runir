#ifndef RUNIR_GRAMMAR_CANONICALIZATION_HPP_
#define RUNIR_GRAMMAR_CANONICALIZATION_HPP_

#include "runir/canonicalization.hpp"
#include "runir/grammar/concept_data.hpp"
#include "runir/grammar/derivation_rule_data.hpp"
#include "runir/grammar/grammar_data.hpp"
#include "runir/grammar/role_data.hpp"

#include <tyr/common/canonicalization.hpp>

namespace runir::grammar
{

template<runir::ConceptConstructorTag Tag>
bool is_canonical(const tyr::Data<Concept<Tag>>&) noexcept
{
    return true;
}

template<runir::RoleConstructorTag Tag>
bool is_canonical(const tyr::Data<Role<Tag>>&) noexcept
{
    return true;
}

template<runir::BooleanConstructorTag Tag>
bool is_canonical(const tyr::Data<Boolean<Tag>>&) noexcept
{
    return true;
}

template<runir::NumericalConstructorTag Tag>
bool is_canonical(const tyr::Data<Numerical<Tag>>&) noexcept
{
    return true;
}

template<runir::CategoryTag Category>
bool is_canonical(const tyr::Data<Constructor<Category>>&) noexcept
{
    return true;
}

template<runir::CategoryTag Category>
bool is_canonical(const tyr::Data<ConstructorOrNonTerminal<Category>>&) noexcept
{
    return true;
}

template<runir::CategoryTag Category>
bool is_canonical(const tyr::Data<NonTerminal<Category>>&) noexcept
{
    return true;
}

inline bool is_canonical(const tyr::Data<Concept<runir::IntersectionTag>>& data) noexcept { return !tyr::Less<decltype(data.lhs)> {}(data.rhs, data.lhs); }

inline bool is_canonical(const tyr::Data<Concept<runir::UnionTag>>& data) noexcept { return !tyr::Less<decltype(data.lhs)> {}(data.rhs, data.lhs); }

inline bool is_canonical(const tyr::Data<Concept<runir::RoleValueMapEqualityTag>>& data) noexcept
{
    return !tyr::Less<decltype(data.lhs)> {}(data.rhs, data.lhs);
}

inline bool is_canonical(const tyr::Data<Role<runir::IntersectionTag>>& data) noexcept { return !tyr::Less<decltype(data.lhs)> {}(data.rhs, data.lhs); }

inline bool is_canonical(const tyr::Data<Role<runir::UnionTag>>& data) noexcept { return !tyr::Less<decltype(data.lhs)> {}(data.rhs, data.lhs); }

template<runir::ConceptConstructorTag Tag>
void canonicalize(tyr::Data<Concept<Tag>>&) noexcept
{
    // Trivially canonical
}

template<runir::RoleConstructorTag Tag>
void canonicalize(tyr::Data<Role<Tag>>&) noexcept
{
    // Trivially canonical
}

template<runir::BooleanConstructorTag Tag>
void canonicalize(tyr::Data<Boolean<Tag>>&) noexcept
{
    // Trivially canonical
}

template<runir::NumericalConstructorTag Tag>
void canonicalize(tyr::Data<Numerical<Tag>>&) noexcept
{
    // Trivially canonical
}

template<runir::CategoryTag Category>
void canonicalize(tyr::Data<Constructor<Category>>&) noexcept
{
    // Trivially canonical
}

template<runir::CategoryTag Category>
void canonicalize(tyr::Data<ConstructorOrNonTerminal<Category>>&) noexcept
{
    // Trivially canonical
}

template<runir::CategoryTag Category>
void canonicalize(tyr::Data<NonTerminal<Category>>&) noexcept
{
    // Trivially canonical
}

inline void canonicalize(tyr::Data<Concept<runir::IntersectionTag>>& data) noexcept { runir::canonicalize_commutative_binary(data); }

inline void canonicalize(tyr::Data<Concept<runir::UnionTag>>& data) noexcept { runir::canonicalize_commutative_binary(data); }

inline void canonicalize(tyr::Data<Concept<runir::RoleValueMapEqualityTag>>& data) noexcept { runir::canonicalize_commutative_binary(data); }

inline void canonicalize(tyr::Data<Role<runir::IntersectionTag>>& data) noexcept { runir::canonicalize_commutative_binary(data); }

inline void canonicalize(tyr::Data<Role<runir::UnionTag>>& data) noexcept { runir::canonicalize_commutative_binary(data); }

template<runir::CategoryTag Category>
inline bool is_canonical(const tyr::Data<DerivationRule<Category>>& data)
{
    return tyr::is_canonical(data.rhs);
}

template<runir::CategoryTag Category>
inline void canonicalize(tyr::Data<DerivationRule<Category>>& data)
{
    tyr::canonicalize(data.rhs);
}

inline bool is_canonical(const tyr::Data<GrammarTag>& data)
{
    return tyr::is_canonical(data.concept_derivation_rules) && tyr::is_canonical(data.role_derivation_rules) && tyr::is_canonical(data.boolean_derivation_rules)
           && tyr::is_canonical(data.numerical_derivation_rules);
}

inline void canonicalize(tyr::Data<GrammarTag>& data)
{
    tyr::canonicalize(data.concept_derivation_rules);
    tyr::canonicalize(data.role_derivation_rules);
    tyr::canonicalize(data.boolean_derivation_rules);
    tyr::canonicalize(data.numerical_derivation_rules);
}

}  // namespace runir::grammar

#endif
