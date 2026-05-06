#include <template_feature/template_feature.hpp>

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>

namespace nb = nanobind;
using namespace nanobind::literals;

NB_MODULE(_core, m)
{
    nb::class_<template_feature::FeatureSummary>(m, "FeatureSummary")
        .def(nb::init<>())
        .def_rw("name", &template_feature::FeatureSummary::name)
        .def_rw("score", &template_feature::FeatureSummary::score)
        .def("__repr__", [](const template_feature::FeatureSummary& summary) { return template_feature::format_summary(summary); });

    m.def("add", &template_feature::add, "lhs"_a, "rhs"_a);
    m.def("describe", &template_feature::describe, "name"_a, "score"_a);
    m.def("format_summary", &template_feature::format_summary, "summary"_a);
}
