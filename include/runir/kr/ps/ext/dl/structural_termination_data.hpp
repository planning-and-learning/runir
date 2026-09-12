#ifndef RUNIR_KR_PS_EXT_DL_STRUCTURAL_TERMINATION_DATA_HPP_
#define RUNIR_KR_PS_EXT_DL_STRUCTURAL_TERMINATION_DATA_HPP_

#include "runir/graphs/static_graph.hpp"
#include "runir/graphs/static_graph_builder.hpp"
#include "runir/kr/ps/dl/structural_termination.hpp"
#include "runir/kr/ps/ext/dl/incomplete_structural_termination_data.hpp"
#include "runir/kr/ps/ext/memory_state_view.hpp"
#include "runir/kr/ps/ext/repository.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/semantics/comparison.hpp>

namespace runir::kr::ps::ext::dl
{

enum class StructuralTerminationStatus
{
    TERMINATING,
    NON_TERMINATING,
};

/// Feature valuation paired with a memory state. Boolean and numerical bits
/// follow the originating module's declared feature order. A numerical bit
/// encodes a value greater than zero.
struct ModulePolicyGraphVertexLabel : ygg::comparison::Mixin<ModulePolicyGraphVertexLabel>
{
    std::uint64_t boolean_values;
    std::uint64_t numerical_values;
    MemoryStateView memory_state;

    ModulePolicyGraphVertexLabel(std::uint64_t boolean_values_, std::uint64_t numerical_values_, MemoryStateView memory_state_) noexcept :
        boolean_values(boolean_values_),
        numerical_values(numerical_values_),
        memory_state(memory_state_)
    {
    }

    auto cista_members() noexcept { return std::tie(boolean_values, numerical_values, memory_state); }
    auto identifying_members() const noexcept { return std::tie(boolean_values, numerical_values, memory_state); }
};

using ModulePolicyGraphBuilder = graphs::StaticGraphBuilder<ModulePolicyGraphVertexLabel, RuleVariantView>;
using ModulePolicyGraph = graphs::StaticGraph<ModulePolicyGraphVertexLabel, RuleVariantView>;
using SccStructuralTerminationResult = runir::kr::ps::dl::SccStructuralTerminationResult<runir::kr::ExtFamilyTag, runir::kr::ps::ext::Repository>;

struct ModuleSieveStructuralTerminationResult
{
    std::shared_ptr<ModulePolicyGraph> counterexample;  ///< nullptr iff terminating.
    std::vector<SccStructuralTerminationResult> scc_results;

    /// Distinct rules labeling all retained policy-graph edges; empty if terminating.
    std::vector<RuleVariantView> surviving_rules;

    bool is_terminating() const noexcept { return !counterexample; }
};

struct ModuleStructuralTerminationResult
{
    StructuralTerminationStatus status = StructuralTerminationStatus::TERMINATING;
    std::optional<ModuleIncompleteStructuralTerminationResult> incomplete_result;  ///< Populated iff incomplete preprocessing was enabled.
    std::optional<ModuleSieveStructuralTerminationResult> sieve_result;            ///< Populated iff complete SIEVE was run.

    bool is_terminating() const noexcept { return status == StructuralTerminationStatus::TERMINATING; }
};

struct ModuleProgramStructuralTerminationResult
{
    StructuralTerminationStatus status = StructuralTerminationStatus::TERMINATING;
    std::vector<ModuleStructuralTerminationResult> module_results;
    std::vector<RuleVariantView> recursive_call_rules;

    bool is_terminating() const noexcept { return status == StructuralTerminationStatus::TERMINATING; }
};

}  // namespace runir::kr::ps::ext::dl

#endif
