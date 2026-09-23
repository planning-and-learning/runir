#ifndef RUNIR_KR_PS_EXT_PROGRAM_EXECUTOR_DATA_HPP_
#define RUNIR_KR_PS_EXT_PROGRAM_EXECUTOR_DATA_HPP_

#include "runir/graphs/declarations.hpp"
#include "runir/kr/declarations.hpp"
#include "runir/kr/ps/ext/program_proof_graph.hpp"
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

namespace runir::kr::ps::ext
{

enum class ProgramProofStatus
{
    SUCCESS,
    FAILURE,
    OUT_OF_TIME,
    OUT_OF_STATES,
};

constexpr std::string_view to_string(ProgramProofStatus status)
{
    switch (status)
    {
        case ProgramProofStatus::SUCCESS:
            return "success";
        case ProgramProofStatus::FAILURE:
            return "failure";
        case ProgramProofStatus::OUT_OF_TIME:
            return "out_of_time";
        case ProgramProofStatus::OUT_OF_STATES:
            return "out_of_states";
    }
    throw std::invalid_argument("invalid ProgramProofStatus");
}

template<tyr::TaskKind Kind>
struct ProgramSearchOptions
{
    bool universal = false;
    std::optional<runir::kr::uns::ClassifierView> classifier = std::nullopt;
    ygg::uint_t max_num_states = std::numeric_limits<ygg::uint_t>::max();
    std::optional<std::chrono::steady_clock::duration> max_time = std::nullopt;
};

struct ProgramSearchStatistics
{
    /// Distinct extended-state expansions started, including those yielding no successors.
    /// Each program state is expanded at most once per search.
    uint64_t num_expanded = 0;
    /// Extended successors emitted before selection and duplicate detection; Choose emits only attempted bindings.
    /// Counts applied rules and caller returns, not rejected planning candidates or failure markers.
    /// The initial state is excluded; both counters retain work from abandoned branches and resource-limited searches.
    uint64_t num_generated = 0;

    /// Non-singleton Choose count on the first discovered goal's predecessor path.
    /// Counts bindings after effect filtering and is zero unless the search succeeds.
    ygg::uint_t choice_depth = 0;
};

template<tyr::TaskKind Kind>
struct ProgramProofResults
{
    ProgramProofStatus status = ProgramProofStatus::SUCCESS;
    runir::kr::TaskContextPtr<Kind> task_context_owner;
    std::shared_ptr<ProgramProofGraph<Kind>> graph;
    std::optional<tyr::planning::PackedPlan<Kind>> plan = std::nullopt;
    runir::graphs::VertexIndexList deadend_states;
    runir::graphs::VertexIndexList open_states;
    runir::graphs::VertexIndexList cycle;
    ProgramSearchStatistics statistics;

    bool is_successful() const noexcept { return status == ProgramProofStatus::SUCCESS; }
};

}  // namespace runir::kr::ps::ext

#endif
