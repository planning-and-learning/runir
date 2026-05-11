#include "module.hpp"

#include <memory>
#include <nanobind/stl/shared_ptr.h>
#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>

namespace runir::kr::dl
{

namespace cnf = runir::kr::dl::cnf_grammar;
namespace fp = tyr::formalism::planning;

using namespace nanobind::literals;

void bind_cnf_grammar_constructor_repository(nb::module_& m)
{
    auto repository = nb::class_<cnf::ConstructorRepository>(m, "ConstructorRepository");
    repository.def("clear", &cnf::ConstructorRepository::clear).def("get_index", &cnf::ConstructorRepository::get_index);

    auto factory = nb::class_<cnf::ConstructorRepositoryFactory>(m, "ConstructorRepositoryFactory");
    factory.def(nb::init<>())
        .def(
            "create",
            [](cnf::ConstructorRepositoryFactory& self, fp::PlanningDomain planning_domain) { return self.create_shared(planning_domain.get_repository()); },
            "planning_domain"_a);
}

}  // namespace runir::kr::dl
