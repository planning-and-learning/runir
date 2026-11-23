#include "bindings.hpp"
#include "pyrunir/kr/dl/conversions.hpp"
#include "pyrunir/module.hpp"

namespace runir::kr::database
{
void bind_query_union(nb::module_& m, QueryRepositoryBinding& repository)
{
    urd::python::bind_index<Query<QueryUnionTag>>(m);
    urd::python::bind_data<Query<QueryUnionTag>>(m);
    urd::python::bind_builder<Query<QueryUnionTag>, QueryRepository>(m);
    bind_query_operation<QueryUnionTag>(m, repository);
}

}  // namespace runir::kr::database
