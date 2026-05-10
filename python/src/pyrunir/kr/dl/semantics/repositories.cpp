#include "module.hpp"

#include <memory>
#include <runir/knowledge_representation/dl/repository.hpp>

namespace runir::kr::dl
{

using namespace nanobind::literals;

void bind_semantics_repositories(nb::module_& m)
{
    auto cls = nb::class_<runir::kr::dl::ConstructorRepository>(m, "ConstructorRepository");
    cls.def(nb::new_([](size_t index) { return new runir::kr::dl::ConstructorRepository(index); }), "index"_a = 0);
    cls.def("clear", &runir::kr::dl::ConstructorRepository::clear).def("get_index", &runir::kr::dl::ConstructorRepository::get_index);
}

}  // namespace runir::kr::dl
