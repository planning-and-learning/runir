#include "bindings.hpp"
#include "pyrunir/module.hpp"

namespace runir::kr::database
{
void bind_query_input(nb::module_& m, QueryRepositoryBinding& repository)
{
    urd::python::bind_index<Query<QueryInputTag>>(m);
    urd::python::bind_data<Query<QueryInputTag>>(m);
    urd::python::bind_builder<Query<QueryInputTag>, QueryRepository>(m);
    bind_query_operation<QueryInputTag>(m, repository);
}

}  // namespace runir::kr::database
