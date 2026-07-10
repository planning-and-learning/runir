#include "module.hpp"

#include <memory>
#include <nanobind/stl/shared_ptr.h>
#include <runir/kr/dl/repository.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>
#include <tyr/planning/ground/task.hpp>
#include <tyr/planning/lifted/task.hpp>

namespace runir::kr::dl::uns
{

void bind_repository(nb::module_& m)
{
    auto repository = nb::class_<runir::kr::dl::UnsConstructorRepository>(m, "ConstructorRepository");
    repository.def("clear", &runir::kr::dl::UnsConstructorRepository::clear).def("get_index", &runir::kr::dl::UnsConstructorRepository::get_index);

    auto factory = nb::class_<runir::kr::dl::UnsConstructorRepositoryFactory>(m, "ConstructorRepositoryFactory");
    factory.def(nb::init<>())
        .def(
            "create",
            [](runir::kr::dl::UnsConstructorRepositoryFactory& self, tyr::formalism::planning::PlanningDomain planning_domain)
            { return self.create(planning_domain.get_repository()); },
            nb::arg("planning_domain"))
        .def(
            "create",
            [](runir::kr::dl::UnsConstructorRepositoryFactory& self, const tyr::planning::Task<tyr::planning::GroundTag>& task)
            { return self.create(task.get_repository()); },
            nb::arg("ground_task"))
        .def(
            "create",
            [](runir::kr::dl::UnsConstructorRepositoryFactory& self, const tyr::planning::Task<tyr::planning::LiftedTag>& task)
            { return self.create(task.get_repository()); },
            nb::arg("lifted_task"));
}

}  // namespace runir::kr::dl::uns
