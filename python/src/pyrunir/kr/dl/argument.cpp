#include "bindings.hpp"
#include "pyrunir/module.hpp"

#include <runir/kr/dl/argument_data.hpp>
#include <runir/kr/dl/argument_view.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <urd/python/bindings.hpp>

namespace runir::kr::dl
{

void bind_argument(nb::module_& m)
{
    urd::python::bind_index<ArgumentIdentifier<ConceptTag>>(m, "ConceptArgumentIdentifier");
    urd::python::bind_index<ArgumentIdentifier<RoleTag>>(m, "RoleArgumentIdentifier");
    urd::python::bind_index<ArgumentIdentifier<BooleanTag>>(m, "BooleanArgumentIdentifier");
    urd::python::bind_index<ArgumentIdentifier<NumericalTag>>(m, "NumericalArgumentIdentifier");
    urd::python::bind_data<Argument<ConceptTag>>(m);
    urd::python::bind_data<Argument<RoleTag>>(m);
    urd::python::bind_data<Argument<BooleanTag>>(m);
    urd::python::bind_data<Argument<NumericalTag>>(m);
    urd::python::bind_view<urd::index_view<Argument<ConceptTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Argument<RoleTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Argument<BooleanTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Argument<NumericalTag>, ConstructorRepository>>(m);
}

}  // namespace runir::kr::dl
