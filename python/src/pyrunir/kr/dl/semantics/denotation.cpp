#include "bindings.hpp"
#include "pyrunir/module.hpp"

#include <concepts>
#include <cstddef>
#include <nanobind/make_iterator.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/vector.h>
#include <optional>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/denotation_view.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <tyr/formalism/object_view.hpp>
#include <vector>
#include <yggdrasil/python/bindings.hpp>
#include <urd/python/owner.hpp>

namespace runir::kr::dl
{
namespace
{

template<CategoryTag Category>
void bind_denotation_view(nb::module_& m, const char* name)
{
    using Type = runir::kr::dl::semantics::Denotation<Category>;
    using View = urd::index_view<Type, runir::kr::dl::semantics::DenotationRepository>;

    auto cls = nb::class_<View>(m, name).def("get_index", &View::index);
    ygg::add_print(cls);
    urd::python::add_comparison(cls);
    urd::python::add_hash(cls);

    if constexpr (BooleanOrNumericalTag<Category>)
        cls.def("get", [](View view) { return view.value(); });

    if constexpr (std::same_as<Category, ConceptTag>)
        cls.def("__iter__",
                [](const View& view) { return nb::make_iterator(nb::type<View>(), "ConceptDenotationIterator", view.begin(), view.end()); },
                nb::keep_alive<0, 1>());

    if constexpr (std::same_as<Category, RoleTag>)
        cls.def("__iter__",
                [](const View& view) { return nb::make_iterator(nb::type<View>(), "RoleDenotationIterator", view.begin(), view.end()); },
                nb::keep_alive<0, 1>());
}

}  // namespace

void bind_semantics_denotation(nb::module_& m)
{
    urd::python::bind_index<urd::index<runir::kr::dl::semantics::Denotation<BooleanTag>>>(m, "BooleanDenotationIndex");
    urd::python::bind_index<urd::index<runir::kr::dl::semantics::Denotation<NumericalTag>>>(m, "NumericalDenotationIndex");
    urd::python::bind_index<urd::index<runir::kr::dl::semantics::Denotation<ConceptTag>>>(m, "ConceptDenotationIndex");
    urd::python::bind_index<urd::index<runir::kr::dl::semantics::Denotation<RoleTag>>>(m, "RoleDenotationIndex");

    bind_denotation_view<BooleanTag>(m, "BooleanDenotation");
    bind_denotation_view<NumericalTag>(m, "NumericalDenotation");
    bind_denotation_view<ConceptTag>(m, "ConceptDenotation");
    bind_denotation_view<RoleTag>(m, "RoleDenotation");
}

}  // namespace runir::kr::dl
