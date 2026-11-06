#include "runir/kr/dl/semantics/heuristics/projection.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

namespace runir::kr::dl::semantics::heuristics::detail
{

ygg::database::QueryPlan compile_edge_plan(const urd::Store<urd::sequence<ygg::database::QueryView<>>>& roots) { return ygg::database::compile(roots); }

}  // namespace runir::kr::dl::semantics::heuristics::detail
