#ifndef RUNIR_KR_DL_SEMANTICS_HEURISTICS_CONCEPTS_HPP_
#define RUNIR_KR_DL_SEMANTICS_HEURISTICS_CONCEPTS_HPP_

#include <yggdrasil/database/semantics/search/concepts.hpp>

namespace runir::kr::dl::semantics::heuristics
{

/// A heuristic for n_distance, maintained by runir and consumed by ygg::database::MinDistance<..., true, H>.
///
/// Contract:
///  - estimate(vertex) is consistent (h(u) <= 1 + h(v) on every edge, 0 on targets) in every state while the
///    targets stay unchanged, and uint_t's maximum only where no target is reachable (see
///    ygg::database::DistanceHeuristic);
///  - update(targets) adapts the estimate to new targets; it is called before every search.
template<typename H, typename Targets>
concept DistanceHeuristicConcept = ygg::database::DistanceHeuristic<H> && std::default_initializable<H> && requires(H& h, const Targets& targets) {
    h.update(targets);
};

}  // namespace runir::kr::dl::semantics::heuristics

#endif
