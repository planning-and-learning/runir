#include "bindings.hpp"
#include "pyrunir/conversions.hpp"
#include "pyrunir/module.hpp"

namespace runir::kr::database
{
void bind_query_semijoin(nb::module_& m, QueryRepositoryBinding& repository)
{
    urd::python::bind_index<Query<QuerySemijoinTag>>(m);
    urd::python::bind_data<Query<QuerySemijoinTag>>(m);
    urd::python::bind_builder<Query<QuerySemijoinTag>, QueryRepository>(m);
    bind_query_operation<QuerySemijoinTag>(m, repository);
}

}  // namespace runir::kr::database
