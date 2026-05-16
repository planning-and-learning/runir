#ifndef RUNIR_KR_DL_SEMANTICS_SYNTACTIC_COMPLEXITY_HPP_
#define RUNIR_KR_DL_SEMANTICS_SYNTACTIC_COMPLEXITY_HPP_

#include "runir/kr/dl/semantics/views.hpp"

#include <concepts>
#include <cstddef>
#include <tyr/common/types.hpp>

namespace runir::kr::dl::semantics
{

template<runir::kr::dl::ConceptConstructorTag Tag, typename C>
std::size_t syntactic_complexity(tyr::View<tyr::Index<runir::kr::dl::Concept<Tag>>, C> view);

template<runir::kr::dl::RoleConstructorTag Tag, typename C>
std::size_t syntactic_complexity(tyr::View<tyr::Index<runir::kr::dl::Role<Tag>>, C> view);

template<runir::kr::dl::BooleanConstructorTag Tag, typename C>
std::size_t syntactic_complexity(tyr::View<tyr::Index<runir::kr::dl::Boolean<Tag>>, C> view);

template<runir::kr::dl::NumericalConstructorTag Tag, typename C>
std::size_t syntactic_complexity(tyr::View<tyr::Index<runir::kr::dl::Numerical<Tag>>, C> view);

template<runir::kr::dl::CategoryTag Category, typename C>
std::size_t syntactic_complexity(tyr::View<tyr::Index<runir::kr::dl::Constructor<Category>>, C> view)
{
    return view.get_variant().apply([](auto child) { return syntactic_complexity(child); });
}

template<runir::kr::dl::ConceptConstructorTag Tag, typename C>
std::size_t syntactic_complexity(tyr::View<tyr::Index<runir::kr::dl::Concept<Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::NegationTag>)
        return 1 + syntactic_complexity(view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag> || std::same_as<Tag, runir::kr::dl::UnionTag>
                       || std::same_as<Tag, runir::kr::dl::ValueRestrictionTag> || std::same_as<Tag, runir::kr::dl::ExistentialQuantificationTag>
                       || std::same_as<Tag, runir::kr::dl::RoleValueMapContainmentTag> || std::same_as<Tag, runir::kr::dl::RoleValueMapEqualityTag>)
        return 1 + syntactic_complexity(view.get_lhs()) + syntactic_complexity(view.get_rhs());
    else
        return 1;
}

template<runir::kr::dl::RoleConstructorTag Tag, typename C>
std::size_t syntactic_complexity(tyr::View<tyr::Index<runir::kr::dl::Role<Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::ComplementTag> || std::same_as<Tag, runir::kr::dl::InverseTag>
                  || std::same_as<Tag, runir::kr::dl::TransitiveClosureTag> || std::same_as<Tag, runir::kr::dl::ReflexiveTransitiveClosureTag>
                  || std::same_as<Tag, runir::kr::dl::IdentityTag>)
        return 1 + syntactic_complexity(view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag> || std::same_as<Tag, runir::kr::dl::UnionTag>
                       || std::same_as<Tag, runir::kr::dl::CompositionTag> || std::same_as<Tag, runir::kr::dl::RestrictionTag>)
        return 1 + syntactic_complexity(view.get_lhs()) + syntactic_complexity(view.get_rhs());
    else
        return 1;
}

template<runir::kr::dl::BooleanConstructorTag Tag, typename C>
std::size_t syntactic_complexity(tyr::View<tyr::Index<runir::kr::dl::Boolean<Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::NonemptyTag>)
        return 1 + view.get_arg().apply([](auto child) { return syntactic_complexity(child); });
    else
        return 1;
}

template<runir::kr::dl::NumericalConstructorTag Tag, typename C>
std::size_t syntactic_complexity(tyr::View<tyr::Index<runir::kr::dl::Numerical<Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::CountTag>)
        return 1 + view.get_arg().apply([](auto child) { return syntactic_complexity(child); });
    else
        return 1 + syntactic_complexity(view.get_lhs()) + syntactic_complexity(view.get_mid()) + syntactic_complexity(view.get_rhs());
}

}  // namespace runir::kr::dl::semantics

#endif
