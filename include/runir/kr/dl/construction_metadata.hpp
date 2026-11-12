#ifndef RUNIR_KR_DL_CONSTRUCTION_METADATA_HPP_
#define RUNIR_KR_DL_CONSTRUCTION_METADATA_HPP_

#include "runir/kr/dl/constructor_data.hpp"
#include "runir/kr/dl/semantics/constructor_view.hpp"

#include <algorithm>
#include <boost/mp11.hpp>
#include <concepts>
#include <variant>

namespace runir::kr::dl::detail
{

// Static means constant for a fixed task, including its goal and object universe.
template<template<typename...> typename Expression, typename... Args, typename C>
bool infer_is_static(urd::index_view<Expression<Args...>, C> constructor)
{
    using Tag = boost::mp11::mp_back<boost::mp11::mp_list<Args...>>;
    if constexpr (is_atomic_state_tag_v<Tag>)
        return std::same_as<TagFactKind<Tag>, tyr::formalism::StaticTag>;
    else if constexpr (is_atomic_goal_tag_v<Tag> || std::same_as<Tag, BotTag> || std::same_as<Tag, TopTag> || std::same_as<Tag, UniversalTag>
                       || std::same_as<Tag, OneOfTag> || std::same_as<Tag, NominalTag> || std::same_as<Tag, BooleanConstantTag>
                       || std::same_as<Tag, NumericalConstantTag> || std::same_as<Tag, QueryEmptyTag>)
        return true;
    else if constexpr (requires { constructor.get_register(); } || requires { constructor.get_argument(); })
        return false;
    else if constexpr (std::same_as<Tag, QueryHeadTag>)
        return constructor.get_program().is_static();
    else if constexpr (requires {
                           constructor.get_lhs();
                           constructor.get_rhs();
                       })
    {
        // An operand is a view, or a variant of views (comparisons of scalars).
        const auto is_static = [](auto argument)
        {
            if constexpr (requires { argument.is_static(); })
                return argument.is_static();
            else
                return std::visit([](auto child) { return child.is_static(); }, argument);
        };
        return is_static(constructor.get_lhs()) && is_static(constructor.get_rhs());
    }
    else if constexpr (requires { constructor.get_args(); })
        return std::ranges::all_of(constructor.get_args(), [](auto child) { return child.is_static(); });
    else if constexpr (std::same_as<Tag, CountTag> || std::same_as<Tag, NonemptyTag>)
        return std::visit([](auto child) { return child.is_static(); }, constructor.get_arg());
    else if constexpr (requires { constructor.get_arg(); })
        return constructor.get_arg().is_static();
    else if constexpr (requires {
                           constructor.get_role();
                           constructor.get_concept();
                       })
        return constructor.get_role().is_static() && constructor.get_concept().is_static();
    else if constexpr (requires { constructor.get_role(); })
        return constructor.get_role().is_static();
    else
        static_assert(false, "unhandled DL constructor staticness");
}

template<CategoryTag Category, typename Repository>
void prepare(urd::data<Constructor<Category>>& data, const Repository& repository)
{
    data.is_static = std::visit([&](auto index) { return infer_is_static(repository.view(index)); }, data.variant);
}

void prepare(urd::data<Query<>>& data, const ConstructorRepository& repository);

}  // namespace runir::kr::dl::detail

#endif
