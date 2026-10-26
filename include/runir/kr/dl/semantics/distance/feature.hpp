#ifndef RUNIR_KR_DL_SEMANTICS_DISTANCE_FEATURE_HPP_
#define RUNIR_KR_DL_SEMANTICS_DISTANCE_FEATURE_HPP_

#include "runir/kr/dl/semantics/heuristics/projection.hpp"
#include "runir/kr/dl/translations/ygg_query.hpp"

#include <optional>
#include <vector>
#include <yggdrasil/database/search/must_pass.hpp>

namespace runir::kr::dl::semantics
{

/// The edges of one n_distance: the input slots of its translated edge query and a yggdrasil min-distance evaluator
/// over them (ygg::database::MinDistance in full evaluation, ygg::database::incremental::MinDistance in incremental
/// evaluation), and for k > 1 query edges the abstraction its pattern database heuristic is built from.
template<typename Distance>
struct DistanceFeature
{
    std::vector<QuerySlot> slots;
    Distance distance;
    std::optional<heuristics::EdgeAbstraction> abstraction;
};

/// The edges of one q_must_pass: the input slots of its translated edge query, a yggdrasil must-pass evaluator over them
/// (ygg::database::MustPass), and the abstraction of its keyed pattern database heuristic, built from the static slots at
/// first use (heuristic).
struct MustPassFeature
{
    using MustPass = ygg::database::MustPass<heuristics::KeyedProjectionHeuristic>;

    std::vector<QuerySlot> slots;
    MustPass must_pass;
    std::optional<heuristics::EdgeAbstraction> abstraction;
    std::optional<heuristics::KeyedProjectionHeuristic> heuristic;
};

}  // namespace runir::kr::dl::semantics

#endif
