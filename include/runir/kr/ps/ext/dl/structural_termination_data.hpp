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

/// Feature valuation paired with a memory state; bit i corresponds to
/// position i in the result's concept (boolean, numerical) feature list. A
/// concept or numerical bit encodes a value greater than zero.
struct ModulePolicyGraphVertexLabel
{
    boost::dynamic_bitset<> concept_values;
    boost::dynamic_bitset<> boolean_values;
    boost::dynamic_bitset<> numerical_values;
    MemoryStateView memory_state;

    ModulePolicyGraphVertexLabel(boost::dynamic_bitset<> concept_values_,
                                 boost::dynamic_bitset<> boolean_values_,
                                 boost::dynamic_bitset<> numerical_values_,
                                 MemoryStateView memory_state_) noexcept :
        concept_values(std::move(concept_values_)),
        boolean_values(std::move(boolean_values_)),
        numerical_values(std::move(numerical_values_)),
        memory_state(memory_state_)
    {
    }

    auto identifying_members() const noexcept { return std::tie(concept_values, boolean_values, numerical_values, memory_state); }
};

/// Rule labeling an edge together with its qualitative changes of the
/// numerical-like features: concepts first, then numericals, aligned with
/// the result's feature lists.
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
    std::vector<ygg::Index<Feature<runir::kr::dl::ConceptTag>>> concepts;
    std::vector<ygg::Index<Feature<runir::kr::ps::dl::BooleanFeature>>> booleans;
    std::vector<ygg::Index<Feature<runir::kr::ps::dl::NumericalFeature>>> numericals;
    std::shared_ptr<ModulePolicyGraph> counterexample;  ///< nullptr iff terminating.

    bool is_terminating() const noexcept { return status == StructuralTerminationStatus::TERMINATING; }
};

}  // namespace runir::kr::ps::ext::dl

#endif
