#include "bindings.hpp"
#include "pyrunir/kr/dl/conversions.hpp"
#include "pyrunir/module.hpp"

namespace runir::kr::database
{
void bind_query_select_equal(nb::module_& m, QueryRepositoryBinding& repository)
{
    urd::python::bind_index<Query<QuerySelectEqualTag>>(m);
    urd::python::bind_data<Query<QuerySelectEqualTag>>(m);
    urd::python::bind_builder<Query<QuerySelectEqualTag>, QueryRepository>(m);
    bind_query_operation<QuerySelectEqualTag>(m, repository);
}

}  // namespace runir::kr::database
