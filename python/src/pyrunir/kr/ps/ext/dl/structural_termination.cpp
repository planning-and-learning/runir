#include "pyrunir/kr/ps/ext/dl/module.hpp"

#include <boost/dynamic_bitset.hpp>
#include <cstddef>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/variant.h>
#include <nanobind/stl/vector.h>
#include <pyrunir/graphs/graph.hpp>
#include <runir/kr/ps/ext/dl/incomplete_structural_termination.hpp>
#include <runir/kr/ps/ext/dl/structural_termination.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <vector>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::ps::ext::dl
{

using namespace nanobind::literals;
using runir::graphs::bind_forward_graph;
using runir::graphs::bind_readable_graph_methods;

namespace
{

std::vector<bool> materialize(const boost::dynamic_bitset<>& values)
{
    auto result = std::vector<bool>(values.size());
    for (std::size_t position = 0; position < values.size(); ++position)
        result[position] = values.test(position);
    return result;
}

}  // namespace

void bind_structural_termination(nb::module_& m)
{
    nb::enum_<StructuralTerminationStatus>(m, "StructuralTerminationStatus")
        .value("TERMINATING", StructuralTerminationStatus::TERMINATING)
        .value("NON_TERMINATING", StructuralTerminationStatus::NON_TERMINATING);

    nb::enum_<IncompleteStructuralTerminationStatus>(m, "IncompleteStructuralTerminationStatus")
        .value("TERMINATING", IncompleteStructuralTerminationStatus::TERMINATING)
        .value("UNKNOWN", IncompleteStructuralTerminationStatus::UNKNOWN);

    nb::class_<IncompleteBlockingReason>(m, "IncompleteBlockingReason")
        .def_ro("feature", &IncompleteBlockingReason::feature)
        .def_ro("opposing_rules", &IncompleteBlockingReason::opposing_rules);

    nb::class_<IncompleteSurvivingRule>(m, "IncompleteSurvivingRule")
        .def_ro("rule", &IncompleteSurvivingRule::rule)
        .def_ro("blocking_reasons", &IncompleteSurvivingRule::blocking_reasons);

    nb::class_<ModuleIncompleteStructuralTerminationResult>(m, "ModuleIncompleteStructuralTerminationResult")
        .def_ro("status", &ModuleIncompleteStructuralTerminationResult::status)
        .def_ro("booleans", &ModuleIncompleteStructuralTerminationResult::booleans)
        .def_ro("numericals", &ModuleIncompleteStructuralTerminationResult::numericals)
        .def_ro("surviving_rules", &ModuleIncompleteStructuralTerminationResult::surviving_rules)
        .def("is_terminating", &ModuleIncompleteStructuralTerminationResult::is_terminating);

    nb::class_<ModuleProgramIncompleteStructuralTerminationResult>(m, "ModuleProgramIncompleteStructuralTerminationResult")
        .def_ro("status", &ModuleProgramIncompleteStructuralTerminationResult::status)
        .def_ro("module_results", &ModuleProgramIncompleteStructuralTerminationResult::module_results)
        .def_ro("recursive_call_rules", &ModuleProgramIncompleteStructuralTerminationResult::recursive_call_rules)
        .def("is_terminating", &ModuleProgramIncompleteStructuralTerminationResult::is_terminating);

    nb::class_<ModulePolicyGraphVertexLabel>(m, "ModulePolicyGraphVertexLabel")
        .def_prop_ro("boolean_values", [](const ModulePolicyGraphVertexLabel& label) { return materialize(label.boolean_values); })
        .def_prop_ro("numerical_values", [](const ModulePolicyGraphVertexLabel& label) { return materialize(label.numerical_values); })
        .def_ro("memory_state", &ModulePolicyGraphVertexLabel::memory_state);

    nb::class_<ModulePolicyGraphEdgeLabel>(m, "ModulePolicyGraphEdgeLabel")
        .def_ro("rule", &ModulePolicyGraphEdgeLabel::rule)
        .def_ro("numerical_changes", &ModulePolicyGraphEdgeLabel::numerical_changes);

    auto graph = nb::class_<ModulePolicyGraph>(m, "ModulePolicyGraph");
    bind_readable_graph_methods(graph);
    bind_forward_graph(graph);

    nb::class_<ModuleStructuralTerminationResult>(m, "ModuleStructuralTerminationResult")
        .def_ro("status", &ModuleStructuralTerminationResult::status)
        .def_ro("booleans", &ModuleStructuralTerminationResult::booleans)
        .def_ro("numericals", &ModuleStructuralTerminationResult::numericals)
        .def_ro("counterexample", &ModuleStructuralTerminationResult::counterexample, nb::keep_alive<0, 1>())
        .def("is_terminating", &ModuleStructuralTerminationResult::is_terminating);

    nb::class_<ModuleProgramStructuralTerminationResult>(m, "ModuleProgramStructuralTerminationResult")
        .def_ro("status", &ModuleProgramStructuralTerminationResult::status)
        .def_ro("module_results", &ModuleProgramStructuralTerminationResult::module_results)
        .def_ro("recursive_call_rules", &ModuleProgramStructuralTerminationResult::recursive_call_rules)
        .def("is_terminating", &ModuleProgramStructuralTerminationResult::is_terminating);

    m.def("structural_termination",
          nb::overload_cast<ModuleView, std::size_t, bool>(&structural_termination),
          "module"_a,
          "max_features"_a = runir::kr::ps::dl::default_max_features,
          "use_incomplete_preprocessing"_a = runir::kr::ps::dl::default_use_incomplete_preprocessing,
          nb::keep_alive<0, 1>(),
          "Decide structural termination with configurable feature limit and optional incomplete preprocessing.");

    m.def("structural_termination",
          nb::overload_cast<ModuleProgramView, std::size_t, bool>(&structural_termination),
          "program"_a,
          "max_features"_a = runir::kr::ps::dl::default_max_features,
          "use_incomplete_preprocessing"_a = runir::kr::ps::dl::default_use_incomplete_preprocessing,
          nb::keep_alive<0, 1>(),
          "Conservatively decide structural termination of a module program, including inter-module call graph cycles.");

    m.def("incomplete_structural_termination",
          nb::overload_cast<ModuleView>(&incomplete_structural_termination),
          "module"_a,
          nb::keep_alive<0, 1>(),
          "Apply the sound incomplete termination proof within recurrent memory components.");

    m.def("incomplete_structural_termination",
          nb::overload_cast<ModuleProgramView>(&incomplete_structural_termination),
          "program"_a,
          nb::keep_alive<0, 1>(),
          "Apply the incomplete proof to every module and reject recursive or unresolved calls.");
}

}  // namespace runir::kr::ps::ext::dl
