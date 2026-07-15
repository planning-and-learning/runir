#ifndef RUNIR_KR_PS_EXT_DL_STRUCTURAL_TERMINATION_DATA_HPP_
#define RUNIR_KR_PS_EXT_DL_STRUCTURAL_TERMINATION_DATA_HPP_

#include "runir/graphs/static_graph.hpp"
#include "runir/graphs/static_graph_builder.hpp"
#include "runir/kr/ps/dl/structural_termination.hpp"
#include "runir/kr/ps/ext/repository.hpp"

#include <boost/dynamic_bitset.hpp>
#include <memory>
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

/// Feature valuation paired with a memory state. A numerical bit encodes a
/// value greater than zero.
struct ModulePolicyGraphVertexLabel
{
    boost::dynamic_bitset<> boolean_values;
    boost::dynamic_bitset<> numerical_values;
    MemoryStateView memory_state;

    ModulePolicyGraphVertexLabel(boost::dynamic_bitset<> boolean_values_,
                                 boost::dynamic_bitset<> numerical_values_,
                                 MemoryStateView memory_state_) noexcept :
        boolean_values(std::move(boolean_values_)),
        numerical_values(std::move(numerical_values_)),
        memory_state(memory_state_)
    {
    }

    auto identifying_members() const noexcept { return std::tie(boolean_values, numerical_values, memory_state); }
};

/// Rule labeling an edge together with its qualitative numerical feature
/// changes, aligned with the result's feature list.
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

struct ModuleStructuralTerminationResult
{
    StructuralTerminationStatus status = StructuralTerminationStatus::TERMINATING;
    std::vector<ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::BooleanFeature>>, Repository>> booleans;
    std::vector<ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::NumericalFeature>>, Repository>> numericals;
    std::shared_ptr<ModulePolicyGraph> counterexample;  ///< nullptr iff terminating.

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
