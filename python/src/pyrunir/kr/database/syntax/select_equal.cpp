#include "bindings.hpp"
#include "pyrunir/module.hpp"

namespace runir::kr::database_python
{
void bind_query_select_equal(nb::module_& m, QueryRepositoryBinding& repository)
{
    urd::python::bind_index<runir::kr::database::Query<runir::kr::database::QuerySelectEqualTag>>(m);
    urd::python::bind_data<runir::kr::database::Query<runir::kr::database::QuerySelectEqualTag>>(m);
    urd::python::bind_builder<runir::kr::database::Query<runir::kr::database::QuerySelectEqualTag>, runir::kr::database::QueryRepository>(m);
    bind_query_operation<runir::kr::database::QuerySelectEqualTag>(m, repository);
}

}  // namespace runir::kr::database_python
