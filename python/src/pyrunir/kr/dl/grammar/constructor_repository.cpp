#include "module.hpp"

#include <memory>
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
    repository.def(
        nb::new_([](size_t index, fp::PlanningDomain planning_domain) { return new grammar::ConstructorRepository(index, planning_domain.get_repository()); }),
        "index"_a,
        "planning_domain"_a);
    repository.def("clear", &grammar::ConstructorRepository::clear).def("get_index", &grammar::ConstructorRepository::get_index);
}

}  // namespace runir::kr::dl
