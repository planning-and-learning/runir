#include "pyrunir/kr/ps/base/module.hpp"

#include <memory>
#include <nanobind/stl/shared_ptr.h>
#include <runir/kr/ps/base/repository.hpp>

namespace runir::kr::ps::base
{

using namespace nanobind::literals;

void bind_repository(nb::module_& m)
{
    auto repository = nb::class_<Repository>(m, "Repository");
    repository.def("clear", &Repository::clear)
        .def("get_index", &Repository::get_index)
        .def("get_dl_repository", nb::overload_cast<>(&Repository::get_dl_repository), nb::rv_policy::reference_internal);

    auto factory = nb::class_<RepositoryFactory>(m, "RepositoryFactory");
    factory.def(nb::init<>()).def("create", &RepositoryFactory::create, "dl_repository"_a);
}

}  // namespace runir::kr::ps::base
