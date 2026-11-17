#ifndef PYRUNIR_KR_DL_QUERY_BINDINGS_HPP_
#define PYRUNIR_KR_DL_QUERY_BINDINGS_HPP_

#include "module.hpp"
#include "pyrunir/module.hpp"

#include <boost/mp11.hpp>
#include <concepts>
#include <nanobind/stl/variant.h>
#include <nanobind/stl/vector.h>
#include <pyrunir/kr/dl/evaluation_bindings.hpp>
#include <pyrunir/kr/dl/optimization_bindings.hpp>
#include <runir/kr/dl/query_view.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <urd/python/sequences/sequence.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::dl::python
{

/// The view of the query of operation Tag (void: the query of any operation; its record is bound first, bind_data); the
/// query of any operation is also evaluated and optimized.
template<typename Tag>
void bind_query_view(nb::module_& m)
{
    using View = urd::index_view<Query<Tag>, ConstructorRepository>;
    urd::python::bind_view<View>(m);
    if constexpr (std::same_as<Tag, void>)
    {
        runir::kr::python::bind_evaluate<View>(m);
        runir::kr::python::bind_optimize<View>(m);
    }
}

/// The views of the queries, their columns and projections, and the query statistics.
template<typename... Tags>
void bind_query_views(nb::module_& m, boost::mp11::mp_list<Tags...> /*tags*/)
{
    urd::python::bind_view<urd::index_view<QueryColumn, ConstructorRepository>>(m);
    runir::kr::python::bind_query_statistics(m);
    urd::python::bind_view<urd::index_view<QueryProjection<ConceptTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<QueryProjection<RoleTag>, ConstructorRepository>>(m);
    bind_query_view<void>(m);
    (bind_query_view<Tags>(m), ...);
}

}  // namespace runir::kr::dl::python

#endif
