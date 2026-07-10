#include "pyrunir/kr/ps/ext/dl/module.hpp"

#include <cstddef>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/vector.h>
#include <optional>
#include <runir/kr/ps/ext/dl/ceg_structural_termination.hpp>
#include <runir/kr/ps/ext/dl/structural_termination.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::ps::ext::dl
{

using namespace nanobind::literals;

namespace
{

using BooleanFeatureView = ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::BooleanFeature>>, Repository>;
using NumericalFeatureView = ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::NumericalFeature>>, Repository>;
using runir::kr::ps::ext::dl::ModuleProgramStructuralTerminationResult;
using runir::kr::ps::ext::dl::ModuleStructuralTerminationResult;
using runir::kr::ps::ext::dl::NumericalChange;
using runir::kr::ps::ext::dl::StructuralTerminationStatus;

/// Python-facing counterexample with feature-value pairs: the
/// positional bitsets of the C++ labels are resolved against the result's
/// feature lists at conversion time.
struct ModulePolicyGraphVertexWrapper
{
    std::vector<std::pair<BooleanFeatureView, bool>> booleans;
    std::vector<std::pair<NumericalFeatureView, bool>> numericals;
    std::optional<MemoryStateView> memory_state;
};

struct ModulePolicyGraphEdgeWrapper
{
    std::size_t source;
    std::size_t target;
    std::optional<RuleVariantView> rule;
    std::vector<std::pair<NumericalFeatureView, NumericalChange>> numerical_changes;
};

struct ModulePolicyGraphWrapper
{
    std::vector<ModulePolicyGraphVertexWrapper> vertices;
    std::vector<ModulePolicyGraphEdgeWrapper> edges;
};

struct ModuleStructuralTerminationResultWrapper
{
    StructuralTerminationStatus status;
    std::vector<BooleanFeatureView> booleans;
    std::vector<NumericalFeatureView> numericals;
    std::optional<ModulePolicyGraphWrapper> counterexample;

    bool is_terminating() const noexcept { return status == StructuralTerminationStatus::TERMINATING; }
};

struct ModuleProgramStructuralTerminationResultWrapper
{
    StructuralTerminationStatus status;
    std::vector<ModuleStructuralTerminationResultWrapper> module_results;
    std::vector<RuleVariantView> recursive_call_rules;

    bool is_terminating() const noexcept { return status == StructuralTerminationStatus::TERMINATING; }
};

ModuleStructuralTerminationResultWrapper wrap_result(ModuleStructuralTerminationResult result, const Repository& context)
{
    auto wrapper = ModuleStructuralTerminationResultWrapper {};
    wrapper.status = result.status;
    for (auto feature : result.booleans)
        wrapper.booleans.push_back(BooleanFeatureView(feature, context));
    for (auto feature : result.numericals)
        wrapper.numericals.push_back(NumericalFeatureView(feature, context));

    if (result.counterexample)
    {
        auto graph = ModulePolicyGraphWrapper {};
        for (const auto& vertex : result.counterexample->get_vertices())
        {
            const auto& label = vertex.get_property();
            auto vertex_wrapper = ModulePolicyGraphVertexWrapper {};
            for (std::size_t position = 0; position < wrapper.booleans.size(); ++position)
                vertex_wrapper.booleans.emplace_back(wrapper.booleans[position], label.boolean_values.test(position));
            for (std::size_t position = 0; position < wrapper.numericals.size(); ++position)
                vertex_wrapper.numericals.emplace_back(wrapper.numericals[position], label.numerical_values.test(position));
            vertex_wrapper.memory_state = label.memory_state;
            graph.vertices.push_back(std::move(vertex_wrapper));
        }
        for (const auto& edge : result.counterexample->get_edges())
        {
            const auto& label = edge.get_property();
            auto edge_wrapper = ModulePolicyGraphEdgeWrapper { edge.get_source(), edge.get_target(), label.rule, {} };
            for (std::size_t position = 0; position < wrapper.numericals.size(); ++position)
                edge_wrapper.numerical_changes.emplace_back(wrapper.numericals[position], label.numerical_changes[position]);
            graph.edges.push_back(std::move(edge_wrapper));
        }
        wrapper.counterexample = std::move(graph);
    }

    return wrapper;
}

ModuleProgramStructuralTerminationResultWrapper wrap_result(ModuleProgramStructuralTerminationResult result, const Repository& context)
{
    auto wrapper = ModuleProgramStructuralTerminationResultWrapper {};
    wrapper.status = result.status;
    for (auto& module_result : result.module_results)
        wrapper.module_results.push_back(wrap_result(std::move(module_result), context));
    for (auto rule : result.recursive_call_rules)
        wrapper.recursive_call_rules.push_back(rule);
    return wrapper;
}

}  // namespace

void bind_structural_termination(nb::module_& m)
{
    nb::enum_<StructuralTerminationStatus>(m, "StructuralTerminationStatus")
        .value("TERMINATING", StructuralTerminationStatus::TERMINATING)
        .value("NON_TERMINATING", StructuralTerminationStatus::NON_TERMINATING);

    nb::class_<ModulePolicyGraphVertexWrapper>(m, "ModulePolicyGraphVertex")
        .def_ro("booleans", &ModulePolicyGraphVertexWrapper::booleans)
        .def_ro("numericals", &ModulePolicyGraphVertexWrapper::numericals)
        .def_ro("memory_state", &ModulePolicyGraphVertexWrapper::memory_state);

    nb::class_<ModulePolicyGraphEdgeWrapper>(m, "ModulePolicyGraphEdge")
        .def_ro("source", &ModulePolicyGraphEdgeWrapper::source)
        .def_ro("target", &ModulePolicyGraphEdgeWrapper::target)
        .def_ro("rule", &ModulePolicyGraphEdgeWrapper::rule)
        .def_ro("numerical_changes", &ModulePolicyGraphEdgeWrapper::numerical_changes);

    nb::class_<ModulePolicyGraphWrapper>(m, "ModulePolicyGraph")
        .def_ro("vertices", &ModulePolicyGraphWrapper::vertices)
        .def_ro("edges", &ModulePolicyGraphWrapper::edges);

    nb::class_<ModuleStructuralTerminationResultWrapper>(m, "ModuleStructuralTerminationResult")
        .def_ro("status", &ModuleStructuralTerminationResultWrapper::status)
        .def_ro("booleans", &ModuleStructuralTerminationResultWrapper::booleans)
        .def_ro("numericals", &ModuleStructuralTerminationResultWrapper::numericals)
        .def_ro("counterexample", &ModuleStructuralTerminationResultWrapper::counterexample)
        .def("is_terminating", &ModuleStructuralTerminationResultWrapper::is_terminating);

    nb::class_<ModuleProgramStructuralTerminationResultWrapper>(m, "ModuleProgramStructuralTerminationResult")
        .def_ro("status", &ModuleProgramStructuralTerminationResultWrapper::status)
        .def_ro("module_results", &ModuleProgramStructuralTerminationResultWrapper::module_results)
        .def_ro("recursive_call_rules", &ModuleProgramStructuralTerminationResultWrapper::recursive_call_rules)
        .def("is_terminating", &ModuleProgramStructuralTerminationResultWrapper::is_terminating);

    m.def(
        "structural_termination",
        [](ModuleView module_) { return wrap_result(runir::kr::ps::ext::dl::structural_termination(module_), module_.get_context()); },
        "module"_a,
        "Decide structural termination of the extended sketch module on the extended policy graph.");

    m.def(
        "structural_termination",
        [](ModuleProgramView program) { return wrap_result(runir::kr::ps::ext::dl::structural_termination(program), program.get_context()); },
        "program"_a,
        "Conservatively decide structural termination of a module program, including inter-module call graph cycles.");

    m.def(
        "ceg_structural_termination",
        [](ModuleView module_) { return wrap_result(runir::kr::ps::ext::dl::ceg_structural_termination(module_), module_.get_context()); },
        "module"_a,
        "Equivalent to structural_termination but decomposes by memory-graph components and projects features.");
}

}  // namespace runir::kr::ps::ext::dl
