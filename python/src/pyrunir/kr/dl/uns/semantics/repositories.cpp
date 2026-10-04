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

namespace runir::kr::dl::uns
{

namespace
{

template<typename... Types>
void bind_constructors(nb::class_<UnsConstructorRepository>& repository, ygg::TypeList<Types...>)
{
    (repository.def(
         "get_or_create",
         [](UnsConstructorRepository& self, ygg::Data<Types>& data) { return runir::kr::dl::get_or_create(self, data).first; },
         nb::arg("data"),
         nb::keep_alive<0, 1>()),
     ...);
}

}  // namespace

void bind_semantics_repositories(nb::module_& m)
{
    auto repository = nb::class_<runir::kr::dl::UnsConstructorRepository>(m, "ConstructorRepository");
    repository.def("clear", &runir::kr::dl::UnsConstructorRepository::clear).def("get_index", &runir::kr::dl::UnsConstructorRepository::get_index);
    bind_constructors(repository, FamilyConstructorRepositoryTypes<runir::kr::UnsFamilyTag> {});

    auto factory = nb::class_<runir::kr::dl::UnsConstructorRepositoryFactory>(m, "ConstructorRepositoryFactory");
    factory.def(nb::init<>())
        .def(
            "create",
            [](runir::kr::dl::UnsConstructorRepositoryFactory& self, tyr::formalism::planning::PlanningDomain planning_domain)
            { return self.create(planning_domain.get_repository()); },
            nb::arg("planning_domain"))
        .def(
            "create",
            [](runir::kr::dl::UnsConstructorRepositoryFactory& self, const tyr::planning::Task<tyr::GroundTag>& task)
            { return self.create(task.get_repository()); },
            nb::arg("ground_task"))
        .def(
            "create",
            [](runir::kr::dl::UnsConstructorRepositoryFactory& self, const tyr::planning::Task<tyr::LiftedTag>& task)
            { return self.create(task.get_repository()); },
            nb::arg("lifted_task"));

    runir::kr::python::bind_evaluation_storage<runir::kr::UnsFamilyTag>(m);

    runir::kr::python::bind_state_evaluation_context<runir::kr::UnsFamilyTag, tyr::GroundTag>(m, "GroundStateEvaluationContext");
    runir::kr::python::bind_state_evaluation_context<runir::kr::UnsFamilyTag, tyr::LiftedTag>(m, "LiftedStateEvaluationContext");
}

}  // namespace runir::kr::dl::uns
