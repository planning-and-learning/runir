#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <runir/runir.hpp>
#include <string>
#include <utility>

namespace nb = nanobind;
using namespace nanobind::literals;

namespace
{

std::string format_summary(std::string name, int score) { return runir::format_summary(runir::describe(std::move(name), score)); }

}

NB_MODULE(downstream_runir_user, m)
{
    m.def("add", &runir::add, "lhs"_a, "rhs"_a);
    m.def("format_summary", &format_summary, "name"_a, "score"_a);
}
