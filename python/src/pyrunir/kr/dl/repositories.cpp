#include "dl.hpp"

#include "binding_helpers.hpp"

#include <memory>
#include <runir/knowledge_representation/dl/repository.hpp>

namespace runir::kr::dl
{

using namespace nanobind::literals;

void bind_repositories(nb::module_& m)
{
    auto cls = nb::class_<runir::kr::dl::ConstructorRepository>(m, "ConstructorRepository");
    cls.def(nb::new_([](size_t index) { return std::make_shared<runir::kr::dl::ConstructorRepository>(index); }), "index"_a = 0);
    python_detail::bind_clearable_repository(cls);
}

}  // namespace runir::kr::dl
