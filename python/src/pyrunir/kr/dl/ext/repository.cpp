#include "module.hpp"

#include <memory>
#include <nanobind/stl/shared_ptr.h>
#include <runir/kr/dl/ext/repository.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>
#include <tyr/planning/ground_task.hpp>
#include <tyr/planning/lifted_task.hpp>

namespace runir::kr::dl::ext
{

void bind_repository(nb::module_& m)
{
    auto repository = nb::class_<ConstructorRepository>(m, "ConstructorRepository");
    repository.def("clear", &ConstructorRepository::clear).def("get_index", &ConstructorRepository::get_index);

    auto factory = nb::class_<ConstructorRepositoryFactory>(m, "ConstructorRepositoryFactory");
    factory.def(nb::init<>())
        .def(
            "create",
            [](ConstructorRepositoryFactory& self, tyr::formalism::planning::PlanningDomain planning_domain)
            { return self.create(planning_domain.get_repository()); },
            nb::arg("planning_domain"))
        .def(
            "create",
            [](ConstructorRepositoryFactory& self, const tyr::planning::Task<tyr::planning::GroundTag>& task)
            { return self.create(task.get_repository()); },
            nb::arg("ground_task"))
        .def(
            "create",
            [](ConstructorRepositoryFactory& self, const tyr::planning::Task<tyr::planning::LiftedTag>& task)
            { return self.create(task.get_repository()); },
            nb::arg("lifted_task"));
}

}  // namespace runir::kr::dl::ext
