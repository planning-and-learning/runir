#include "pyrunir/kr/ps/base/module.hpp"

#include <memory>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/shared_ptr.h>
#include <runir/kr/ps/base/repository.hpp>

namespace runir::kr::ps::base
{

using namespace nanobind::literals;

namespace
{

template<typename... Ts>
void bind_get_or_create(nb::class_<Repository>& repository, ygg::TypeList<Ts...>)
{
    (repository.def("get_or_create", [](Repository& self, ygg::Data<Ts>& data) { return self.get_or_create(data); }, "data"_a), ...);
}

}  // namespace

void bind_repository(nb::module_& m)
{
    auto repository = nb::class_<Repository>(m, "Repository");
    repository.def("clear", &Repository::clear)
        .def("get_index", &Repository::get_index)
        .def("get_dl_repository", [](Repository& self) -> auto& { return self.get_dl_repository(); }, nb::rv_policy::reference_internal);

    bind_get_or_create(repository, RepositoryTypes {});

    auto factory = nb::class_<RepositoryFactory>(m, "RepositoryFactory");
    factory.def(nb::init<>()).def("create", &RepositoryFactory::create, "dl_repository"_a);
}

}  // namespace runir::kr::ps::base
