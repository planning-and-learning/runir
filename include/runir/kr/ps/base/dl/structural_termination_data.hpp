#ifndef RUNIR_KR_PS_BASE_DL_STRUCTURAL_TERMINATION_DATA_HPP_
#define RUNIR_KR_PS_BASE_DL_STRUCTURAL_TERMINATION_DATA_HPP_

#include "runir/graphs/static_graph.hpp"
#include "runir/graphs/static_graph_builder.hpp"
#include "runir/kr/ps/base/dl/incomplete_structural_termination_data.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/dl/structural_termination.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/semantics/comparison.hpp>

namespace runir::kr::ps::base::dl
{

enum class StructuralTerminationStatus
{
    TERMINATING,
    NON_TERMINATING,
};

/// Feature valuation; bit i corresponds to position i in the originating
/// sketch's declared Boolean (resp. numerical) feature order. A numerical bit
/// encodes n > 0.
struct PolicyGraphVertexLabel : ygg::comparison::Mixin<PolicyGraphVertexLabel>
{
    std::uint64_t boolean_values;
    std::uint64_t numerical_values;

    PolicyGraphVertexLabel(std::uint64_t boolean_values_, std::uint64_t numerical_values_) noexcept :
        boolean_values(boolean_values_),
        numerical_values(numerical_values_)
    {
    }

    auto cista_members() noexcept { return std::tie(boolean_values, numerical_values); }
    auto identifying_members() const noexcept { return std::tie(boolean_values, numerical_values); }
};

using PolicyGraphBuilder = graphs::StaticGraphBuilder<PolicyGraphVertexLabel, RuleView>;
using PolicyGraph = graphs::StaticGraph<PolicyGraphVertexLabel, RuleView>;
using SccStructuralTerminationResult = runir::kr::ps::dl::SccStructuralTerminationResult<runir::kr::BaseFamilyTag, runir::kr::ps::base::Repository>;

struct SieveStructuralTerminationResult
{
    std::shared_ptr<PolicyGraph> counterexample;  ///< nullptr iff terminating; otherwise a surviving counterexample graph.
    std::vector<SccStructuralTerminationResult> scc_results;

    /// Distinct rules labeling all retained policy-graph edges; empty if terminating.
    std::vector<RuleView> surviving_rules;

    bool is_terminating() const noexcept { return !counterexample; }
};

struct StructuralTerminationResult
{
    StructuralTerminationStatus status = StructuralTerminationStatus::TERMINATING;
    std::optional<IncompleteStructuralTerminationResult> incomplete_result;  ///< Populated iff incomplete preprocessing was enabled.
    std::optional<SieveStructuralTerminationResult> sieve_result;            ///< Populated iff complete SIEVE was run.

    bool is_terminating() const noexcept { return status == StructuralTerminationStatus::TERMINATING; }
};

}  // namespace runir::kr::ps::base::dl

#endif
