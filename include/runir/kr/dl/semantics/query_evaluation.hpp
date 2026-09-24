#ifndef RUNIR_KR_DL_SEMANTICS_QUERY_EVALUATION_HPP_
#define RUNIR_KR_DL_SEMANTICS_QUERY_EVALUATION_HPP_

#include "runir/kr/dl/query_view.hpp"
#include "runir/kr/dl/semantics/evaluation.hpp"

#include <span>
#include <yggdrasil/database/operations.hpp>

namespace runir::kr::dl::semantics
{

template<FamilyTag Family, typename C, tyr::TaskKind Kind>
auto evaluate(ygg::View<ygg::Index<Query<Family, QueryRenameTag>>, C> constructor, StateEvaluationContext<Family, Kind>& context)
{
    return ygg::database::rename(evaluate(constructor.get_arg(), context), constructor.get_schema());
}

template<FamilyTag Family, typename Tag, typename C, tyr::TaskKind Kind>
    requires(!std::same_as<Tag, void> && !std::same_as<Tag, QueryRenameTag>)
auto evaluate(ygg::View<ygg::Index<Query<Family, Tag>>, C> constructor,
              StateEvaluationContext<Family, Kind>& context) -> ygg::UniqueObjectPoolPtr<ygg::database::Relation<>>
{
    const auto& data = constructor.get_data();
    const auto schema = constructor.get_schema();
    auto result = context.get_workspace().get_relations().get_or_allocate(schema);
    if constexpr (is_atomic_state_tag_v<Tag>)
    {
        auto& tuple = context.get_workspace().get_database_workspace().row;
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
        auto& tuple = context.get_workspace().get_database_workspace().row;
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
        const auto child = evaluate(constructor.get_arg(), context);
        const auto bits = child.get();
        for (auto object = bits.find_first(); object != decltype(bits)::npos; object = bits.find_next(object))
            result->insert({ static_cast<ygg::uint_t>(object) });
    }
    else if constexpr (std::same_as<Tag, QueryRoleTag>)
    {
        const auto child = evaluate(constructor.get_arg(), context);
        for (ygg::uint_t source = 0; source < detail::num_objects(context); ++source)
        {
            const auto row = child.get(source);
            for (auto target = row.find_first(); target != decltype(row)::npos; target = row.find_next(target))
                result->insert({ source, static_cast<ygg::uint_t>(target) });
        }
    }
    else if constexpr (std::same_as<Tag, QueryJoinTag> || std::same_as<Tag, QueryUnionTag> || std::same_as<Tag, QueryDifferenceTag>)
    {
        const auto lhs = evaluate(constructor.get_lhs(), context);
        const auto rhs = evaluate(constructor.get_rhs(), context);
        if constexpr (std::same_as<Tag, QueryJoinTag>)
        {
            const auto lhs_static = constructor.get_lhs().is_static();
            const auto rhs_static = constructor.get_rhs().is_static();
            // Fully static joins already retain their complete result.
            ygg::database::join(lhs,
                                rhs,
                                data.plan,
                                context.get_caches().get_static_join_indexes(),
                                { .lhs = lhs_static && !rhs_static, .rhs = rhs_static && !lhs_static },
                                *result,
                                context.get_workspace().get_database_workspace());
        }
        else if constexpr (std::same_as<Tag, QueryUnionTag>)
            ygg::database::union_(lhs, rhs, *result);
        else
            ygg::database::difference(lhs, rhs, *result);
    }
    else if constexpr (std::same_as<Tag, QueryProjectTag>)
    {
        const auto child = evaluate(constructor.get_arg(), context);
        ygg::database::project(child, data.plan, *result, context.get_workspace().get_database_workspace());
    }
    else if constexpr (std::same_as<Tag, QuerySelectEqualTag>)
    {
        const auto child = evaluate(constructor.get_arg(), context);
        ygg::database::select(child, [&](std::span<const ygg::uint_t> row) { return row[data.lhs_position] == row[data.rhs_position]; }, *result);
    }
    else if constexpr (std::same_as<Tag, QuerySelectValueTag>)
    {
        const auto child = evaluate(constructor.get_arg(), context);
        const auto object = ygg::uint_t(constructor.get_object().get_index());
        ygg::database::select(child, [&](std::span<const ygg::uint_t> row) { return row[data.position] == object; }, *result);
    }
    else
    {
        static_assert(ygg::dependent_false<Tag>::value, "unhandled relational query constructor");
    }
    return result;
}

template<FamilyTag Family, tyr::TaskKind Kind, typename C>
auto evaluate(ygg::View<ygg::Index<Query<Family>>, C> constructor, StateEvaluationContext<Family, Kind>& context) -> ygg::database::RelationView<>
{
    auto& cache = context.get_caches().get_queries(constructor.is_static());
    if (const auto it = cache.find(constructor); it != cache.end())
        return it->second;

    const auto result =
        ygg::visit([&](auto child) { return context.get_caches().retain(constructor.is_static(), evaluate(child, context)); }, constructor.get_variant());
    return cache.emplace(constructor, result).first->second;
}

template<FamilyTag Family, CategoryTag Category, tyr::TaskKind Kind, typename C>
auto evaluate_impl(ygg::View<ygg::Index<QueryProjection<Family, Category>>, C> constructor,
                   StateEvaluationContext<Family, Kind>& context) -> ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<Category>>>
{
    const auto positions = constructor.get_data().plan.positions();
    const auto relation = evaluate(constructor.get_arg(), context);
    auto result = context.get_builder().template get_builder<Denotation<Category>>(detail::num_objects(context));
    if constexpr (std::same_as<Category, ConceptTag>)
    {
        auto bits = result->get();
        for (size_t i = 0; i < relation.size(); ++i)
            bits.set(relation[i][positions[0]]);
    }
    else
    {
        static_assert(std::same_as<Category, RoleTag>);
        for (size_t i = 0; i < relation.size(); ++i)
        {
            const auto row = relation[i];
            result->get(row[positions[0]]).set(row[positions[1]]);
        }
    }
    return result;
}

}  // namespace runir::kr::dl::semantics

#endif
