#include "bindings.hpp"
#include "pyrunir/conversions.hpp"
#include "pyrunir/module.hpp"

namespace runir::kr::database
{
void bind_query_select_value(nb::module_& m, QueryRepositoryBinding& repository)
{
    urd::python::bind_index<Query<QuerySelectValueTag>>(m);
    urd::python::bind_data<Query<QuerySelectValueTag>>(m);
    urd::python::bind_builder<Query<QuerySelectValueTag>, QueryRepository>(m);
    bind_query_operation<QuerySelectValueTag>(m, repository);
}

}  // namespace runir::kr::database
