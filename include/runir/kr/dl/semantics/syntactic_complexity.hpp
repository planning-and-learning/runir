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
std::size_t syntactic_complexity(const urd::index_view<runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, Tag>, C>& view)
{
    if constexpr (requires { view.operand(); })
        return 1 + syntactic_complexity(view.operand());
    else if constexpr (requires {
                           view.left_operand();
                           view.right_operand();
                       })
        return 1 + syntactic_complexity(view.left_operand()) + syntactic_complexity(view.right_operand());
    else if constexpr (requires {
                           view.role();
                           view.qualifier();
                       })
        return 1 + syntactic_complexity(view.role()) + syntactic_complexity(view.qualifier());
    else if constexpr (requires { view.role(); })
        return 1 + syntactic_complexity(view.role());
    else
        return 1;
}

template<typename Tag, typename C>
    requires runir::kr::dl::RoleConstructorTag<Tag>
std::size_t syntactic_complexity(const urd::index_view<runir::kr::dl::Role<runir::kr::dl::SemanticFamily, Tag>, C>& view)
{
    if constexpr (requires { view.operand(); })
        return 1 + syntactic_complexity(view.operand());
    else if constexpr (requires {
                           view.left_operand();
                           view.right_operand();
                       })
        return 1 + syntactic_complexity(view.left_operand()) + syntactic_complexity(view.right_operand());
    else if constexpr (requires {
                           view.role();
                           view.qualifier();
                       })
        return 1 + syntactic_complexity(view.role()) + syntactic_complexity(view.qualifier());
    else
        return 1;
}

template<typename Tag, typename C>
    requires runir::kr::dl::BooleanConstructorTag<Tag>
std::size_t syntactic_complexity(const urd::index_view<runir::kr::dl::Boolean<runir::kr::dl::SemanticFamily, Tag>, C>& view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::NonemptyTag>)
        return 1 + std::visit([](auto child) { return syntactic_complexity(child); }, view.operand());
    else if constexpr (requires { view.operand(); })
        return 1 + syntactic_complexity(view.operand());
    else if constexpr (requires {
                           view.left_operand();
                           view.right_operand();
                       })
        return 1 + syntactic_complexity(view.left_operand()) + syntactic_complexity(view.right_operand());
    else
        return 1;
}

template<typename Tag, typename C>
    requires runir::kr::dl::NumericalConstructorTag<Tag>
std::size_t syntactic_complexity(const urd::index_view<runir::kr::dl::Numerical<runir::kr::dl::SemanticFamily, Tag>, C>& view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::CountTag>)
        return 1 + std::visit([](auto child) { return syntactic_complexity(child); }, view.operand());
    else if constexpr (std::same_as<Tag, runir::kr::dl::DistanceTag>)
    {
        const auto complexity = [](auto argument) { return std::visit([](auto child) { return syntactic_complexity(child); }, argument); };
        return 1 + complexity(view.sources()) + complexity(view.edges()) + complexity(view.targets());
    }
    else if constexpr (std::same_as<Tag, runir::kr::dl::ValueTag>)
        return 1 + syntactic_complexity(view.operand());
    else if constexpr (requires {
                           view.left_operand();
                           view.right_operand();
                       })
        return 1 + syntactic_complexity(view.left_operand()) + syntactic_complexity(view.right_operand());
    else
        return 1;
}

template<typename Tag, typename C>
std::size_t syntactic_complexity(const urd::index_view<runir::kr::dl::Query<Tag>, C>& view)
{
    if constexpr (requires { view.args(); })
    {
        auto result = std::size_t { 1 };
        for (const auto arg : view.args())
            result += syntactic_complexity(arg);
        return result;
    }
    else if constexpr (requires { view.operand(); })
        return 1 + syntactic_complexity(view.operand());
    else if constexpr (requires {
                           view.left_operand();
                           view.right_operand();
                       })
        return 1 + syntactic_complexity(view.left_operand()) + syntactic_complexity(view.right_operand());
    else
        return 1;
}

template<runir::kr::dl::ConceptOrRoleTag Category, typename C>
std::size_t syntactic_complexity(const urd::index_view<runir::kr::dl::QueryProjection<Category>, C>& view)
{ return 1 + syntactic_complexity(view.operand()); }

}  // namespace runir::kr::dl::semantics

#endif
