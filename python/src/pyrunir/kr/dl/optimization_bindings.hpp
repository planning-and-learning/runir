#ifndef PYRUNIR_KR_DL_OPTIMIZATION_BINDINGS_HPP_
#define PYRUNIR_KR_DL_OPTIMIZATION_BINDINGS_HPP_

#include <nanobind/nanobind.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/vector.h>
#include <runir/kr/dl/optimization.hpp>
#include <runir/kr/dl/semantics/ext/evaluation.hpp>
#include <vector>
#include <yggdrasil/python/diagnostics.hpp>

namespace runir::kr::python
{

inline void bind_query_statistics(nanobind::module_& m)
{
    namespace nb = nanobind;
    using Statistics = runir::kr::dl::QueryStatistics;
    nb::class_<Statistics>(m, "QueryStatistics")
        .def(nb::init<>())
        .def_rw("objects", &Statistics::objects)
        .def("num_leaves", [](const Statistics& statistics) { return statistics.leaves.size(); });

    // add(context) measures one sample state; its context and storage may be dropped right after, bounding memory.
    using Sampler = runir::kr::dl::QueryStatisticsSampler;
    using GroundContext = runir::kr::dl::semantics::StateEvaluationContext<tyr::GroundTag>;
    using LiftedContext = runir::kr::dl::semantics::StateEvaluationContext<tyr::LiftedTag>;
    nb::class_<Sampler>(m, "QueryStatisticsSampler")
        .def("add", [](Sampler& sampler, GroundContext& context) { sampler.template add<tyr::GroundTag>(context); }, nb::arg("context"))
        .def("add", [](Sampler& sampler, LiftedContext& context) { sampler.template add<tyr::LiftedTag>(context); }, nb::arg("context"))
        .def("finish",
             [](const Sampler& sampler)
             {
                 auto warnings = ygg::diagnostics::Warnings();
                 auto statistics = sampler.finish(warnings);
                 ygg::issue_warnings(warnings);
                 return statistics;
             });
}

/// optimize(repository[, statistics]) and collect_query_statistics(contexts) on a DL expression view.
template<typename View>
void bind_optimize(nanobind::class_<View>& cls)
{
    namespace nb = nanobind;
    using Repository = runir::kr::dl::ConstructorRepositoryFor;
    using Statistics = runir::kr::dl::QueryStatistics;
    using GroundContext = runir::kr::dl::semantics::StateEvaluationContext<tyr::GroundTag>;
    using LiftedContext = runir::kr::dl::semantics::StateEvaluationContext<tyr::LiftedTag>;
    cls.def(
           "optimize",
           [](const View& view, Repository& repository) { return runir::kr::dl::optimize(view, repository); },
           nb::arg("repository"),
           nb::keep_alive<0, 2>())
        .def(
            "optimize",
            [](const View& view, Repository& repository, const Statistics& statistics) { return runir::kr::dl::optimize(view, repository, statistics); },
            nb::arg("repository"),
            nb::arg("statistics"),
            nb::keep_alive<0, 2>())
        .def(
            "collect_query_statistics",
            [](const View& view, const std::vector<GroundContext*>& contexts)
            {
                auto warnings = ygg::diagnostics::Warnings();
                auto statistics = runir::kr::dl::collect_query_statistics<tyr::GroundTag>(view, contexts, warnings);
                ygg::issue_warnings(warnings);
                return statistics;
            },
            nb::arg("contexts"))
        .def(
            "collect_query_statistics",
            [](const View& view, const std::vector<LiftedContext*>& contexts)
            {
                auto warnings = ygg::diagnostics::Warnings();
                auto statistics = runir::kr::dl::collect_query_statistics<tyr::LiftedTag>(view, contexts, warnings);
                ygg::issue_warnings(warnings);
                return statistics;
            },
            nb::arg("contexts"))
        .def("query_statistics_sampler",
             [](const View& view)
             {
                 auto collector = runir::kr::dl::QueryStatisticsCollector();
                 collector.add(view);
                 return collector.sampler();
             });
}

}  // namespace runir::kr::python

#endif
