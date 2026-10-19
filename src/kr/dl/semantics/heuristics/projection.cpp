#include "runir/kr/dl/semantics/heuristics/projection.hpp"

namespace runir::kr::dl::semantics::heuristics::detail
{

ygg::database::QueryPlan<QueryValues> compile_edge_plan(const std::vector<ygg::database::QueryView<QueryValues>>& roots)
{
    return ygg::database::compile(roots);
}

}  // namespace runir::kr::dl::semantics::heuristics::detail
