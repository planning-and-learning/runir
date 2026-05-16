#include "runir/kr/gp/policy_executor_impl.hpp"

#ifndef RUNIR_HEADER_INSTANTIATION
namespace runir::kr::gp
{

template auto prove_solution<tyr::planning::GroundTag>(const datasets::AnnotatedStateGraph<tyr::planning::GroundTag>& graph,
                                                       PolicyView policy) -> PolicyProofResults<tyr::planning::GroundTag>;

template auto prove_solution<tyr::planning::LiftedTag>(const datasets::AnnotatedStateGraph<tyr::planning::LiftedTag>& graph,
                                                       PolicyView policy) -> PolicyProofResults<tyr::planning::LiftedTag>;

template auto
find_solution<tyr::planning::GroundTag>(const datasets::TaskSearchContext<tyr::planning::GroundTag>& context,
                                        PolicyView policy,
                                        const PolicySearchOptions<tyr::planning::GroundTag>& options) -> tyr::planning::SearchResult<tyr::planning::GroundTag>;

template auto
find_solution<tyr::planning::LiftedTag>(const datasets::TaskSearchContext<tyr::planning::LiftedTag>& context,
                                        PolicyView policy,
                                        const PolicySearchOptions<tyr::planning::LiftedTag>& options) -> tyr::planning::SearchResult<tyr::planning::LiftedTag>;

}  // namespace runir::kr::gp
#endif
