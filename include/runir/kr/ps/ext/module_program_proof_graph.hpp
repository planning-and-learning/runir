#ifndef RUNIR_KR_PS_EXT_MODULE_PROGRAM_PROOF_GRAPH_HPP_
#define RUNIR_KR_PS_EXT_MODULE_PROGRAM_PROOF_GRAPH_HPP_

#include "runir/graphs/static_graph.hpp"
#include "runir/datasets/state_graph.hpp"
#include "runir/kr/dl/ext/declarations.hpp"
#include "runir/graphs/static_graph_builder.hpp"
#include "runir/kr/ps/ext/memory_state.hpp"
#include "runir/kr/ps/ext/module_view.hpp"
#include "runir/kr/ps/ext/repository.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"

#include <array>
#include <cstddef>
#include <limits>
#include <optional>
#include <tuple>
#include <tyr/formalism/object_index.hpp>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_view.hpp>
#include <utility>

namespace runir::kr::ps::ext
{

template<tyr::planning::TaskKind Kind>
struct ExtendedState
{
    using ConceptRegisterValue = std::optional<ygg::Index<tyr::formalism::Object>>;
    using RoleRegisterValue = std::optional<std::pair<ygg::Index<tyr::formalism::Object>, ygg::Index<tyr::formalism::Object>>>;
    using ConceptRegisters = std::array<ConceptRegisterValue, runir::kr::dl::num_registers>;
    using RoleRegisters = std::array<RoleRegisterValue, runir::kr::dl::num_registers>;

    runir::datasets::AnnotatedStateGraphVertexLabel<Kind> annotated_state;
    MemoryStateVariant memory_state;
    ConceptRegisters concept_registers;
    RoleRegisters role_registers;

    auto identifying_members() const noexcept
    {
        return std::tie(annotated_state.state, memory_state, concept_registers, role_registers);
    }
};

template<tyr::planning::TaskKind Kind>
struct ModuleProgramProofVertexLabel
{
    ExtendedState<Kind> extended_state;
    ModuleView module_;

    ModuleProgramProofVertexLabel(ExtendedState<Kind> extended_state_, ModuleView module__) noexcept :
        extended_state(std::move(extended_state_)),
        module_(module__)
    {
    }

    auto identifying_members() const noexcept { return std::tie(extended_state, module_); }
};

struct ModuleProgramProofEdgeLabel
{
    std::optional<datasets::StateGraphEdgeLabel> state_transition = std::nullopt;
    std::optional<RuleVariantView> rule = std::nullopt;

    ModuleProgramProofEdgeLabel(std::optional<datasets::StateGraphEdgeLabel> state_transition_, std::optional<RuleVariantView> rule_) noexcept :
        state_transition(std::move(state_transition_)),
        rule(rule_)
    {
    }

    auto identifying_members() const noexcept { return std::tie(state_transition, rule); }
};

template<tyr::planning::TaskKind Kind>
using ModuleProgramProofGraphBuilder = graphs::StaticGraphBuilder<ModuleProgramProofVertexLabel<Kind>, ModuleProgramProofEdgeLabel>;

template<tyr::planning::TaskKind Kind>
using ModuleProgramProofGraph = graphs::StaticGraph<ModuleProgramProofVertexLabel<Kind>, ModuleProgramProofEdgeLabel>;

}  // namespace runir::kr::ps::ext

#endif
