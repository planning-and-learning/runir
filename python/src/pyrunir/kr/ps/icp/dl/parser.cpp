#include "pyrunir/kr/ps/icp/dl/module.hpp"

#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <runir/kr/ps/icp/dl/parser.hpp>
#include <runir/kr/ps/icp/repository.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>

namespace runir::kr::ps::icp::dl
{

using namespace nanobind::literals;

void bind_parser(nb::module_& m)
{
    m.def(
        "parse_module",
        [](const std::string& description, tyr::formalism::planning::PlanningDomain domain, runir::kr::ps::icp::Repository& repository)
        { return runir::kr::ps::icp::dl::parse_module(description, domain.get_domain(), repository); },
        "description"_a,
        "domain"_a,
        "repository"_a,
        nb::keep_alive<0, 3>());

    m.def(
        "parse_program",
        [](const std::string& description, tyr::formalism::planning::PlanningDomain domain, runir::kr::ps::icp::Repository& repository)
        { return runir::kr::ps::icp::dl::parse_program(description, domain.get_domain(), repository); },
        "description"_a,
        "domain"_a,
        "repository"_a,
        nb::keep_alive<0, 3>());
}

}  // namespace runir::kr::ps::icp::dl
