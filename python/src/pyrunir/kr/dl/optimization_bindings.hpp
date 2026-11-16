#ifndef PYRUNIR_KR_DL_OPTIMIZATION_BINDINGS_HPP_
#define PYRUNIR_KR_DL_OPTIMIZATION_BINDINGS_HPP_

#include "runir/kr/dl/datas.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/dl/semantics/state_evaluation_context.hpp"

#include <nanobind/nanobind.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/vector.h>
#include <runir/kr/dl/optimization.hpp>
#include <urd/python/sequences/sequence.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
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
        .def(
            "add",
            [](Sampler& sampler, GroundContext& context) { sampler.template add<tyr::GroundTag>(context); },
            nb::arg("context"))
        .def(
            "add",
            [](Sampler& sampler, LiftedContext& context) { sampler.template add<tyr::LiftedTag>(context); },
            nb::arg("context"))
        .def("finish",
             [](const Sampler& sampler)
             {
                 auto warnings = ygg::diagnostics::Warnings();
                 auto statistics = sampler.finish(warnings);
                 ygg::issue_warnings(warnings);
                 return statistics;
             });
}

/// optimize(expression, repository[, statistics]), collect_query_statistics(expression, contexts) and
/// query_statistics_sampler(expression) of a DL expression view.
template<typename View>
void bind_optimize(nanobind::module_& m)
{
    namespace nb = nanobind;
    using Repository = runir::kr::dl::ConstructorRepository;
    using Statistics = runir::kr::dl::QueryStatistics;
    using GroundContext = runir::kr::dl::semantics::StateEvaluationContext<tyr::GroundTag>;
    using LiftedContext = runir::kr::dl::semantics::StateEvaluationContext<tyr::LiftedTag>;
    m.def(
         "optimize",
         [](const View& expression, Repository& repository) { return runir::kr::dl::optimize(expression, repository); },
         nb::arg("expression"),
         nb::arg("repository"),
         nb::keep_alive<0, 2>())
        .def(
            "optimize",
            [](const View& expression, Repository& repository, const Statistics& statistics)
            { return runir::kr::dl::optimize(expression, repository, statistics); },
            nb::arg("expression"),
            nb::arg("repository"),
            nb::arg("statistics"),
            nb::keep_alive<0, 2>())
        .def(
            "collect_query_statistics",
            [](const View& expression, const urd::Store<urd::sequence<GroundContext*>>& contexts)
            {
                auto warnings = ygg::diagnostics::Warnings();
                auto statistics = runir::kr::dl::collect_query_statistics<tyr::GroundTag>(expression, contexts, warnings);
                ygg::issue_warnings(warnings);
                return statistics;
            },
            nb::arg("expression"),
            nb::arg("contexts"))
        .def(
            "collect_query_statistics",
            [](const View& expression, const urd::Store<urd::sequence<LiftedContext*>>& contexts)
            {
                auto warnings = ygg::diagnostics::Warnings();
                auto statistics = runir::kr::dl::collect_query_statistics<tyr::LiftedTag>(expression, contexts, warnings);
                ygg::issue_warnings(warnings);
                return statistics;
            },
            nb::arg("expression"),
            nb::arg("contexts"))
        .def(
            "query_statistics_sampler",
            [](const View& expression)
            {
                auto collector = runir::kr::dl::QueryStatisticsCollector();
                collector.add(expression);
                return collector.sampler();
            },
            nb::arg("expression"));
}

}  // namespace runir::kr::python

#endif
