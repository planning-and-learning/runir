#include <template_feature/template_feature.hpp>

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>

#include <string>
#include <utility>

namespace nb = nanobind;
using namespace nanobind::literals;

namespace
{

std::string format_summary(std::string name, int score)
{
    return template_feature::format_summary(template_feature::describe(std::move(name), score));
}

}

NB_MODULE(downstream_template_user, m)
{
    m.def("add", &template_feature::add, "lhs"_a, "rhs"_a);
    m.def("format_summary", &format_summary, "name"_a, "score"_a);
}
