#ifndef RUNIR_KR_PS_EXT_DL_STRUCTURAL_TERMINATION_DATA_HPP_
#define RUNIR_KR_PS_EXT_DL_STRUCTURAL_TERMINATION_DATA_HPP_

#include "runir/datasets/graph_types.hpp"
#include "runir/kr/ps/dl/structural_termination.hpp"
#include "runir/kr/ps/ext/dl/incomplete_structural_termination_data.hpp"
#include "runir/kr/ps/ext/repository.hpp"
#include "runir/kr/ps/memory_state_view.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <urd/elements/flat.hpp>
#include <urd/repr/graphs/csr_graph.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <vector>

namespace runir::kr::ps::ext::dl
{

enum class StructuralTerminationStatus : std::uint8_t
{
    TERMINATING,
    NON_TERMINATING,
};

/// Feature valuation paired with a memory state. Boolean and numerical bits
/// follow the originating module's declared feature order. A numerical bit
/// encodes a value greater than zero.
struct ModulePolicyGraphVertexLabel : urd::compared_by_identity<ModulePolicyGraphVertexLabel>
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

    static constexpr auto cista_members() noexcept
    {
        return urd::Members<&ModulePolicyGraphVertexLabel::boolean_values,
                            &ModulePolicyGraphVertexLabel::numerical_values,
                            &ModulePolicyGraphVertexLabel::memory_state> {};
    }
    static constexpr auto identifying_members() noexcept
    {
        return urd::Members<&ModulePolicyGraphVertexLabel::boolean_values,
                            &ModulePolicyGraphVertexLabel::numerical_values,
                            &ModulePolicyGraphVertexLabel::memory_state> {};
    }
};

/// A counterexample to a module's termination: feature valuations with memory states and the rules between them.
using ModulePolicyGraph = urd::Store<urd::graph<VertexIndex, RuleVariantView, ModulePolicyGraphVertexLabel>, urd::frozen>;
// The concepts it is bound and searched by (urd/graphs.hpp): a renamed member is a compile error, not a lost feature.
static_assert(urd::incidence_graph<ModulePolicyGraph> && urd::edge_list_graph<ModulePolicyGraph>);
using SccStructuralTerminationResult = runir::kr::ps::dl::SccStructuralTerminationResult<runir::kr::ExtFamilyTag, runir::kr::ps::ext::Repository>;

struct ModuleSieveStructuralTerminationResult
{
    std::shared_ptr<ModulePolicyGraph> counterexample;  ///< nullptr iff terminating.
    urd::Store<urd::sequence<SccStructuralTerminationResult>> scc_results;

    /// Distinct rules labeling all retained policy-graph edges; empty if terminating.
    urd::Store<urd::sequence<RuleVariantView>> surviving_rules;

    [[nodiscard]] bool is_terminating() const noexcept { return !counterexample; }
};

struct ModuleStructuralTerminationResult
{
    StructuralTerminationStatus status = StructuralTerminationStatus::TERMINATING;
    std::optional<runir::kr::ps::dl::IncompleteStructuralTerminationResult<runir::kr::ExtFamilyTag>>
        incomplete_result;                                               ///< Populated iff incomplete preprocessing was enabled.
    std::optional<ModuleSieveStructuralTerminationResult> sieve_result;  ///< Populated iff complete SIEVE was run.

    [[nodiscard]] bool is_terminating() const noexcept { return status == StructuralTerminationStatus::TERMINATING; }
};

struct ProgramStructuralTerminationResult
{
    StructuralTerminationStatus status = StructuralTerminationStatus::TERMINATING;
    urd::Store<urd::sequence<ModuleStructuralTerminationResult>> module_results;
    urd::Store<urd::sequence<RuleVariantView>> recursive_call_rules;

    [[nodiscard]] bool is_terminating() const noexcept { return status == StructuralTerminationStatus::TERMINATING; }
};

}  // namespace runir::kr::ps::ext::dl

#endif
