#include "module.hpp"

#include <memory>
#include <nanobind/stl/shared_ptr.h>
#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>

namespace runir::kr::dl::base
{

void bind_cnf_grammar_constructor_repository(nb::module_& m)
{
    auto repository = nb::class_<runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "ConstructorRepository");
    repository.def("clear", &runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>::clear)
        .def("get_index", &runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>::get_index);

    auto factory = nb::class_<runir::kr::dl::cnf_grammar::ConstructorRepositoryFactoryFor<runir::kr::BaseFamilyTag>>(m, "ConstructorRepositoryFactory");
    factory.def(nb::init<>())
        .def(
            "create",
            [](runir::kr::dl::cnf_grammar::ConstructorRepositoryFactoryFor<runir::kr::BaseFamilyTag>& self,
               tyr::formalism::planning::PlanningDomain planning_domain) { return self.create(planning_domain.get_repository()); },
            nb::arg("planning_domain"));
}

}  // namespace runir::kr::dl::base
