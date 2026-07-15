#include "pyrunir/kr/ps/ext/module.hpp"

#include <nanobind/stl/chrono.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <optional>
#include <pyrunir/graphs/graph.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/module_program_executor.hpp>
#include <runir/kr/ps/ext/successor_expander.hpp>
#include <runir/kr/task_context.hpp>
#include <string>
#include <tyr/planning/declarations.hpp>
#include <utility>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::ps::ext
{

using namespace nanobind::literals;
using runir::graphs::bind_forward_graph;
using runir::graphs::bind_readable_graph_methods;

namespace
{

template<tyr::planning::TaskKind Kind>
void bind_execution_types(nb::module_& m, const char* prefix)
{
    using RegisterView = RegisterValuesView<Kind>;
    using ArgumentsView = CallArgumentsView<Kind>;
    using StackView = CallStackView<Kind>;
    using StateView = ExecutionStateView<Kind>;
    using VertexLabel = ModuleProgramProofVertexLabel<Kind>;
    using Graph = ModuleProgramProofGraph<Kind>;
    using Results = ModuleProgramProofResults<Kind>;
    using Options = ModuleProgramSearchOptions<Kind>;
    using Step = detail::ModuleProgramStep<Kind>;
    using Expander = SuccessorExpander<Kind>;

    auto register_values = nb::class_<RegisterView>(m, (std::string(prefix) + "RegisterValues").c_str())
                               .def_prop_ro("concept_values", &RegisterView::get_concept_values)
                               .def_prop_ro("role_values", &RegisterView::get_role_values);
    ygg::add_hash(register_values);

    auto call_arguments = nb::class_<ArgumentsView>(m, (std::string(prefix) + "CallArguments").c_str())
                              .def_prop_ro("concept_arguments", [](const ArgumentsView& self) { return self.template get<runir::kr::dl::ConceptTag>(); })
                              .def_prop_ro("role_arguments", [](const ArgumentsView& self) { return self.template get<runir::kr::dl::RoleTag>(); })
                              .def_prop_ro("boolean_arguments", [](const ArgumentsView& self) { return self.template get<runir::kr::dl::BooleanTag>(); })
                              .def_prop_ro("numerical_arguments", [](const ArgumentsView& self) { return self.template get<runir::kr::dl::NumericalTag>(); });
    ygg::add_hash(call_arguments);

    auto call_stack = nb::class_<StackView>(m, (std::string(prefix) + "CallStack").c_str())
                          .def_prop_ro("module", &StackView::get_module, nb::keep_alive<0, 1>())
                          .def_prop_ro("memory_state", &StackView::get_memory_state, nb::keep_alive<0, 1>())
                          .def_prop_ro("registers", &StackView::get_registers, nb::keep_alive<0, 1>())
                          .def_prop_ro("arguments", &StackView::get_arguments, nb::keep_alive<0, 1>())
                          .def_prop_ro("caller", &StackView::get_caller, nb::keep_alive<0, 1>())
                          .def_prop_ro("has_caller", [](const StackView& self) { return self.get_data().caller.has_value(); });
    ygg::add_hash(call_stack);

    auto execution_state = nb::class_<StateView>(m, (std::string(prefix) + "ExecutionState").c_str())
                               .def_prop_ro("state", &StateView::get_state, nb::keep_alive<0, 1>())
                               .def_prop_ro("program", &StateView::get_program, nb::keep_alive<0, 1>())
                               .def_prop_ro("phase", &StateView::get_phase)
                               .def_prop_ro("call_stack", &StateView::get_call_stack, nb::keep_alive<0, 1>());
    ygg::add_hash(execution_state);

    auto vertex_label = nb::class_<VertexLabel>(m, (std::string(prefix) + "ModuleProgramProofVertexLabel").c_str())
                            .def_ro("execution_state", &VertexLabel::execution_state)
                            .def_prop_ro(
                                "state",
                                [](const VertexLabel& self) { return self.execution_state.get_state(); },
                                nb::keep_alive<0, 1>())
                            .def_ro("is_initial", &VertexLabel::is_initial)
                            .def_ro("is_goal", &VertexLabel::is_goal)
                            .def_ro("is_alive", &VertexLabel::is_alive)
                            .def_ro("is_unsolvable", &VertexLabel::is_unsolvable);
    ygg::add_print(vertex_label);
    ygg::add_hash(vertex_label);

    auto graph = nb::class_<Graph>(m, (std::string(prefix) + "ModuleProgramProofGraph").c_str());
    graph.def(nb::init<>());
    bind_readable_graph_methods<true>(graph);
    bind_forward_graph(graph);

    nb::class_<Results>(m, (std::string(prefix) + "ModuleProgramProofResults").c_str())
        .def_ro("status", &Results::status)
        .def_ro("graph", &Results::graph, nb::keep_alive<0, 1>())
        .def_ro("plan", &Results::plan, nb::keep_alive<0, 1>())
        .def_ro("deadend_states", &Results::deadend_states)
        .def_ro("open_states", &Results::open_states)
        .def_ro("cycle", &Results::cycle)
        .def("is_successful", &Results::is_successful);

    nb::class_<Options>(m, (std::string(prefix) + "ModuleProgramSearchOptions").c_str())
        .def(nb::init<>())
        .def_rw("universal", &Options::universal)
        .def_rw("classifier", &Options::classifier, nb::for_setter(nb::keep_alive<1, 2>()))
        .def_rw("max_num_states", &Options::max_num_states)
        .def_rw("max_time", &Options::max_time)
        .def_rw("random_seed", &Options::random_seed)
        .def_rw("shuffle_labeled_succ_nodes", &Options::shuffle_labeled_succ_nodes)
        .def_rw("shuffle_choice_points", &Options::shuffle_choice_points);

    nb::class_<Step>(m, (std::string(prefix) + "ModuleProgramExecutionStep").c_str())
        .def_prop_ro("status", &Step::get_status_name)
        .def_prop_ro("target", &Step::get_target, nb::keep_alive<0, 1>())
        .def_prop_ro("state_transition", &Step::get_state_transition, nb::keep_alive<0, 1>())
        .def_prop_ro("rule", &Step::get_rule, nb::keep_alive<0, 1>());

    nb::class_<Expander>(m, (std::string(prefix) + "SuccessorExpander").c_str())
        .def(nb::init<runir::kr::TaskContextPtr<Kind>, ModuleProgramView>(), "task_context"_a, "program"_a)
        .def("initial_state", &Expander::initial_state, nb::keep_alive<0, 1>())
        .def("load_steps", &Expander::load_steps, "state"_a)
        .def(
            "control_steps",
            [](Expander& self, StateView state) { return self.control_steps(std::move(state)); },
            "state"_a)
        .def("matching_rule", &Expander::matching_rule, "state"_a, "action"_a, "target_state"_a, nb::keep_alive<0, 1>())
        .def("apply", &Expander::apply, "state"_a, "rule"_a, "action"_a = std::nullopt, "target_state"_a = std::nullopt);
}

}  // namespace

void bind_module_program_executor(nb::module_& m)
{
    nb::enum_<ExecutionPhase>(m, "ExecutionPhase").value("INTERNAL", ExecutionPhase::INTERNAL).value("EXTERNAL", ExecutionPhase::EXTERNAL);

    nb::enum_<ModuleProgramProofStatus>(m, "ModuleProgramProofStatus")
        .value("SUCCESS", ModuleProgramProofStatus::SUCCESS)
        .value("FAILURE", ModuleProgramProofStatus::FAILURE)
        .value("OUT_OF_TIME", ModuleProgramProofStatus::OUT_OF_TIME)
        .value("OUT_OF_STATES", ModuleProgramProofStatus::OUT_OF_STATES);

    auto state_transition = nb::class_<ModuleProgramProofStateTransition>(m, "ModuleProgramProofStateTransition")
                                .def_ro("action", &ModuleProgramProofStateTransition::action)
                                .def_ro("cost", &ModuleProgramProofStateTransition::cost);
    ygg::add_hash(state_transition);

    auto edge_label = nb::class_<ModuleProgramProofEdgeLabel>(m, "ModuleProgramProofEdgeLabel")
                          .def_ro("state_transition", &ModuleProgramProofEdgeLabel::state_transition)
                          .def_ro("rule", &ModuleProgramProofEdgeLabel::rule);
    ygg::add_print(edge_label);
    ygg::add_hash(edge_label);
    m.attr("GroundModuleProgramProofEdgeLabel") = edge_label;
    m.attr("LiftedModuleProgramProofEdgeLabel") = edge_label;

    bind_execution_types<tyr::planning::GroundTag>(m, "Ground");
    bind_execution_types<tyr::planning::LiftedTag>(m, "Lifted");

    m.def(
        "find_ground_solution",
        [](runir::kr::TaskContextPtr<tyr::planning::GroundTag> task_context,
           ModuleProgramView program,
           const ModuleProgramSearchOptions<tyr::planning::GroundTag>& options) { return find_solution(std::move(task_context), program, options); },
        nb::call_guard<nb::gil_scoped_release>(),
        "task_context"_a,
        "program"_a,
        "options"_a);
    m.def(
        "find_lifted_solution",
        [](runir::kr::TaskContextPtr<tyr::planning::LiftedTag> task_context,
           ModuleProgramView program,
           const ModuleProgramSearchOptions<tyr::planning::LiftedTag>& options) { return find_solution(std::move(task_context), program, options); },
        nb::call_guard<nb::gil_scoped_release>(),
        "task_context"_a,
        "program"_a,
        "options"_a);
}

}  // namespace runir::kr::ps::ext
