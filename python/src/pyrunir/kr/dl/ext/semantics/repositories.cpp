#include "bindings.hpp"

#include <memory>
#include <nanobind/stl/shared_ptr.h>
#include <pyrunir/kr/dl/evaluation_bindings.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>
#include <tyr/planning/ground/task.hpp>
#include <tyr/planning/lifted/task.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::dl::ext
{

namespace
{

template<typename... Types>
void bind_constructors(nb::class_<ExtConstructorRepository>& repository, ygg::TypeList<Types...>)
{
    (repository.def(
         "get_or_create",
         [](ExtConstructorRepository& self, ygg::Data<Types>& data) { return runir::kr::dl::get_or_create(self, data).first; },
         nb::arg("data"),
         nb::keep_alive<0, 1>()),
     ...);
}

}  // namespace

void bind_semantics_repositories(nb::module_& m)
{
    runir::kr::python::bind_evaluation_storage<runir::kr::ExtFamilyTag>(m);

    // Builder and the DenotationRepository[Factory] are family-independent and registered once by
    // the base semantics module.
    runir::kr::python::bind_state_evaluation_context<runir::kr::ExtFamilyTag, tyr::GroundTag>(m, "GroundStateEvaluationContext");
    runir::kr::python::bind_state_evaluation_context<runir::kr::ExtFamilyTag, tyr::LiftedTag>(m, "LiftedStateEvaluationContext");

    auto repository = nb::class_<runir::kr::dl::ExtConstructorRepository>(m, "ConstructorRepository");
    repository.def("clear", &runir::kr::dl::ExtConstructorRepository::clear).def("get_index", &runir::kr::dl::ExtConstructorRepository::get_index);
    bind_constructors(repository, FamilyConstructorRepositoryTypes<runir::kr::ExtFamilyTag> {});

    auto factory = nb::class_<runir::kr::dl::ExtConstructorRepositoryFactory>(m, "ConstructorRepositoryFactory");
    factory.def(nb::init<>())
        .def(
            "create",
            [](runir::kr::dl::ExtConstructorRepositoryFactory& self, tyr::formalism::planning::PlanningDomain planning_domain)
            { return self.create(planning_domain.get_repository()); },
            nb::arg("planning_domain"))
        .def(
            "create",
            [](runir::kr::dl::ExtConstructorRepositoryFactory& self, const tyr::planning::Task<tyr::GroundTag>& task)
            { return self.create(task.get_repository()); },
            nb::arg("ground_task"))
        .def(
            "create",
            [](runir::kr::dl::ExtConstructorRepositoryFactory& self, const tyr::planning::Task<tyr::LiftedTag>& task)
            { return self.create(task.get_repository()); },
            nb::arg("lifted_task"));
}

}  // namespace runir::kr::dl::ext
