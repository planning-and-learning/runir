#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/gp/dl/parser.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>

namespace runir::kr::gp::dl
{

void bind_parser(nb::module_& m)
{
    m.def(
        "parse_policy",
        [](const std::string& description, tyr::formalism::planning::PlanningDomain domain, runir::kr::gp::Repository& repository)
        { return runir::kr::gp::dl::parse_policy(description, domain.get_domain(), repository); },
        nb::arg("description"),
        nb::arg("domain"),
        nb::arg("repository"));
}

}  // namespace runir::kr::gp::dl
