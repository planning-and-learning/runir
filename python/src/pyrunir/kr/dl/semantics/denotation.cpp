#include "bindings.hpp"
#include "pyrunir/module.hpp"

#include <concepts>
#include <cstddef>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/vector.h>
#include <optional>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/denotation_view.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <tyr/formalism/object_view.hpp>
#include <urd/python/bindings.hpp>
#include <urd/python/owner.hpp>
#include <vector>

namespace runir::kr::dl
{
namespace
{

/// The view of the denotation of Category (its record is bound first, bind_data); a concept's or role's denotation also gives its objects (or
/// object pairs) as the views it holds, and a boolean's or numerical's its value.
template<CategoryTag Category>
void bind_denotation_view(nb::module_& m)
{
    using View = urd::index_view<runir::kr::dl::semantics::Denotation<Category>, runir::kr::dl::semantics::DenotationRepository>;
    auto cls = urd::python::bind_view<View>(m);
    if constexpr (ConceptOrRoleTag<Category>)
        cls.def("views", &View::views);
}

}  // namespace

void bind_semantics_denotation(nb::module_& m)
{
    urd::python::bind_data<semantics::Denotation<BooleanTag>>(m);
    urd::python::bind_data<semantics::Denotation<NumericalTag>>(m);
    urd::python::bind_data<semantics::Denotation<ConceptTag>>(m);
    urd::python::bind_data<semantics::Denotation<RoleTag>>(m);
    bind_denotation_view<BooleanTag>(m);
    bind_denotation_view<NumericalTag>(m);
    bind_denotation_view<ConceptTag>(m);
    bind_denotation_view<RoleTag>(m);
}

}  // namespace runir::kr::dl
