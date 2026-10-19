#include "runir/kr/dl/detail/optimization_impl.hpp"
#include "runir/kr/dl/detail/repository_instantiations_uns.hpp"

namespace runir::kr::dl
{

template class QueryOptimizer<runir::kr::UnsFamilyTag>;
template class QueryStatisticsSampler<runir::kr::UnsFamilyTag>;
template class QueryStatisticsCollector<runir::kr::UnsFamilyTag>;

template FamilyRecordView<runir::kr::UnsFamilyTag, Constructor<runir::kr::UnsFamilyTag, ConceptTag>> QueryOptimizer<runir::kr::UnsFamilyTag>::optimize<Constructor<runir::kr::UnsFamilyTag, ConceptTag>>(FamilyRecordView<runir::kr::UnsFamilyTag, Constructor<runir::kr::UnsFamilyTag, ConceptTag>>);
template void QueryStatisticsCollector<runir::kr::UnsFamilyTag>::add<Constructor<runir::kr::UnsFamilyTag, ConceptTag>>(FamilyRecordView<runir::kr::UnsFamilyTag, Constructor<runir::kr::UnsFamilyTag, ConceptTag>>);
template FamilyRecordView<runir::kr::UnsFamilyTag, Constructor<runir::kr::UnsFamilyTag, RoleTag>> QueryOptimizer<runir::kr::UnsFamilyTag>::optimize<Constructor<runir::kr::UnsFamilyTag, RoleTag>>(FamilyRecordView<runir::kr::UnsFamilyTag, Constructor<runir::kr::UnsFamilyTag, RoleTag>>);
template void QueryStatisticsCollector<runir::kr::UnsFamilyTag>::add<Constructor<runir::kr::UnsFamilyTag, RoleTag>>(FamilyRecordView<runir::kr::UnsFamilyTag, Constructor<runir::kr::UnsFamilyTag, RoleTag>>);
template FamilyRecordView<runir::kr::UnsFamilyTag, Constructor<runir::kr::UnsFamilyTag, BooleanTag>> QueryOptimizer<runir::kr::UnsFamilyTag>::optimize<Constructor<runir::kr::UnsFamilyTag, BooleanTag>>(FamilyRecordView<runir::kr::UnsFamilyTag, Constructor<runir::kr::UnsFamilyTag, BooleanTag>>);
template void QueryStatisticsCollector<runir::kr::UnsFamilyTag>::add<Constructor<runir::kr::UnsFamilyTag, BooleanTag>>(FamilyRecordView<runir::kr::UnsFamilyTag, Constructor<runir::kr::UnsFamilyTag, BooleanTag>>);
template FamilyRecordView<runir::kr::UnsFamilyTag, Constructor<runir::kr::UnsFamilyTag, NumericalTag>> QueryOptimizer<runir::kr::UnsFamilyTag>::optimize<Constructor<runir::kr::UnsFamilyTag, NumericalTag>>(FamilyRecordView<runir::kr::UnsFamilyTag, Constructor<runir::kr::UnsFamilyTag, NumericalTag>>);
template void QueryStatisticsCollector<runir::kr::UnsFamilyTag>::add<Constructor<runir::kr::UnsFamilyTag, NumericalTag>>(FamilyRecordView<runir::kr::UnsFamilyTag, Constructor<runir::kr::UnsFamilyTag, NumericalTag>>);
template FamilyRecordView<runir::kr::UnsFamilyTag, Query<runir::kr::UnsFamilyTag>> QueryOptimizer<runir::kr::UnsFamilyTag>::optimize<Query<runir::kr::UnsFamilyTag>>(FamilyRecordView<runir::kr::UnsFamilyTag, Query<runir::kr::UnsFamilyTag>>);
template void QueryStatisticsCollector<runir::kr::UnsFamilyTag>::add<Query<runir::kr::UnsFamilyTag>>(FamilyRecordView<runir::kr::UnsFamilyTag, Query<runir::kr::UnsFamilyTag>>);

template void QueryStatisticsSampler<runir::kr::UnsFamilyTag>::add<tyr::GroundTag, semantics::StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::GroundTag>>(semantics::StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::GroundTag>&);
template void QueryStatisticsSampler<runir::kr::UnsFamilyTag>::add<tyr::LiftedTag, semantics::StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::LiftedTag>>(semantics::StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::LiftedTag>&);

}  // namespace runir::kr::dl
