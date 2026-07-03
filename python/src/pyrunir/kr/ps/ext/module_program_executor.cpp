#include "pyrunir/kr/ps/ext/module.hpp"

#include <nanobind/stl/array.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/tuple.h>
#include <nanobind/stl/variant.h>
#include <nanobind/stl/vector.h>
#include <optional>
#include <pyrunir/graphs/graph.hpp>
#include <runir/datasets/task_class.hpp>
#include <runir/kr/ps/ext/evaluation_context.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/module_program_executor.hpp>
#include <runir/kr/ps/ext/successor_expander.hpp>
#include <string>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/node.hpp>
#include <utility>
#include <variant>
#include <vector>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

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

    auto vertex_label = nb::class_<VertexLabel>(m, (std::string(prefix) + "ModuleProgramProofVertexLabel").c_str())
                            .def_prop_ro("state", [](const VertexLabel& label) { return label.extended_state.annotated_state.state; })
                            .def_prop_ro("memory_state", [](const VertexLabel& label) { return label.extended_state.memory_state; })
                            .def_prop_ro("concept_registers", [](const VertexLabel& label) { return label.extended_state.concept_registers; })
                            .def_prop_ro("role_registers", [](const VertexLabel& label) { return label.extended_state.role_registers; })
                            .def_prop_ro("module", [](const VertexLabel& label) { return label.module_; })
                            .def_prop_ro("goal_distance", [](const VertexLabel& label) { return label.extended_state.annotated_state.goal_distance; })
                            .def_prop_ro("is_initial", [](const VertexLabel& label) { return label.extended_state.annotated_state.is_initial; })
                            .def_prop_ro("is_goal", [](const VertexLabel& label) { return label.extended_state.annotated_state.is_goal; })
                            .def_prop_ro("is_alive", [](const VertexLabel& label) { return label.extended_state.annotated_state.is_alive; })
                            .def_prop_ro("is_unsolvable", [](const VertexLabel& label) { return label.extended_state.annotated_state.is_unsolvable; });
    ygg::add_print(vertex_label);
    ygg::add_hash(vertex_label);

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

}  // namespace

void bind_module_program_executor(nb::module_& m)
{
    auto internal_memory_state = nb::class_<InternalMemoryState>(m, "InternalMemoryState").def_ro("value", &InternalMemoryState::value);
    ygg::add_print(internal_memory_state);
    ygg::add_hash(internal_memory_state);

    auto external_memory_state = nb::class_<ExternalMemoryState>(m, "ExternalMemoryState").def_ro("value", &ExternalMemoryState::value);
    ygg::add_print(external_memory_state);
    ygg::add_hash(external_memory_state);

    auto edge_label = nb::class_<ModuleProgramProofEdgeLabel>(m, "ModuleProgramProofEdgeLabel")
                          .def_ro("state_transition", &ModuleProgramProofEdgeLabel::state_transition)
                          .def_ro("rule", &ModuleProgramProofEdgeLabel::rule);
    ygg::add_print(edge_label);
    ygg::add_hash(edge_label);

    nb::enum_<ModuleProgramProofStatus>(m, "ModuleProgramProofStatus")
        .value("SUCCESS", ModuleProgramProofStatus::SUCCESS)
        .value("FAILURE", ModuleProgramProofStatus::FAILURE)
        .value("OUT_OF_TIME", ModuleProgramProofStatus::OUT_OF_TIME)
        .value("OUT_OF_STATES", ModuleProgramProofStatus::OUT_OF_STATES);

    bind_module_program_proof_types<tyr::planning::GroundTag>(m, "Ground");
    bind_module_program_proof_types<tyr::planning::LiftedTag>(m, "Lifted");

    m.def(
        "prove_ground_solution",
        [](runir::datasets::TaskSearchContextPtr<tyr::planning::GroundTag> context,
           ModuleProgramView program,
           const ModuleProgramSearchOptions<tyr::planning::GroundTag>& options) { return prove_solution(std::move(context), program, options); },
        nb::call_guard<nb::gil_scoped_release>(),
        "context"_a,
        "program"_a,
        "options"_a = ModuleProgramSearchOptions<tyr::planning::GroundTag>());
    m.def(
        "prove_lifted_solution",
        [](runir::datasets::TaskSearchContextPtr<tyr::planning::LiftedTag> context,
           ModuleProgramView program,
           const ModuleProgramSearchOptions<tyr::planning::LiftedTag>& options) { return prove_solution(std::move(context), program, options); },
        nb::call_guard<nb::gil_scoped_release>(),
        "context"_a,
        "program"_a,
        "options"_a = ModuleProgramSearchOptions<tyr::planning::LiftedTag>());
    m.def(
        "find_ground_solution",
        [](runir::datasets::TaskSearchContextPtr<tyr::planning::GroundTag> context,
           ModuleProgramView program,
           const ModuleProgramSearchOptions<tyr::planning::GroundTag>& options) { return find_solution(std::move(context), program, options); },
        nb::call_guard<nb::gil_scoped_release>(),
        "context"_a,
        "program"_a,
        "options"_a = ModuleProgramSearchOptions<tyr::planning::GroundTag>());
    m.def(
        "find_lifted_solution",
        [](runir::datasets::TaskSearchContextPtr<tyr::planning::LiftedTag> context,
           ModuleProgramView program,
           const ModuleProgramSearchOptions<tyr::planning::LiftedTag>& options) { return find_solution(std::move(context), program, options); },
        nb::call_guard<nb::gil_scoped_release>(),
        "context"_a,
        "program"_a,
        "options"_a = ModuleProgramSearchOptions<tyr::planning::LiftedTag>());

    using Kind = tyr::planning::GroundTag;
    using Context = EvaluationContext<Kind>;
    using Step = detail::ModuleProgramStep<Kind>;
    using Expander = SuccessorExpander<Kind>;

    auto execution_context = nb::class_<Context>(m, "ModuleProgramExecutionContext")
                                 .def_prop_ro("state", [](const Context& context) { return context.get_state(); })
                                 .def_prop_ro("memory_state", [](const Context& context) { return context.get_memory_state(); })
                                 .def_prop_ro("module", [](const Context& context) { return context.get_module(); })
                                 .def_prop_ro("concept_registers", [](const Context& context) { return context.concept_registers(); })
                                 .def_prop_ro("role_registers", [](const Context& context) { return context.role_registers(); });
    ygg::add_hash(execution_context);

    nb::class_<Step>(m, "ModuleProgramExecutionStep")
        .def_prop_ro("status", &Step::get_status_name)
        .def_prop_ro("target", &Step::get_target)
        .def_prop_ro("edge", &Step::get_edge);

    nb::class_<Expander>(m, "SuccessorExpander")
        .def(nb::init<const runir::datasets::TaskSearchContext<Kind>&, tyr::planning::StateView<Kind>, std::vector<ModuleView>>(),
             "search_context"_a,
             "initial_state"_a,
             "modules"_a,
             nb::keep_alive<1, 2>())
        .def("context_at", &Expander::context_at, "module"_a, "memory_state"_a, "concept_registers"_a, "role_registers"_a, "source_state"_a)
        .def("initial_context", &Expander::initial_context, "program"_a)
        .def("load_successors", &Expander::load_successors, "context"_a)
        .def("control_successors", &Expander::control_successors, "context"_a, "options"_a)
        .def("matching_rule_at", &Expander::matching_rule_at, "context"_a, "action"_a, "target_state"_a)
        .def("apply_at", &Expander::apply_at, "context"_a, "rule"_a, "action"_a, "target_state"_a);
}

}  // namespace runir::kr::ps::ext
