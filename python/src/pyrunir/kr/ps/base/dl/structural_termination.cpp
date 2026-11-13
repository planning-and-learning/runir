#include "pyrunir/kr/ps/base/dl/bindings.hpp"
#include "pyrunir/module.hpp"

#include <nanobind/stl/optional.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/variant.h>
#include <nanobind/stl/vector.h>
#include <runir/kr/ps/base/dl/formatter.hpp>
#include <runir/kr/ps/base/dl/incomplete_structural_termination.hpp>
#include <runir/kr/ps/base/dl/structural_termination.hpp>
#include <runir/kr/ps/base/formatter.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <urd/python/graphs.hpp>
#include <vector>
#include <vefr/dot.hpp>
#include <vefr/python/bindings.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::ps::base::dl
{

void bind_structural_termination(nb::module_& m)
{
    using namespace nb::literals;
    nb::enum_<StructuralTerminationStatus>(m, "StructuralTerminationStatus")
        .value("TERMINATING", StructuralTerminationStatus::TERMINATING)
        .value("NON_TERMINATING", StructuralTerminationStatus::NON_TERMINATING);

    auto vertex_label = nb::class_<PolicyGraphVertexLabel>(m, "PolicyGraphVertexLabel")
                            .def_ro("boolean_values", &PolicyGraphVertexLabel::boolean_values)
                            .def_ro("numerical_values", &PolicyGraphVertexLabel::numerical_values);
    urd::python::add_comparison(vertex_label);

    {
        auto graph = nb::class_<PolicyGraph>(m, "PolicyGraph");
        urd::python::bind_graph(graph);
        vefr::python::bind_algorithms(graph, vefr::vertex_data, vefr::edge_data);
    }

    nb::class_<SccStructuralTerminationResult>(m, "SccStructuralTerminationResult")
        .def_ro("booleans", &SccStructuralTerminationResult::booleans)
        .def_ro("numericals", &SccStructuralTerminationResult::numericals);

    nb::class_<SieveStructuralTerminationResult>(m, "SieveStructuralTerminationResult")
        .def_ro("scc_results", &SieveStructuralTerminationResult::scc_results)
        .def_ro("counterexample", &SieveStructuralTerminationResult::counterexample, nb::keep_alive<0, 1>())
        .def_ro("surviving_rules", &SieveStructuralTerminationResult::surviving_rules)
        .def("is_terminating", &SieveStructuralTerminationResult::is_terminating);

    nb::class_<StructuralTerminationResult>(m, "StructuralTerminationResult")
        .def_ro("status", &StructuralTerminationResult::status)
        .def_ro("incomplete_result", &StructuralTerminationResult::incomplete_result, nb::keep_alive<0, 1>())
        .def_ro("sieve_result", &StructuralTerminationResult::sieve_result, nb::keep_alive<0, 1>())
        .def("is_terminating", &StructuralTerminationResult::is_terminating);

    m.def("structural_termination",
          nb::overload_cast<SketchView, std::size_t, bool, bool>(&structural_termination),
          "sketch"_a,
          "max_features"_a = runir::kr::ps::dl::default_max_features,
          "use_incomplete_preprocessing"_a = runir::kr::ps::dl::default_use_incomplete_preprocessing,
          "use_memory_scc_scope"_a = runir::kr::ps::dl::default_use_memory_scc_scope,
          nb::keep_alive<0, 1>(),
          "Decide structural termination with configurable feature limit and optional memory-SCC-scoped incomplete preprocessing.");

    nb::class_<runir::kr::ps::dl::IncompleteBlockingReason<runir::kr::BaseFamilyTag>>(m, "IncompleteBlockingReason")
        .def_ro("feature",
                &runir::kr::ps::dl::IncompleteBlockingReason<runir::kr::BaseFamilyTag>::feature,
                "The feature whose decrease or flip the rule performs.")
        .def_ro("opposing_rules",
                &runir::kr::ps::dl::IncompleteBlockingReason<runir::kr::BaseFamilyTag>::opposing_rules,
                "Remaining rules that oppose the change and survive R3's marked-complementary-condition check.");

    nb::class_<runir::kr::ps::dl::IncompleteSurvivingRule<runir::kr::BaseFamilyTag>>(m, "IncompleteSurvivingRule")
        .def_ro("rule", &runir::kr::ps::dl::IncompleteSurvivingRule<runir::kr::BaseFamilyTag>::rule)
        .def_ro("blocking_reasons", &runir::kr::ps::dl::IncompleteSurvivingRule<runir::kr::BaseFamilyTag>::blocking_reasons);

    nb::class_<runir::kr::ps::dl::IncompleteStructuralTerminationResult<runir::kr::BaseFamilyTag>>(m, "IncompleteStructuralTerminationResult")
        .def_ro("status", &runir::kr::ps::dl::IncompleteStructuralTerminationResult<runir::kr::BaseFamilyTag>::status)
        .def_ro("surviving_rules", &runir::kr::ps::dl::IncompleteStructuralTerminationResult<runir::kr::BaseFamilyTag>::surviving_rules)
        .def("is_terminating", &runir::kr::ps::dl::IncompleteStructuralTerminationResult<runir::kr::BaseFamilyTag>::is_terminating);

    m.def("incomplete_structural_termination",
          &incomplete_structural_termination,
          "sketch"_a,
          "use_memory_scc_scope"_a = runir::kr::ps::dl::default_use_memory_scc_scope,
          nb::keep_alive<0, 1>(),
          "Sound but incomplete syntactic termination test, scoped to residual memory SCCs by default.");
}

}  // namespace runir::kr::ps::base::dl
