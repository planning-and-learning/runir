#include "../query_bindings.hpp"
#include "bindings.hpp"
#include "pyrunir/module.hpp"
#include "runir/kr/dl/detail/repository_instantiations.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <tyr/datalog/lifted/database/syntax/formatter.hpp>
#include <tyr/datalog/lifted/database/syntax/query_repository.hpp>
#include <urd/python/bindings.hpp>

namespace runir::kr::dl
{

void bind_semantics_query(nb::module_& m)
{
    using tyr::formalism::DerivedTag;
    using tyr::formalism::FluentTag;
    using tyr::formalism::StaticTag;

    urd::python::bind_index<QueryColumn>(m);
    urd::python::bind_data<QueryColumn>(m);
    urd::python::bind_index<QueryProjection<ConceptTag>>(m);
    urd::python::bind_data<QueryProjection<ConceptTag>>(m);
    urd::python::bind_index<QueryProjection<RoleTag>>(m);
    urd::python::bind_data<QueryProjection<RoleTag>>(m);
    urd::python::bind_index<Query<>>(m);
    urd::python::bind_data<Query<>>(m);
    urd::python::bind_index<Query<AtomicStateTag<StaticTag>>>(m);
    urd::python::bind_data<Query<AtomicStateTag<StaticTag>>>(m);
    urd::python::bind_index<Query<AtomicStateTag<FluentTag>>>(m);
    urd::python::bind_data<Query<AtomicStateTag<FluentTag>>>(m);
    urd::python::bind_index<Query<AtomicStateTag<DerivedTag>>>(m);
    urd::python::bind_data<Query<AtomicStateTag<DerivedTag>>>(m);
    urd::python::bind_index<Query<AtomicGoalTag<StaticTag>>>(m);
    urd::python::bind_data<Query<AtomicGoalTag<StaticTag>>>(m);
    urd::python::bind_index<Query<AtomicGoalTag<FluentTag>>>(m);
    urd::python::bind_data<Query<AtomicGoalTag<FluentTag>>>(m);
    urd::python::bind_index<Query<AtomicGoalTag<DerivedTag>>>(m);
    urd::python::bind_data<Query<AtomicGoalTag<DerivedTag>>>(m);
    urd::python::bind_index<Query<QueryConceptTag>>(m);
    urd::python::bind_data<Query<QueryConceptTag>>(m);
    urd::python::bind_index<Query<QueryRoleTag>>(m);
    urd::python::bind_data<Query<QueryRoleTag>>(m);
    urd::python::bind_index<Query<QueryJoinTag>>(m);
    urd::python::bind_data<Query<QueryJoinTag>>(m);
    urd::python::bind_index<Query<QueryProjectTag>>(m);
    urd::python::bind_data<Query<QueryProjectTag>>(m);
    urd::python::bind_index<Query<QueryRenameTag>>(m);
    urd::python::bind_data<Query<QueryRenameTag>>(m);
    urd::python::bind_index<Query<QuerySelectEqualTag>>(m);
    urd::python::bind_data<Query<QuerySelectEqualTag>>(m);
    urd::python::bind_index<Query<QuerySelectValueTag>>(m);
    urd::python::bind_data<Query<QuerySelectValueTag>>(m);
    urd::python::bind_index<Query<QueryUnionTag>>(m);
    urd::python::bind_data<Query<QueryUnionTag>>(m);
    urd::python::bind_index<Query<QueryDifferenceTag>>(m);
    urd::python::bind_data<Query<QueryDifferenceTag>>(m);
    urd::python::bind_index<Query<QueryGenericJoinTag>>(m);
    urd::python::bind_data<Query<QueryGenericJoinTag>>(m);
    urd::python::bind_index<Query<QueryEmptyTag>>(m);
    urd::python::bind_data<Query<QueryEmptyTag>>(m);
    urd::python::bind_index<Query<QueryBooleanTag>>(m);
    urd::python::bind_data<Query<QueryBooleanTag>>(m);
    urd::python::bind_index<Query<QueryNumericalTag>>(m);
    urd::python::bind_data<Query<QueryNumericalTag>>(m);
    urd::python::bind_index<DatalogProgram>(m);
    urd::python::bind_data<DatalogProgram>(m);
    urd::python::bind_view<urd::index_view<DatalogProgram, ConstructorRepository>>(m);
    urd::python::bind_index<Query<QueryHeadTag>>(m);
    urd::python::bind_data<Query<QueryHeadTag>>(m);
    python::bind_query_views(m, QueryConstructorTags {});
}

}  // namespace runir::kr::dl
