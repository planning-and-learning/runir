#ifndef RUNIR_KR_DL_SEMANTICS_DISTANCE_FEATURE_HPP_
#define RUNIR_KR_DL_SEMANTICS_DISTANCE_FEATURE_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <optional>
#include <vector>
#include <yggdrasil/core/config.hpp>

namespace runir::kr::dl::semantics
{

/// An input slot of a translated edge query: a leaf, or the argument of a complemented leaf (r_complement, c_not).
struct QuerySlot
{
    ygg::uint_t leaf = 0;  ///< The leaf query's index; unused for role edges.
    bool complement = false;
    bool is_static = false;
};

/// One n_distance's evaluation state: the edge query's slots, a yggdrasil min-distance evaluator over them
/// (ygg::database::MinDistance in full evaluation, ygg::database::incremental::MinDistance in incremental
/// evaluation), and its heuristic.
template<typename Distance, typename Heuristic>
struct DistanceFeature
{
    std::vector<QuerySlot> slots;
    std::optional<Distance> distance;
    Heuristic heuristic;
    bool prepared = false;  ///< The heuristic was built.
};

}  // namespace runir::kr::dl::semantics

#endif
