#include "bindings.hpp"
#include "pyrunir/module.hpp"

namespace runir::kr::database_python
{
void bind_query_empty(nb::module_& m, QueryRepositoryBinding& repository)
{
    urd::python::bind_index<runir::kr::database::Query<runir::kr::database::QueryEmptyTag>>(m);
    urd::python::bind_data<runir::kr::database::Query<runir::kr::database::QueryEmptyTag>>(m);
    urd::python::bind_builder<runir::kr::database::Query<runir::kr::database::QueryEmptyTag>, runir::kr::database::QueryRepository>(m);
    bind_query_operation<runir::kr::database::QueryEmptyTag>(m, repository);
}

}  // namespace runir::kr::database_python
