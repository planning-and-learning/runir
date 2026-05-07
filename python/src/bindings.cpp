#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <runir/runir.hpp>

namespace nb = nanobind;
using namespace nanobind::literals;

NB_MODULE(_core, m)
{
    nb::class_<runir::FeatureSummary>(m, "FeatureSummary")
        .def(nb::init<>())
        .def_rw("name", &runir::FeatureSummary::name)
        .def_rw("score", &runir::FeatureSummary::score)
        .def("__repr__", [](const runir::FeatureSummary& summary) { return runir::format_summary(summary); });

    m.def("add", &runir::add, "lhs"_a, "rhs"_a);
    m.def("describe", &runir::describe, "name"_a, "score"_a);
    m.def("format_summary", &runir::format_summary, "summary"_a);
}
