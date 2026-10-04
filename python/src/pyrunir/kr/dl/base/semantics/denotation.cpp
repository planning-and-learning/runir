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

namespace runir::kr::dl::base
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

    if constexpr (std::same_as<Category, BooleanTag> || std::same_as<Category, NumericalTag>)
        cls.def("get", [](View view) { return view.get(); });

    if constexpr (std::same_as<Category, ConceptTag>)
        cls.def("__iter__",
                [retainer](nb::typed<nb::handle, View> owner)
                {
                    const auto& view = nb::cast<const View&>(owner);
                    return nb::borrow<nb::typed<nb::iterator, semantics::DenotationElementView<Category>>>(
                        ygg::python::make_iterator_with_owner(nb::make_iterator(nb::type<View>(), "ConceptDenotationIterator", view.begin(), view.end()),
                                                              owner,
                                                              retainer));
                });

    if constexpr (std::same_as<Category, RoleTag>)
        cls.def("__iter__",
                [retainer](nb::typed<nb::handle, View> owner)
                {
                    const auto& view = nb::cast<const View&>(owner);
                    return nb::borrow<nb::typed<nb::iterator, semantics::DenotationElementView<Category>>>(
                        ygg::python::make_iterator_with_owner(nb::make_iterator(nb::type<View>(), "RoleDenotationIterator", view.begin(), view.end()),
                                                              owner,
                                                              retainer));
                });
}

void bind_query_denotation(nb::module_& m)
{
    using ObjectIndex = ygg::Index<tyr::formalism::Object>;
    using Type = ygg::database::Relation<ObjectIndex>;
    using View = semantics::QueryDenotationView;
    using Row = tyr::formalism::planning::ObjectSpanView;
    using Repository = ygg::database::RelationRepository<ObjectIndex>;
    const auto retainer = ygg::python::make_owner_retainer();

    ygg::bind_index<ygg::Index<Type>>(m, "QueryDenotationIndex");
    nb::class_<Row>(m, "QueryDenotationRow", "Read-only borrowed row of Tyr objects; keeps its query result alive.")
        .def("__len__", &Row::size)
        .def(
            "__getitem__",
            [](const Row& row, std::ptrdiff_t index)
            {
                if (index < 0)
                    index += static_cast<std::ptrdiff_t>(row.size());
                if (index < 0 || static_cast<std::size_t>(index) >= row.size())
                    throw nb::index_error();
                return row[index];
            },
            nb::keep_alive<0, 1>())
        .def("__iter__",
             [retainer](nb::typed<nb::handle, Row> owner)
             {
                 const auto& row = nb::cast<const Row&>(owner);
                 return nb::borrow<nb::typed<nb::iterator, tyr::formalism::planning::ObjectView>>(
                     ygg::python::make_iterator_with_owner(nb::make_iterator(nb::type<Row>(), "QueryDenotationRowIterator", row.begin(), row.end()),
                                                           owner,
                                                           retainer));
             });

    auto view = nb::class_<View>(m,
                                 "QueryDenotation",
                                 "Interned query result with ordered columns and Tyr object rows. Clearing its result repository invalidates its views.")
                    .def("get_index", &View::get_index)
                    .def("__len__", &View::size)
                    .def(
                        "__getitem__",
                        [](const View& relation, std::ptrdiff_t index)
                        {
                            if (index < 0)
                                index += static_cast<std::ptrdiff_t>(relation.size());
                            if (index < 0 || static_cast<std::size_t>(index) >= relation.size())
                                throw nb::index_error();
                            return relation[index];
                        },
                        nb::keep_alive<0, 1>())
                    .def("arity", &View::arity)
                    .def("empty", &View::empty)
                    .def("at", &View::at, nb::arg("index"), nb::keep_alive<0, 1>())
                    .def("columns", [](const View& relation) { return relation.columns().span(); }, nb::keep_alive<0, 1>());
    ygg::add_comparison(view);
    ygg::add_hash(view);

    nb::class_<Repository>(m, "QueryDenotationRepository", "Canonical storage of typed query rows; clearing it invalidates borrowed query views.")
        .def("__len__", &Repository::size)
        .def("clear", &Repository::clear)
        .def(
            "rename",
            [](Repository& repository, View source, const std::vector<ygg::uint_t>& columns, std::optional<std::size_t> schema_namespace)
            {
                auto labels = std::vector<ygg::Index<ygg::database::Column>> {};
                labels.reserve(columns.size());
                for (const auto column : columns)
                    labels.emplace_back(column);
                return schema_namespace ? repository.rename(source, labels, *schema_namespace) : repository.rename(source, labels);
            },
            nb::arg("relation"),
            nb::arg("columns"),
            nb::arg("schema_namespace") = nb::none(),
            nb::keep_alive<0, 1>(),
            nb::keep_alive<0, 2>());
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
    bind_query_denotation(m);
}

}  // namespace runir::kr::dl::base
