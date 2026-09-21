#ifndef PYRUNIR_KR_DL_QUERY_BINDINGS_HPP_
#define PYRUNIR_KR_DL_QUERY_BINDINGS_HPP_

#include "module.hpp"

#include <runir/kr/dl/query_view.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <string>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::dl::python
{

template<typename View>
nb::object query_columns(nb::handle self)
{
    auto result = nb::cast(nb::cast<const View&>(self).get_columns());
    // The list caster copies views, not their repository; each element must retain its parent.
    for (auto column : nb::borrow<nb::list>(result))
        NB_CALL(keep_alive_py)(NB_CTX, column.ptr(), self.ptr());
    return result;
}

template<FamilyTag Family, typename Tag>
void bind_query(nb::module_& m, const char* name)
{
    using Type = Query<Family, Tag>;
    using Data = ygg::Data<Type>;
    using View = ygg::View<ygg::Index<Type>, ConstructorRepositoryFor<Family>>;

    ygg::bind_index<ygg::Index<Type>>(m, (std::string(name) + "Index").c_str());
    auto data = nb::class_<Data>(m, (std::string(name) + "Data").c_str()).def(nb::init<>()).def_rw("index", &Data::index);
    ygg::add_comparison(data);
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
    if constexpr (requires(Data value) { value.lhs; })
        data.def_rw("lhs", &Data::lhs).def_rw("rhs", &Data::rhs);
    if constexpr (requires(Data value) { value.lhs_column; })
        data.def_rw("lhs_column", &Data::lhs_column).def_rw("rhs_column", &Data::rhs_column);
    if constexpr (requires(Data value) { value.column; })
        data.def_rw("column", &Data::column).def_rw("object", &Data::object);

    auto view = nb::class_<View>(m, name).def("get_index", &View::get_index);
    ygg::add_print(view);
    ygg::add_comparison(view);
    ygg::add_hash(view);
    view.def("syntactic_complexity", [](View value) { return semantics::syntactic_complexity(value); });
    if constexpr (std::same_as<Tag, void>)
    {
        view.def("get_variant", &View::get_variant, nb::keep_alive<0, 1>());
        m.def("syntactic_complexity", [](View value) { return semantics::syntactic_complexity(value); }, nb::arg("query"));
    }
    if constexpr (requires(View value) { value.get_predicate(); })
        view.def("get_predicate", &View::get_predicate, nb::keep_alive<0, 1>());
    if constexpr (requires(View value) { value.get_polarity(); })
        view.def("get_polarity", &View::get_polarity);
    view.def("get_columns", &query_columns<View>);
    if constexpr (requires(View value) { value.get_arg(); })
        view.def("get_arg", &View::get_arg, nb::keep_alive<0, 1>());
    if constexpr (requires(View value) { value.get_lhs(); })
        view.def("get_lhs", &View::get_lhs, nb::keep_alive<0, 1>()).def("get_rhs", &View::get_rhs, nb::keep_alive<0, 1>());
    if constexpr (requires(View value) { value.get_lhs_column(); })
        view.def("get_lhs_column", &View::get_lhs_column, nb::keep_alive<0, 1>()).def("get_rhs_column", &View::get_rhs_column, nb::keep_alive<0, 1>());
    if constexpr (requires(View value) { value.get_column(); })
        view.def("get_column", &View::get_column, nb::keep_alive<0, 1>()).def("get_object", &View::get_object, nb::keep_alive<0, 1>());
}

template<FamilyTag Family, CategoryTag Category>
void bind_query_projection(nb::module_& m, const char* name)
{
    using Type = QueryProjection<Family, Category>;
    using Data = ygg::Data<Type>;
    using View = ygg::View<ygg::Index<Type>, ConstructorRepositoryFor<Family>>;
    ygg::bind_index<ygg::Index<Type>>(m, (std::string(name) + "Index").c_str());
    auto data = nb::class_<Data>(m, (std::string(name) + "Data").c_str())
                    .def(nb::init<>())
                    .def_rw("index", &Data::index)
                    .def_rw("arg", &Data::arg)
                    .def_rw("columns", &Data::columns);
    ygg::add_comparison(data);
    auto view = nb::class_<View>(m, name)
                    .def("get_index", &View::get_index)
                    .def("get_arg", &View::get_arg, nb::keep_alive<0, 1>())
                    .def("get_columns", &query_columns<View>)
                    .def("syntactic_complexity", [](View value) { return semantics::syntactic_complexity(value); });
    ygg::add_print(view);
    ygg::add_comparison(view);
    ygg::add_hash(view);
}

template<FamilyTag Family>
void bind_queries(nb::module_& m)
{
    using ColumnData = ygg::Data<QueryColumn>;
    using ColumnView = ygg::View<ygg::Index<QueryColumn>, ConstructorRepositoryFor<Family>>;
    if constexpr (std::same_as<Family, runir::kr::BaseFamilyTag>)
    {
        ygg::bind_index<ygg::Index<QueryColumn>>(m, "QueryColumnIndex");
        auto data = nb::class_<ColumnData>(m, "QueryColumnData").def(nb::init<>()).def_rw("index", &ColumnData::index).def_rw("name", &ColumnData::name);
        ygg::add_comparison(data);
    }
    else
    {
        m.attr("QueryColumnIndex") = nb::type<ygg::Index<QueryColumn>>();
        m.attr("QueryColumnData") = nb::type<ColumnData>();
    }
    auto column = nb::class_<ColumnView>(m, "QueryColumn").def("get_index", &ColumnView::get_index).def("get_name", &ColumnView::get_name);
    ygg::add_print(column);
    ygg::add_comparison(column);
    ygg::add_hash(column);

    bind_query_projection<Family, ConceptTag>(m, "ConceptProject");
    bind_query_projection<Family, RoleTag>(m, "RoleProject");
    bind_query<Family, void>(m, "Query");
    bind_query<Family, AtomicStateTag<tyr::formalism::StaticTag>>(m, "QueryAtomicStateStatic");
    bind_query<Family, AtomicStateTag<tyr::formalism::FluentTag>>(m, "QueryAtomicStateFluent");
    bind_query<Family, AtomicStateTag<tyr::formalism::DerivedTag>>(m, "QueryAtomicStateDerived");
    bind_query<Family, AtomicGoalTag<tyr::formalism::StaticTag>>(m, "QueryAtomicGoalStatic");
    bind_query<Family, AtomicGoalTag<tyr::formalism::FluentTag>>(m, "QueryAtomicGoalFluent");
    bind_query<Family, AtomicGoalTag<tyr::formalism::DerivedTag>>(m, "QueryAtomicGoalDerived");
    bind_query<Family, QueryConceptTag>(m, "QueryConcept");
    bind_query<Family, QueryRoleTag>(m, "QueryRole");
    bind_query<Family, QueryJoinTag>(m, "QueryJoin");
    bind_query<Family, QueryProjectTag>(m, "QueryProject");
    bind_query<Family, QueryRenameTag>(m, "QueryRename");
    bind_query<Family, QuerySelectEqualTag>(m, "QuerySelectEqual");
    bind_query<Family, QuerySelectValueTag>(m, "QuerySelectValue");
    bind_query<Family, QueryUnionTag>(m, "QueryUnion");
    bind_query<Family, QueryDifferenceTag>(m, "QueryDifference");
}

}  // namespace runir::kr::dl::python

#endif
