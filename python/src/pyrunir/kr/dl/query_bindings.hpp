#ifndef PYRUNIR_KR_DL_QUERY_BINDINGS_HPP_
#define PYRUNIR_KR_DL_QUERY_BINDINGS_HPP_

#include "module.hpp"

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
#include <yggdrasil/core/dependent_false.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::dl::python
{

/// Binds a getter returning a range of views as a Python list whose elements keep the owner alive.
template<typename Element, typename View, typename Getter>
void bind_view_list(nb::class_<View>& cls, const char* name, Getter getter)
{
    const auto retain_owner = ygg::python::make_owner_retainer();
    cls.def(name,
            [retain_owner, getter](nb::typed<nb::handle, View> owner)
            {
                nb::typed<nb::list, Element> result(nb::list {});
                for (auto element : getter(nb::cast<const View&>(owner)))
                    result.append(retain_owner(nb::cast(std::move(element)), owner));
                return result;
            });
}

template<typename View>
void bind_query_columns(nb::class_<View>& cls)
{
    bind_view_list<ygg::View<ygg::Index<QueryColumn>, ConstructorRepositoryFor>>(cls, "get_columns", [](const View& value) { return value.get_columns(); });
}

template<typename Tag, typename Class>
void def_query_data_constructors(Class& cls)
{
    using namespace nb::literals;
    using Repository = ConstructorRepositoryFor;
    using ColumnIndexList = ygg::IndexList<QueryColumn>;
    using ColumnViewList = std::vector<ygg::View<ygg::Index<QueryColumn>, Repository>>;
    using QueryIndex = ygg::Index<Query<>>;
    using QueryView = ygg::View<ygg::Index<Query<>>, Repository>;

    if constexpr (std::same_as<Tag, void>)
        cls.def(nb::init<DataVariant<ygg::Data<Query<>>>>(), "variant"_a).def(nb::init<DataViewVariant<ygg::Data<Query<>>, Repository>>(), "variant"_a);
    else if constexpr (is_atomic_state_tag_v<Tag>)
        cls.def(nb::init<ygg::Index<tyr::formalism::Predicate<TagFactKind<Tag>>>, ColumnIndexList>(), "predicate"_a, "columns"_a)
            .def(nb::init<tyr::formalism::planning::PredicateView<TagFactKind<Tag>>, const ColumnViewList&>(), "predicate"_a, "columns"_a);
    else if constexpr (is_atomic_goal_tag_v<Tag>)
        cls.def(nb::init<ygg::Index<tyr::formalism::Predicate<TagFactKind<Tag>>>, bool, ColumnIndexList>(), "predicate"_a, "polarity"_a, "columns"_a)
            .def(nb::init<tyr::formalism::planning::PredicateView<TagFactKind<Tag>>, bool, const ColumnViewList&>(), "predicate"_a, "polarity"_a, "columns"_a);
    else if constexpr (std::same_as<Tag, QueryConceptTag> || std::same_as<Tag, QueryRoleTag>)
    {
        using ArgIndex = ygg::Index<Constructor<std::conditional_t<std::same_as<Tag, QueryConceptTag>, ConceptTag, RoleTag>>>;
        cls.def(nb::init<ArgIndex, ColumnIndexList>(), "arg"_a, "columns"_a)
            .def(nb::init<ygg::View<ArgIndex, Repository>, const ColumnViewList&>(), "arg"_a, "columns"_a);
    }
    else if constexpr (std::same_as<Tag, QueryJoinTag> || std::same_as<Tag, QueryUnionTag> || std::same_as<Tag, QueryDifferenceTag>)
        cls.def(nb::init<QueryIndex, QueryIndex>(), "lhs"_a, "rhs"_a).def(nb::init<QueryView, QueryView>(), "lhs"_a, "rhs"_a);
    else if constexpr (std::same_as<Tag, QueryProjectTag> || std::same_as<Tag, QueryRenameTag>)
        cls.def(nb::init<QueryIndex, ColumnIndexList>(), "arg"_a, "columns"_a).def(nb::init<QueryView, const ColumnViewList&>(), "arg"_a, "columns"_a);
    else if constexpr (std::same_as<Tag, QuerySelectEqualTag>)
        cls.def(nb::init<QueryIndex, ygg::Index<QueryColumn>, ygg::Index<QueryColumn>>(), "arg"_a, "lhs_column"_a, "rhs_column"_a)
            .def(nb::init<QueryView, ygg::View<ygg::Index<QueryColumn>, Repository>, ygg::View<ygg::Index<QueryColumn>, Repository>>(),
                 "arg"_a,
                 "lhs_column"_a,
                 "rhs_column"_a);
    else if constexpr (std::same_as<Tag, QuerySelectValueTag>)
        cls.def(nb::init<QueryIndex, ygg::Index<QueryColumn>, ygg::Index<tyr::formalism::Object>>(), "arg"_a, "column"_a, "object"_a)
            .def(nb::init<QueryView, ygg::View<ygg::Index<QueryColumn>, Repository>, tyr::formalism::planning::ObjectView>(), "arg"_a, "column"_a, "object"_a);
    else if constexpr (std::same_as<Tag, QueryGenericJoinTag>)
        cls.def(nb::init<ygg::IndexList<Query<>>, ColumnIndexList, ColumnIndexList>(), "args"_a, "variable_order"_a, "output_order"_a)
            .def(nb::init<const std::vector<QueryView>&, const ColumnViewList&, const ColumnViewList&>(), "args"_a, "variable_order"_a, "output_order"_a);
    else if constexpr (std::same_as<Tag, QueryEmptyTag>)
        cls.def(nb::init<ColumnIndexList>(), "columns"_a).def(nb::init<const ColumnViewList&>(), "columns"_a);
    else if constexpr (std::same_as<Tag, QueryMustPassTag>)
        cls.def(nb::init<ColumnIndexList, QueryIndex, QueryIndex, QueryIndex, QueryIndex>(), "keys"_a, "sources"_a, "edges"_a, "targets"_a, "probes"_a)
            .def(nb::init<const ColumnViewList&, QueryView, QueryView, QueryView, QueryView>(), "keys"_a, "sources"_a, "edges"_a, "targets"_a, "probes"_a);
    else
        static_assert(ygg::dependent_false<Tag>::value);
}

template<typename Tag>
void bind_query(nb::module_& m, const char* name)
{
    using Type = Query<Tag>;
    using Data = ygg::Data<Type>;
    using View = ygg::View<ygg::Index<Type>, ConstructorRepositoryFor>;

    ygg::bind_index<ygg::Index<Type>>(m, (std::string(name) + "Index").c_str());
    auto data = nb::class_<Data>(m, (std::string(name) + "Data").c_str()).def(nb::init<>()).def_rw("index", &Data::index);
    ygg::add_comparison(data);
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
        data.def_rw("keys", &Data::keys).def_rw("sources", &Data::sources).def_rw("edges", &Data::edges).def_rw("targets", &Data::targets).def_rw("probes", &Data::probes);

    auto view = nb::class_<View>(m, name).def("get_index", &View::get_index);
    ygg::add_print(view);
    ygg::add_comparison(view);
    ygg::add_hash(view);
    view.def("syntactic_complexity", [](View value) { return semantics::syntactic_complexity(value); });
    if constexpr (std::same_as<Tag, void>)
    {
        view.def("get_variant", &View::get_variant, nb::keep_alive<0, 1>());
        runir::kr::python::bind_evaluate(view);
        runir::kr::python::bind_optimize(view);
        m.def("syntactic_complexity", [](View value) { return semantics::syntactic_complexity(value); }, nb::arg("query"));
    }
    if constexpr (requires(View value) { value.get_predicate(); })
        view.def("get_predicate", &View::get_predicate, nb::keep_alive<0, 1>());
    if constexpr (requires(View value) { value.get_polarity(); })
        view.def("get_polarity", &View::get_polarity);
    bind_query_columns<>(view);
    if constexpr (requires(View value) { value.get_arg(); })
        view.def("get_arg", &View::get_arg, nb::keep_alive<0, 1>());
    if constexpr (requires(View value) { value.get_args(); })
    {
        using ColumnView = ygg::View<ygg::Index<QueryColumn>, ConstructorRepositoryFor>;
        bind_view_list<ygg::View<ygg::Index<Query<>>, ConstructorRepositoryFor>>(view, "get_args", [](const View& value) { return value.get_args(); });
        bind_view_list<ColumnView>(view, "get_variable_order", [](const View& value) { return value.get_variable_order(); });
        bind_view_list<ColumnView>(view, "get_output_order", [](const View& value) { return value.get_output_order(); });
    }
    if constexpr (requires(View value) { value.get_lhs(); })
        view.def("get_lhs", &View::get_lhs, nb::keep_alive<0, 1>()).def("get_rhs", &View::get_rhs, nb::keep_alive<0, 1>());
    if constexpr (requires(View value) { value.get_lhs_column(); })
        view.def("get_lhs_column", &View::get_lhs_column, nb::keep_alive<0, 1>()).def("get_rhs_column", &View::get_rhs_column, nb::keep_alive<0, 1>());
    if constexpr (requires(View value) { value.get_column(); })
        view.def("get_column", &View::get_column, nb::keep_alive<0, 1>()).def("get_object", &View::get_object, nb::keep_alive<0, 1>());
    if constexpr (requires(View value) { value.get_keys(); })
    {
        bind_view_list<ygg::View<ygg::Index<QueryColumn>, ConstructorRepositoryFor>>(view, "get_keys", [](const View& value) { return value.get_keys(); });
        view.def("get_sources", &View::get_sources, nb::keep_alive<0, 1>())
            .def("get_edges", &View::get_edges, nb::keep_alive<0, 1>())
            .def("get_targets", &View::get_targets, nb::keep_alive<0, 1>())
            .def("get_probes", &View::get_probes, nb::keep_alive<0, 1>());
    }
}

template<CategoryTag Category>
void bind_query_projection(nb::module_& m, const char* name)
{
    using Type = QueryProjection<Category>;
    using Data = ygg::Data<Type>;
    using View = ygg::View<ygg::Index<Type>, ConstructorRepositoryFor>;
    ygg::bind_index<ygg::Index<Type>>(m, (std::string(name) + "Index").c_str());
    auto data = nb::class_<Data>(m, (std::string(name) + "Data").c_str())
                    .def(nb::init<>())
                    .def(nb::init<ygg::Index<Query<>>, ygg::IndexList<QueryColumn>>(), nb::arg("arg"), nb::arg("columns"))
                    .def(nb::init<ygg::View<ygg::Index<Query<>>, ConstructorRepositoryFor>,
                                  const std::vector<ygg::View<ygg::Index<QueryColumn>, ConstructorRepositoryFor>>&>(),
                         nb::arg("arg"),
                         nb::arg("columns"))
                    .def_rw("index", &Data::index)
                    .def_rw("arg", &Data::arg)
                    .def_rw("columns", &Data::columns);
    ygg::add_comparison(data);
    auto view = nb::class_<View>(m, name)
                    .def("get_index", &View::get_index)
                    .def("get_arg", &View::get_arg, nb::keep_alive<0, 1>())
                    .def("syntactic_complexity", [](View value) { return semantics::syntactic_complexity(value); });
    bind_query_columns<>(view);
    ygg::add_print(view);
    ygg::add_comparison(view);
    ygg::add_hash(view);
}

inline void bind_queries(nb::module_& m)
{
    using ColumnView = ygg::View<ygg::Index<QueryColumn>, ConstructorRepositoryFor>;
    auto column = nb::class_<ColumnView>(m, "QueryColumn").def("get_index", &ColumnView::get_index).def("get_name", &ColumnView::get_name);
    ygg::add_print(column);
    ygg::add_comparison(column);
    ygg::add_hash(column);

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
