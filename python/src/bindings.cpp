#include "pyrunir/graphs/graphs.hpp"
#include "pyrunir/kr/dl/dl.hpp"

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <runir/runir.hpp>

namespace nb = nanobind;
using namespace nanobind::literals;

namespace runir
{

NB_MODULE(pyrunir, m)
{
    auto graphs = m.def_submodule("graphs");
    python::bind_graphs(graphs);

    auto kr = m.def_submodule("kr");
    auto dl = kr.def_submodule("dl");
    kr::dl::bind_module_definitions(dl);

    nb::class_<FeatureSummary>(m, "FeatureSummary")
        .def(nb::init<>())
        .def_rw("name", &FeatureSummary::name)
        .def_rw("score", &FeatureSummary::score)
        .def("__repr__", [](const FeatureSummary& summary) { return format_summary(summary); });

    m.def("add", &add, "lhs"_a, "rhs"_a);
    m.def("describe", &describe, "name"_a, "score"_a);
    m.def("format_summary", &format_summary, "summary"_a);
}

}  // namespace runir
