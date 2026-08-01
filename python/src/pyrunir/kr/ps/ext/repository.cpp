#include "bindings.hpp"

#include <memory>
#include <nanobind/stl/shared_ptr.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/ps/ext/repository.hpp>

namespace runir::kr::ps::ext
{

using namespace nanobind::literals;

void bind_repository(nb::module_& m, RepositoryBinding& repository)
{
    repository.def("clear", &Repository::clear)
        .def("get_index", &Repository::get_index)
        .def("get_dl_repository", [](Repository& self) -> auto& { return self.get_dl_repository(); }, nb::rv_policy::reference_internal);

    auto factory = nb::class_<RepositoryFactory>(m, "RepositoryFactory");
    factory.def(nb::init<>()).def("create", &RepositoryFactory::create, "dl_repository"_a);
}

}  // namespace runir::kr::ps::ext
