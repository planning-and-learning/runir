#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/uns/dl/parser.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>

namespace runir::kr::uns::dl
{

void bind_parser(nb::module_& m)
{
    m.def(
        "parse_classifier",
        [](const std::string& description, tyr::formalism::planning::PlanningDomain domain, runir::kr::uns::Repository& repository)
        { return runir::kr::uns::dl::parse_classifier(description, domain.get_domain(), repository); },
        nb::arg("description"),
        nb::arg("domain"),
        nb::arg("repository"),
        nb::keep_alive<0, 3>());
}

}  // namespace runir::kr::uns::dl
