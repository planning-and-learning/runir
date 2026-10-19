#include "runir/kr/dl/detail/optimization_impl.hpp"
#include "runir/kr/dl/detail/repository_instantiations_base.hpp"

namespace runir::kr::dl
{

template class QueryOptimizer<runir::kr::BaseFamilyTag>;
template class QueryStatisticsSampler<runir::kr::BaseFamilyTag>;
template class QueryStatisticsCollector<runir::kr::BaseFamilyTag>;

template FamilyRecordView<runir::kr::BaseFamilyTag, Constructor<runir::kr::BaseFamilyTag, ConceptTag>> QueryOptimizer<runir::kr::BaseFamilyTag>::optimize<Constructor<runir::kr::BaseFamilyTag, ConceptTag>>(FamilyRecordView<runir::kr::BaseFamilyTag, Constructor<runir::kr::BaseFamilyTag, ConceptTag>>);
template void QueryStatisticsCollector<runir::kr::BaseFamilyTag>::add<Constructor<runir::kr::BaseFamilyTag, ConceptTag>>(FamilyRecordView<runir::kr::BaseFamilyTag, Constructor<runir::kr::BaseFamilyTag, ConceptTag>>);
template FamilyRecordView<runir::kr::BaseFamilyTag, Constructor<runir::kr::BaseFamilyTag, RoleTag>> QueryOptimizer<runir::kr::BaseFamilyTag>::optimize<Constructor<runir::kr::BaseFamilyTag, RoleTag>>(FamilyRecordView<runir::kr::BaseFamilyTag, Constructor<runir::kr::BaseFamilyTag, RoleTag>>);
template void QueryStatisticsCollector<runir::kr::BaseFamilyTag>::add<Constructor<runir::kr::BaseFamilyTag, RoleTag>>(FamilyRecordView<runir::kr::BaseFamilyTag, Constructor<runir::kr::BaseFamilyTag, RoleTag>>);
template FamilyRecordView<runir::kr::BaseFamilyTag, Constructor<runir::kr::BaseFamilyTag, BooleanTag>> QueryOptimizer<runir::kr::BaseFamilyTag>::optimize<Constructor<runir::kr::BaseFamilyTag, BooleanTag>>(FamilyRecordView<runir::kr::BaseFamilyTag, Constructor<runir::kr::BaseFamilyTag, BooleanTag>>);
template void QueryStatisticsCollector<runir::kr::BaseFamilyTag>::add<Constructor<runir::kr::BaseFamilyTag, BooleanTag>>(FamilyRecordView<runir::kr::BaseFamilyTag, Constructor<runir::kr::BaseFamilyTag, BooleanTag>>);
template FamilyRecordView<runir::kr::BaseFamilyTag, Constructor<runir::kr::BaseFamilyTag, NumericalTag>> QueryOptimizer<runir::kr::BaseFamilyTag>::optimize<Constructor<runir::kr::BaseFamilyTag, NumericalTag>>(FamilyRecordView<runir::kr::BaseFamilyTag, Constructor<runir::kr::BaseFamilyTag, NumericalTag>>);
template void QueryStatisticsCollector<runir::kr::BaseFamilyTag>::add<Constructor<runir::kr::BaseFamilyTag, NumericalTag>>(FamilyRecordView<runir::kr::BaseFamilyTag, Constructor<runir::kr::BaseFamilyTag, NumericalTag>>);
template FamilyRecordView<runir::kr::BaseFamilyTag, Query<runir::kr::BaseFamilyTag>> QueryOptimizer<runir::kr::BaseFamilyTag>::optimize<Query<runir::kr::BaseFamilyTag>>(FamilyRecordView<runir::kr::BaseFamilyTag, Query<runir::kr::BaseFamilyTag>>);
template void QueryStatisticsCollector<runir::kr::BaseFamilyTag>::add<Query<runir::kr::BaseFamilyTag>>(FamilyRecordView<runir::kr::BaseFamilyTag, Query<runir::kr::BaseFamilyTag>>);

template void QueryStatisticsSampler<runir::kr::BaseFamilyTag>::add<tyr::GroundTag, semantics::StateEvaluationContext<runir::kr::BaseFamilyTag, tyr::GroundTag>>(semantics::StateEvaluationContext<runir::kr::BaseFamilyTag, tyr::GroundTag>&);
template void QueryStatisticsSampler<runir::kr::BaseFamilyTag>::add<tyr::LiftedTag, semantics::StateEvaluationContext<runir::kr::BaseFamilyTag, tyr::LiftedTag>>(semantics::StateEvaluationContext<runir::kr::BaseFamilyTag, tyr::LiftedTag>&);

}  // namespace runir::kr::dl
