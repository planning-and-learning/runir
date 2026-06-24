#include "pyrunir/kr/ps/ext/module.hpp"

#include <nanobind/stl/array.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/variant.h>
#include <nanobind/stl/vector.h>
#include <pyrunir/graphs/graph.hpp>
#include <runir/datasets/task_class.hpp>
#include <runir/kr/dl/semantics/builder.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/ps/ext/evaluation_context.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/successor_expander.hpp>
#include <runir/kr/ps/ext/module_program_executor.hpp>
#include <yggdrasil/python/type_casters.hpp>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/node.hpp>
#include <optional>
#include <utility>
#include <vector>

namespace runir::kr::ps::ext
{

using namespace nanobind::literals;
using runir::graphs::bind_forward_graph;
using runir::graphs::bind_readable_graph;

namespace
{

template<tyr::planning::TaskKind Kind>
void bind_module_program_proof_types(nb::module_& m, const char* prefix)
{
    using VertexLabel = ModuleProgramProofVertexLabel<Kind>;
    using Graph = ModuleProgramProofGraph<Kind>;
    using Results = ModuleProgramProofResults<Kind>;
    using Options = ModuleProgramSearchOptions<Kind>;

    nb::class_<VertexLabel>(m, (std::string(prefix) + "ModuleProgramProofVertexLabel").c_str())
        .def_prop_ro("state", [](const VertexLabel& label) { return label.extended_state.annotated_state.state; })
        .def_prop_ro("memory_state", [](const VertexLabel& label) { return label.extended_state.memory_state; })
        .def_prop_ro("concept_registers", [](const VertexLabel& label) { return label.extended_state.concept_registers; })
        .def_prop_ro("role_registers", [](const VertexLabel& label) { return label.extended_state.role_registers; })
        .def_prop_ro("module_", [](const VertexLabel& label) { return label.module_; })
        .def_prop_ro("goal_distance", [](const VertexLabel& label) { return label.extended_state.annotated_state.goal_distance; })
        .def_prop_ro("is_initial", [](const VertexLabel& label) { return label.extended_state.annotated_state.is_initial; })
        .def_prop_ro("is_goal", [](const VertexLabel& label) { return label.extended_state.annotated_state.is_goal; })
        .def_prop_ro("is_alive", [](const VertexLabel& label) { return label.extended_state.annotated_state.is_alive; })
        .def_prop_ro("is_unsolvable", [](const VertexLabel& label) { return label.extended_state.annotated_state.is_unsolvable; });

    auto graph = nb::class_<Graph>(m, (std::string(prefix) + "ModuleProgramProofGraph").c_str());
    graph.def(nb::init<>());
    bind_readable_graph(graph);
    bind_forward_graph(graph);

    nb::class_<Results>(m, (std::string(prefix) + "ModuleProgramProofResults").c_str())
        .def_ro("status", &Results::status)
        .def_ro("graph", &Results::graph)
        .def_ro("final_state", &Results::final_state)
        .def_ro("plan", &Results::plan)
        .def_ro("deadend_transitions", &Results::deadend_transitions)
        .def_ro("open_states", &Results::open_states)
        .def_ro("cycle", &Results::cycle)
        .def("is_successful", &Results::is_successful);

    nb::class_<Options>(m, (std::string(prefix) + "ModuleProgramSearchOptions").c_str())
        .def(nb::init<>())
        .def_rw("brfs_options", &Options::brfs_options)
        .def_rw("iw_options", &Options::iw_options)
        .def_rw("max_arity", &Options::max_arity);
}

// Python-facing bundle around `SuccessorExpander`: it adds the run-fixed bits a vertex context
// needs (the program's modules + the DL builder/denotation repository) so callers pass only the
// per-vertex contents (state + module + memory + registers). Methods return the proof graph's own
// node/edge labels, so runir-mcp reuses the exact structures it renders from the proof graph.
struct BoundExpander
{
    using Kind = tyr::planning::GroundTag;
    using ConceptRegisters = EvaluationContext<Kind>::ConceptRegisters;
    using RoleRegisters = EvaluationContext<Kind>::RoleRegisters;
    using Successor = SuccessorExpander<Kind>::Successor;

    SuccessorExpander<Kind> expander;
    std::vector<ModuleView> modules;
    runir::kr::dl::semantics::Builder* builder;
    runir::kr::dl::semantics::DenotationRepository* denotation_repository;

    BoundExpander(const runir::datasets::TaskSearchContext<Kind>& search_context,
                  tyr::planning::StateView<Kind> initial_state,
                  std::vector<ModuleView> modules_,
                  runir::kr::dl::semantics::Builder& builder_,
                  runir::kr::dl::semantics::DenotationRepository& denotation_repository_) :
        expander(search_context, tyr::planning::Node<Kind>(initial_state, ygg::float_t(0))),
        modules(std::move(modules_)),
        builder(&builder_),
        denotation_repository(&denotation_repository_)
    {
    }

    EvaluationContext<Kind> context_at(ModuleView module, MemoryStateView memory_state, ConceptRegisters concept_registers, RoleRegisters role_registers, tyr::planning::StateView<Kind> source_state)
    {
        return make_evaluation_context<Kind>(source_state, module, memory_state, std::move(concept_registers), std::move(role_registers), modules, *builder, *denotation_repository);
    }
};

}  // namespace

void bind_module_program_executor(nb::module_& m)
{
    nb::class_<InternalMemoryState>(m, "InternalMemoryState")
        .def_ro("value", &InternalMemoryState::value);
    nb::class_<ExternalMemoryState>(m, "ExternalMemoryState")
        .def_ro("value", &ExternalMemoryState::value);

    nb::class_<ModuleProgramProofEdgeLabel>(m, "ModuleProgramProofEdgeLabel")
        .def_ro("state_transition", &ModuleProgramProofEdgeLabel::state_transition)
        .def_ro("rule", &ModuleProgramProofEdgeLabel::rule);

    nb::enum_<ModuleProgramProofStatus>(m, "ModuleProgramProofStatus")
        .value("SUCCESS", ModuleProgramProofStatus::SUCCESS)
        .value("FAILURE", ModuleProgramProofStatus::FAILURE)
        .value("OUT_OF_TIME", ModuleProgramProofStatus::OUT_OF_TIME)
        .value("OUT_OF_STATES", ModuleProgramProofStatus::OUT_OF_STATES);

    bind_module_program_proof_types<tyr::planning::GroundTag>(m, "Ground");
    bind_module_program_proof_types<tyr::planning::LiftedTag>(m, "Lifted");

    m.def("prove_ground_solution",
          [](runir::datasets::TaskSearchContextPtr<tyr::planning::GroundTag> context, ModuleProgramView program, const ModuleProgramSearchOptions<tyr::planning::GroundTag>& options) { return prove_solution(std::move(context), program, options); },
          nb::call_guard<nb::gil_scoped_release>(),
          "context"_a,
          "program"_a,
          "options"_a = ModuleProgramSearchOptions<tyr::planning::GroundTag>());
    m.def("prove_lifted_solution",
          [](runir::datasets::TaskSearchContextPtr<tyr::planning::LiftedTag> context, ModuleProgramView program, const ModuleProgramSearchOptions<tyr::planning::LiftedTag>& options) { return prove_solution(std::move(context), program, options); },
          nb::call_guard<nb::gil_scoped_release>(),
          "context"_a,
          "program"_a,
          "options"_a = ModuleProgramSearchOptions<tyr::planning::LiftedTag>());
    m.def("find_ground_solution",
          [](runir::datasets::TaskSearchContextPtr<tyr::planning::GroundTag> context, ModuleProgramView program, const ModuleProgramSearchOptions<tyr::planning::GroundTag>& options) { return find_solution(std::move(context), program, options); },
          nb::call_guard<nb::gil_scoped_release>(),
          "context"_a,
          "program"_a,
          "options"_a = ModuleProgramSearchOptions<tyr::planning::GroundTag>());
    m.def("find_lifted_solution",
          [](runir::datasets::TaskSearchContextPtr<tyr::planning::LiftedTag> context, ModuleProgramView program, const ModuleProgramSearchOptions<tyr::planning::LiftedTag>& options) { return find_solution(std::move(context), program, options); },
          nb::call_guard<nb::gil_scoped_release>(),
          "context"_a,
          "program"_a,
          "options"_a = ModuleProgramSearchOptions<tyr::planning::LiftedTag>());

    // Module-program successor expansion. Built once per task; reuses the same node/edge
    // construction the proof search does (`ModuleProgramProofBuilder` delegates to the same
    // `SuccessorExpander`), so `apply`/`internal_successors` return the proof graph's own
    // `ModuleProgramProofVertexLabel`/`ModuleProgramProofEdgeLabel`. Callers pass the per-vertex
    // contents (state + module + memory + registers); the program's modules + DL builder/repository
    // are fixed at construction.
    using Kind = tyr::planning::GroundTag;
    nb::class_<BoundExpander>(m, "SuccessorExpander")
        .def(nb::init<const runir::datasets::TaskSearchContext<Kind>&,
                      tyr::planning::StateView<Kind>,
                      std::vector<ModuleView>,
                      runir::kr::dl::semantics::Builder&,
                      runir::kr::dl::semantics::DenotationRepository&>(),
             "search_context"_a,
             "initial_state"_a,
             "modules"_a,
             "builder"_a,
             "denotation_repository"_a,
             nb::keep_alive<1, 2>(),
             nb::keep_alive<1, 5>(),
             nb::keep_alive<1, 6>())
        // Which control rule (Sketch/Do) of the vertex selects the candidate planning successor
        // `source --action--> target`, or None (the gap).
        .def(
            "matching_rule",
            [](BoundExpander& self,
               ModuleView module,
               MemoryStateView memory_state,
               BoundExpander::ConceptRegisters concept_registers,
               BoundExpander::RoleRegisters role_registers,
               tyr::planning::StateView<Kind> source_state,
               tyr::formalism::planning::GroundActionView action,
               tyr::planning::StateView<Kind> target_state) -> std::optional<RuleVariantView>
            {
                auto context = self.context_at(module, memory_state, std::move(concept_registers), std::move(role_registers), source_state);
                return self.expander.matching_rule(context, tyr::planning::LabeledNode<Kind> { action, tyr::planning::Node<Kind>(target_state, ygg::float_t(0)) });
            },
            "module"_a, "memory_state"_a, "concept_registers"_a, "role_registers"_a, "source_state"_a, "action"_a, "target_state"_a)
        // Apply a rule from the vertex, returning the (edge, node) it produces, or None. For
        // Sketch/Do pass the candidate planning successor (`action` + `target_state`); for Load/Call
        // pass None for both.
        .def(
            "apply",
            [](BoundExpander& self,
               ModuleView module,
               MemoryStateView memory_state,
               BoundExpander::ConceptRegisters concept_registers,
               BoundExpander::RoleRegisters role_registers,
               tyr::planning::StateView<Kind> source_state,
               RuleVariantView rule,
               std::optional<tyr::formalism::planning::GroundActionView> action,
               std::optional<tyr::planning::StateView<Kind>> target_state) -> std::optional<BoundExpander::Successor>
            {
                auto context = self.context_at(module, memory_state, std::move(concept_registers), std::move(role_registers), source_state);
                std::optional<tyr::planning::LabeledNode<Kind>> candidate;
                if (action && target_state)
                    candidate = tyr::planning::LabeledNode<Kind> { *action, tyr::planning::Node<Kind>(*target_state, ygg::float_t(0)) };
                return self.expander.apply(std::move(context), rule, std::move(candidate));
            },
            "module"_a, "memory_state"_a, "concept_registers"_a, "role_registers"_a, "source_state"_a, "rule"_a, "action"_a, "target_state"_a)
        // The applicable internal moves (Load/Call) at the vertex, as (edge, node) pairs.
        .def(
            "internal_successors",
            [](BoundExpander& self,
               ModuleView module,
               MemoryStateView memory_state,
               BoundExpander::ConceptRegisters concept_registers,
               BoundExpander::RoleRegisters role_registers,
               tyr::planning::StateView<Kind> source_state) -> std::vector<BoundExpander::Successor>
            {
                auto context = self.context_at(module, memory_state, std::move(concept_registers), std::move(role_registers), source_state);
                return self.expander.internal_successors(context);
            },
            "module"_a, "memory_state"_a, "concept_registers"_a, "role_registers"_a, "source_state"_a);
}

}  // namespace runir::kr::ps::ext
