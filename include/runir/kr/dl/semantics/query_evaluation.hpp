#ifndef RUNIR_KR_DL_SEMANTICS_QUERY_EVALUATION_HPP_
#define RUNIR_KR_DL_SEMANTICS_QUERY_EVALUATION_HPP_

#include "runir/kr/dl/query_view.hpp"
#include "runir/kr/dl/semantics/evaluation.hpp"

#include <span>
#include <yggdrasil/database/operations.hpp>

namespace runir::kr::dl::semantics
{

template<FamilyTag Family, typename Tag, typename C, tyr::TaskKind Kind, typename Evaluate>
auto evaluate_query(ygg::View<ygg::Index<Query<Family, Tag>>, C> constructor,
                    EvaluationContext<Family, Kind>& context,
                    EvaluationWorkspace& workspace,
                    Evaluate&& evaluate_child) -> ygg::UniqueObjectPoolPtr<ygg::database::Relation<>>
{
    const auto& data = constructor.get_data();
    const auto schema = constructor.get_schema();
    if constexpr (std::same_as<Tag, QueryRenameTag>)
    {
        auto child = evaluate_child(constructor.get_arg());
        child->rename(schema);
        return child;
    }
    else
    {
        auto result = workspace.get_relations().get_or_allocate(schema);
        if constexpr (is_atomic_state_tag_v<Tag>)
        {
            auto& tuple = workspace.get_database_workspace().row;
            tuple.resize(schema.size());
            detail::for_each_current_atom<typename Tag::FactKind>(context,
                                                                  [&](auto atom)
                                                                  {
                                                                      if (atom.get_predicate().get_index() != data.predicate)
                                                                          return;
                                                                      for (size_t i = 0; i < tuple.size(); ++i)
                                                                          tuple[i] = ygg::uint_t(detail::object_index(atom, i));
                                                                      result->insert(std::span<const ygg::uint_t>(tuple));
                                                                  });
        }
        else if constexpr (is_atomic_goal_tag_v<Tag>)
        {
            auto& tuple = workspace.get_database_workspace().row;
            tuple.resize(schema.size());
            detail::for_each_goal_atom<typename Tag::FactKind>(context,
                                                               constructor.get_polarity(),
                                                               [&](auto atom)
                                                               {
                                                                   if (atom.get_predicate().get_index() != data.predicate)
                                                                       return;
                                                                   for (size_t i = 0; i < tuple.size(); ++i)
                                                                       tuple[i] = ygg::uint_t(detail::object_index(atom, i));
                                                                   result->insert(std::span<const ygg::uint_t>(tuple));
                                                               });
        }
        else if constexpr (std::same_as<Tag, QueryConceptTag>)
        {
            const auto child = evaluate_child(constructor.get_arg());
            const auto bits = detail::deref(child).get();
            for (auto object = bits.find_first(); object != decltype(bits)::npos; object = bits.find_next(object))
                result->insert({ static_cast<ygg::uint_t>(object) });
        }
        else if constexpr (std::same_as<Tag, QueryRoleTag>)
        {
            const auto child = evaluate_child(constructor.get_arg());
            for (ygg::uint_t source = 0; source < detail::num_objects(context); ++source)
            {
                const auto row = detail::deref(child).get(source);
                for (auto target = row.find_first(); target != decltype(row)::npos; target = row.find_next(target))
                    result->insert({ source, static_cast<ygg::uint_t>(target) });
            }
        }
        else if constexpr (std::same_as<Tag, QueryJoinTag> || std::same_as<Tag, QueryUnionTag> || std::same_as<Tag, QueryDifferenceTag>)
        {
            const auto lhs = evaluate_child(constructor.get_lhs());
            const auto rhs = evaluate_child(constructor.get_rhs());
            if constexpr (std::same_as<Tag, QueryJoinTag>)
                ygg::database::join(lhs->view(), rhs->view(), data.plan, *result, workspace.get_database_workspace());
            else if constexpr (std::same_as<Tag, QueryUnionTag>)
                ygg::database::union_(lhs->view(), rhs->view(), *result);
            else
                ygg::database::difference(lhs->view(), rhs->view(), *result);
        }
        else if constexpr (std::same_as<Tag, QueryProjectTag>)
        {
            const auto child = evaluate_child(constructor.get_arg());
            ygg::database::project(child->view(), data.plan, *result, workspace.get_database_workspace());
        }
        else if constexpr (std::same_as<Tag, QuerySelectEqualTag>)
        {
            const auto child = evaluate_child(constructor.get_arg());
            ygg::database::select(child->view(), [&](std::span<const ygg::uint_t> row) { return row[data.lhs_position] == row[data.rhs_position]; }, *result);
        }
        else if constexpr (std::same_as<Tag, QuerySelectValueTag>)
        {
            const auto child = evaluate_child(constructor.get_arg());
            const auto object = ygg::uint_t(constructor.get_object().get_index());
            ygg::database::select(child->view(), [&](std::span<const ygg::uint_t> row) { return row[data.position] == object; }, *result);
        }
        else
        {
            static_assert(ygg::dependent_false<Tag>::value, "unhandled relational query constructor");
        }
        return result;
    }
}

template<FamilyTag Family, tyr::TaskKind Kind, typename C>
auto evaluate_impl(ygg::View<ygg::Index<Query<Family>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace) -> ygg::UniqueObjectPoolPtr<ygg::database::Relation<>>
{
    return ygg::visit([&](auto child)
                      { return evaluate_query(child, context, workspace, [&](auto argument) { return evaluate_impl(argument, context, workspace); }); },
                      constructor.get_variant());
}

template<FamilyTag Family, tyr::TaskKind Kind, typename C>
auto evaluate(ygg::View<ygg::Index<Query<Family>>, C> constructor,
              EvaluationContext<Family, Kind>& context,
              EvaluationWorkspace& workspace,
              DenotationCaches<Family>& caches) -> ygg::UniqueObjectPoolPtr<ygg::database::Relation<>>
{
    return ygg::visit([&](auto child)
                      { return evaluate_query(child, context, workspace, [&](auto argument) { return evaluate(argument, context, workspace, caches); }); },
                      constructor.get_variant());
}

namespace detail
{

template<FamilyTag Family, CategoryTag Category, typename C, tyr::TaskKind Kind, typename Evaluate>
auto evaluate_query_projection(ygg::View<ygg::Index<QueryProjection<Family, Category>>, C> constructor,
                               EvaluationContext<Family, Kind>& context,
                               Evaluate&& evaluate_child) -> ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<Category>>>
{
    const auto positions = constructor.get_data().plan.positions();
    const auto query = evaluate_child(constructor.get_arg());
    auto result = context.get_builder().template get_builder<Denotation<Category>>(num_objects(context));
    if constexpr (std::same_as<Category, ConceptTag>)
    {
        auto bits = result->get();
        for (size_t i = 0; i < query->size(); ++i)
            bits.set((*query)[i][positions[0]]);
    }
    else
    {
        static_assert(std::same_as<Category, RoleTag>);
        for (size_t i = 0; i < query->size(); ++i)
        {
            const auto row = (*query)[i];
            result->get(row[positions[0]]).set(row[positions[1]]);
        }
    }
    return result;
}

}  // namespace detail

template<FamilyTag Family, CategoryTag Category, tyr::TaskKind Kind, typename C>
auto evaluate_impl(ygg::View<ygg::Index<QueryProjection<Family, Category>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace) -> ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<Category>>>
{
    return detail::evaluate_query_projection(constructor, context, [&](auto child) { return evaluate_impl(child, context, workspace); });
}

template<FamilyTag Family, CategoryTag Category, tyr::TaskKind Kind, typename C>
auto evaluate_impl(ygg::View<ygg::Index<QueryProjection<Family, Category>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace,
                   DenotationCaches<Family>& caches) -> ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<Category>>>
{
    return detail::evaluate_query_projection(constructor, context, [&](auto child) { return evaluate(child, context, workspace, caches); });
}

}  // namespace runir::kr::dl::semantics

#endif
