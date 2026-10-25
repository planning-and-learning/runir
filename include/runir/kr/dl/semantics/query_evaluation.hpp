#ifndef RUNIR_KR_DL_SEMANTICS_QUERY_EVALUATION_HPP_
#define RUNIR_KR_DL_SEMANTICS_QUERY_EVALUATION_HPP_

#include "runir/kr/dl/object_rows.hpp"
#include "runir/kr/dl/query_view.hpp"
#include "runir/kr/dl/semantics/detail/evaluation_helpers.hpp"
#include "runir/kr/dl/semantics/evaluation_decl.hpp"

#include <span>
#include <tuple>
#include <vector>
#include <yggdrasil/database/semantics/generic_join.hpp>
#include <yggdrasil/database/semantics/operations.hpp>

namespace runir::kr::dl::semantics::detail
{

/// Materialize rows into an empty, matching builder without publishing the root.
template<tyr::TaskKind Kind, typename Tag, typename C, StateEvaluationContextConcept<Kind> Context>
    requires(!std::same_as<Tag, void>)
void evaluate_query(ygg::View<ygg::Index<Query<Tag>>, C> constructor, Context& context, ygg::Builder<ygg::database::Relation>& result)
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
            result.insert(encode_object_row(atom.get_row().get_data(), row));
    }
    else if constexpr (is_atomic_goal_tag_v<Tag>)
    {
        auto& row = context.get_workspace().get_database_workspace().row;
        const auto predicate = constructor.get_predicate();
        for (const auto atom :
             context.get_state().get_task().get_task().get_goal().template get_atoms_view<TagFactKind<Tag>>(constructor.get_polarity(), predicate))
            result.insert(encode_object_row(atom.get_row().get_data(), row));
    }
    else if constexpr (std::same_as<Tag, QueryConceptTag>)
        insert_denotation_rows<ConceptTag>(evaluate<Kind>(constructor.get_arg(), children), result);
    else if constexpr (std::same_as<Tag, QueryRoleTag>)
        insert_denotation_rows<RoleTag>(evaluate<Kind>(constructor.get_arg(), children), result);
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
        ygg::database::select_equal_value(child, ygg::Index<ygg::database::Column>(data.column.get_value()), data.object.get_value(), result);
    }
    else if constexpr (std::same_as<Tag, QueryGenericJoinTag>)
    {
        std::vector<QueryDenotationView> inputs;
        for (const auto arg : constructor.get_args())
            inputs.push_back(evaluate<Kind>(arg, children));
        ygg::database::generic_join(inputs, data.plan, result, context.get_workspace().get_generic_join_workspace(data.plan));
    }
    else if constexpr (std::same_as<Tag, QueryEmptyTag>)
    {
        // The result builder starts empty.
    }
    else if constexpr (std::same_as<Tag, QueryMustPassTag>)
        on_demand_must_pass<Kind>(constructor, context, result);
    else if constexpr (std::same_as<Tag, QueryHeadTag>)
        on_demand_head<Kind>(constructor, context, result);
    else
    {
        static_assert(ygg::dependent_false<Tag>::value, "unhandled relational query constructor");
    }
}

}  // namespace runir::kr::dl::semantics::detail

#endif
