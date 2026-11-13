#ifndef PYRUNIR_KR_DL_QUERY_BINDINGS_HPP_
#define PYRUNIR_KR_DL_QUERY_BINDINGS_HPP_

#include "module.hpp"
#include "pyrunir/module.hpp"

#include <concepts>
#include <nanobind/stl/variant.h>
#include <nanobind/stl/vector.h>
#include <pyrunir/kr/binding_utils.hpp>
#include <pyrunir/kr/dl/evaluation_bindings.hpp>
#include <pyrunir/kr/dl/optimization_bindings.hpp>
#include <pyrunir/kr/dl/record_bindings.hpp>
#include <runir/kr/dl/query_view.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <string>
#include <type_traits>
#include <urd/python/sequences/sequence.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::dl::python
{

template<typename Tag>
void bind_query(nb::module_& m, const char* name)
{
    using Type = Query<Tag>;
    using Data = urd::data<Type>;
    using View = urd::index_view<Type, ConstructorRepository>;

    urd::python::bind_index<urd::index<Type>>(m, std::string(name) + "Index");
    auto data = nb::class_<Data>(m, (std::string(name) + "Data").c_str()).def(nb::init<>()).def_rw("index", &Data::index);
    urd::python::add_comparison(data);
    def_fields<ConstructorRepository>(data);

    auto view = nb::class_<View>(m, name).def("index", &View::index);
    ygg::add_print(view);
    urd::python::add_comparison(view);
    urd::python::add_hash(view);
    view.def("syntactic_complexity", [](View value) { return semantics::syntactic_complexity(value); });
    urd::python::bind_field_getters(view);
    // A query's result columns: a field where they are given, derived elsewhere.
    if constexpr (!std::same_as<Tag, void> && !urd::has_field<Data>("columns"))
        view.def("get_columns", &View::get_columns);
    if constexpr (std::same_as<Tag, void>)
    {
        runir::kr::python::bind_evaluate(view);
        runir::kr::python::bind_optimize(view);
        m.def("syntactic_complexity", [](View value) { return semantics::syntactic_complexity(value); }, nb::arg("query"));
    }
}

template<CategoryTag Category>
void bind_query_projection(nb::module_& m, const char* name)
{
    using Type = QueryProjection<Category>;
    using Data = urd::data<Type>;
    using View = urd::index_view<Type, ConstructorRepository>;
    urd::python::bind_index<urd::index<Type>>(m, std::string(name) + "Index");
    auto data = nb::class_<Data>(m, (std::string(name) + "Data").c_str()).def(nb::init<>()).def_rw("index", &Data::index);
    def_fields<ConstructorRepository>(data);
    urd::python::add_comparison(data);
    auto view =
        nb::class_<View>(m, name).def("index", &View::index).def("syntactic_complexity", [](View value) { return semantics::syntactic_complexity(value); });
    urd::python::bind_field_getters(view);
    ygg::add_print(view);
    urd::python::add_comparison(view);
    urd::python::add_hash(view);
}

inline void bind_queries(nb::module_& m)
{
    using ColumnView = urd::index_view<QueryColumn, ConstructorRepository>;
    auto column = nb::class_<ColumnView>(m, "QueryColumn").def("index", &ColumnView::index).def("get_name", &ColumnView::get_name);
    ygg::add_print(column);
    urd::python::add_comparison(column);
    urd::python::add_hash(column);

    runir::kr::python::bind_query_statistics(m);
    bind_query_projection<ConceptTag>(m, "ConceptProject");
    bind_query_projection<RoleTag>(m, "RoleProject");
    bind_query<void>(m, "Query");
    bind_query<AtomicStateTag<tyr::formalism::StaticTag>>(m, "QueryAtomicStateStatic");
    bind_query<AtomicStateTag<tyr::formalism::FluentTag>>(m, "QueryAtomicStateFluent");
    bind_query<AtomicStateTag<tyr::formalism::DerivedTag>>(m, "QueryAtomicStateDerived");
    bind_query<AtomicGoalTag<tyr::formalism::StaticTag>>(m, "QueryAtomicGoalStatic");
    bind_query<AtomicGoalTag<tyr::formalism::FluentTag>>(m, "QueryAtomicGoalFluent");
    bind_query<AtomicGoalTag<tyr::formalism::DerivedTag>>(m, "QueryAtomicGoalDerived");
    bind_query<QueryConceptTag>(m, "QueryConcept");
    bind_query<QueryRoleTag>(m, "QueryRole");
    bind_query<QueryJoinTag>(m, "QueryJoin");
    bind_query<QueryProjectTag>(m, "QueryProject");
    bind_query<QueryRenameTag>(m, "QueryRename");
    bind_query<QuerySelectEqualTag>(m, "QuerySelectEqual");
    bind_query<QuerySelectValueTag>(m, "QuerySelectValue");
    bind_query<QueryUnionTag>(m, "QueryUnion");
    bind_query<QueryDifferenceTag>(m, "QueryDifference");
    bind_query<QueryGenericJoinTag>(m, "QueryGenericJoin");
    bind_query<QueryEmptyTag>(m, "QueryEmpty");
    bind_query<QueryMustPassTag>(m, "QueryMustPass");
}

}  // namespace runir::kr::dl::python

#endif
