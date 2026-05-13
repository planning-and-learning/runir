#include "module.hpp"

#include <memory>
#include <nanobind/stl/shared_ptr.h>
#include <runir/kr/gp/repository.hpp>

namespace runir::kr::gp
{

using namespace nanobind::literals;

void bind_repository(nb::module_& m)
{
    auto repository = nb::class_<Repository>(m, "Repository");
    repository.def("clear", &Repository::clear)
        .def("get_index", &Repository::get_index)
        .def("get_dl_repository", [](Repository& self) -> runir::kr::dl::ConstructorRepository& { return self.get_dl_repository(); });

    auto factory = nb::class_<RepositoryFactory>(m, "RepositoryFactory");
    factory.def(nb::init<>()).def("create", &RepositoryFactory::create_shared, "dl_repository"_a);
}

}  // namespace runir::kr::gp
