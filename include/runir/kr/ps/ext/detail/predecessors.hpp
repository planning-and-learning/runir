#ifndef RUNIR_KR_PS_EXT_DETAIL_PREDECESSORS_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_PREDECESSORS_HPP_

#include "runir/kr/ps/ext/detail/search_node.hpp"

#include <vector>

namespace runir::kr::ps::ext::detail
{

/// ALL retains each admitted transition in discovery order, including parallel edges.
template<tyr::TaskKind Kind>
using Predecessors = std::vector<Predecessor<Kind>>;

}  // namespace runir::kr::ps::ext::detail

#endif
