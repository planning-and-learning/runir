#include "pyrunir/kr/ps/ext/dl/module.hpp"

#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>

namespace runir::kr::ps::ext::dl
{

using namespace nanobind::literals;

void bind_parser(nb::module_& m)
{
    m.def(
        "parse_module",
        [](const std::string& description, tyr::formalism::planning::PlanningDomain domain, runir::kr::ps::ext::Repository& repository)
        { return runir::kr::ps::ext::dl::parse_module(description, domain.get_domain(), repository); },
        "description"_a,
        "domain"_a,
        "repository"_a);

    m.def(
        "parse_module_program",
        [](const std::string& description, tyr::formalism::planning::PlanningDomain domain, runir::kr::ps::ext::Repository& repository)
        { return runir::kr::ps::ext::dl::parse_module_program(description, domain.get_domain(), repository); },
        "description"_a,
        "domain"_a,
        "repository"_a);

    m.def(
        "parse_modules",
        [](const std::vector<std::string>& descriptions, tyr::formalism::planning::PlanningDomain domain, runir::kr::ps::ext::Repository& repository)
        { return runir::kr::ps::ext::dl::parse_modules(descriptions, domain.get_domain(), repository); },
        "descriptions"_a,
        "domain"_a,
        "repository"_a);
}

}  // namespace runir::kr::ps::ext::dl
