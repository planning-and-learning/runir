#include "pyrunir/kr/ps/ext/dl/module.hpp"

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <runir/kr/ps/ext/dl/module_factory.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>

namespace runir::kr::ps::ext::dl
{

namespace nb = nanobind;
using namespace nanobind::literals;

void bind_module_factory(nb::module_& m)
{
    namespace ext_dl = runir::kr::ps::ext::dl;

    nb::enum_<ext_dl::ModuleSpecification>(m, "ModuleSpecification")
        .value("ON_BONET_ET_AL_ICAPS2024", ext_dl::ModuleSpecification::ON_BONET_ET_AL_ICAPS2024)
        .value("ON_TABLE_BONET_ET_AL_ICAPS2024", ext_dl::ModuleSpecification::ON_TABLE_BONET_ET_AL_ICAPS2024)
        .value("TOWER_BONET_ET_AL_ICAPS2024", ext_dl::ModuleSpecification::TOWER_BONET_ET_AL_ICAPS2024)
        .value("BLOCKS_BONET_ET_AL_ICAPS2024", ext_dl::ModuleSpecification::BLOCKS_BONET_ET_AL_ICAPS2024);

    nb::class_<ext_dl::ModuleFactory>(m, "ModuleFactory")
        .def_static("create_empty", &ext_dl::ModuleFactory::create_empty, "repository"_a)
        .def_static(
            "create",
            [](ext_dl::ModuleSpecification specification, tyr::formalism::planning::PlanningDomain domain, runir::kr::ps::ext::Repository& repository)
            { return ext_dl::ModuleFactory::create(specification, domain.get_domain(), repository); },
            "specification"_a,
            "domain"_a,
            "repository"_a)
        .def_static("create_empty_description", &ext_dl::ModuleFactory::create_empty_description)
        .def_static("create_description", &ext_dl::ModuleFactory::create_description, "specification"_a)
        .def_static(
            "create_bonet_et_al_icaps2024_modules",
            [](tyr::formalism::planning::PlanningDomain domain, runir::kr::ps::ext::Repository& repository)
            { return ext_dl::ModuleFactory::create_bonet_et_al_icaps2024_modules(domain.get_domain(), repository); },
            "domain"_a,
            "repository"_a)
        .def_static("create_bonet_et_al_icaps2024_descriptions", &ext_dl::ModuleFactory::create_bonet_et_al_icaps2024_descriptions)
        .def_static("create_bonet_et_al_icaps2024_program_description", &ext_dl::ModuleFactory::create_bonet_et_al_icaps2024_program_description)
        .def_static(
            "create_bonet_et_al_icaps2024_program",
            [](tyr::formalism::planning::PlanningDomain domain, runir::kr::ps::ext::Repository& repository)
            { return ext_dl::ModuleFactory::create_bonet_et_al_icaps2024_program(domain.get_domain(), repository); },
            "domain"_a,
            "repository"_a)
        .def_static(
            "create_on_bonet_et_al_icaps2024",
            [](tyr::formalism::planning::PlanningDomain domain, runir::kr::ps::ext::Repository& repository)
            { return ext_dl::ModuleFactory::create_on_bonet_et_al_icaps2024(domain.get_domain(), repository); },
            "domain"_a,
            "repository"_a)
        .def_static(
            "create_on_table_bonet_et_al_icaps2024",
            [](tyr::formalism::planning::PlanningDomain domain, runir::kr::ps::ext::Repository& repository)
            { return ext_dl::ModuleFactory::create_on_table_bonet_et_al_icaps2024(domain.get_domain(), repository); },
            "domain"_a,
            "repository"_a)
        .def_static(
            "create_tower_bonet_et_al_icaps2024",
            [](tyr::formalism::planning::PlanningDomain domain, runir::kr::ps::ext::Repository& repository)
            { return ext_dl::ModuleFactory::create_tower_bonet_et_al_icaps2024(domain.get_domain(), repository); },
            "domain"_a,
            "repository"_a)
        .def_static(
            "create_blocks_bonet_et_al_icaps2024",
            [](tyr::formalism::planning::PlanningDomain domain, runir::kr::ps::ext::Repository& repository)
            { return ext_dl::ModuleFactory::create_blocks_bonet_et_al_icaps2024(domain.get_domain(), repository); },
            "domain"_a,
            "repository"_a)
        .def_static("create_on_bonet_et_al_icaps2024_description", &ext_dl::ModuleFactory::create_on_bonet_et_al_icaps2024_description)
        .def_static("create_on_table_bonet_et_al_icaps2024_description", &ext_dl::ModuleFactory::create_on_table_bonet_et_al_icaps2024_description)
        .def_static("create_tower_bonet_et_al_icaps2024_description", &ext_dl::ModuleFactory::create_tower_bonet_et_al_icaps2024_description)
        .def_static("create_blocks_bonet_et_al_icaps2024_description", &ext_dl::ModuleFactory::create_blocks_bonet_et_al_icaps2024_description);
}

}  // namespace runir::kr::ps::ext::dl
