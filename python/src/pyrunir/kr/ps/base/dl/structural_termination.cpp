#include "pyrunir/kr/ps/base/dl/module.hpp"

#include <cstddef>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/variant.h>
#include <nanobind/stl/vector.h>
#include <optional>
#include <runir/kr/ps/base/dl/incomplete_structural_termination.hpp>
#include <runir/kr/ps/base/dl/structural_termination.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::ps::base::dl
{

using namespace nanobind::literals;

namespace
{

/// Python-facing counterexample with dict-shaped feature valuations: the
/// positional bitsets of the C++ labels are resolved against the result's
/// feature lists at conversion time.
struct PolicyGraphVertexWrapper
{
    std::vector<std::pair<BooleanFeatureView, bool>> booleans;
    std::vector<std::pair<NumericalFeatureView, bool>> numericals;
};

struct PolicyGraphEdgeWrapper
{
    std::size_t source;
    std::size_t target;
    RuleView rule;
    std::vector<std::pair<NumericalFeatureView, NumericalChange>> numerical_changes;
};

struct PolicyGraphWrapper
{
    std::vector<PolicyGraphVertexWrapper> vertices;
    std::vector<PolicyGraphEdgeWrapper> edges;
};

struct StructuralTerminationResultWrapper
{
    StructuralTerminationStatus status;
    std::vector<BooleanFeatureView> booleans;
    std::vector<NumericalFeatureView> numericals;
    std::optional<PolicyGraphWrapper> counterexample;

    bool is_terminating() const noexcept { return status == StructuralTerminationStatus::TERMINATING; }
};

template<typename Pairs>
nb::dict to_dict(const Pairs& pairs)
{
    auto dict = nb::dict();
    for (const auto& [key, value] : pairs)
        dict[nb::cast(key)] = nb::cast(value);
    return dict;
}

StructuralTerminationResultWrapper wrap_result(StructuralTerminationResult result, const Repository& context)
{
    auto wrapper = StructuralTerminationResultWrapper {};
    wrapper.status = result.status;
    for (auto feature : result.booleans)
        wrapper.booleans.push_back(BooleanFeatureView(feature, context));
    for (auto feature : result.numericals)
        wrapper.numericals.push_back(NumericalFeatureView(feature, context));

    if (result.counterexample)
    {
        auto graph = PolicyGraphWrapper {};
        for (const auto& vertex : result.counterexample->get_vertices())
        {
            const auto& label = vertex.get_property();
            auto vertex_wrapper = PolicyGraphVertexWrapper {};
            for (std::size_t position = 0; position < wrapper.booleans.size(); ++position)
                vertex_wrapper.booleans.emplace_back(wrapper.booleans[position], label.boolean_values.test(position));
            for (std::size_t position = 0; position < wrapper.numericals.size(); ++position)
                vertex_wrapper.numericals.emplace_back(wrapper.numericals[position], label.numerical_values.test(position));
            graph.vertices.push_back(std::move(vertex_wrapper));
        }
        for (const auto& edge : result.counterexample->get_edges())
        {
            const auto& label = edge.get_property();
            auto edge_wrapper = PolicyGraphEdgeWrapper { edge.get_source(), edge.get_target(), label.rule, {} };
            for (std::size_t position = 0; position < wrapper.numericals.size(); ++position)
                edge_wrapper.numerical_changes.emplace_back(wrapper.numericals[position], label.numerical_changes[position]);
            graph.edges.push_back(std::move(edge_wrapper));
        }
        wrapper.counterexample = std::move(graph);
    }

    return wrapper;
}

}  // namespace

void bind_structural_termination(nb::module_& m)
{
    nb::enum_<NumericalChange>(m, "NumericalChange")
        .value("UNCONSTRAINED", NumericalChange::UNCONSTRAINED)
        .value("INCREASES", NumericalChange::INCREASES)
        .value("DECREASES", NumericalChange::DECREASES)
        .value("UNCHANGED", NumericalChange::UNCHANGED);

    nb::enum_<StructuralTerminationStatus>(m, "StructuralTerminationStatus")
        .value("TERMINATING", StructuralTerminationStatus::TERMINATING)
        .value("NON_TERMINATING", StructuralTerminationStatus::NON_TERMINATING);

    nb::class_<PolicyGraphVertexWrapper>(m, "PolicyGraphVertex")
        .def("get_booleans", [](const PolicyGraphVertexWrapper& self) { return to_dict(self.booleans); },
             "Truth value per Boolean feature.")
        .def("get_numericals", [](const PolicyGraphVertexWrapper& self) { return to_dict(self.numericals); },
             "Per numerical feature: whether its value is greater than zero.");

    nb::class_<PolicyGraphEdgeWrapper>(m, "PolicyGraphEdge")
        .def("get_source", [](const PolicyGraphEdgeWrapper& self) { return self.source; },
             "Index of the source vertex in the counterexample's vertex list.")
        .def("get_target", [](const PolicyGraphEdgeWrapper& self) { return self.target; },
             "Index of the target vertex in the counterexample's vertex list.")
        .def("get_rule", [](const PolicyGraphEdgeWrapper& self) { return self.rule; })
        .def("get_numerical_changes", [](const PolicyGraphEdgeWrapper& self) { return to_dict(self.numerical_changes); },
             "Qualitative change per numerical feature.");

    nb::class_<PolicyGraphWrapper>(m, "PolicyGraph")
        .def("get_num_vertices", [](const PolicyGraphWrapper& self) { return self.vertices.size(); })
        .def("get_num_edges", [](const PolicyGraphWrapper& self) { return self.edges.size(); })
        .def("get_vertices", [](const PolicyGraphWrapper& self) { return self.vertices; })
        .def("get_edges", [](const PolicyGraphWrapper& self) { return self.edges; });

    nb::class_<StructuralTerminationResultWrapper>(m, "StructuralTerminationResult")
        .def_ro("status", &StructuralTerminationResultWrapper::status)
        .def("is_terminating", &StructuralTerminationResultWrapper::is_terminating)
        .def("get_booleans", [](const StructuralTerminationResultWrapper& self) { return self.booleans; })
        .def("get_numericals", [](const StructuralTerminationResultWrapper& self) { return self.numericals; })
        .def("get_counterexample", [](const StructuralTerminationResultWrapper& self) { return self.counterexample; },
             "The surviving non-trivial strongly connected components, or None if terminating.");

    m.def(
        "structural_termination",
        [](SketchView sketch) { return wrap_result(structural_termination(sketch), sketch.get_context()); },
        "sketch"_a,
        "Decide structural termination of the sketch with the complete Sieve algorithm on the policy graph.");

    nb::enum_<IncompleteStructuralTerminationStatus>(m, "IncompleteStructuralTerminationStatus")
        .value("TERMINATING", IncompleteStructuralTerminationStatus::TERMINATING)
        .value("UNKNOWN", IncompleteStructuralTerminationStatus::UNKNOWN);

    nb::class_<IncompleteBlockingReason>(m, "IncompleteBlockingReason")
        .def_ro("feature", &IncompleteBlockingReason::feature, "The feature whose decrease or flip the rule performs.")
        .def_ro("opposing_rules", &IncompleteBlockingReason::opposing_rules,
                "Remaining rules that oppose the change and survive R3's marked-complementary-condition check.");

    nb::class_<IncompleteSurvivingRule>(m, "IncompleteSurvivingRule")
        .def_ro("rule", &IncompleteSurvivingRule::rule)
        .def_ro("blocking_reasons", &IncompleteSurvivingRule::blocking_reasons);

    nb::class_<IncompleteStructuralTerminationResult>(m, "IncompleteStructuralTerminationResult")
        .def_ro("status", &IncompleteStructuralTerminationResult::status)
        .def_ro("surviving_rules", &IncompleteStructuralTerminationResult::surviving_rules)
        .def("is_terminating", &IncompleteStructuralTerminationResult::is_terminating);

    m.def("incomplete_structural_termination",
          &incomplete_structural_termination,
          "sketch"_a,
          "Sound but incomplete syntactic termination test (rule elimination with feature marking).");
}

}  // namespace runir::kr::ps::base::dl
