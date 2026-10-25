#include "pyrunir/kr/ps/ext/dl/bindings.hpp"

#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <runir/kr/ps/ext/module_view.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>

namespace runir::kr::ps::ext::dl
{

void bind_parser(nb::module_& m)
{
    using namespace nb::literals;
    m.def(
        "parse_module",
        [](const std::string& description, const tyr::formalism::planning::PlanningDomain& domain, runir::kr::ps::ext::Repository& repository)
        { return runir::kr::ps::ext::dl::parse_module(description, domain.get_domain(), repository); },
        "description"_a,
        "domain"_a,
        "repository"_a);

    m.def(
        "parse_program",
        [](const std::string& description, const tyr::formalism::planning::PlanningDomain& domain, runir::kr::ps::ext::Repository& repository)
        { return runir::kr::ps::ext::dl::parse_program(description, domain.get_domain(), repository); },
        "description"_a,
        "domain"_a,
        "repository"_a);

    m.def(
        "parse_modules",
        [](const std::vector<std::string>& descriptions, const tyr::formalism::planning::PlanningDomain& domain, runir::kr::ps::ext::Repository& repository)
        { return runir::kr::ps::ext::dl::parse_modules(descriptions, domain.get_domain(), repository); },
        "descriptions"_a,
        "domain"_a,
        "repository"_a);

    m.def(
        "parse_concept",
        [](const std::string& description,
           const tyr::formalism::planning::PlanningDomain& domain,
           runir::kr::ps::ext::ModuleView module_,
           runir::kr::ps::ext::Repository& repository) { return runir::kr::ps::ext::dl::parse_concept(description, domain.get_domain(), module_, repository); },
        "description"_a,
        "domain"_a,
        "module"_a,
        "repository"_a,
        "Parse a standalone expression in the scope of a module; its registers and arguments may be referenced by name.");

    m.def(
        "parse_role",
        [](const std::string& description,
           const tyr::formalism::planning::PlanningDomain& domain,
           runir::kr::ps::ext::ModuleView module_,
           runir::kr::ps::ext::Repository& repository) { return runir::kr::ps::ext::dl::parse_role(description, domain.get_domain(), module_, repository); },
        "description"_a,
        "domain"_a,
        "module"_a,
        "repository"_a,
        "Parse a standalone expression in the scope of a module; its registers and arguments may be referenced by name.");

    m.def(
        "parse_boolean",
        [](const std::string& description,
           const tyr::formalism::planning::PlanningDomain& domain,
           runir::kr::ps::ext::ModuleView module_,
           runir::kr::ps::ext::Repository& repository) { return runir::kr::ps::ext::dl::parse_boolean(description, domain.get_domain(), module_, repository); },
        "description"_a,
        "domain"_a,
        "module"_a,
        "repository"_a,
        "Parse a standalone expression in the scope of a module; its registers and arguments may be referenced by name.");

    m.def(
        "parse_numerical",
        [](const std::string& description,
           const tyr::formalism::planning::PlanningDomain& domain,
           runir::kr::ps::ext::ModuleView module_,
           runir::kr::ps::ext::Repository& repository)
        { return runir::kr::ps::ext::dl::parse_numerical(description, domain.get_domain(), module_, repository); },
        "description"_a,
        "domain"_a,
        "module"_a,
        "repository"_a,
        "Parse a standalone expression in the scope of a module; its registers and arguments may be referenced by name.");
}

}  // namespace runir::kr::ps::ext::dl
