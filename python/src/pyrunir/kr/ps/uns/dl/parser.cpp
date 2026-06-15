#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/ps/uns/dl/parser.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>

namespace runir::kr::ps::uns::dl
{

void bind_parser(nb::module_& m)
{
    m.def(
        "parse_sketch",
        [](const std::string& description, tyr::formalism::planning::PlanningDomain domain, runir::kr::ps::uns::Repository& repository)
        { return runir::kr::ps::uns::dl::parse_sketch(description, domain.get_domain(), repository); },
        nb::arg("description"),
        nb::arg("domain"),
        nb::arg("repository"));
}

}  // namespace runir::kr::ps::uns::dl
