#include "bindings.hpp"
#include "pyrunir/module.hpp"

#include <memory>
#include <nanobind/stl/shared_ptr.h>
#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>

namespace runir::kr::dl
{

void bind_grammar_constructor_repository(nb::module_& m)
{
    auto repository = nb::class_<runir::kr::dl::grammar::ConstructorRepository>(m, "ConstructorRepository");
    repository.def("clear", &runir::kr::dl::grammar::ConstructorRepository::clear).def("layer", &runir::kr::dl::grammar::ConstructorRepository::layer);

    repository.def(nb::new_([](const tyr::formalism::PlanningDomain& planning_domain)
                            { return std::make_shared<runir::kr::dl::grammar::ConstructorRepository>(planning_domain.get_repository()); }),
                   nb::arg("planning_domain"));
}

}  // namespace runir::kr::dl
