#include "bindings.hpp"
#include "pyrunir/module.hpp"

namespace runir::kr::database
{
void bind_query_antijoin(nb::module_& m, QueryRepositoryBinding& repository)
{
    urd::python::bind_index<Query<QueryAntijoinTag>>(m);
    urd::python::bind_data<Query<QueryAntijoinTag>>(m);
    urd::python::bind_builder<Query<QueryAntijoinTag>, QueryRepository>(m);
    bind_query_operation<QueryAntijoinTag>(m, repository);
}

}  // namespace runir::kr::database
