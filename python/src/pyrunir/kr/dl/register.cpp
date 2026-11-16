#include "bindings.hpp"
#include "pyrunir/module.hpp"

#include <runir/kr/dl/register_data.hpp>
#include <runir/kr/dl/register_view.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <urd/python/bindings.hpp>

namespace runir::kr::dl
{

void bind_register(nb::module_& m)
{
    urd::python::bind_index<RegisterIdentifier<ConceptTag>>(m, "ConceptRegisterIdentifier");
    urd::python::bind_index<RegisterIdentifier<RoleTag>>(m, "RoleRegisterIdentifier");
    urd::python::bind_data<Register<ConceptTag>>(m);
    urd::python::bind_data<Register<RoleTag>>(m);
    urd::python::bind_view<urd::index_view<Register<ConceptTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Register<RoleTag>, ConstructorRepository>>(m);
}

}  // namespace runir::kr::dl
