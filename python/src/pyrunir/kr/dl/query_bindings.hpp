#ifndef PYRUNIR_KR_DL_QUERY_BINDINGS_HPP_
#define PYRUNIR_KR_DL_QUERY_BINDINGS_HPP_

#include "module.hpp"

#include <concepts>
#include <nanobind/stl/vector.h>
#include <pyrunir/kr/binding_utils.hpp>
#include <pyrunir/kr/dl/evaluation_bindings.hpp>
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

template<FamilyTag Family, typename View>
void bind_query_columns(nb::class_<View>& cls)
{
    using ColumnView = ygg::View<ygg::Index<QueryColumn>, ConstructorRepositoryFor<Family>>;
    const auto retain_owner = ygg::python::make_owner_retainer();
    cls.def("get_columns",
            [retain_owner](nb::typed<nb::handle, View> owner)
            {
                nb::typed<nb::list, ColumnView> result(nb::list {});
                for (auto column : nb::cast<const View&>(owner).get_columns())
                    result.append(retain_owner(nb::cast(std::move(column)), owner));
                return result;
            });
}

template<FamilyTag Family, typename Tag, typename Class>
void def_query_data_constructors(Class& cls)
{
    using namespace nb::literals;
    using Repository = ConstructorRepositoryFor<Family>;
    using ColumnIndexList = ygg::IndexList<QueryColumn>;
    using ColumnViewList = std::vector<ygg::View<ygg::Index<QueryColumn>, Repository>>;
    using QueryIndex = ygg::Index<Query<Family>>;
    using QueryView = ygg::View<ygg::Index<Query<Family>>, Repository>;

    if constexpr (std::same_as<Tag, void>)
        cls.def(nb::init<typename ygg::Data<Query<Family>>::Variant>(), "variant"_a);
    else if constexpr (is_atomic_state_tag_v<Tag>)
        cls.def(nb::init<ygg::Index<tyr::formalism::Predicate<typename Tag::FactKind>>, ColumnIndexList>(), "predicate"_a, "columns"_a)
            .def(nb::init<tyr::formalism::planning::PredicateView<typename Tag::FactKind>, const ColumnViewList&>(), "predicate"_a, "columns"_a);
    else if constexpr (is_atomic_goal_tag_v<Tag>)
        cls.def(nb::init<ygg::Index<tyr::formalism::Predicate<typename Tag::FactKind>>, bool, ColumnIndexList>(), "predicate"_a, "polarity"_a, "columns"_a)
            .def(nb::init<tyr::formalism::planning::PredicateView<typename Tag::FactKind>, bool, const ColumnViewList&>(),
                 "predicate"_a,
                 "polarity"_a,
                 "columns"_a);
    else if constexpr (std::same_as<Tag, QueryConceptTag> || std::same_as<Tag, QueryRoleTag>)
    {
        using ArgIndex = ygg::Index<Constructor<Family, std::conditional_t<std::same_as<Tag, QueryConceptTag>, ConceptTag, RoleTag>>>;
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
    else
        static_assert(ygg::dependent_false<Tag>::value);
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
    def_query_data_constructors<Family, Tag>(data);
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
        runir::kr::python::bind_evaluate<Family>(view);
        m.def("syntactic_complexity", [](View value) { return semantics::syntactic_complexity(value); }, nb::arg("query"));
    }
    if constexpr (requires(View value) { value.get_predicate(); })
        view.def("get_predicate", &View::get_predicate, nb::keep_alive<0, 1>());
    if constexpr (requires(View value) { value.get_polarity(); })
        view.def("get_polarity", &View::get_polarity);
    bind_query_columns<Family>(view);
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
                    .def(nb::init<ygg::Index<Query<Family>>, ygg::IndexList<QueryColumn>>(), nb::arg("arg"), nb::arg("columns"))
                    .def(nb::init<ygg::View<ygg::Index<Query<Family>>, ConstructorRepositoryFor<Family>>,
                                  const std::vector<ygg::View<ygg::Index<QueryColumn>, ConstructorRepositoryFor<Family>>>&>(),
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
    bind_query_columns<Family>(view);
    ygg::add_print(view);
    ygg::add_comparison(view);
    ygg::add_hash(view);
}

template<FamilyTag Family>
void bind_queries(nb::module_& m)
{
    using ColumnView = ygg::View<ygg::Index<QueryColumn>, ConstructorRepositoryFor<Family>>;
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
