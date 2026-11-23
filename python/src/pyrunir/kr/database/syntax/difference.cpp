#include "bindings.hpp"
#include "pyrunir/conversions.hpp"
#include "pyrunir/module.hpp"

namespace runir::kr::database
{
void bind_query_difference(nb::module_& m, QueryRepositoryBinding& repository)
{
    urd::python::bind_index<Query<QueryDifferenceTag>>(m);
    urd::python::bind_data<Query<QueryDifferenceTag>>(m);
    urd::python::bind_builder<Query<QueryDifferenceTag>, QueryRepository>(m);
    bind_query_operation<QueryDifferenceTag>(m, repository);
}

}  // namespace runir::kr::database
