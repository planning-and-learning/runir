#include "module.hpp"

#include <memory>
#include <runir/kr/dl/repository.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>

namespace runir::kr::dl
{

namespace fp = tyr::formalism::planning;

using namespace nanobind::literals;

void bind_semantics_repositories(nb::module_& m)
{
    auto cls = nb::class_<runir::kr::dl::ConstructorRepository>(m, "ConstructorRepository");
    cls.def(nb::new_([](size_t index, fp::PlanningDomain planning_domain) { return new runir::kr::dl::ConstructorRepository(index, planning_domain.get_repository()); }),
            "index"_a,
            "planning_domain"_a);
    cls.def("clear", &runir::kr::dl::ConstructorRepository::clear).def("get_index", &runir::kr::dl::ConstructorRepository::get_index);
}

}  // namespace runir::kr::dl
