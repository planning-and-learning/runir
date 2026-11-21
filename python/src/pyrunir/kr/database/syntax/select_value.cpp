#include "bindings.hpp"
#include "pyrunir/module.hpp"

namespace runir::kr::database_python
{
void bind_query_select_value(nb::module_& m, QueryRepositoryBinding& repository)
{
    urd::python::bind_index<runir::kr::database::Query<runir::kr::database::QuerySelectValueTag>>(m);
    urd::python::bind_data<runir::kr::database::Query<runir::kr::database::QuerySelectValueTag>>(m);
    urd::python::bind_builder<runir::kr::database::Query<runir::kr::database::QuerySelectValueTag>, runir::kr::database::QueryRepository>(m);
    bind_query_operation<runir::kr::database::QuerySelectValueTag>(m, repository);
}

}  // namespace runir::kr::database_python
