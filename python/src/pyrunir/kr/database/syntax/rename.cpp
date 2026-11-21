#include "bindings.hpp"
#include "pyrunir/module.hpp"

#include <urd/python/sequences/sequence.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

namespace runir::kr::database
{
void bind_query_rename(nb::module_& m, QueryRepositoryBinding& repository)
{
    urd::python::bind_index<Query<QueryRenameTag>>(m);
    urd::python::bind_data<Query<QueryRenameTag>>(m);
    urd::python::bind_builder<Query<QueryRenameTag>, QueryRepository>(m);
    bind_query_operation<QueryRenameTag>(m, repository);
}

}  // namespace runir::kr::database
