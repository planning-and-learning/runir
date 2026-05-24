#include "module.hpp"

#include <memory>
#include <nanobind/stl/shared_ptr.h>
#include <runir/kr/dl/grammar/base/repository.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>

namespace runir::kr::dl
{

void bind_grammar_constructor_repository(nb::module_& m)
{
    auto repository = nb::class_<runir::kr::dl::grammar::base::ConstructorRepository>(m, "ConstructorRepository");
    repository.def("clear", &runir::kr::dl::grammar::base::ConstructorRepository::clear)
        .def("get_index", &runir::kr::dl::grammar::base::ConstructorRepository::get_index);

    auto factory = nb::class_<runir::kr::dl::grammar::base::ConstructorRepositoryFactory>(m, "ConstructorRepositoryFactory");
    factory.def(nb::init<>())
        .def(
            "create",
            [](runir::kr::dl::grammar::base::ConstructorRepositoryFactory& self, tyr::formalism::planning::PlanningDomain planning_domain)
            { return self.create_shared(planning_domain.get_repository()); },
            nb::arg("planning_domain"));
}

}  // namespace runir::kr::dl
