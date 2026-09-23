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
#include <stdexcept>
#include <string_view>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/plan.hpp>

namespace runir::kr::ps::base
{

enum class SketchProofStatus
{
    SUCCESS,
    FAILURE,
    OUT_OF_TIME,
    OUT_OF_STATES,
};

constexpr std::string_view to_string(SketchProofStatus status)
{
    switch (status)
    {
        case SketchProofStatus::SUCCESS:
            return "success";
        case SketchProofStatus::FAILURE:
            return "failure";
        case SketchProofStatus::OUT_OF_TIME:
            return "out_of_time";
        case SketchProofStatus::OUT_OF_STATES:
            return "out_of_states";
    }
    throw std::invalid_argument("invalid SketchProofStatus");
}

struct SketchSearchStatistics
{
    /// Successor-generation invocations, including zero-successor and interrupted expansions; excludes goals and classifier-pruned states.
    uint64_t num_expanded = 0;
    /// Planning successors generated before sketch-rule filtering and visited-state deduplication; excludes the initial state.
    /// Counts repeated target states separately and retains partial counts on failure or resource limits.
    uint64_t num_generated = 0;
};

template<tyr::TaskKind Kind>
struct SketchProofResults
{
    SketchProofStatus status = SketchProofStatus::SUCCESS;
    runir::kr::TaskContextPtr<Kind> task_context_owner;
    std::shared_ptr<SketchProofGraph<Kind>> graph;
    std::optional<tyr::planning::PackedPlan<Kind>> plan = std::nullopt;
    runir::graphs::VertexIndexList deadend_states;
    runir::graphs::VertexIndexList open_states;
    runir::graphs::VertexIndexList cycle;
    SketchSearchStatistics statistics;

    bool is_successful() const noexcept { return status == SketchProofStatus::SUCCESS; }
};

template<tyr::TaskKind Kind>
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
