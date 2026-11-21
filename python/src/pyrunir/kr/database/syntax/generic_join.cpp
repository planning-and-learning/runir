#include "bindings.hpp"
#include "pyrunir/module.hpp"

#include <urd/python/sequences/sequence.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

namespace runir::kr::database_python
{
void bind_query_generic_join(nb::module_& m, QueryRepositoryBinding& repository)
{
    urd::python::bind_index<runir::kr::database::Query<runir::kr::database::QueryGenericJoinTag>>(m);
    urd::python::bind_data<runir::kr::database::Query<runir::kr::database::QueryGenericJoinTag>>(m);
    urd::python::bind_builder<runir::kr::database::Query<runir::kr::database::QueryGenericJoinTag>, runir::kr::database::QueryRepository>(m);
    bind_query_operation<runir::kr::database::QueryGenericJoinTag>(m, repository);
}

}  // namespace runir::kr::database_python
