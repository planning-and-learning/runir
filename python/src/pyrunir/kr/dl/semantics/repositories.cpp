#include "module.hpp"

#include <memory>
#include <nanobind/stl/shared_ptr.h>
#include <runir/kr/dl/repository.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>

namespace runir::kr::dl
{

namespace fp = tyr::formalism::planning;

using namespace nanobind::literals;

void bind_semantics_repositories(nb::module_& m)
{
    auto cls = nb::class_<runir::kr::dl::ConstructorRepository>(m, "ConstructorRepository");
    cls.def("clear", &runir::kr::dl::ConstructorRepository::clear).def("get_index", &runir::kr::dl::ConstructorRepository::get_index);

    auto factory = nb::class_<runir::kr::dl::ConstructorRepositoryFactory>(m, "ConstructorRepositoryFactory");
    factory.def(nb::init<>())
        .def(
            "create",
            [](runir::kr::dl::ConstructorRepositoryFactory& self, fp::PlanningDomain planning_domain)
            { return self.create_shared(planning_domain.get_repository()); },
            "planning_domain"_a);
}

}  // namespace runir::kr::dl
