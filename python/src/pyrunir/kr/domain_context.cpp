#include "module.hpp"

#include <nanobind/stl/shared_ptr.h>
#include <runir/kr/domain_context.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/uns/repository.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>

namespace runir::kr
{

using namespace nanobind::literals;

void bind_domain_context(nb::module_& m)
{
    nb::class_<DomainContext>(m, "DomainContext")
        .def(nb::new_(&DomainContext::create), "planning_domain"_a)
        .def_prop_ro(
            "base_repository",
            [](DomainContext& self) -> auto& { return *self.base_repository; },
            nb::rv_policy::reference_internal)
        .def_prop_ro(
            "ext_repository",
            [](DomainContext& self) -> auto& { return *self.ext_repository; },
            nb::rv_policy::reference_internal)
        .def_prop_ro("uns_repository", [](DomainContext& self) -> auto& { return *self.uns_repository; }, nb::rv_policy::reference_internal);
}

}  // namespace runir::kr
