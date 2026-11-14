#ifndef RUNIR_KR_DL_SEMANTICS_SYNTACTIC_COMPLEXITY_HPP_
#define RUNIR_KR_DL_SEMANTICS_SYNTACTIC_COMPLEXITY_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/query_view.hpp"
#include "runir/kr/dl/record_view.hpp"
#include "runir/kr/dl/semantics/call_arguments_view.hpp"
#include "runir/kr/dl/semantics/constructor_view.hpp"
#include "runir/kr/dl/semantics/denotation_view.hpp"
#include "runir/kr/dl/semantics/register_values_view.hpp"

#include <concepts>
#include <cstddef>
#include <urd/elements/flat.hpp>

namespace runir::kr::dl::semantics
{

template<typename Tag, typename C>
    requires runir::kr::dl::ConceptConstructorTag<Tag>
std::size_t syntactic_complexity(urd::index_view<runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, Tag>, C> view);

template<typename Tag, typename C>
    requires runir::kr::dl::RoleConstructorTag<Tag>
std::size_t syntactic_complexity(urd::index_view<runir::kr::dl::Role<runir::kr::dl::SemanticFamily, Tag>, C> view);

template<typename Tag, typename C>
    requires runir::kr::dl::BooleanConstructorTag<Tag>
std::size_t syntactic_complexity(urd::index_view<runir::kr::dl::Boolean<runir::kr::dl::SemanticFamily, Tag>, C> view);

template<typename Tag, typename C>
    requires runir::kr::dl::NumericalConstructorTag<Tag>
std::size_t syntactic_complexity(urd::index_view<runir::kr::dl::Numerical<runir::kr::dl::SemanticFamily, Tag>, C> view);

template<typename Tag, typename C>
std::size_t syntactic_complexity(urd::index_view<runir::kr::dl::Query<Tag>, C> view);

template<runir::kr::dl::ConceptOrRoleTag Category, typename C>
std::size_t syntactic_complexity(urd::index_view<runir::kr::dl::QueryProjection<Category>, C> view);

/// The complexity of a constructor or a query of any kind. Each visits its operands, of every kind in turn, so the
/// library compiles it once instead of every file that measures one (brokkr's template rules, rule 24).
std::size_t syntactic_complexity(ConstructorView<ConceptTag> view);
std::size_t syntactic_complexity(ConstructorView<RoleTag> view);
std::size_t syntactic_complexity(ConstructorView<BooleanTag> view);
std::size_t syntactic_complexity(ConstructorView<NumericalTag> view);
std::size_t syntactic_complexity(QueryView<> view);

template<typename Tag, typename C>
    requires runir::kr::dl::ConceptConstructorTag<Tag>
std::size_t syntactic_complexity(urd::index_view<runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, Tag>, C> view)
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
std::size_t syntactic_complexity(urd::index_view<runir::kr::dl::Role<runir::kr::dl::SemanticFamily, Tag>, C> view)
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
std::size_t syntactic_complexity(urd::index_view<runir::kr::dl::Boolean<runir::kr::dl::SemanticFamily, Tag>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::NonemptyTag>)
        return 1 + std::visit([](auto child) { return syntactic_complexity(child); }, view.get_arg());
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
std::size_t syntactic_complexity(urd::index_view<runir::kr::dl::Numerical<runir::kr::dl::SemanticFamily, Tag>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::CountTag>)
        return 1 + std::visit([](auto child) { return syntactic_complexity(child); }, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::DistanceTag>)
    {
        const auto complexity = [](auto argument) { return std::visit([](auto child) { return syntactic_complexity(child); }, argument); };
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
std::size_t syntactic_complexity(urd::index_view<runir::kr::dl::Query<Tag>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::QueryMustPassTag>)
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
std::size_t syntactic_complexity(urd::index_view<runir::kr::dl::QueryProjection<Category>, C> view)
{ return 1 + syntactic_complexity(view.get_arg()); }

}  // namespace runir::kr::dl::semantics

#endif
