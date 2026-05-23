#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/ps/base/dl/sketch_factory.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>

namespace runir::kr::ps::base::dl
{

void bind_sketch_factory(nb::module_& m)
{
    nb::enum_<SketchSpecification>(m, "SketchSpecification")
        .value("GRIPPER_FRANCE_ET_AL_AAAI2021", SketchSpecification::GRIPPER_FRANCE_ET_AL_AAAI2021)
        .value("BLOCKS3OPS_FRANCE_ET_AL_AAAI2021", SketchSpecification::BLOCKS3OPS_FRANCE_ET_AL_AAAI2021)
        .value("SPANNER_FRANCE_ET_AL_AAAI2021", SketchSpecification::SPANNER_FRANCE_ET_AL_AAAI2021)
        .value("DELIVERY_FRANCE_ET_AL_AAAI2021", SketchSpecification::DELIVERY_FRANCE_ET_AL_AAAI2021);

    nb::class_<SketchFactory>(m, "SketchFactory")
        .def_static("create_empty", &SketchFactory::create_empty, nb::arg("repository"), nb::keep_alive<0, 1>())
        .def_static("create_empty_description", &SketchFactory::create_empty_description)
        .def_static(
            "create",
            [](SketchSpecification specification, tyr::formalism::planning::PlanningDomain domain, runir::kr::ps::Repository& repository)
            { return SketchFactory::create(specification, domain.get_domain(), repository); },
            nb::arg("specification"),
            nb::arg("domain"),
            nb::arg("repository"),
            nb::keep_alive<0, 3>())
        .def_static("create_description", &SketchFactory::create_description, nb::arg("specification"))
        .def_static(
            "create_gripper_france_et_al_aaai2021",
            [](tyr::formalism::planning::PlanningDomain domain, runir::kr::ps::Repository& repository)
            { return SketchFactory::create_gripper_france_et_al_aaai2021(domain.get_domain(), repository); },
            nb::arg("domain"),
            nb::arg("repository"),
            nb::keep_alive<0, 2>())
        .def_static(
            "create_blocks3ops_france_et_al_aaai2021",
            [](tyr::formalism::planning::PlanningDomain domain, runir::kr::ps::Repository& repository)
            { return SketchFactory::create_blocks3ops_france_et_al_aaai2021(domain.get_domain(), repository); },
            nb::arg("domain"),
            nb::arg("repository"),
            nb::keep_alive<0, 2>())
        .def_static(
            "create_spanner_france_et_al_aaai2021",
            [](tyr::formalism::planning::PlanningDomain domain, runir::kr::ps::Repository& repository)
            { return SketchFactory::create_spanner_france_et_al_aaai2021(domain.get_domain(), repository); },
            nb::arg("domain"),
            nb::arg("repository"),
            nb::keep_alive<0, 2>())
        .def_static(
            "create_delivery_france_et_al_aaai2021",
            [](tyr::formalism::planning::PlanningDomain domain, runir::kr::ps::Repository& repository)
            { return SketchFactory::create_delivery_france_et_al_aaai2021(domain.get_domain(), repository); },
            nb::arg("domain"),
            nb::arg("repository"),
            nb::keep_alive<0, 2>())
        .def_static("create_gripper_france_et_al_aaai2021_description", &SketchFactory::create_gripper_france_et_al_aaai2021_description)
        .def_static("create_blocks3ops_france_et_al_aaai2021_description", &SketchFactory::create_blocks3ops_france_et_al_aaai2021_description)
        .def_static("create_spanner_france_et_al_aaai2021_description", &SketchFactory::create_spanner_france_et_al_aaai2021_description)
        .def_static("create_delivery_france_et_al_aaai2021_description", &SketchFactory::create_delivery_france_et_al_aaai2021_description);
}

}  // namespace runir::kr::ps::base::dl
