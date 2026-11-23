#include "bindings.hpp"
#include "pyrunir/conversions.hpp"
#include "pyrunir/module.hpp"

namespace runir::kr::database
{
void bind_query_empty(nb::module_& m, QueryRepositoryBinding& repository)
{
    urd::python::bind_index<Query<QueryEmptyTag>>(m);
    urd::python::bind_data<Query<QueryEmptyTag>>(m);
    urd::python::bind_builder<Query<QueryEmptyTag>, QueryRepository>(m);
    bind_query_operation<QueryEmptyTag>(m, repository);
}

}  // namespace runir::kr::database
