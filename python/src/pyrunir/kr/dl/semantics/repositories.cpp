#include "bindings.hpp"
#include "pyrunir/conversions.hpp"
#include "pyrunir/module.hpp"

#include <memory>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/vector.h>
#include <optional>
#include <pyrunir/kr/dl/evaluation_bindings.hpp>
#include <runir/kr/database/syntax/query_repository.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/builder.hpp>
#include <runir/kr/dl/semantics/call_arguments_view.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/register_values_view.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>
#include <tyr/planning/ground/task.hpp>
#include <tyr/planning/lifted/task.hpp>
#include <urd/python/bindings.hpp>
#include <urd/python/sequences/sequence.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::dl
{

namespace
{

template<typename... Types>
void bind_constructors(nb::class_<ConstructorRepository>& repository, boost::mp11::mp_list<Types...>)
{ (ygg::bind_insert<Types>(repository), ...); }

}  // namespace

void bind_semantics_repositories(nb::module_& m)
{
    using DenotationRepository = semantics::DenotationRepository;
    urd::python::bind_index<semantics::CallArguments>(m);
    urd::python::bind_data<semantics::CallArguments>(m);
    urd::python::bind_index<semantics::RegisterValues>(m);
    urd::python::bind_data<semantics::RegisterValues>(m);
    urd::python::bind_view<semantics::RegisterValuesView>(m);
    urd::python::bind_builder<semantics::RegisterValues, semantics::DenotationRepository>(m);
    urd::python::bind_view<semantics::CallArgumentsView>(m);
    urd::python::bind_builder<semantics::CallArguments, semantics::DenotationRepository>(m);

    nb::class_<runir::kr::dl::semantics::Builder>(m, "Builder").def(nb::init<>());

    auto denotations = nb::class_<DenotationRepository>(m, "DenotationRepository")
                           .def(nb::new_([](const tyr::formalism::PlanningDomain& planning_domain)
                                         { return std::make_shared<DenotationRepository>(planning_domain.get_repository()); }),
                                nb::arg("planning_domain"))
                           .def("layer", &DenotationRepository::layer)
                           .def(
                               "get_relation_repository",
                               [](DenotationRepository& self) -> auto& { return self.get_relation_repository(); },
                               nb::rv_policy::reference_internal)
                           .def(
                               "get_object",
                               [](const DenotationRepository& self, urd::index<tyr::formalism::Object> index) -> tyr::formalism::ObjectView
                               { return self.get<tyr::formalism::Repository>().view(index); },
                               nb::arg("index"));
    ygg::bind_insert<semantics::CallArguments>(denotations);
    ygg::bind_insert<semantics::RegisterValues>(denotations);

    runir::kr::python::bind_evaluation_storage(m);
    runir::kr::python::bind_state_evaluation_context<tyr::GroundTag>(m, "GroundStateEvaluationContext");
    runir::kr::python::bind_state_evaluation_context<tyr::LiftedTag>(m, "LiftedStateEvaluationContext");

    auto repository = nb::class_<runir::kr::dl::ConstructorRepository>(m, "ConstructorRepository");
    repository.def("clear", &runir::kr::dl::ConstructorRepository::clear).def("layer", &runir::kr::dl::ConstructorRepository::layer);
    bind_constructors(repository, ConstructorRepositoryTypes {});

    repository
        .def(nb::new_([](const tyr::formalism::PlanningDomain& planning_domain)
                      { return std::make_shared<ConstructorRepository>(planning_domain.get_repository()); }),
             nb::arg("planning_domain"))
        .def(nb::new_([](const tyr::planning::Task<tyr::GroundTag>& task) { return std::make_shared<ConstructorRepository>(task.get_repository()); }),
             nb::arg("ground_task"))
        .def(nb::new_([](const tyr::planning::Task<tyr::LiftedTag>& task) { return std::make_shared<ConstructorRepository>(task.get_repository()); }),
             nb::arg("lifted_task"));
}

}  // namespace runir::kr::dl
