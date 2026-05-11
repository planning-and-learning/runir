#include "module.hpp"

#include <memory>
#include <nanobind/stl/shared_ptr.h>
#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>

namespace runir::kr::dl
{

namespace grammar = runir::kr::dl::grammar;
namespace fp = tyr::formalism::planning;

using namespace nanobind::literals;

void bind_grammar_constructor_repository(nb::module_& m)
{
    auto repository = nb::class_<grammar::ConstructorRepository>(m, "ConstructorRepository");
    repository.def("clear", &grammar::ConstructorRepository::clear).def("get_index", &grammar::ConstructorRepository::get_index);

    auto factory = nb::class_<grammar::ConstructorRepositoryFactory>(m, "ConstructorRepositoryFactory");
    factory.def(nb::init<>())
        .def(
            "create",
            [](grammar::ConstructorRepositoryFactory& self, fp::PlanningDomain planning_domain)
            { return self.create_shared(planning_domain.get_repository()); },
            "planning_domain"_a);
}

}  // namespace runir::kr::dl
