#ifndef RUNIR_KR_GP_POLICY_EXECUTOR_HPP_
#define RUNIR_KR_GP_POLICY_EXECUTOR_HPP_

#include "runir/datasets/state_graph.hpp"
#include "runir/datasets/task_class.hpp"
#include "runir/kr/gp/repository.hpp"

#include <tyr/planning/declarations.hpp>
#include <tyr/planning/planning.hpp>

#include <memory>

namespace runir::kr::gp
{

enum class PolicyProofStatus
{
    SUCCESS,
    FAILURE,
    OUT_OF_TIME,
    OUT_OF_STATES,
};

template<tyr::planning::TaskKind Kind>
struct PolicyProofResults
{
    PolicyProofStatus status = PolicyProofStatus::SUCCESS;
    runir::datasets::TaskSearchContext<Kind> context;
    std::shared_ptr<runir::datasets::StaticAnnotatedStateGraph<Kind>> graph;
    runir::graphs::EdgeIndexList deadend_transitions;
    runir::graphs::VertexIndexList open_states;
    runir::graphs::VertexIndexList cycle;

    bool is_successful() const noexcept { return status == PolicyProofStatus::SUCCESS; }
};

template<tyr::planning::TaskKind Kind>
struct PolicySearchOptions
{
    tyr::planning::brfs::Options<Kind> brfs_options;
    tyr::planning::iw::Options<Kind> iw_options;
    tyr::planning::siw::Options<Kind> siw_options;
    uint_t max_arity = 0;
};

template<tyr::planning::TaskKind Kind>
auto prove_solution(const runir::datasets::TaskSearchContext<Kind>& context,
                    PolicyView policy,
                    const PolicySearchOptions<Kind>& options = PolicySearchOptions<Kind>()) -> PolicyProofResults<Kind>;

template<tyr::planning::TaskKind Kind>
auto find_solution(const runir::datasets::TaskSearchContext<Kind>& context,
                   PolicyView policy,
                   const PolicySearchOptions<Kind>& options = PolicySearchOptions<Kind>()) -> tyr::planning::SearchResult<Kind>;

#ifndef RUNIR_HEADER_INSTANTIATION
extern template auto
prove_solution<tyr::planning::GroundTag>(const runir::datasets::TaskSearchContext<tyr::planning::GroundTag>& context,
                                         PolicyView policy,
                                         const PolicySearchOptions<tyr::planning::GroundTag>& options) -> PolicyProofResults<tyr::planning::GroundTag>;

extern template auto
prove_solution<tyr::planning::LiftedTag>(const runir::datasets::TaskSearchContext<tyr::planning::LiftedTag>& context,
                                         PolicyView policy,
                                         const PolicySearchOptions<tyr::planning::LiftedTag>& options) -> PolicyProofResults<tyr::planning::LiftedTag>;

extern template auto
find_solution<tyr::planning::GroundTag>(const runir::datasets::TaskSearchContext<tyr::planning::GroundTag>& context,
                                        PolicyView policy,
                                        const PolicySearchOptions<tyr::planning::GroundTag>& options) -> tyr::planning::SearchResult<tyr::planning::GroundTag>;

extern template auto
find_solution<tyr::planning::LiftedTag>(const runir::datasets::TaskSearchContext<tyr::planning::LiftedTag>& context,
                                        PolicyView policy,
                                        const PolicySearchOptions<tyr::planning::LiftedTag>& options) -> tyr::planning::SearchResult<tyr::planning::LiftedTag>;
#endif

}  // namespace runir::kr::gp

#ifdef RUNIR_HEADER_INSTANTIATION
#include "runir/kr/gp/policy_executor_impl.hpp"
#endif

#endif
