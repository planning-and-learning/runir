#ifndef RUNIR_KR_PS_ICP_PROGRAM_EXECUTOR_DATA_HPP_
#define RUNIR_KR_PS_ICP_PROGRAM_EXECUTOR_DATA_HPP_

#include "runir/graphs/declarations.hpp"
#include "runir/kr/declarations.hpp"
#include "runir/kr/ps/icp/execution_view.hpp"
#include "runir/kr/ps/program_proof_graph.hpp"
#include "runir/kr/ps/proof_status.hpp"
#include "runir/kr/uns/classifier_view.hpp"
#include "runir/kr/uns/declarations.hpp"

#include <chrono>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/plan.hpp>

namespace runir::kr::ps::icp
{

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
    /// Successors emitted before selection and duplicate detection.
    /// Counts applied rules, not rejected planning candidates or failure markers.
    /// The initial state is excluded; both counters retain work from resource-limited searches.
    uint64_t num_generated = 0;
};

template<tyr::TaskKind Kind>
struct ProgramProofResults
{
    ProofStatus status = ProofStatus::SUCCESS;
    runir::kr::TaskContextPtr<Kind> task_context_owner;
    std::shared_ptr<ProgramProofGraph<IcpFamilyTag, Kind>> graph;
    std::optional<tyr::planning::PackedPlan<Kind>> plan = std::nullopt;
    runir::graphs::VertexIndexList deadend_states;
    runir::graphs::VertexIndexList open_states;
    runir::graphs::VertexIndexList cycle;
    ProgramSearchStatistics statistics;

    [[nodiscard]] bool is_successful() const noexcept { return status == ProofStatus::SUCCESS; }
};

}  // namespace runir::kr::ps::icp

#endif
