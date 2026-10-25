#include "bindings.hpp"

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
#include <yggdrasil/containers/span.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/owner.hpp>

namespace runir::kr::dl
{
namespace
{

template<CategoryTag Category>
void bind_denotation_view(nb::module_& m, const char* name)
{
    using Type = runir::kr::dl::semantics::Denotation<Category>;
    using View = ygg::View<ygg::Index<Type>, runir::kr::dl::semantics::DenotationRepository>;
    const auto retainer = ygg::python::make_owner_retainer();

    auto cls = nb::class_<View>(m, name).def("get_index", &View::get_index);
    ygg::add_print(cls);
    ygg::add_comparison(cls);
    ygg::add_hash(cls);

    if constexpr (BooleanOrNumericalTag<Category>)
        cls.def("get", [](View view) { return view.get(); });

    if constexpr (std::same_as<Category, ConceptTag>)
        cls.def("__iter__",
                [retainer](nb::typed<nb::handle, View> owner)
                {
                    const auto& view = nb::cast<const View&>(owner);
                    // Transfer the iterator reference instead of slicing the typed iterator into an object.
                    auto iterator = nb::make_iterator(nb::type<View>(), "ConceptDenotationIterator", view.begin(), view.end());
                    return nb::borrow<nb::typed<nb::iterator, semantics::DenotationElementView<Category>>>(
                        ygg::python::make_iterator_with_owner(nb::steal(iterator.release()), owner, retainer));
                });

    if constexpr (std::same_as<Category, RoleTag>)
        cls.def("__iter__",
                [retainer](nb::typed<nb::handle, View> owner)
                {
                    const auto& view = nb::cast<const View&>(owner);
                    // Transfer the iterator reference instead of slicing the typed iterator into an object.
                    auto iterator = nb::make_iterator(nb::type<View>(), "RoleDenotationIterator", view.begin(), view.end());
                    return nb::borrow<nb::typed<nb::iterator, semantics::DenotationElementView<Category>>>(
                        ygg::python::make_iterator_with_owner(nb::steal(iterator.release()), owner, retainer));
                });
}

}  // namespace

void bind_semantics_denotation(nb::module_& m)
{
    ygg::bind_index<ygg::Index<runir::kr::dl::semantics::Denotation<BooleanTag>>>(m, "BooleanDenotationIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::semantics::Denotation<NumericalTag>>>(m, "NumericalDenotationIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::semantics::Denotation<ConceptTag>>>(m, "ConceptDenotationIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::semantics::Denotation<RoleTag>>>(m, "RoleDenotationIndex");

    bind_denotation_view<BooleanTag>(m, "BooleanDenotation");
    bind_denotation_view<NumericalTag>(m, "NumericalDenotation");
    bind_denotation_view<ConceptTag>(m, "ConceptDenotation");
    bind_denotation_view<RoleTag>(m, "RoleDenotation");
}

}  // namespace runir::kr::dl
