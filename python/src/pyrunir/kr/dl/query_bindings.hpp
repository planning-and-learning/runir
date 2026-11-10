#ifndef PYRUNIR_KR_DL_QUERY_BINDINGS_HPP_
#define PYRUNIR_KR_DL_QUERY_BINDINGS_HPP_

#include "module.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <urd/python/sequences/sequence.hpp>

#include <concepts>
#include <nanobind/stl/variant.h>
#include <nanobind/stl/vector.h>
#include <pyrunir/kr/binding_utils.hpp>
#include <pyrunir/kr/dl/evaluation_bindings.hpp>
#include <pyrunir/kr/dl/optimization_bindings.hpp>
#include <runir/kr/dl/query_view.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <yggdrasil/python/bindings.hpp>
#include "pyrunir/module.hpp"

namespace runir::kr::dl::python
{

template<typename Tag, typename Class>
void def_query_data_constructors(Class& cls)
{
    using namespace nb::literals;
    using Repository = ConstructorRepository;
    using ColumnIndexList = urd::flat_vector<urd::index<QueryColumn>>;
    using ColumnViewList = urd::Store<urd::sequence<urd::index_view<QueryColumn, Repository>>>;
    using QueryIndex = urd::index<Query<>>;
    using QueryView = urd::index_view<Query<>, Repository>;

    if constexpr (std::same_as<Tag, void>)
        cls.def(nb::init<DataVariant<urd::data<Query<>>>>(), "variant"_a).def(urd::python::init_from_views<urd::resolved_t<DataVariant<urd::data<Query<>>>, Repository>>(), "variant"_a);
    else if constexpr (is_atomic_state_tag_v<Tag>)
        cls.def(nb::init<urd::index<tyr::formalism::Predicate<TagFactKind<Tag>>>, ColumnIndexList>(), "predicate"_a, "columns"_a)
            .def(urd::python::init_from_views<tyr::formalism::planning::PredicateView<TagFactKind<Tag>>, const ColumnViewList&>(), "predicate"_a, "columns"_a);
    else if constexpr (is_atomic_goal_tag_v<Tag>)
        cls.def(nb::init<urd::index<tyr::formalism::Predicate<TagFactKind<Tag>>>, bool, ColumnIndexList>(), "predicate"_a, "polarity"_a, "columns"_a)
            .def(urd::python::init_from_views<tyr::formalism::planning::PredicateView<TagFactKind<Tag>>, bool, const ColumnViewList&>(), "predicate"_a, "polarity"_a, "columns"_a);
    else if constexpr (std::same_as<Tag, QueryConceptTag> || std::same_as<Tag, QueryRoleTag>)
    {
        using ArgIndex = urd::index<Constructor<std::conditional_t<std::same_as<Tag, QueryConceptTag>, ConceptTag, RoleTag>>>;
        cls.def(nb::init<ArgIndex, ColumnIndexList>(), "arg"_a, "columns"_a)
            .def(urd::python::init_from_views<urd::index_view<Constructor<std::conditional_t<std::same_as<Tag, QueryConceptTag>, ConceptTag, RoleTag>>, Repository>, const ColumnViewList&>(), "arg"_a, "columns"_a);
    }
    else if constexpr (std::same_as<Tag, QueryJoinTag> || std::same_as<Tag, QueryUnionTag> || std::same_as<Tag, QueryDifferenceTag>)
        cls.def(nb::init<QueryIndex, QueryIndex>(), "lhs"_a, "rhs"_a).def(urd::python::init_from_views<QueryView, QueryView>(), "lhs"_a, "rhs"_a);
    else if constexpr (std::same_as<Tag, QueryProjectTag> || std::same_as<Tag, QueryRenameTag>)
        cls.def(nb::init<QueryIndex, ColumnIndexList>(), "arg"_a, "columns"_a).def(urd::python::init_from_views<QueryView, const ColumnViewList&>(), "arg"_a, "columns"_a);
    else if constexpr (std::same_as<Tag, QuerySelectEqualTag>)
        cls.def(nb::init<QueryIndex, urd::index<QueryColumn>, urd::index<QueryColumn>>(), "arg"_a, "lhs_column"_a, "rhs_column"_a)
            .def(urd::python::init_from_views<QueryView, urd::index_view<QueryColumn, Repository>, urd::index_view<QueryColumn, Repository>>(),
                 "arg"_a,
                 "lhs_column"_a,
                 "rhs_column"_a);
    else if constexpr (std::same_as<Tag, QuerySelectValueTag>)
        cls.def(nb::init<QueryIndex, urd::index<QueryColumn>, urd::index<tyr::formalism::Object>>(), "arg"_a, "column"_a, "object"_a)
            .def(urd::python::init_from_views<QueryView, urd::index_view<QueryColumn, Repository>, tyr::formalism::planning::ObjectView>(), "arg"_a, "column"_a, "object"_a);
    else if constexpr (std::same_as<Tag, QueryGenericJoinTag>)
        cls.def(nb::init<urd::flat_vector<urd::index<Query<>>>, ColumnIndexList, ColumnIndexList>(), "args"_a, "variable_order"_a, "output_order"_a)
            .def(urd::python::init_from_views<const urd::Store<urd::sequence<QueryView>>&, const ColumnViewList&, const ColumnViewList&>(), "args"_a, "variable_order"_a, "output_order"_a);
    else if constexpr (std::same_as<Tag, QueryEmptyTag>)
        cls.def(nb::init<ColumnIndexList>(), "columns"_a).def(urd::python::init_from_views<const ColumnViewList&>(), "columns"_a);
    else if constexpr (std::same_as<Tag, QueryMustPassTag>)
        cls.def(nb::init<ColumnIndexList, QueryIndex, QueryIndex, QueryIndex, QueryIndex>(), "keys"_a, "sources"_a, "edges"_a, "targets"_a, "probes"_a)
            .def(urd::python::init_from_views<const ColumnViewList&, QueryView, QueryView, QueryView, QueryView>(), "keys"_a, "sources"_a, "edges"_a, "targets"_a, "probes"_a);
    else
        static_assert(false);
}

template<typename Tag>
void bind_query(nb::module_& m, const char* name)
{
    using Type = Query<Tag>;
    using Data = urd::data<Type>;
    using View = urd::index_view<Type, ConstructorRepository>;

    urd::python::bind_index<urd::index<Type>>(m, std::string(name) + "Index");
    auto data = nb::class_<Data>(m, (std::string(name) + "Data").c_str()).def(nb::init<>()).def_rw("index", &Data::index);
    urd::python::add_comparison(data);
    def_query_data_constructors<Tag>(data);
    if constexpr (std::same_as<Tag, void>)
        data.def_rw("variant", &Data::variant);
    if constexpr (requires(Data value) { value.predicate; })
        data.def_rw("predicate", &Data::predicate);
    if constexpr (requires(Data value) { value.polarity; })
        data.def_rw("polarity", &Data::polarity);
    if constexpr (requires(Data value) { value.columns; })
        data.def_rw("columns", &Data::columns);
    if constexpr (requires(Data value) { value.arg; })
        data.def_rw("arg", &Data::arg);
    if constexpr (requires(Data value) { value.args; })
        data.def_rw("args", &Data::args).def_rw("variable_order", &Data::variable_order).def_rw("output_order", &Data::output_order);
    if constexpr (requires(Data value) { value.lhs; })
        data.def_rw("lhs", &Data::lhs).def_rw("rhs", &Data::rhs);
    if constexpr (requires(Data value) { value.lhs_column; })
        data.def_rw("lhs_column", &Data::lhs_column).def_rw("rhs_column", &Data::rhs_column);
    if constexpr (requires(Data value) { value.column; })
        data.def_rw("column", &Data::column).def_rw("object", &Data::object);
    if constexpr (requires(Data value) { value.keys; })
        data.def_rw("keys", &Data::keys)
            .def_rw("sources", &Data::sources)
            .def_rw("edges", &Data::edges)
            .def_rw("targets", &Data::targets)
            .def_rw("probes", &Data::probes);

    auto view = nb::class_<View>(m, name).def("index", &View::index);
    ygg::add_print(view);
    urd::python::add_comparison(view);
    urd::python::add_hash(view);
    view.def("syntactic_complexity", [](View value) { return semantics::syntactic_complexity(value); });
    if constexpr (std::same_as<Tag, void>)
    {
        view.def("get_variant", &View::get_variant);
        runir::kr::python::bind_evaluate(view);
        runir::kr::python::bind_optimize(view);
        m.def("syntactic_complexity", [](View value) { return semantics::syntactic_complexity(value); }, nb::arg("query"));
    }
    if constexpr (requires(View value) { value.get_predicate(); })
        view.def("get_predicate", &View::get_predicate);
    if constexpr (requires(View value) { value.get_polarity(); })
        view.def("get_polarity", &View::get_polarity);
    view.def("get_columns", &View::get_columns);
    if constexpr (requires(View value) { value.get_arg(); })
        view.def("get_arg", &View::get_arg);
    if constexpr (requires(View value) { value.get_args(); })
    {
        view.def("get_args", &View::get_args).def("get_variable_order", &View::get_variable_order).def("get_output_order", &View::get_output_order);
    }
    if constexpr (requires(View value) { value.get_lhs(); })
        view.def("get_lhs", &View::get_lhs).def("get_rhs", &View::get_rhs);
    if constexpr (requires(View value) { value.get_lhs_column(); })
        view.def("get_lhs_column", &View::get_lhs_column).def("get_rhs_column", &View::get_rhs_column);
    if constexpr (requires(View value) { value.get_column(); })
        view.def("get_column", &View::get_column).def("get_object", &View::get_object);
    if constexpr (requires(View value) { value.get_keys(); })
    {
        view.def("get_keys", &View::get_keys)
            .def("get_sources", &View::get_sources)
            .def("get_edges", &View::get_edges)
            .def("get_targets", &View::get_targets)
            .def("get_probes", &View::get_probes);
    }
}

template<CategoryTag Category>
void bind_query_projection(nb::module_& m, const char* name)
{
    using Type = QueryProjection<Category>;
    using Data = urd::data<Type>;
    using View = urd::index_view<Type, ConstructorRepository>;
    urd::python::bind_index<urd::index<Type>>(m, std::string(name) + "Index");
    auto data = nb::class_<Data>(m, (std::string(name) + "Data").c_str())
                    .def(nb::init<>())
                    .def(nb::init<urd::index<Query<>>, urd::flat_vector<urd::index<QueryColumn>>>(), nb::arg("arg"), nb::arg("columns"))
                    .def(urd::python::init_from_views<urd::index_view<Query<>, ConstructorRepository>,
                                  const urd::Store<urd::sequence<urd::index_view<QueryColumn, ConstructorRepository>>>&>(),
                         nb::arg("arg"),
                         nb::arg("columns"))
                    .def_rw("index", &Data::index)
                    .def_rw("arg", &Data::arg)
                    .def_rw("columns", &Data::columns);
    urd::python::add_comparison(data);
    auto view = nb::class_<View>(m, name)
                    .def("index", &View::index)
                    .def("get_arg", &View::get_arg)
                    .def("syntactic_complexity", [](View value) { return semantics::syntactic_complexity(value); });
    view.def("get_columns", &View::get_columns);
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
