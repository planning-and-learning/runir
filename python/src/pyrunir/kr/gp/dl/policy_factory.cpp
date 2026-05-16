#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/gp/dl/policy_factory.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>

namespace runir::kr::gp::dl
{

void bind_policy_factory(nb::module_& m)
{
    nb::enum_<PolicySpecification>(m, "PolicySpecification")
        .value("GRIPPER_FRANCE_ET_AL_AAAI2021", PolicySpecification::GRIPPER_FRANCE_ET_AL_AAAI2021)
        .value("BLOCKS3OPS_FRANCE_ET_AL_AAAI2021", PolicySpecification::BLOCKS3OPS_FRANCE_ET_AL_AAAI2021)
        .value("SPANNER_FRANCE_ET_AL_AAAI2021", PolicySpecification::SPANNER_FRANCE_ET_AL_AAAI2021)
        .value("DELIVERY_FRANCE_ET_AL_AAAI2021", PolicySpecification::DELIVERY_FRANCE_ET_AL_AAAI2021);

    nb::class_<PolicyFactory>(m, "PolicyFactory")
        .def_static("create_empty", &PolicyFactory::create_empty, nb::arg("repository"), nb::keep_alive<0, 1>())
        .def_static("create_empty_description", &PolicyFactory::create_empty_description)
        .def_static(
            "create",
            [](PolicySpecification specification, tyr::formalism::planning::PlanningDomain domain, runir::kr::gp::Repository& repository)
            { return PolicyFactory::create(specification, domain.get_domain(), repository); },
            nb::arg("specification"),
            nb::arg("domain"),
            nb::arg("repository"),
            nb::keep_alive<0, 3>())
        .def_static("create_description", &PolicyFactory::create_description, nb::arg("specification"))
        .def_static(
            "create_gripper_france_et_al_aaai2021",
            [](tyr::formalism::planning::PlanningDomain domain, runir::kr::gp::Repository& repository)
            { return PolicyFactory::create_gripper_france_et_al_aaai2021(domain.get_domain(), repository); },
            nb::arg("domain"),
            nb::arg("repository"),
            nb::keep_alive<0, 2>())
        .def_static(
            "create_blocks3ops_france_et_al_aaai2021",
            [](tyr::formalism::planning::PlanningDomain domain, runir::kr::gp::Repository& repository)
            { return PolicyFactory::create_blocks3ops_france_et_al_aaai2021(domain.get_domain(), repository); },
            nb::arg("domain"),
            nb::arg("repository"),
            nb::keep_alive<0, 2>())
        .def_static(
            "create_spanner_france_et_al_aaai2021",
            [](tyr::formalism::planning::PlanningDomain domain, runir::kr::gp::Repository& repository)
            { return PolicyFactory::create_spanner_france_et_al_aaai2021(domain.get_domain(), repository); },
            nb::arg("domain"),
            nb::arg("repository"),
            nb::keep_alive<0, 2>())
        .def_static(
            "create_delivery_france_et_al_aaai2021",
            [](tyr::formalism::planning::PlanningDomain domain, runir::kr::gp::Repository& repository)
            { return PolicyFactory::create_delivery_france_et_al_aaai2021(domain.get_domain(), repository); },
            nb::arg("domain"),
            nb::arg("repository"),
            nb::keep_alive<0, 2>())
        .def_static("create_gripper_france_et_al_aaai2021_description", &PolicyFactory::create_gripper_france_et_al_aaai2021_description)
        .def_static("create_blocks3ops_france_et_al_aaai2021_description", &PolicyFactory::create_blocks3ops_france_et_al_aaai2021_description)
        .def_static("create_spanner_france_et_al_aaai2021_description", &PolicyFactory::create_spanner_france_et_al_aaai2021_description)
        .def_static("create_delivery_france_et_al_aaai2021_description", &PolicyFactory::create_delivery_france_et_al_aaai2021_description);
}

}  // namespace runir::kr::gp::dl
