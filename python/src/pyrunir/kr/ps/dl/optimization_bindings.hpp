#ifndef PYRUNIR_KR_PS_DL_OPTIMIZATION_BINDINGS_HPP_
#define PYRUNIR_KR_PS_DL_OPTIMIZATION_BINDINGS_HPP_

#include <nanobind/nanobind.h>
#include <nanobind/stl/vector.h>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/evaluation_storage.hpp>
#include <runir/kr/dl/semantics/register_values_view.hpp>
#include <runir/kr/dl/semantics/state_evaluation.hpp>
#include <runir/kr/ps/dl/optimization.hpp>
#include <tyr/planning/state_repository.hpp>
#include <vector>
#include <yggdrasil/python/diagnostics.hpp>

namespace runir::kr::python
{

/// optimize(repository[, statistics]) and collect_query_statistics(contexts) on any ps record view: a feature, a
/// sketch, a program, a module or a classifier.
template<runir::kr::FamilyTag Family, typename Repository, typename View>
void bind_ps_optimize(nanobind::class_<View>& cls)
{
    namespace nb = nanobind;
    using Statistics = runir::kr::dl::QueryStatistics;
    using GroundContext = runir::kr::dl::semantics::StateEvaluationContext<tyr::GroundTag>;
    using LiftedContext = runir::kr::dl::semantics::StateEvaluationContext<tyr::LiftedTag>;
    cls.def(
           "optimize",
           [](const View& view, Repository& repository) { return runir::kr::ps::dl::optimize(view, repository); },
           nb::arg("repository"),
           nb::keep_alive<0, 2>())
        .def(
            "optimize",
            [](const View& view, Repository& repository, const Statistics& statistics) { return runir::kr::ps::dl::optimize(view, repository, statistics); },
            nb::arg("repository"),
            nb::arg("statistics"),
            nb::keep_alive<0, 2>())
        .def(
            "collect_query_statistics",
            [](const View& view, const std::vector<GroundContext*>& contexts)
            {
                auto warnings = ygg::diagnostics::Warnings();
                auto statistics = runir::kr::ps::dl::collect_query_statistics<tyr::GroundTag>(view, contexts, warnings);
                ygg::issue_warnings(warnings);
                return statistics;
            },
            nb::arg("contexts"))
        .def(
            "collect_query_statistics",
            [](const View& view, const std::vector<LiftedContext*>& contexts)
            {
                auto warnings = ygg::diagnostics::Warnings();
                auto statistics = runir::kr::ps::dl::collect_query_statistics<tyr::LiftedTag>(view, contexts, warnings);
                ygg::issue_warnings(warnings);
                return statistics;
            },
            nb::arg("contexts"))
        .def("query_statistics_sampler", [](const View& view) { return runir::kr::ps::dl::query_statistics_sampler(view); });
}

}  // namespace runir::kr::python

#endif
