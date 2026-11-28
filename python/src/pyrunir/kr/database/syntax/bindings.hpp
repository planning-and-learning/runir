#ifndef PYRUNIR_KR_DATABASE_SYNTAX_BINDINGS_HPP_
#define PYRUNIR_KR_DATABASE_SYNTAX_BINDINGS_HPP_

#include "../bindings.hpp"
#include "pyrunir/module.hpp"
#include "runir/kr/database/syntax/formatter.hpp"
#include "runir/kr/database/syntax/query.hpp"
#include "runir/kr/database/syntax/query_construction.hpp"
#include "yggdrasil/python/bindings.hpp"

#include <nanobind/nanobind.h>
#include <nanobind/stl/vector.h>
#include <urd/python/bindings.hpp>

namespace runir::kr::database
{
using QueryRepositoryBinding = nb::class_<QueryRepository>;

/// A query of operation Tag, whose record is bound first (bind_data; Python builds it through its empty __init__ and
/// its fields): its view (bind_view, and its columns), and QueryRepository.insert of the record, prepared first: its plan
/// derived from its children (prepare_for_insert).
template<typename Tag>
void bind_query_operation(nb::module_& m, QueryRepositoryBinding& repository)
{
    repository.def("insert", [](QueryRepository& self, urd::data<Query<Tag>>& data) { return self.insert(data); }, nb::arg("data"));
    urd::python::bind_view<QueryView<Tag>>(m);
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

}  // namespace runir::kr::database

#endif
