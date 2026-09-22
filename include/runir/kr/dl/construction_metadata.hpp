#ifndef RUNIR_KR_DL_CONSTRUCTION_METADATA_HPP_
#define RUNIR_KR_DL_CONSTRUCTION_METADATA_HPP_

#include "runir/kr/dl/constructor_data.hpp"
#include "runir/kr/dl/semantics/constructor_view.hpp"

#include <concepts>
#include <variant>
#include <yggdrasil/core/dependent_false.hpp>

namespace runir::kr::dl::detail
{

// Static means constant for a fixed task, including its goal and object universe.
template<template<typename, typename> typename Expression, FamilyTag Family, typename Tag, typename C>
bool infer_is_static(ygg::View<ygg::Index<Expression<Family, Tag>>, C> constructor)
{
    if constexpr (is_atomic_state_tag_v<Tag>)
        return std::same_as<typename Tag::FactKind, tyr::formalism::StaticTag>;
    else if constexpr (is_atomic_goal_tag_v<Tag>)
        return true;
    else if constexpr (requires { constructor.get_register(); } || requires { constructor.get_argument(); })
        return false;
    else if constexpr (requires { constructor.get_mid(); })
        return constructor.get_lhs().is_static() && constructor.get_mid().is_static() && constructor.get_rhs().is_static();
    else if constexpr (requires {
                           constructor.get_lhs();
                           constructor.get_rhs();
                       })
        return constructor.get_lhs().is_static() && constructor.get_rhs().is_static();
    else if constexpr (std::same_as<Tag, CountTag> || std::same_as<Tag, NonemptyTag>)
        return ygg::visit([](auto child) { return child.is_static(); }, constructor.get_arg());
    else if constexpr (requires { constructor.get_arg(); })
        return constructor.get_arg().is_static();
    else if constexpr (requires {
                           constructor.get_role();
                           constructor.get_concept();
                       })
        return constructor.get_role().is_static() && constructor.get_concept().is_static();
    else if constexpr (requires { constructor.get_role(); })
        return constructor.get_role().is_static();
    else if constexpr (std::same_as<Tag, BotTag> || std::same_as<Tag, TopTag> || std::same_as<Tag, UniversalTag> || std::same_as<Tag, OneOfTag>
                       || std::same_as<Tag, NominalTag> || std::same_as<Tag, BooleanConstantTag> || std::same_as<Tag, NumericalConstantTag>)
        return true;
    else
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL constructor staticness");
}

template<FamilyTag Family, CategoryTag Category>
void prepare(ygg::Data<Constructor<Family, Category>>& data, const ConstructorRepositoryFor<Family>& repository)
{
    data.is_static = std::visit([&](auto index) { return infer_is_static(ygg::make_view(index, repository)); }, data.variant);
}

template<FamilyTag Family>
void prepare(ygg::Data<Query<Family>>& data, const ConstructorRepositoryFor<Family>& repository)
{
    data.is_static = std::visit([&](auto index) { return infer_is_static(ygg::make_view(index, repository)); }, data.variant);
}

}  // namespace runir::kr::dl::detail

#endif
