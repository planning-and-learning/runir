#ifndef RUNIR_KR_DL_SEMANTICS_SYNTACTIC_COMPLEXITY_HPP_
#define RUNIR_KR_DL_SEMANTICS_SYNTACTIC_COMPLEXITY_HPP_

#include "runir/kr/dl/query_view.hpp"
#include "runir/kr/dl/semantics/views.hpp"

#include <concepts>
#include <cstddef>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::dl::semantics
{

template<typename Tag, typename C>
    requires runir::kr::dl::ConceptConstructorTag<Tag>
std::size_t syntactic_complexity(ygg::View<ygg::Index<runir::kr::dl::Concept<Tag>>, C> view);

template<typename Tag, typename C>
    requires runir::kr::dl::RoleConstructorTag<Tag>
std::size_t syntactic_complexity(ygg::View<ygg::Index<runir::kr::dl::Role<Tag>>, C> view);

template<typename Tag, typename C>
    requires runir::kr::dl::BooleanConstructorTag<Tag>
std::size_t syntactic_complexity(ygg::View<ygg::Index<runir::kr::dl::Boolean<Tag>>, C> view);

template<typename Tag, typename C>
    requires runir::kr::dl::NumericalConstructorTag<Tag>
std::size_t syntactic_complexity(ygg::View<ygg::Index<runir::kr::dl::Numerical<Tag>>, C> view);

template<typename Tag, typename C>
std::size_t syntactic_complexity(ygg::View<ygg::Index<runir::kr::dl::Query<Tag>>, C> view);

template<runir::kr::dl::ConceptOrRoleTag Category, typename C>
std::size_t syntactic_complexity(ygg::View<ygg::Index<runir::kr::dl::QueryProjection<Category>>, C> view);

template<runir::kr::dl::CategoryTag Category, typename C>
std::size_t syntactic_complexity(ygg::View<ygg::Index<runir::kr::dl::Constructor<Category>>, C> view)
{
    return ygg::visit([](auto child) { return syntactic_complexity(child); }, view.get_variant());
}

template<typename Tag, typename C>
    requires runir::kr::dl::ConceptConstructorTag<Tag>
std::size_t syntactic_complexity(ygg::View<ygg::Index<runir::kr::dl::Concept<Tag>>, C> view)
{
    if constexpr (requires { view.get_arg(); })
        return 1 + syntactic_complexity(view.get_arg());
    else if constexpr (requires {
                           view.get_lhs();
                           view.get_rhs();
                       })
        return 1 + syntactic_complexity(view.get_lhs()) + syntactic_complexity(view.get_rhs());
    else if constexpr (requires {
                           view.get_role();
                           view.get_concept();
                       })
        return 1 + syntactic_complexity(view.get_role()) + syntactic_complexity(view.get_concept());
    else if constexpr (requires { view.get_role(); })
        return 1 + syntactic_complexity(view.get_role());
    else
        return 1;
}

template<typename Tag, typename C>
    requires runir::kr::dl::RoleConstructorTag<Tag>
std::size_t syntactic_complexity(ygg::View<ygg::Index<runir::kr::dl::Role<Tag>>, C> view)
{
    if constexpr (requires { view.get_arg(); })
        return 1 + syntactic_complexity(view.get_arg());
    else if constexpr (requires {
                           view.get_lhs();
                           view.get_rhs();
                       })
        return 1 + syntactic_complexity(view.get_lhs()) + syntactic_complexity(view.get_rhs());
    else
        return 1;
}

template<typename Tag, typename C>
    requires runir::kr::dl::BooleanConstructorTag<Tag>
std::size_t syntactic_complexity(ygg::View<ygg::Index<runir::kr::dl::Boolean<Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::NonemptyTag>)
        return 1 + ygg::visit([](auto child) { return syntactic_complexity(child); }, view.get_arg());
    else if constexpr (requires { view.get_arg(); })
        return 1 + syntactic_complexity(view.get_arg());
    else if constexpr (requires {
                           view.get_lhs();
                           view.get_rhs();
                       })
        return 1 + syntactic_complexity(view.get_lhs()) + syntactic_complexity(view.get_rhs());
    else
        return 1;
}

template<typename Tag, typename C>
    requires runir::kr::dl::NumericalConstructorTag<Tag>
std::size_t syntactic_complexity(ygg::View<ygg::Index<runir::kr::dl::Numerical<Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::CountTag>)
        return 1 + ygg::visit([](auto child) { return syntactic_complexity(child); }, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ValueTag>)
        return 1 + syntactic_complexity(view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::DistanceTag>)
    {
        const auto complexity = [](auto argument) { return ygg::visit([](auto child) { return syntactic_complexity(child); }, argument); };
        return 1 + complexity(view.get_lhs()) + complexity(view.get_mid()) + complexity(view.get_rhs());
    }
    else if constexpr (requires {
                           view.get_lhs();
                           view.get_rhs();
                       })
        return 1 + syntactic_complexity(view.get_lhs()) + syntactic_complexity(view.get_rhs());
    else
        return 1;
}

template<typename Tag, typename C>
std::size_t syntactic_complexity(ygg::View<ygg::Index<runir::kr::dl::Query<Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, void>)
        return ygg::visit([](auto child) { return syntactic_complexity(child); }, view.get_variant());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QueryMustPassTag>)
        return 1 + syntactic_complexity(view.get_sources()) + syntactic_complexity(view.get_edges()) + syntactic_complexity(view.get_targets())
               + syntactic_complexity(view.get_probes());
    else if constexpr (requires { view.get_args(); })
    {
        auto result = std::size_t { 1 };
        for (const auto arg : view.get_args())
            result += syntactic_complexity(arg);
        return result;
    }
    else if constexpr (requires { view.get_arg(); })
        return 1 + syntactic_complexity(view.get_arg());
    else if constexpr (requires {
                           view.get_lhs();
                           view.get_rhs();
                       })
        return 1 + syntactic_complexity(view.get_lhs()) + syntactic_complexity(view.get_rhs());
    else
        return 1;
}

template<runir::kr::dl::ConceptOrRoleTag Category, typename C>
std::size_t syntactic_complexity(ygg::View<ygg::Index<runir::kr::dl::QueryProjection<Category>>, C> view)
{ return 1 + syntactic_complexity(view.get_arg()); }

}  // namespace runir::kr::dl::semantics

#endif
