#ifndef RUNIR_KR_DL_SEMANTICS_DISTANCE_FEATURE_HPP_
#define RUNIR_KR_DL_SEMANTICS_DISTANCE_FEATURE_HPP_

#include "runir/kr/dl/translations/ygg_query.hpp"

#include <vector>

namespace runir::kr::dl::semantics
{

/// The edges of one n_distance: the input slots of its translated edge query and a yggdrasil min-distance evaluator
/// over them (ygg::database::MinDistance in full evaluation, ygg::database::incremental::MinDistance in incremental
/// evaluation).
template<typename Distance>
struct DistanceFeature
{
    std::vector<QuerySlot> slots;
    Distance distance;
};

}  // namespace runir::kr::dl::semantics

#endif
