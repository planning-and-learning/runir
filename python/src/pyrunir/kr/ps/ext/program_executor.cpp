#include "pyrunir/kr/ps/ext/module.hpp"

#include <functional>
#include <nanobind/stl/chrono.h>
#include <nanobind/stl/function.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/string_view.h>
#include <nanobind/stl/variant.h>
#include <nanobind/stl/vector.h>
#include <optional>
#include <pyrunir/graphs/graph.hpp>
#include <runir/kr/dl/semantics/ext/evaluation.hpp>
#include <runir/kr/ps/dl/evaluation.hpp>
#include <runir/kr/ps/ext/action_rule_contract_error.hpp>
#include <runir/kr/ps/ext/evaluation_environment.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/program_executor.hpp>
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

template<tyr::TaskKind Kind, typename FeatureTag>
void bind_feature_evaluation(nb::module_& m)
{
    using FeatureView = ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>>, Repository>;
    using Context = runir::kr::dl::semantics::StateEvaluationContext<runir::kr::ExtFamilyTag, Kind>;

    m.def(
        "evaluate",
        [](const FeatureView& feature, Context& context) { return runir::kr::ps::evaluate(feature, context); },
        "feature"_a,
        "context"_a,
        nb::keep_alive<0, 2>());
}

template<runir::kr::dl::CategoryTag Category>
void bind_choice(nb::module_& m, const char* name)
{
    using Choice = detail::Choice<Category>;
    nb::class_<Choice>(m, name)
        .def_ro("rule", &Choice::rule)
        .def_ro("denotation", &Choice::denotation)
        .def("exhausted", &Choice::exhausted)
        .def("count", &Choice::count)
        .def("current",
             [](const Choice& self)
             {
                 if (self.exhausted())
                     throw nb::index_error("Choice is exhausted.");
                 return self.current();
             })
        .def("advance",
             [](Choice& self)
             {
                 if (self.exhausted())
                     throw nb::index_error("Choice is exhausted.");
                 self.advance();
             });
}

template<tyr::TaskKind Kind>
void bind_execution_types(nb::module_& m, const char* prefix)
{
    using LocalStateView = ModuleStateView<Kind>;
    using StackView = CallStackView<Kind>;
    using StateView = ProgramStateView<Kind>;
    using VertexLabel = ProgramProofVertexLabel<Kind>;
    using Graph = ProgramProofGraph<Kind>;
    using Results = ProgramProofResults<Kind>;
    using Options = ProgramSearchOptions<Kind>;
    using Step = detail::ProgramStep<Kind>;
    using Expander = SuccessorExpander<Kind>;
    using Environment = EvaluationEnvironment<Kind>;
    using Expansion = typename Expander::Expansion;

    nb::class_<ExecutionRepository<Kind>>(m, (std::string(prefix) + "ExecutionRepository").c_str());
    nb::class_<ExecutionBuilder<Kind>>(m, (std::string(prefix) + "ExecutionBuilder").c_str());

    nb::class_<Environment>(m, (std::string(prefix) + "EvaluationEnvironment").c_str())
        .def(nb::init<runir::kr::TaskContext<Kind>&, ProgramView>(), "task_context"_a, "program"_a, nb::keep_alive<1, 2>(), nb::keep_alive<1, 3>())
        .def(
            "make_dl_context",
            [](Environment& self, StateView state) { return self.make_dl_context(state); },
            "program_state"_a,
            nb::keep_alive<0, 1>(),
            nb::keep_alive<0, 2>())
        .def("get_dl_caches", &Environment::get_dl_caches, nb::rv_policy::reference_internal)
        .def("get_dl_target_caches", &Environment::get_dl_target_caches, nb::rv_policy::reference_internal);

    bind_feature_evaluation<Kind, runir::kr::dl::ConceptTag>(m);
    bind_feature_evaluation<Kind, runir::kr::dl::RoleTag>(m);
    bind_feature_evaluation<Kind, runir::kr::ps::dl::BooleanFeature>(m);
    bind_feature_evaluation<Kind, runir::kr::ps::dl::NumericalFeature>(m);
    bind_feature_evaluation<Kind, runir::kr::ps::dl::QueryFeature>(m);

    auto module_state = nb::class_<LocalStateView>(m, (std::string(prefix) + "ModuleState").c_str())
                            .def_prop_ro("state", &LocalStateView::get_state, nb::keep_alive<0, 1>())
                            .def_prop_ro("module", &LocalStateView::get_module, nb::keep_alive<0, 1>())
                            .def_prop_ro("memory_state", &LocalStateView::get_memory_state, nb::keep_alive<0, 1>())
                            .def_prop_ro("registers", &LocalStateView::get_registers, nb::keep_alive<0, 1>())
                            .def_prop_ro("arguments", &LocalStateView::get_arguments, nb::keep_alive<0, 1>());
    ygg::add_print(module_state);
    ygg::add_comparison(module_state);
    ygg::add_hash(module_state);

    auto call_stack = nb::class_<StackView>(m, (std::string(prefix) + "CallStack").c_str())
                          .def_prop_ro("module", &StackView::get_module, nb::keep_alive<0, 1>())
                          .def_prop_ro("return_memory_state", &StackView::get_return_memory_state, nb::keep_alive<0, 1>())
                          .def_prop_ro("registers", &StackView::get_registers, nb::keep_alive<0, 1>())
                          .def_prop_ro("arguments", &StackView::get_arguments, nb::keep_alive<0, 1>())
                          .def_prop_ro("caller", &StackView::get_caller, nb::keep_alive<0, 1>())
                          .def_prop_ro("has_caller", [](const StackView& self) { return self.get_data().caller.has_value(); });
    ygg::add_print(call_stack);
    ygg::add_comparison(call_stack);
    ygg::add_hash(call_stack);

    auto program_state = nb::class_<StateView>(m, (std::string(prefix) + "ProgramState").c_str())
                             .def_prop_ro("state", &StateView::get_state, nb::keep_alive<0, 1>())
                             .def_prop_ro("program", &StateView::get_program, nb::keep_alive<0, 1>())
                             .def_prop_ro("module_state", &StateView::get_module_state, nb::keep_alive<0, 1>())
                             .def_prop_ro("call_stack", &StateView::get_call_stack, nb::keep_alive<0, 1>());
    ygg::add_print(program_state);
    ygg::add_comparison(program_state);
    ygg::add_hash(program_state);

    auto vertex_label = nb::class_<VertexLabel>(m, (std::string(prefix) + "ProgramProofVertexLabel").c_str())
                            .def_ro("program_state", &VertexLabel::program_state)
                            .def_prop_ro(
                                "state",
                                [](const VertexLabel& self) { return self.program_state.get_state(); },
                                nb::keep_alive<0, 1>())
                            .def_ro("is_initial", &VertexLabel::is_initial)
                            .def_ro("is_goal", &VertexLabel::is_goal)
                            .def_ro("is_alive", &VertexLabel::is_alive)
                            .def_ro("is_unsolvable", &VertexLabel::is_unsolvable);
    ygg::add_print(vertex_label);
    ygg::add_comparison(vertex_label);
    ygg::add_hash(vertex_label);

    auto graph = nb::class_<Graph>(m, (std::string(prefix) + "ProgramProofGraph").c_str());
    graph.def(nb::init<>());
    bind_readable_graph_methods<true>(graph);
    bind_forward_graph(graph);

    nb::class_<Results>(m, (std::string(prefix) + "ProgramProofResults").c_str())
        .def_ro("status", &Results::status)
        .def_ro("graph", &Results::graph, nb::keep_alive<0, 1>())
        .def_ro("plan", &Results::plan, nb::rv_policy::copy)
        .def_ro("deadend_states", &Results::deadend_states)
        .def_ro("open_states", &Results::open_states)
        .def_ro("cycle", &Results::cycle)
        .def_ro("statistics", &Results::statistics)
        .def("is_successful", &Results::is_successful);

    nb::class_<Options>(m, (std::string(prefix) + "ProgramSearchOptions").c_str())
        .def(nb::init<>())
        .def_rw("universal", &Options::universal)
        .def_rw("classifier", &Options::classifier, nb::for_setter(nb::keep_alive<1, 2>()))
        .def_rw("max_num_states", &Options::max_num_states)
        .def_rw("max_time", &Options::max_time);

    nb::class_<Step>(m, (std::string(prefix) + "ProgramExecutionStep").c_str())
        .def_prop_ro("status", &Step::get_status_name)
        .def_prop_ro("target", &Step::get_target, nb::keep_alive<0, 1>())
        .def_prop_ro("state_transition", &Step::get_state_transition, nb::keep_alive<0, 1>())
        .def_prop_ro("rule", &Step::get_rule, nb::keep_alive<0, 1>())
        .def_ro("planning_successor", &Step::planning_successor, nb::rv_policy::copy);

    nb::class_<Expander>(m, (std::string(prefix) + "SuccessorExpander").c_str())
        .def(nb::init<runir::kr::TaskContextPtr<Kind>, ProgramView>(), "task_context"_a, "program"_a)
        .def("initial_state", &Expander::initial_state, "node"_a, nb::keep_alive<0, 1>())
        .def(
            "for_each_successor",
            [](Expander& self,
               StateView state,
               const tyr::planning::Node<Kind>& node,
               ProgramSearchStatistics& statistics,
               const std::function<bool(Expansion)>& emit,
               const std::function<bool()>& stop)
            { return self.for_each_successor(state, node, statistics, emit, stop); },
            "state"_a,
            "node"_a,
            "statistics"_a,
            "emit"_a,
            "stop"_a)
        .def("apply_choice", &Expander::template apply_choice<runir::kr::dl::ConceptTag>, "state"_a, "node"_a, "choice"_a, "statistics"_a)
        .def("apply_choice", &Expander::template apply_choice<runir::kr::dl::RoleTag>, "state"_a, "node"_a, "choice"_a, "statistics"_a)
        .def("matching_rule", &Expander::matching_rule, "state"_a, "node"_a, "successor"_a, nb::keep_alive<0, 1>())
        .def("apply", &Expander::apply, "state"_a, "node"_a, "rule"_a, "successor"_a = std::nullopt);
}

}  // namespace

void bind_program_executor(nb::module_& m)
{
    nb::exception<ActionRuleContractError>(m, "ActionRuleContractError", PyExc_RuntimeError);

    bind_choice<runir::kr::dl::ConceptTag>(m, "ConceptChoice");
    bind_choice<runir::kr::dl::RoleTag>(m, "RoleChoice");

    nb::class_<ProgramSearchStatistics>(m, "ProgramSearchStatistics")
        .def(nb::init<>())
        .def_ro("num_expanded", &ProgramSearchStatistics::num_expanded)
        .def_ro("num_generated", &ProgramSearchStatistics::num_generated)
        .def_ro("choice_depth", &ProgramSearchStatistics::choice_depth);

    nb::enum_<ProgramProofStatus>(m, "ProgramProofStatus")
        .value("SUCCESS", ProgramProofStatus::SUCCESS)
        .value("FAILURE", ProgramProofStatus::FAILURE)
        .value("OUT_OF_TIME", ProgramProofStatus::OUT_OF_TIME)
        .value("OUT_OF_STATES", ProgramProofStatus::OUT_OF_STATES);

    auto state_transition = nb::class_<ProgramProofStateTransition>(m, "ProgramProofStateTransition")
                                .def_ro("action", &ProgramProofStateTransition::action)
                                .def_ro("cost", &ProgramProofStateTransition::cost);
    ygg::add_print(state_transition);
    ygg::add_comparison(state_transition);
    ygg::add_hash(state_transition);

    auto edge_label = nb::class_<ProgramProofEdgeLabel>(m, "ProgramProofEdgeLabel")
                          .def_ro("state_transition", &ProgramProofEdgeLabel::state_transition)
                          .def_ro("rule", &ProgramProofEdgeLabel::rule);
    ygg::add_print(edge_label);
    ygg::add_comparison(edge_label);
    ygg::add_hash(edge_label);
    m.attr("GroundProgramProofEdgeLabel") = edge_label;
    m.attr("LiftedProgramProofEdgeLabel") = edge_label;

    bind_execution_types<tyr::GroundTag>(m, "Ground");
    bind_execution_types<tyr::LiftedTag>(m, "Lifted");

    m.def(
        "find_ground_solution",
        [](runir::kr::TaskContextPtr<tyr::GroundTag> task_context, ProgramView program, const ProgramSearchOptions<tyr::GroundTag>& options)
        { return find_solution(std::move(task_context), program, options); },
        nb::call_guard<nb::gil_scoped_release>(),
        "task_context"_a,
        "program"_a,
        "options"_a);
    m.def(
        "find_lifted_solution",
        [](runir::kr::TaskContextPtr<tyr::LiftedTag> task_context, ProgramView program, const ProgramSearchOptions<tyr::LiftedTag>& options)
        { return find_solution(std::move(task_context), program, options); },
        nb::call_guard<nb::gil_scoped_release>(),
        "task_context"_a,
        "program"_a,
        "options"_a);
}

}  // namespace runir::kr::ps::ext
