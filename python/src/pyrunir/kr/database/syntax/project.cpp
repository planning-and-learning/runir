#include "bindings.hpp"
#include "pyrunir/kr/dl/conversions.hpp"
#include "pyrunir/module.hpp"

#include <urd/python/sequences/sequence.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

namespace runir::kr::database
{
void bind_query_project(nb::module_& m, QueryRepositoryBinding& repository)
{
    urd::python::bind_index<Query<QueryProjectTag>>(m);
    urd::python::bind_data<Query<QueryProjectTag>>(m);
    urd::python::bind_builder<Query<QueryProjectTag>, QueryRepository>(m);
    bind_query_operation<QueryProjectTag>(m, repository);
}

}  // namespace runir::kr::database
