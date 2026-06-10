#ifndef RUNIR_KR_PS_BASE_SKETCH_EXECUTOR_DATA_HPP_
#define RUNIR_KR_PS_BASE_SKETCH_EXECUTOR_DATA_HPP_

#include "runir/datasets/task_class.hpp"
#include "runir/graphs/declarations.hpp"
#include "runir/kr/ps/base/sketch_proof_graph.hpp"

#include <memory>
#include <tyr/planning/algorithms/brfs.hpp>
#include <tyr/planning/algorithms/iw.hpp>
#include <tyr/planning/algorithms/siw.hpp>
#include <tyr/planning/declarations.hpp>

namespace runir::kr::ps::base
{

enum class SketchProofStatus
{
    SUCCESS,
    FAILURE,
    OUT_OF_TIME,
    OUT_OF_STATES,
};

template<tyr::planning::TaskKind Kind>
struct SketchProofResults
{
    SketchProofStatus status = SketchProofStatus::SUCCESS;
    runir::datasets::TaskSearchContextPtr<Kind> context_owner;
    std::shared_ptr<SketchProofGraph<Kind>> graph;
    runir::graphs::EdgeIndexList deadend_transitions;
    runir::graphs::VertexIndexList open_states;
    runir::graphs::VertexIndexList cycle;

    bool is_successful() const noexcept { return status == SketchProofStatus::SUCCESS; }
};

template<tyr::planning::TaskKind Kind>
struct SketchSearchOptions
{
    tyr::planning::brfs::Options<Kind> brfs_options;
    tyr::planning::iw::Options<Kind> iw_options;
    tyr::planning::siw::Options<Kind> siw_options;
    uint_t max_arity = 0;
};

}  // namespace runir::kr::ps::base

#endif
