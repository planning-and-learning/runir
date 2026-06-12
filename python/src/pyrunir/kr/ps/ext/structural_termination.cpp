#include "pyrunir/kr/ps/ext/module.hpp"

#include <cstddef>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/variant.h>
#include <nanobind/stl/vector.h>
#include <optional>
#include <runir/kr/ps/ext/dl/ceg_structural_termination.hpp>
#include <runir/kr/ps/ext/dl/structural_termination.hpp>
#include <utility>
#include <variant>
#include <vector>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::ps::ext
{

using namespace nanobind::literals;

namespace
{

using ConceptFeatureView = ygg::View<ygg::Index<Feature<runir::kr::dl::ConceptTag>>, Repository>;
using BooleanFeatureView = ygg::View<ygg::Index<Feature<runir::kr::ps::dl::BooleanFeature>>, Repository>;
using NumericalFeatureView = ygg::View<ygg::Index<Feature<runir::kr::ps::dl::NumericalFeature>>, Repository>;
using NumericalLikeFeatureView = std::variant<ConceptFeatureView, NumericalFeatureView>;
using runir::kr::ps::ext::dl::ModuleStructuralTerminationResult;
using runir::kr::ps::ext::dl::ModuleProgramStructuralTerminationResult;
using runir::kr::ps::ext::dl::NumericalChange;
using runir::kr::ps::ext::dl::StructuralTerminationStatus;

/// Python-facing counterexample with dict-shaped feature valuations: the
/// positional bitsets of the C++ labels are resolved against the result's
/// feature lists at conversion time.
struct ModulePolicyGraphVertexWrapper
{
    std::vector<std::pair<ConceptFeatureView, bool>> concepts;
    std::vector<std::pair<BooleanFeatureView, bool>> booleans;
    std::vector<std::pair<NumericalFeatureView, bool>> numericals;
    std::optional<MemoryStateView> memory_state;
};

struct ModulePolicyGraphEdgeWrapper
{
    std::size_t source;
    std::size_t target;
    std::optional<RuleVariantView> rule;
    std::vector<std::pair<NumericalLikeFeatureView, NumericalChange>> numerical_changes;
};

struct ModulePolicyGraphWrapper
{
    std::vector<ModulePolicyGraphVertexWrapper> vertices;
    std::vector<ModulePolicyGraphEdgeWrapper> edges;
};

struct ModuleStructuralTerminationResultWrapper
{
    StructuralTerminationStatus status;
    std::vector<ConceptFeatureView> concepts;
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

template<typename Pairs>
nb::dict to_dict(const Pairs& pairs)
{
    auto dict = nb::dict();
    for (const auto& [key, value] : pairs)
    {
        if constexpr (requires { std::visit([](auto) {}, key); })
            dict[std::visit([](auto view) { return nb::cast(view); }, key)] = nb::cast(value);
        else
            dict[nb::cast(key)] = nb::cast(value);
    }
    return dict;
}

ModuleStructuralTerminationResultWrapper wrap_result(ModuleStructuralTerminationResult result, const Repository& context)
{
    auto wrapper = ModuleStructuralTerminationResultWrapper {};
    wrapper.status = result.status;
    for (auto feature : result.concepts)
        wrapper.concepts.push_back(ConceptFeatureView(feature, context));
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
            for (std::size_t position = 0; position < wrapper.concepts.size(); ++position)
                vertex_wrapper.concepts.emplace_back(wrapper.concepts[position], label.concept_values.test(position));
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
            // Numerical-like positions span the concepts first, then the
            // numericals.
            for (std::size_t position = 0; position < wrapper.concepts.size(); ++position)
                edge_wrapper.numerical_changes.emplace_back(wrapper.concepts[position], label.numerical_changes[position]);
            for (std::size_t position = 0; position < wrapper.numericals.size(); ++position)
                edge_wrapper.numerical_changes.emplace_back(wrapper.numericals[position],
                                                            label.numerical_changes[wrapper.concepts.size() + position]);
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
        .def("get_concepts", [](const ModulePolicyGraphVertexWrapper& self) { return to_dict(self.concepts); },
             "Per concept feature: whether its cardinality is greater than zero.")
        .def("get_booleans", [](const ModulePolicyGraphVertexWrapper& self) { return to_dict(self.booleans); },
             "Truth value per Boolean feature.")
        .def("get_numericals", [](const ModulePolicyGraphVertexWrapper& self) { return to_dict(self.numericals); },
             "Per numerical feature: whether its value is greater than zero.")
        .def("get_memory_state", [](const ModulePolicyGraphVertexWrapper& self) { return self.memory_state.value(); });

    nb::class_<ModulePolicyGraphEdgeWrapper>(m, "ModulePolicyGraphEdge")
        .def("get_source", [](const ModulePolicyGraphEdgeWrapper& self) { return self.source; },
             "Index of the source vertex in the counterexample's vertex list.")
        .def("get_target", [](const ModulePolicyGraphEdgeWrapper& self) { return self.target; },
             "Index of the target vertex in the counterexample's vertex list.")
        .def("get_rule", [](const ModulePolicyGraphEdgeWrapper& self) { return self.rule.value(); })
        .def("get_numerical_changes", [](const ModulePolicyGraphEdgeWrapper& self) { return to_dict(self.numerical_changes); },
             "Qualitative change per concept and numerical feature.");

    nb::class_<ModulePolicyGraphWrapper>(m, "ModulePolicyGraph")
        .def("get_num_vertices", [](const ModulePolicyGraphWrapper& self) { return self.vertices.size(); })
        .def("get_num_edges", [](const ModulePolicyGraphWrapper& self) { return self.edges.size(); })
        .def("get_vertices", [](const ModulePolicyGraphWrapper& self) { return self.vertices; })
        .def("get_edges", [](const ModulePolicyGraphWrapper& self) { return self.edges; });

    nb::class_<ModuleStructuralTerminationResultWrapper>(m, "ModuleStructuralTerminationResult")
        .def_ro("status", &ModuleStructuralTerminationResultWrapper::status)
        .def("is_terminating", &ModuleStructuralTerminationResultWrapper::is_terminating)
        .def("get_concepts", [](const ModuleStructuralTerminationResultWrapper& self) { return self.concepts; })
        .def("get_booleans", [](const ModuleStructuralTerminationResultWrapper& self) { return self.booleans; })
        .def("get_numericals", [](const ModuleStructuralTerminationResultWrapper& self) { return self.numericals; })
        .def("get_counterexample", [](const ModuleStructuralTerminationResultWrapper& self) { return self.counterexample; },
             "The surviving non-trivial strongly connected components, or None if terminating.");

    nb::class_<ModuleProgramStructuralTerminationResultWrapper>(m, "ModuleProgramStructuralTerminationResult")
        .def_ro("status", &ModuleProgramStructuralTerminationResultWrapper::status)
        .def("is_terminating", &ModuleProgramStructuralTerminationResultWrapper::is_terminating)
        .def("get_module_results", [](const ModuleProgramStructuralTerminationResultWrapper& self) { return self.module_results; })
        .def("get_recursive_call_rules", [](const ModuleProgramStructuralTerminationResultWrapper& self) { return self.recursive_call_rules; },
             "Call rules that participate in the recursive module call graph.");

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

}  // namespace runir::kr::ps::ext
