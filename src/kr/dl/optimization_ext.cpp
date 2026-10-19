#include "runir/kr/dl/detail/optimization_impl.hpp"
#include "runir/kr/dl/detail/repository_instantiations_ext.hpp"
#include "runir/kr/dl/semantics/ext/evaluation.hpp"

namespace runir::kr::dl
{

template class QueryOptimizer<runir::kr::ExtFamilyTag>;
template class QueryStatisticsSampler<runir::kr::ExtFamilyTag>;
template class QueryStatisticsCollector<runir::kr::ExtFamilyTag>;

template FamilyRecordView<runir::kr::ExtFamilyTag, Constructor<runir::kr::ExtFamilyTag, ConceptTag>> QueryOptimizer<runir::kr::ExtFamilyTag>::optimize<Constructor<runir::kr::ExtFamilyTag, ConceptTag>>(FamilyRecordView<runir::kr::ExtFamilyTag, Constructor<runir::kr::ExtFamilyTag, ConceptTag>>);
template void QueryStatisticsCollector<runir::kr::ExtFamilyTag>::add<Constructor<runir::kr::ExtFamilyTag, ConceptTag>>(FamilyRecordView<runir::kr::ExtFamilyTag, Constructor<runir::kr::ExtFamilyTag, ConceptTag>>);
template FamilyRecordView<runir::kr::ExtFamilyTag, Constructor<runir::kr::ExtFamilyTag, RoleTag>> QueryOptimizer<runir::kr::ExtFamilyTag>::optimize<Constructor<runir::kr::ExtFamilyTag, RoleTag>>(FamilyRecordView<runir::kr::ExtFamilyTag, Constructor<runir::kr::ExtFamilyTag, RoleTag>>);
template void QueryStatisticsCollector<runir::kr::ExtFamilyTag>::add<Constructor<runir::kr::ExtFamilyTag, RoleTag>>(FamilyRecordView<runir::kr::ExtFamilyTag, Constructor<runir::kr::ExtFamilyTag, RoleTag>>);
template FamilyRecordView<runir::kr::ExtFamilyTag, Constructor<runir::kr::ExtFamilyTag, BooleanTag>> QueryOptimizer<runir::kr::ExtFamilyTag>::optimize<Constructor<runir::kr::ExtFamilyTag, BooleanTag>>(FamilyRecordView<runir::kr::ExtFamilyTag, Constructor<runir::kr::ExtFamilyTag, BooleanTag>>);
template void QueryStatisticsCollector<runir::kr::ExtFamilyTag>::add<Constructor<runir::kr::ExtFamilyTag, BooleanTag>>(FamilyRecordView<runir::kr::ExtFamilyTag, Constructor<runir::kr::ExtFamilyTag, BooleanTag>>);
template FamilyRecordView<runir::kr::ExtFamilyTag, Constructor<runir::kr::ExtFamilyTag, NumericalTag>> QueryOptimizer<runir::kr::ExtFamilyTag>::optimize<Constructor<runir::kr::ExtFamilyTag, NumericalTag>>(FamilyRecordView<runir::kr::ExtFamilyTag, Constructor<runir::kr::ExtFamilyTag, NumericalTag>>);
template void QueryStatisticsCollector<runir::kr::ExtFamilyTag>::add<Constructor<runir::kr::ExtFamilyTag, NumericalTag>>(FamilyRecordView<runir::kr::ExtFamilyTag, Constructor<runir::kr::ExtFamilyTag, NumericalTag>>);
template FamilyRecordView<runir::kr::ExtFamilyTag, Query<runir::kr::ExtFamilyTag>> QueryOptimizer<runir::kr::ExtFamilyTag>::optimize<Query<runir::kr::ExtFamilyTag>>(FamilyRecordView<runir::kr::ExtFamilyTag, Query<runir::kr::ExtFamilyTag>>);
template void QueryStatisticsCollector<runir::kr::ExtFamilyTag>::add<Query<runir::kr::ExtFamilyTag>>(FamilyRecordView<runir::kr::ExtFamilyTag, Query<runir::kr::ExtFamilyTag>>);

template void QueryStatisticsSampler<runir::kr::ExtFamilyTag>::add<tyr::GroundTag, semantics::StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag>>(semantics::StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag>&);
template void QueryStatisticsSampler<runir::kr::ExtFamilyTag>::add<tyr::LiftedTag, semantics::StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::LiftedTag>>(semantics::StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::LiftedTag>&);

}  // namespace runir::kr::dl
