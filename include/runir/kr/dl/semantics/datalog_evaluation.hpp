#ifndef RUNIR_KR_DL_SEMANTICS_DATALOG_EVALUATION_HPP_
#define RUNIR_KR_DL_SEMANTICS_DATALOG_EVALUATION_HPP_

#include "runir/kr/dl/semantics/datalog_feature.hpp"
#include "runir/kr/dl/semantics/evaluation_decl.hpp"
#include "runir/kr/dl/semantics/state_evaluation_context.hpp"

#include <vector>

namespace runir::kr::dl::semantics::detail
{

/// q_head in full evaluation: the program is evaluated once per state for all its heads (DatalogFeature), and the
/// head's rows are copied into the query's columns.
template<tyr::TaskKind Kind, StateEvaluationContextConcept<Kind> Context, typename C>
void on_demand_head(ygg::View<ygg::Index<Query<QueryHeadTag>>, C> constructor, Context& context, ygg::Builder<ygg::database::Relation>& result)
{
    const auto program = constructor.get_program();
    auto& feature = context.get_workspace().get_datalog_feature(context.get_state().get_formalism_repository(),
                                                                constructor.get_context(),
                                                                ygg::uint_t(program.get_index()),
                                                                [&] { return make_datalog_feature(program); });
    auto& generations = context.get_caches().get_programs(program.is_static());
    if (const auto it = generations.find(program); it == generations.end() || it->second != feature.generation)
    {
        auto children = context.child_context();
        auto inputs = std::vector<QueryDenotationView>();
        for (const auto input : program.get_inputs())
            inputs.push_back(evaluate<Kind>(input, children));
        feature.evaluator.evaluate(inputs);
        generations.insert_or_assign(program, ++feature.generation);
    }
    const auto& head = feature.evaluator.get_result(feature.output(constructor.get_data().relation));
    for (size_t i = 0; i < head.size(); ++i)
        result.insert(head.row(i));
}

}  // namespace runir::kr::dl::semantics::detail

#endif
