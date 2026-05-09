#ifndef RUNIR_GRAMMAR_CANONICALIZATION_HPP_
#define RUNIR_GRAMMAR_CANONICALIZATION_HPP_

#include "runir/knowledge_representation/dl/canonicalization.hpp"
#include "runir/knowledge_representation/dl/grammar/concept_data.hpp"
#include "runir/knowledge_representation/dl/grammar/derivation_rule_data.hpp"
#include "runir/knowledge_representation/dl/grammar/grammar_data.hpp"
#include "runir/knowledge_representation/dl/grammar/role_data.hpp"

#include <tyr/common/canonicalization.hpp>

namespace runir::kr::dl::grammar
{

template<runir::kr::dl::ConceptConstructorTag Tag>
bool is_canonical(const tyr::Data<Concept<Tag>>&) noexcept
{
    return true;
}

template<runir::kr::dl::RoleConstructorTag Tag>
bool is_canonical(const tyr::Data<Role<Tag>>&) noexcept
{
    return true;
}

template<runir::kr::dl::BooleanConstructorTag Tag>
bool is_canonical(const tyr::Data<Boolean<Tag>>&) noexcept
{
    return true;
}

template<runir::kr::dl::NumericalConstructorTag Tag>
bool is_canonical(const tyr::Data<Numerical<Tag>>&) noexcept
{
    return true;
}

template<runir::kr::dl::CategoryTag Category>
bool is_canonical(const tyr::Data<Constructor<Category>>&) noexcept
{
    return true;
}

template<runir::kr::dl::CategoryTag Category>
bool is_canonical(const tyr::Data<ConstructorOrNonTerminal<Category>>&) noexcept
{
    return true;
}

template<runir::kr::dl::CategoryTag Category>
bool is_canonical(const tyr::Data<NonTerminal<Category>>&) noexcept
{
    return true;
}

inline bool is_canonical(const tyr::Data<Concept<runir::kr::dl::IntersectionTag>>& data) noexcept
{
    return !tyr::Less<decltype(data.lhs)> {}(data.rhs, data.lhs);
}

inline bool is_canonical(const tyr::Data<Concept<runir::kr::dl::UnionTag>>& data) noexcept { return !tyr::Less<decltype(data.lhs)> {}(data.rhs, data.lhs); }

inline bool is_canonical(const tyr::Data<Concept<runir::kr::dl::RoleValueMapEqualityTag>>& data) noexcept
{
    return !tyr::Less<decltype(data.lhs)> {}(data.rhs, data.lhs);
}

inline bool is_canonical(const tyr::Data<Role<runir::kr::dl::IntersectionTag>>& data) noexcept { return !tyr::Less<decltype(data.lhs)> {}(data.rhs, data.lhs); }

inline bool is_canonical(const tyr::Data<Role<runir::kr::dl::UnionTag>>& data) noexcept { return !tyr::Less<decltype(data.lhs)> {}(data.rhs, data.lhs); }

template<runir::kr::dl::ConceptConstructorTag Tag>
void canonicalize(tyr::Data<Concept<Tag>>&) noexcept
{
    // Trivially canonical
}

template<runir::kr::dl::RoleConstructorTag Tag>
void canonicalize(tyr::Data<Role<Tag>>&) noexcept
{
    // Trivially canonical
}

template<runir::kr::dl::BooleanConstructorTag Tag>
void canonicalize(tyr::Data<Boolean<Tag>>&) noexcept
{
    // Trivially canonical
}

template<runir::kr::dl::NumericalConstructorTag Tag>
void canonicalize(tyr::Data<Numerical<Tag>>&) noexcept
{
    // Trivially canonical
}

template<runir::kr::dl::CategoryTag Category>
void canonicalize(tyr::Data<Constructor<Category>>&) noexcept
{
    // Trivially canonical
}

template<runir::kr::dl::CategoryTag Category>
void canonicalize(tyr::Data<ConstructorOrNonTerminal<Category>>&) noexcept
{
    // Trivially canonical
}

template<runir::kr::dl::CategoryTag Category>
void canonicalize(tyr::Data<NonTerminal<Category>>&) noexcept
{
    // Trivially canonical
}

inline void canonicalize(tyr::Data<Concept<runir::kr::dl::IntersectionTag>>& data) noexcept { runir::kr::dl::canonicalize_commutative_binary(data); }

inline void canonicalize(tyr::Data<Concept<runir::kr::dl::UnionTag>>& data) noexcept { runir::kr::dl::canonicalize_commutative_binary(data); }

inline void canonicalize(tyr::Data<Concept<runir::kr::dl::RoleValueMapEqualityTag>>& data) noexcept { runir::kr::dl::canonicalize_commutative_binary(data); }

inline void canonicalize(tyr::Data<Role<runir::kr::dl::IntersectionTag>>& data) noexcept { runir::kr::dl::canonicalize_commutative_binary(data); }

inline void canonicalize(tyr::Data<Role<runir::kr::dl::UnionTag>>& data) noexcept { runir::kr::dl::canonicalize_commutative_binary(data); }

template<runir::kr::dl::CategoryTag Category>
inline bool is_canonical(const tyr::Data<DerivationRule<Category>>& data)
{
    return tyr::is_canonical(data.rhs);
}

template<runir::kr::dl::CategoryTag Category>
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

}  // namespace runir::kr::dl::grammar

#endif
