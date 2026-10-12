#ifndef RUNIR_KR_DL_COMPLEMENT_HPP_
#define RUNIR_KR_DL_COMPLEMENT_HPP_

#include "runir/kr/dl/query_view.hpp"

namespace runir::kr::dl::detail
{

template<typename V>
inline constexpr bool is_complement_role_view_v = false;
template<FamilyTag Family, typename C>
inline constexpr bool is_complement_role_view_v<ygg::View<ygg::Index<FamilyRole<Family, ComplementTag>>, C>> = true;
template<typename V>
inline constexpr bool is_negation_concept_view_v = false;
template<FamilyTag Family, typename C>
inline constexpr bool is_negation_concept_view_v<ygg::View<ygg::Index<FamilyConcept<Family, NegationTag>>, C>> = true;

/// Whether a query is a role or concept leaf over a complement (r_complement, c_not).
template<FamilyTag Family, typename C>
bool is_complement_leaf(ygg::View<ygg::Index<Query<Family>>, C> query)
{
    return ygg::visit(
        []<typename Tag>(ygg::View<ygg::Index<Query<Family, Tag>>, C> concrete)
        {
            if constexpr (std::same_as<Tag, QueryRoleTag>)
                return ygg::visit([]<typename R>(R) { return is_complement_role_view_v<R>; }, concrete.get_arg().get_variant());
            else if constexpr (std::same_as<Tag, QueryConceptTag>)
                return ygg::visit([]<typename R>(R) { return is_negation_concept_view_v<R>; }, concrete.get_arg().get_variant());
            else
                return false;
        },
        query.get_variant());
}

}  // namespace runir::kr::dl::detail

#endif
