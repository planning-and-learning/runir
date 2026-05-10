#include "module.hpp"

#include <memory>
#include <runir/knowledge_representation/dl/cnf_grammar/constructor_repository.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>

namespace runir::kr::dl
{

namespace cnf = runir::kr::dl::cnf_grammar;
namespace fp = tyr::formalism::planning;

using namespace nanobind::literals;

void bind_cnf_grammar_constructor_repository(nb::module_& m)
{
    auto repository = nb::class_<cnf::ConstructorRepository>(m, "ConstructorRepository");
    repository.def(
        nb::new_([](size_t index, fp::PlanningDomain planning_domain) { return new cnf::ConstructorRepository(index, planning_domain.get_repository()); }),
        "index"_a,
        "planning_domain"_a);
    repository.def("clear", &cnf::ConstructorRepository::clear).def("get_index", &cnf::ConstructorRepository::get_index);
}

}  // namespace runir::kr::dl
