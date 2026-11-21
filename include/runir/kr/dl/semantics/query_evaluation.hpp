#ifndef RUNIR_KR_DL_SEMANTICS_QUERY_EVALUATION_HPP_
#define RUNIR_KR_DL_SEMANTICS_QUERY_EVALUATION_HPP_

#include "runir/kr/dl/object_rows.hpp"
#include "runir/kr/dl/query_view.hpp"
#include "runir/kr/dl/semantics/detail/evaluation_helpers.hpp"
#include "runir/kr/dl/semantics/evaluation_decl.hpp"

#include <array>
#include <runir/kr/database/semantics/generic_join.hpp>
#include <runir/kr/database/semantics/operations.hpp>
#include <span>
#include <tuple>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <vector>

namespace runir::kr::dl::semantics::detail
{

/// Materialize rows into an empty, matching builder without publishing the root.
template<tyr::TaskKind Kind, typename Tag, typename C, StateEvaluationContextConcept<Kind> Context>
    requires(!std::same_as<Tag, void>)
void evaluate_query(urd::index_view<Query<Tag>, C> constructor, Context& context, ygg::FreeScratchRelation& result)
{
    auto children = context.child_context();
    if constexpr (std::same_as<Tag, QueryRenameTag>)
    {
        const auto child = evaluate<Kind>(constructor.get_operand(), children);
        for (size_t i = 0; i < child.get_rows().size(); ++i)
            ygg::with_values(child.row(i), [&result](const auto& values) { result.insert(values); });
    }
    else if constexpr (is_atomic_state_tag_v<Tag>)
    {
        auto& row = context.get_workspace().get_values();
        const auto predicate = constructor.get_predicate();
        for (const auto atom : context.get_state().get_atoms(predicate))
            result.insert(encode_object_row(atom.get_row().get_values(), row));
    }
    else if constexpr (is_atomic_goal_tag_v<Tag>)
    {
        auto& row = context.get_workspace().get_values();
        const auto predicate = constructor.get_predicate();
        for (const auto atom :
             context.get_state().context().get_task()->get_task().get_goal().template get_atoms<TagFactKind<Tag>>(constructor.get_polarity(), predicate))
            result.insert(encode_object_row(atom.get_row().get_values(), row));
    }
    else if constexpr (std::same_as<Tag, QueryConceptTag>)
        insert_denotation_rows<ConceptTag>(evaluate<Kind>(constructor.get_operand(), children), result);
    else if constexpr (std::same_as<Tag, QueryRoleTag>)
        insert_denotation_rows<RoleTag>(evaluate<Kind>(constructor.get_operand(), children), result);
    else if constexpr (std::same_as<Tag, QueryJoinTag> || std::same_as<Tag, QueryUnionTag> || std::same_as<Tag, QueryDifferenceTag>)
    {
        const auto lhs = evaluate<Kind>(constructor.get_left_operand(), children);
        const auto rhs = evaluate<Kind>(constructor.get_right_operand(), children);
        if constexpr (std::same_as<Tag, QueryJoinTag>)
        {
            runir::kr::database::join(lhs, rhs, constructor.get_plan(), result, context.get_workspace().get_database_workspace());
        }
        else if constexpr (std::same_as<Tag, QueryUnionTag>)
            runir::kr::database::union_(lhs, rhs, constructor.get_plan(), result);
        else
            runir::kr::database::difference(lhs, rhs, constructor.get_plan(), result);
    }
    else if constexpr (std::same_as<Tag, QueryProjectTag>)
    {
        const auto child = evaluate<Kind>(constructor.get_operand(), children);
        runir::kr::database::project(child, constructor.get_plan(), result, context.get_workspace().get_database_workspace());
    }
    else if constexpr (std::same_as<Tag, QuerySelectEqualTag>)
    {
        const auto child = evaluate<Kind>(constructor.get_operand(), children);
        runir::kr::database::select_equal_columns(child, constructor.get_plan(), result);
    }
    else if constexpr (std::same_as<Tag, QuerySelectValueTag>)
    {
        const auto child = evaluate<Kind>(constructor.get_operand(), children);
        runir::kr::database::select_equal_value(child, constructor.get_plan(), result);
    }
    else if constexpr (std::same_as<Tag, QueryGenericJoinTag>)
    {
        urd::Store<urd::sequence<QueryDenotationView>> inputs;
        for (const auto arg : constructor.get_args())
            inputs.push_back(evaluate<Kind>(arg, children));
        const auto& plan = constructor.get_plan();
        runir::kr::database::generic_join(inputs, plan, result, context.get_workspace().get_generic_join_workspace(plan));
    }
    else if constexpr (std::same_as<Tag, QueryEmptyTag>)
    {
        // The result builder starts empty.
    }
    else if constexpr (std::same_as<Tag, QueryBooleanTag>)
    {
        if (evaluate<Kind>(constructor.get_operand(), children).get_value())
            result.insert(std::span<const ygg::Value> {});
    }
    else if constexpr (std::same_as<Tag, QueryNumericalTag>)
    {
        const auto value = std::array { ygg::encode_value(evaluate<Kind>(constructor.get_operand(), children).get_value()) };
        result.insert(std::span<const ygg::Value>(value));
    }
    else if constexpr (std::same_as<Tag, QueryHeadTag>)
        on_demand_head<Kind>(constructor, context, result);
    else
    {
        static_assert(false, "unhandled relational query constructor");
    }
}

}  // namespace runir::kr::dl::semantics::detail

#endif
