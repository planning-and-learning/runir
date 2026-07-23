#ifndef RUNIR_KR_PS_BASE_SKETCH_EXECUTOR_DATA_HPP_
#define RUNIR_KR_PS_BASE_SKETCH_EXECUTOR_DATA_HPP_

#include "runir/graphs/declarations.hpp"
#include "runir/kr/declarations.hpp"
#include "runir/kr/ps/base/sketch_proof_graph.hpp"
#include "runir/kr/uns/classifier_view.hpp"
#include "runir/kr/uns/declarations.hpp"

#include <chrono>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
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
    runir::kr::TaskContextPtr<Kind> task_context_owner;
    std::shared_ptr<SketchProofGraph<Kind>> graph;
    runir::graphs::VertexIndexList deadend_states;
    runir::graphs::VertexIndexList open_states;
    runir::graphs::VertexIndexList cycle;

    bool is_successful() const noexcept { return status == SketchProofStatus::SUCCESS; }
};

template<tyr::planning::TaskKind Kind>
struct SketchSearchOptions
{
    bool universal = false;
    std::optional<runir::kr::uns::ClassifierView> classifier = std::nullopt;
    ygg::uint_t max_num_states = std::numeric_limits<ygg::uint_t>::max();
    std::optional<std::chrono::steady_clock::duration> max_time = std::nullopt;
    uint64_t random_seed = 0;
    bool shuffle_choice_points = false;
};

}  // namespace runir::kr::ps::base

#endif
