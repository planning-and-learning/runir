#include "bindings.hpp"
#include "pyrunir/module.hpp"

#include <memory>
#include <nanobind/stl/shared_ptr.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/uns/repository.hpp>

namespace runir::kr::uns
{

void bind_repository(nb::module_& /*m*/, RepositoryBinding& repository)
{
    using namespace nb::literals;
    repository
        .def(nb::new_([](runir::kr::dl::ConstructorRepositoryPtr dl_repository) { return std::make_shared<Repository>(std::move(dl_repository)); }),
             "dl_repository"_a)
        .def("clear", &Repository::clear)
        .def("layer", &Repository::layer)
        .def(
            "get_dl_repository",
            [](Repository& self) -> auto& { return self.get<runir::kr::dl::ConstructorRepository>(); },
            nb::rv_policy::reference_internal);
}

}  // namespace runir::kr::uns
