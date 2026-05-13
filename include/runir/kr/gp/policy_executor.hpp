#ifndef RUNIR_KR_GP_POLICY_EXECUTOR_HPP_
#define RUNIR_KR_GP_POLICY_EXECUTOR_HPP_

#include "runir/datasets/state_graph.hpp"
#include "runir/kr/gp/repository.hpp"

#include <tyr/planning/declarations.hpp>

namespace runir::kr::gp
{

enum class PolicyExecutionStatus
{
    SUCCESS,
    FAILURE,
};

template<tyr::planning::TaskKind Kind_>
struct PolicyExecutionResults
{
    using Kind = Kind_;

    PolicyExecutionStatus status = PolicyExecutionStatus::SUCCESS;
    runir::datasets::DynamicAnnotatedStateGraph<Kind_> graph;
    runir::graphs::EdgeIndexList deadend_transitions;
    runir::graphs::VertexIndexList open_states;
    runir::graphs::VertexIndexList cycle;

    bool is_successful() const noexcept { return status == PolicyExecutionStatus::SUCCESS; }
};

template<tyr::planning::TaskKind Kind>
auto execute_policy(const runir::datasets::AnnotatedStateGraph<Kind>& graph, PolicyView policy) -> PolicyExecutionResults<Kind>;

extern template auto execute_policy<tyr::planning::GroundTag>(const runir::datasets::AnnotatedStateGraph<tyr::planning::GroundTag>& graph,
                                                              PolicyView policy) -> PolicyExecutionResults<tyr::planning::GroundTag>;

extern template auto execute_policy<tyr::planning::LiftedTag>(const runir::datasets::AnnotatedStateGraph<tyr::planning::LiftedTag>& graph,
                                                              PolicyView policy) -> PolicyExecutionResults<tyr::planning::LiftedTag>;

}  // namespace runir::kr::gp

#endif
