#ifndef RUNIR_SEMANTICS_CANONICALIZATION_HPP_
#define RUNIR_SEMANTICS_CANONICALIZATION_HPP_

#include "runir/knowledge_representation/dl/canonicalization.hpp"
#include "runir/knowledge_representation/dl/declarations.hpp"
#include "runir/knowledge_representation/dl/semantics/concept_data.hpp"
#include "runir/knowledge_representation/dl/semantics/declarations.hpp"
#include "runir/knowledge_representation/dl/semantics/denotation_data.hpp"
#include "runir/knowledge_representation/dl/semantics/role_data.hpp"

#include <tyr/common/comparators.hpp>
#include <tyr/common/types.hpp>

namespace runir::kr::dl
{

inline bool is_canonical(const tyr::Data<Concept<IntersectionTag>>& data) noexcept { return !tyr::Less<decltype(data.lhs)> {}(data.rhs, data.lhs); }

inline bool is_canonical(const tyr::Data<Concept<UnionTag>>& data) noexcept { return !tyr::Less<decltype(data.lhs)> {}(data.rhs, data.lhs); }

inline bool is_canonical(const tyr::Data<Concept<RoleValueMapEqualityTag>>& data) noexcept { return !tyr::Less<decltype(data.lhs)> {}(data.rhs, data.lhs); }

inline bool is_canonical(const tyr::Data<Role<IntersectionTag>>& data) noexcept { return !tyr::Less<decltype(data.lhs)> {}(data.rhs, data.lhs); }

inline bool is_canonical(const tyr::Data<Role<UnionTag>>& data) noexcept { return !tyr::Less<decltype(data.lhs)> {}(data.rhs, data.lhs); }

inline void canonicalize(tyr::Data<Concept<IntersectionTag>>& data) noexcept { canonicalize_commutative_binary(data); }

inline void canonicalize(tyr::Data<Concept<UnionTag>>& data) noexcept { canonicalize_commutative_binary(data); }

inline void canonicalize(tyr::Data<Concept<RoleValueMapEqualityTag>>& data) noexcept { canonicalize_commutative_binary(data); }

inline void canonicalize(tyr::Data<Role<IntersectionTag>>& data) noexcept { canonicalize_commutative_binary(data); }

inline void canonicalize(tyr::Data<Role<UnionTag>>& data) noexcept { canonicalize_commutative_binary(data); }

}  // namespace runir

namespace runir::kr::dl::semantics
{

template<CategoryTag Category>
bool is_canonical(const tyr::Data<Denotation<Category>>&) noexcept
{
    return true;
}

template<CategoryTag Category>
void canonicalize(tyr::Data<Denotation<Category>>&) noexcept
{
    // Trivially canonical
}

}  // namespace runir::kr::dl::semantics

#endif
