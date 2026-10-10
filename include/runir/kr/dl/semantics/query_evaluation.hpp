#ifndef RUNIR_KR_DL_SEMANTICS_QUERY_EVALUATION_HPP_
#define RUNIR_KR_DL_SEMANTICS_QUERY_EVALUATION_HPP_

#include "runir/kr/dl/query_view.hpp"
#include "runir/kr/dl/semantics/evaluation.hpp"

#include <span>
#include <tuple>
#include <yggdrasil/database/semantics/operations.hpp>

namespace runir::kr::dl::semantics
{

template<tyr::TaskKind Kind, FamilyTag Family, typename C, StateEvaluationContextConcept<Family, Kind> Context>
auto evaluate_impl(ygg::View<ygg::Index<Query<Family, QueryRenameTag>>, C> constructor, Context& context) -> QueryDenotationView
{
    // Keep rows in their owning output partition, even when a low-level query
    // wrapper conservatively marks a static child as dynamic.
    const auto child = evaluate<Kind>(constructor.get_arg(), context);
    auto static_output = context.for_result(true);
    auto dynamic_output = context.for_result(false);
    auto& static_results = static_output.get_denotation_repository().get_relation_repository();
    auto& results = &child.get_context() == &static_results ? static_results : dynamic_output.get_denotation_repository().get_relation_repository();
    return results.rename(child, constructor.get_schema().span());
}

namespace detail
{

/// Materialize rows into an empty, matching builder without publishing the root.
template<tyr::TaskKind Kind, FamilyTag Family, typename Tag, typename C, StateEvaluationContextConcept<Family, Kind> Context>
    requires(!std::same_as<Tag, void>)
void evaluate_query(ygg::View<ygg::Index<Query<Family, Tag>>, C> constructor,
                    Context& context,
                    ygg::Builder<ygg::database::Relation<ObjectValues>>& result)
{
    const auto& data = constructor.get_data();
    auto children = context.child_context();
    if constexpr (std::same_as<Tag, QueryRenameTag>)
    {
        const auto child = evaluate<Kind>(constructor.get_arg(), children);
        for (size_t i = 0; i < child.size(); ++i)
            result.insert(child.row(i));
    }
    else if constexpr (is_atomic_state_tag_v<Tag>)
    {
        auto& row = context.get_workspace().get_database_workspace().row;
        const auto predicate = constructor.get_predicate();
        for (const auto atom : context.get_state().get_atoms_view(predicate))
            result.insert(ygg::database::encode_row<ObjectValues, ygg::Index<tyr::formalism::Object>>(atom.get_row().get_data(), row));
    }
    else if constexpr (is_atomic_goal_tag_v<Tag>)
    {
        auto& row = context.get_workspace().get_database_workspace().row;
        const auto predicate = constructor.get_predicate();
        for (const auto atom :
             context.get_state().get_task().get_task().get_goal().template get_atoms_view<typename Tag::FactKind>(constructor.get_polarity(), predicate))
            result.insert(ygg::database::encode_row<ObjectValues, ygg::Index<tyr::formalism::Object>>(atom.get_row().get_data(), row));
    }
    else if constexpr (std::same_as<Tag, QueryConceptTag>)
    {
        const auto child = evaluate<Kind>(constructor.get_arg(), children);
        for (const auto object : child.indices())
            result.insert(std::tuple { object });
    }
    else if constexpr (std::same_as<Tag, QueryRoleTag>)
    {
        const auto child = evaluate<Kind>(constructor.get_arg(), children);
        for (const auto [source, target] : child.indices())
            result.insert(std::tuple { source, target });
    }
    else if constexpr (std::same_as<Tag, QueryJoinTag> || std::same_as<Tag, QueryUnionTag> || std::same_as<Tag, QueryDifferenceTag>)
    {
        const auto lhs = evaluate<Kind>(constructor.get_lhs(), children);
        const auto rhs = evaluate<Kind>(constructor.get_rhs(), children);
        if constexpr (std::same_as<Tag, QueryJoinTag>)
        {
            const auto lhs_static = constructor.get_lhs().is_static();
            const auto rhs_static = constructor.get_rhs().is_static();
            // Fully static joins already retain their complete result.
            ygg::database::join(lhs,
                                rhs,
                                data.plan,
                                children.get_caches().get_static_join_indexes(),
                                { .lhs = lhs_static && !rhs_static, .rhs = rhs_static && !lhs_static },
                                result,
                                context.get_workspace().get_database_workspace());
        }
        else if constexpr (std::same_as<Tag, QueryUnionTag>)
            ygg::database::union_(lhs, rhs, result);
        else
            ygg::database::difference(lhs, rhs, result);
    }
    else if constexpr (std::same_as<Tag, QueryProjectTag>)
    {
        const auto child = evaluate<Kind>(constructor.get_arg(), children);
        ygg::database::project(child, data.plan, result, context.get_workspace().get_database_workspace());
    }
    else if constexpr (std::same_as<Tag, QuerySelectEqualTag>)
    {
        const auto child = evaluate<Kind>(constructor.get_arg(), children);
        ygg::database::select_equal_columns(child,
                                            ygg::Index<ygg::database::Column>(data.lhs_column.get_value()),
                                            ygg::Index<ygg::database::Column>(data.rhs_column.get_value()),
                                            result);
    }
    else if constexpr (std::same_as<Tag, QuerySelectValueTag>)
    {
        const auto child = evaluate<Kind>(constructor.get_arg(), children);
        ygg::database::select_equal_value(child, ygg::Index<ygg::database::Column>(data.column.get_value()), data.object, result);
    }
    else
    {
        static_assert(ygg::dependent_false<Tag>::value, "unhandled relational query constructor");
    }
}

}  // namespace detail

template<tyr::TaskKind Kind, FamilyTag Family, typename Tag, typename C, StateEvaluationContextConcept<Family, Kind> Context>
    requires(!std::same_as<Tag, void> && !std::same_as<Tag, QueryRenameTag>)
auto evaluate_impl(ygg::View<ygg::Index<Query<Family, Tag>>, C> constructor, Context& context) -> QueryDenotationView
{
    auto result = context.get_builder().template get_builder<ygg::database::Relation<ObjectValues>>(constructor.get_schema().span());
    detail::evaluate_query<Kind>(constructor, context, *result);
    return insert(context.get_denotation_repository(), *result).first;
}

template<tyr::TaskKind Kind, FamilyTag Family, StateEvaluationContextConcept<Family, Kind> Context, typename C>
auto evaluate(ygg::View<ygg::Index<Query<Family>>, C> constructor, Context& context) -> QueryDenotationView
{
    auto& cache = context.get_caches().get_queries(constructor.is_static());
    if (const auto it = cache.find(constructor); it != cache.end())
        return it->second;

    auto output = context.for_result(constructor.is_static());
    const auto result = ygg::visit([&](auto child) { return evaluate_impl<Kind>(child, output); }, constructor.get_variant());
    return cache.emplace(constructor, result).first->second;
}

template<tyr::TaskKind Kind, FamilyTag Family, ConceptOrRoleTag Category, StateEvaluationContextConcept<Family, Kind> Context, typename C>
auto evaluate_impl(ygg::View<ygg::Index<QueryProjection<Family, Category>>, C> constructor, Context& context) -> DenotationView<Category>
{
    const auto& columns = constructor.get_data().columns;
    auto children = context.child_context();
    const auto relation = evaluate<Kind>(constructor.get_arg(), children);
    const auto position = [&](size_t i) { return relation.column_index(ygg::Index<ygg::database::Column>(columns[i].get_value())); };
    auto result = context.get_builder().template get_builder<Denotation<Category>>(detail::num_objects<Kind, Family>(context));
    if constexpr (std::same_as<Category, ConceptTag>)
    {
        auto bits = result->get();
        const auto object = position(0);
        for (const auto row : relation)
            bits.set(row.template get<ygg::Index<tyr::formalism::Object>>(object).get_value());
    }
    else
    {
        static_assert(std::same_as<Category, RoleTag>);
        const auto source = position(0);
        const auto target = position(1);
        for (const auto row : relation)
            result->get(row.template get<ygg::Index<tyr::formalism::Object>>(source).get_value())
                .set(row.template get<ygg::Index<tyr::formalism::Object>>(target).get_value());
    }
    return insert(context.get_denotation_repository(), *result, context.get_builder()).first;
}

}  // namespace runir::kr::dl::semantics

#endif
