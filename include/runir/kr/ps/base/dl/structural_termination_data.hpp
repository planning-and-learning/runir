#ifndef RUNIR_KR_PS_BASE_DL_STRUCTURAL_TERMINATION_DATA_HPP_
#define RUNIR_KR_PS_BASE_DL_STRUCTURAL_TERMINATION_DATA_HPP_

#include "runir/graphs/static_graph.hpp"
#include "runir/graphs/static_graph_builder.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/dl/structural_termination.hpp"

#include <boost/dynamic_bitset.hpp>
#include <memory>
#include <vector>
#include <yggdrasil/containers/dynamic_bitset.hpp>
#include <yggdrasil/containers/dynamic_bitset_hash.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::base::dl
{

enum class StructuralTerminationStatus
{
    TERMINATING,
    NON_TERMINATING,
};

using NumericalChange = runir::kr::ps::dl::NumericalChange;

/// Feature valuation; bit i corresponds to position i in the result's
/// boolean (resp. numerical) feature list. A numerical bit encodes n > 0.
struct PolicyGraphVertexLabel
{
    boost::dynamic_bitset<> boolean_values;
    boost::dynamic_bitset<> numerical_values;

    PolicyGraphVertexLabel(boost::dynamic_bitset<> boolean_values_, boost::dynamic_bitset<> numerical_values_) noexcept :
        boolean_values(std::move(boolean_values_)),
        numerical_values(std::move(numerical_values_))
    {
    }

    auto identifying_members() const noexcept { return std::tie(boolean_values, numerical_values); }
};

/// Rule labeling an edge together with its qualitative numerical changes,
/// aligned with the result's numerical feature list.
struct PolicyGraphEdgeLabel
{
    RuleView rule;
    std::vector<NumericalChange> numerical_changes;

    PolicyGraphEdgeLabel(RuleView rule_, std::vector<NumericalChange> numerical_changes_) noexcept :
        rule(rule_),
        numerical_changes(std::move(numerical_changes_))
    {
    }

    // The changes are determined by the rule.
    auto identifying_members() const noexcept { return std::tie(rule); }
};

using PolicyGraphBuilder = graphs::StaticGraphBuilder<PolicyGraphVertexLabel, PolicyGraphEdgeLabel>;
using PolicyGraph = graphs::StaticGraph<PolicyGraphVertexLabel, PolicyGraphEdgeLabel>;

struct StructuralTerminationResult
{
    StructuralTerminationStatus status = StructuralTerminationStatus::TERMINATING;
    std::vector<ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>>> booleans;
    std::vector<ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>>> numericals;
    std::shared_ptr<PolicyGraph> counterexample;  ///< nullptr iff terminating; otherwise a surviving counterexample graph.

    bool is_terminating() const noexcept { return status == StructuralTerminationStatus::TERMINATING; }
};

}  // namespace runir::kr::ps::base::dl

#endif
