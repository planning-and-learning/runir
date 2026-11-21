#include "bindings.hpp"
#include "pyrunir/module.hpp"

#include <urd/python/sequences/sequence.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

namespace runir::kr::database
{
void bind_query_generic_join(nb::module_& m, QueryRepositoryBinding& repository)
{
    urd::python::bind_index<Query<QueryGenericJoinTag>>(m);
    urd::python::bind_data<Query<QueryGenericJoinTag>>(m);
    urd::python::bind_builder<Query<QueryGenericJoinTag>, QueryRepository>(m);
    bind_query_operation<QueryGenericJoinTag>(m, repository);
}

}  // namespace runir::kr::database
