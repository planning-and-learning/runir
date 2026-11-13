#ifndef RUNIR_KR_PS_BASE_DL_STRUCTURAL_TERMINATION_DATA_HPP_
#define RUNIR_KR_PS_BASE_DL_STRUCTURAL_TERMINATION_DATA_HPP_

#include "runir/datasets/graph_types.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/dl/incomplete_structural_termination_data.hpp"
#include "runir/kr/ps/dl/structural_termination.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <urd/elements/flat.hpp>
#include <urd/repr/graphs/csr_graph.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <vector>

namespace runir::kr::ps::base::dl
{

enum class StructuralTerminationStatus : std::uint8_t
{
    TERMINATING,
    NON_TERMINATING,
};

/// Feature valuation; bit i corresponds to position i in the originating
/// sketch's declared Boolean (resp. numerical) feature order. A numerical bit
/// encodes n > 0.
struct PolicyGraphVertexLabel : urd::compared_by_identity<PolicyGraphVertexLabel>
{
    std::uint64_t boolean_values;
    std::uint64_t numerical_values;

    PolicyGraphVertexLabel(std::uint64_t boolean_values_, std::uint64_t numerical_values_) noexcept :
        boolean_values(boolean_values_),
        numerical_values(numerical_values_)
    {
    }

    static constexpr auto cista_members() noexcept
    { return urd::Members<&PolicyGraphVertexLabel::boolean_values, &PolicyGraphVertexLabel::numerical_values> {}; }
    static constexpr auto identifying_members() noexcept
    { return urd::Members<&PolicyGraphVertexLabel::boolean_values, &PolicyGraphVertexLabel::numerical_values> {}; }
};

/// A counterexample to termination: feature valuations and the rules between them, a frozen graph.
using PolicyGraph = urd::Store<urd::graph<VertexIndex, RuleView, PolicyGraphVertexLabel>, urd::frozen>;
// The concepts it is bound and searched by (urd/graphs.hpp): a renamed member is a compile error, not a lost feature.
static_assert(urd::incidence_graph<PolicyGraph> && urd::edge_list_graph<PolicyGraph>);
using SccStructuralTerminationResult = runir::kr::ps::dl::SccStructuralTerminationResult<runir::kr::BaseFamilyTag, runir::kr::ps::base::Repository>;

struct SieveStructuralTerminationResult
{
    std::shared_ptr<PolicyGraph> counterexample;  ///< nullptr iff terminating; otherwise a surviving counterexample graph.
    urd::Store<urd::sequence<SccStructuralTerminationResult>> scc_results;

    /// Distinct rules labeling all retained policy-graph edges; empty if terminating.
    urd::Store<urd::sequence<RuleView>> surviving_rules;

    [[nodiscard]] bool is_terminating() const noexcept { return !counterexample; }
};

struct StructuralTerminationResult
{
    StructuralTerminationStatus status = StructuralTerminationStatus::TERMINATING;
    std::optional<runir::kr::ps::dl::IncompleteStructuralTerminationResult<runir::kr::BaseFamilyTag>>
        incomplete_result;                                         ///< Populated iff incomplete preprocessing was enabled.
    std::optional<SieveStructuralTerminationResult> sieve_result;  ///< Populated iff complete SIEVE was run.

    [[nodiscard]] bool is_terminating() const noexcept { return status == StructuralTerminationStatus::TERMINATING; }
};

}  // namespace runir::kr::ps::base::dl

#endif
