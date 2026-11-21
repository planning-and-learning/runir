#include "bindings.hpp"
#include "pyrunir/module.hpp"

namespace runir::kr::database
{
void bind_query_join(nb::module_& m, QueryRepositoryBinding& repository)
{
    urd::python::bind_index<Query<QueryJoinTag>>(m);
    urd::python::bind_data<Query<QueryJoinTag>>(m);
    urd::python::bind_builder<Query<QueryJoinTag>, QueryRepository>(m);
    bind_query_operation<QueryJoinTag>(m, repository);
}

}  // namespace runir::kr::database
