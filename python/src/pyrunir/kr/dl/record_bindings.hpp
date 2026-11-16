#ifndef PYRUNIR_KR_DL_RECORD_BINDINGS_HPP_
#define PYRUNIR_KR_DL_RECORD_BINDINGS_HPP_

#include "module.hpp"
#include "pyrunir/module.hpp"

#include <boost/mp11.hpp>
#include <nanobind/stl/variant.h>
#include <nanobind/stl/vector.h>
#include <runir/kr/dl/formatter.hpp>
#include <runir/kr/dl/record_view.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <urd/python/bindings.hpp>
#include <urd/python/sequences/sequence.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::dl::python
{

// The views of the description-logic constructor records (Concept/Role/Boolean/Numerical<F, Tag>) of one family F, bound
// in the family's module by bind_view, under their names (urd::name): ConceptSemanticBot, ConceptGrammarBot, ...

/// A view whose record's syntactic complexity is defined (the semantic family's).
template<typename View>
concept MeasuredView = requires(const View& view) { semantics::syntactic_complexity(view); };

/// The view of a record: bind_view, and syntactic_complexity(expression) where it is defined.
template<typename View>
void bind_record_view(nb::module_& m)
{
    urd::python::bind_view<View>(m);
    if constexpr (MeasuredView<View>)
        m.def("syntactic_complexity", [](View expression) { return semantics::syntactic_complexity(expression); }, nb::arg("expression"));
}

/// The views of the records of Category in family F (each record is bound first, bind_data, in the family's source,
/// where each call names its record).
template<typename F, typename Repository, CategoryTag Category>
void bind_record_views(nb::module_& m, Category /*category*/)
{
    boost::mp11::mp_for_each<boost::mp11::mp_transform<boost::mp11::mp_identity, RecordTypes<F, Category>>>(
        [&]<typename T>(boost::mp11::mp_identity<T> /*record*/) { bind_record_view<urd::index_view<T, Repository>>(m); });
}

}  // namespace runir::kr::dl::python

#endif
