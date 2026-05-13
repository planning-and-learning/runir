#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/gp/dl/parser.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>

namespace runir::kr::gp::dl
{

namespace fp = tyr::formalism::planning;

using namespace nanobind::literals;

void bind_parser(nb::module_& m)
{
    m.def(
        "parse_policy",
        [](const std::string& description, fp::PlanningDomain domain, runir::kr::gp::Repository& repository)
        { return runir::kr::gp::dl::parse_policy(description, domain.get_domain(), repository); },
        "description"_a,
        "domain"_a,
        "repository"_a);
}

}  // namespace runir::kr::gp::dl
