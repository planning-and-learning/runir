#include "module.hpp"
#include "pyrunir/kr/dl/serialization.hpp"

#include <runir/kr/dl/query_view.hpp>
#include <runir/kr/dl/record_view.hpp>
#include <runir/kr/ps/ext/execution_view.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/icp/execution_view.hpp>
#include <runir/kr/ps/icp/formatter.hpp>
#include <tyr/formalism/planning/formatter.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>
#include <tyr/formalism/planning/planning_task.hpp>
#include <tyr/formalism/repository.hpp>
#include <tyr/planning/formatter.hpp>
#include <tyr/planning/ground/task.hpp>
#include <tyr/planning/lifted/task.hpp>
#include <tyr/planning/node.hpp>
#include <tyr/planning/plan.hpp>
#include <tyr/planning/state_view.hpp>

namespace runir::kr::ps
{
void bind_execution_serialization()
{
    using ControlViews = boost::mp11::mp_list<runir::kr::ps::ext::ModuleStateView, runir::kr::ps::ext::CallStackView, runir::kr::ps::icp::HistoriesView>;
    using GroundViews = boost::mp11::mp_list<runir::kr::ps::ext::ProgramStateView<tyr::GroundTag>, runir::kr::ps::icp::ProgramStateView<tyr::GroundTag>>;
    using LiftedViews = boost::mp11::mp_list<runir::kr::ps::ext::ProgramStateView<tyr::LiftedTag>, runir::kr::ps::icp::ProgramStateView<tyr::LiftedTag>>;
    ygg::python::bind_serialization(boost::mp11::mp_append<ControlViews, GroundViews, LiftedViews> {});
}
}  // namespace runir::kr::ps
