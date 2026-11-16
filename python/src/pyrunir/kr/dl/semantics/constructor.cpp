#include "bindings.hpp"
#include "pyrunir/module.hpp"
#include "runir/kr/dl/detail/repository_instantiations.hpp"

#include <nanobind/stl/variant.h>
#include <pyrunir/kr/dl/evaluation_bindings.hpp>
#include <pyrunir/kr/dl/optimization_bindings.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/constructor_view.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <tyr/datalog/lifted/database/syntax/query_repository.hpp>
#include <urd/python/bindings.hpp>

namespace runir::kr::dl
{
namespace
{

/// The constructor of Category: bind_data, bind_view, and evaluate, optimize and syntactic_complexity of it.
template<CategoryTag Category>
void bind_constructor_view(nb::module_& m)
{
    using View = urd::index_view<Constructor<Category>, ConstructorRepository>;
    urd::python::bind_view<View>(m);
    runir::kr::python::bind_evaluate<View>(m);
    runir::kr::python::bind_optimize<View>(m);
    m.def("syntactic_complexity", [](View constructor) { return semantics::syntactic_complexity(constructor); }, nb::arg("constructor"));
}

}  // namespace

void bind_semantics_constructor(nb::module_& m)
{
    urd::python::bind_data<Constructor<ConceptTag>>(m);
    urd::python::bind_data<Constructor<RoleTag>>(m);
    urd::python::bind_data<Constructor<BooleanTag>>(m);
    urd::python::bind_data<Constructor<NumericalTag>>(m);

    bind_constructor_view<ConceptTag>(m);
    bind_constructor_view<RoleTag>(m);
    bind_constructor_view<BooleanTag>(m);
    bind_constructor_view<NumericalTag>(m);
}

}  // namespace runir::kr::dl
