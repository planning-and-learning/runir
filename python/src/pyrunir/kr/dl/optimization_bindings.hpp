#ifndef PYRUNIR_KR_DL_OPTIMIZATION_BINDINGS_HPP_
#define PYRUNIR_KR_DL_OPTIMIZATION_BINDINGS_HPP_

#include <nanobind/nanobind.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/vector.h>
#include <runir/kr/dl/optimization.hpp>
#include <runir/kr/dl/semantics/ext/evaluation.hpp>
#include <vector>

namespace runir::kr::python
{

template<runir::kr::dl::FamilyTag Family>
void bind_query_statistics(nanobind::module_& m)
{
    namespace nb = nanobind;
    using Statistics = runir::kr::dl::QueryStatistics<Family>;
    nb::class_<Statistics>(m, "QueryStatistics")
        .def(nb::init<>())
        .def_rw("objects", &Statistics::objects)
        .def("num_leaves", [](const Statistics& statistics) { return statistics.leaves.size(); });
}

/// optimize(repository[, statistics]) and collect_query_statistics(contexts) on a DL expression view.
template<runir::kr::dl::FamilyTag Family, typename View>
void bind_optimize(nanobind::class_<View>& cls)
{
    namespace nb = nanobind;
    using Repository = runir::kr::dl::ConstructorRepositoryFor<Family>;
    using Statistics = runir::kr::dl::QueryStatistics<Family>;
    using GroundContext = runir::kr::dl::semantics::StateEvaluationContext<Family, tyr::GroundTag>;
    using LiftedContext = runir::kr::dl::semantics::StateEvaluationContext<Family, tyr::LiftedTag>;
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
            { return runir::kr::dl::collect_query_statistics<tyr::GroundTag>(view, contexts); },
            nb::arg("contexts"))
        .def(
            "collect_query_statistics",
            [](const View& view, const std::vector<LiftedContext*>& contexts)
            { return runir::kr::dl::collect_query_statistics<tyr::LiftedTag>(view, contexts); },
            nb::arg("contexts"));
}

}  // namespace runir::kr::python

#endif
