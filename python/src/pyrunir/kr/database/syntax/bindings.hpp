#ifndef PYRUNIR_KR_DATABASE_QUERY_BINDINGS_HPP_
#define PYRUNIR_KR_DATABASE_QUERY_BINDINGS_HPP_

#include "../module.hpp"
#include "pyrunir/module.hpp"
#include "runir/kr/database/syntax/formatter.hpp"
#include "runir/kr/database/syntax/query.hpp"
#include "runir/kr/database/syntax/query_construction.hpp"
#include "yggdrasil/python/bindings.hpp"

#include <nanobind/nanobind.h>
#include <nanobind/stl/vector.h>
#include <urd/python/bindings.hpp>

namespace runir::kr::database_python
{
namespace nb = nanobind;

using Column = urd::index<ygg::Column>;
using ColumnIndices = std::span<const urd::data<ygg::ColumnLayout>>;
using QueryView = runir::kr::database::QueryView<>;
using QueryRepositoryBinding = nb::class_<runir::kr::database::QueryRepository>;

template<typename Tag>
using ConcreteQueryView = runir::kr::database::QueryView<Tag>;

/// A query of operation Tag, whose record is bound first (bind_data; Python builds it through its empty __init__ and
/// its fields): its view (bind_view, and its columns), and QueryRepository.insert of the record, prepared first: its plan
/// derived from its children (prepare_for_insert).
template<typename Tag>
nb::class_<ConcreteQueryView<Tag>> bind_query_operation(nb::module_& m, QueryRepositoryBinding& repository)
{
    repository.def(
        "insert",
        [](runir::kr::database::QueryRepository& self, urd::data<runir::kr::database::Query<Tag>>& data) { return runir::kr::database::insert(self, data); },
        nb::arg("data"));
    return urd::python::bind_view<ConcreteQueryView<Tag>>(m);
}

void bind_query(nb::module_& m, QueryRepositoryBinding& repository);
void bind_query_input(nb::module_& m, QueryRepositoryBinding& repository);
void bind_query_empty(nb::module_& m, QueryRepositoryBinding& repository);
void bind_query_join(nb::module_& m, QueryRepositoryBinding& repository);
void bind_query_project(nb::module_& m, QueryRepositoryBinding& repository);
void bind_query_rename(nb::module_& m, QueryRepositoryBinding& repository);
void bind_query_select_equal(nb::module_& m, QueryRepositoryBinding& repository);
void bind_query_select_value(nb::module_& m, QueryRepositoryBinding& repository);
void bind_query_union(nb::module_& m, QueryRepositoryBinding& repository);
void bind_query_difference(nb::module_& m, QueryRepositoryBinding& repository);
void bind_query_generic_join(nb::module_& m, QueryRepositoryBinding& repository);
void bind_query_semijoin(nb::module_& m, QueryRepositoryBinding& repository);
void bind_query_antijoin(nb::module_& m, QueryRepositoryBinding& repository);

}  // namespace runir::kr::database_python

#endif
