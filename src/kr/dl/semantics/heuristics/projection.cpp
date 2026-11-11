#include "runir/kr/dl/semantics/heuristics/projection.hpp"

#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

namespace runir::kr::dl::semantics::heuristics::detail
{

tyr::datalog::database::QueryPlan compile_edge_plan(const urd::Store<urd::sequence<tyr::datalog::database::QueryView<>>>& roots)
{ return tyr::datalog::database::compile(roots); }

}  // namespace runir::kr::dl::semantics::heuristics::detail
