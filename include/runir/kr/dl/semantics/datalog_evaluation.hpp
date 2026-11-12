#ifndef RUNIR_KR_DL_SEMANTICS_DATALOG_EVALUATION_HPP_
#define RUNIR_KR_DL_SEMANTICS_DATALOG_EVALUATION_HPP_

#include "runir/kr/dl/semantics/datalog_feature.hpp"
#include "runir/kr/dl/semantics/evaluation_decl.hpp"
#include "runir/kr/dl/semantics/state_evaluation_context.hpp"

#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

namespace runir::kr::dl::semantics::detail
{

/// q_head in full evaluation: the program is evaluated once per state for all its heads (DatalogFeature), and the
/// head's rows are copied into the query's columns.
template<tyr::TaskKind Kind, StateEvaluationContextConcept<Kind> Context, typename C>
void on_demand_head(urd::index_view<Query<QueryHeadTag>, C> constructor, Context& context, urd::builder<ygg::Relation<ygg::Schema>>& result)
{
    const auto program = constructor.get_program();
    auto& workspace = context.get_workspace();
    auto& feature = workspace.get_datalog_feature(context.get_state().get_formalism_repository(),
                                                  constructor.context(),
                                                  program.index().value,
                                                  [&] { return make_datalog_feature(workspace.get_layer(), program); });
    auto& generations = context.get_caches().get_programs(program.is_static());
    if (const auto it = generations.find(program); it == generations.end() || it->second != feature.generation)
    {
        auto children = context.child_context();
        auto inputs = urd::Store<urd::sequence<QueryDenotationView>>();
        for (const auto input : program.get_inputs())
            inputs.push_back(evaluate<Kind>(input, children));
        feature.evaluator.evaluate(inputs);
        generations.insert_or_assign(program, ++feature.generation);
    }
    const auto head = feature.evaluator.get_result(feature.output(constructor.data().relation));
    auto& values = workspace.get_values();
    for (size_t i = 0; i < head.size(); ++i)
    {
        const auto row = head.row(i).get_values();
        values.clear();
        for (size_t c = 0; c < row.size(); ++c)
            values.push_back(row.get(c));
        result.insert(std::span<const ygg::Value>(values));
    }
}

}  // namespace runir::kr::dl::semantics::detail

#endif
