#ifndef RUNIR_KR_PS_EXT_DL_STRUCTURAL_TERMINATION_DATA_HPP_
#define RUNIR_KR_PS_EXT_DL_STRUCTURAL_TERMINATION_DATA_HPP_

#include "runir/graphs/static_graph.hpp"
#include "runir/graphs/static_graph_builder.hpp"
#include "runir/kr/ps/dl/structural_termination.hpp"
#include "runir/kr/ps/ext/declarations.hpp"
#include "runir/kr/ps/ext/dl/incomplete_structural_termination_data.hpp"
#include "runir/kr/ps/ext/memory_state_view.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"

#include <boost/dynamic_bitset.hpp>
#include <memory>
#include <optional>
#include <vector>
#include <yggdrasil/containers/dynamic_bitset.hpp>
#include <yggdrasil/containers/dynamic_bitset_hash.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::ext::dl
{

enum class StructuralTerminationStatus
{
    TERMINATING,
    NON_TERMINATING,
};

using NumericalChange = runir::kr::ps::dl::NumericalChange;

/// Feature valuation paired with a memory state. Boolean and numerical bits
/// follow the originating module's declared feature order. A numerical bit
/// encodes a value greater than zero.
struct ModulePolicyGraphVertexLabel
{
    boost::dynamic_bitset<> boolean_values;
    boost::dynamic_bitset<> numerical_values;
    MemoryStateView memory_state;

    ModulePolicyGraphVertexLabel(boost::dynamic_bitset<> boolean_values_, boost::dynamic_bitset<> numerical_values_, MemoryStateView memory_state_) noexcept :
        boolean_values(std::move(boolean_values_)),
        numerical_values(std::move(numerical_values_)),
        memory_state(memory_state_)
    {
    }

    auto identifying_members() const noexcept { return std::tie(boolean_values, numerical_values, memory_state); }
};

/// Rule labeling an edge together with its qualitative numerical feature
/// changes, aligned with the originating module's declared numerical feature
/// order.
struct ModulePolicyGraphEdgeLabel
{
    RuleVariantView rule;
    std::vector<NumericalChange> numerical_changes;

    ModulePolicyGraphEdgeLabel(RuleVariantView rule_, std::vector<NumericalChange> numerical_changes_) noexcept :
        rule(rule_),
        numerical_changes(std::move(numerical_changes_))
    {
    }

    // The changes are determined by the rule.
    auto identifying_members() const noexcept { return std::tie(rule); }
};

using ModulePolicyGraphBuilder = graphs::StaticGraphBuilder<ModulePolicyGraphVertexLabel, ModulePolicyGraphEdgeLabel>;
using ModulePolicyGraph = graphs::StaticGraph<ModulePolicyGraphVertexLabel, ModulePolicyGraphEdgeLabel>;
using SccStructuralTerminationResult = runir::kr::ps::dl::SccStructuralTerminationResult<runir::kr::ExtFamilyTag, runir::kr::ps::ext::Repository>;

struct ModuleStructuralTerminationResult
{
    StructuralTerminationStatus status = StructuralTerminationStatus::TERMINATING;
    std::shared_ptr<ModulePolicyGraph> counterexample;                             ///< nullptr iff terminating.
    std::optional<ModuleIncompleteStructuralTerminationResult> incomplete_result;  ///< Populated iff incomplete preprocessing was enabled.
    std::optional<std::vector<SccStructuralTerminationResult>> scc_results;        ///< Populated iff complete SIEVE was run.

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
