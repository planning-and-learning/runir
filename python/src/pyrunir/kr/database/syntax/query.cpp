#include "bindings.hpp"
#include "pyrunir/conversions.hpp"
#include "pyrunir/module.hpp"

namespace runir::kr::database
{
void bind_query(nb::module_& m, QueryRepositoryBinding& repository)
{
    using AnyQuery = Query<>;
    urd::python::bind_index<AnyQuery>(m);
    urd::python::bind_data<AnyQuery>(m);
    urd::python::bind_builder<AnyQuery, QueryRepository>(m);
    urd::python::bind_view<QueryView<>>(m);
    ygg::bind_insert<AnyQuery>(repository);
}

void bind_queries(nb::module_& m)
{
    auto repository = QueryRepositoryBinding(m, "QueryRepository", "Interns validated relational expressions without reading data.");
    repository.def(nb::init<>()).def("__len__", [](const QueryRepository& self) { return self.size(); });
    bind_query_input(m, repository);
    bind_query_empty(m, repository);
    bind_query_join(m, repository);
    bind_query_project(m, repository);
    bind_query_rename(m, repository);
    bind_query_select_equal(m, repository);
    bind_query_select_value(m, repository);
    bind_query_union(m, repository);
    bind_query_difference(m, repository);
    bind_query_generic_join(m, repository);
    bind_query_semijoin(m, repository);
    bind_query_antijoin(m, repository);
    bind_query(m, repository);
}
}  // namespace runir::kr::database
