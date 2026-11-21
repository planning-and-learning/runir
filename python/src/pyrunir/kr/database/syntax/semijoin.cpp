#include "bindings.hpp"
#include "pyrunir/module.hpp"

namespace runir::kr::database_python
{
void bind_query_semijoin(nb::module_& m, QueryRepositoryBinding& repository)
{
    urd::python::bind_index<runir::kr::database::Query<runir::kr::database::QuerySemijoinTag>>(m);
    urd::python::bind_data<runir::kr::database::Query<runir::kr::database::QuerySemijoinTag>>(m);
    urd::python::bind_builder<runir::kr::database::Query<runir::kr::database::QuerySemijoinTag>, runir::kr::database::QueryRepository>(m);
    bind_query_operation<runir::kr::database::QuerySemijoinTag>(m, repository);
}

}  // namespace runir::kr::database_python
