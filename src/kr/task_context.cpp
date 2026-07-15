#include "runir/kr/task_context.hpp"

namespace runir::kr
{

template struct TaskContext<tyr::planning::GroundTag>;
template struct TaskContext<tyr::planning::LiftedTag>;

}  // namespace runir::kr
