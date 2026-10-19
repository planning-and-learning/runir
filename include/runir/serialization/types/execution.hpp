#ifndef RUNIR_SERIALIZATION_TYPES_EXECUTION_HPP_
#define RUNIR_SERIALIZATION_TYPES_EXECUTION_HPP_

#include "runir/kr/ps/ext/execution_view.hpp"
#include "runir/kr/ps/icp/execution_view.hpp"
#include "runir/serialization/types/views.hpp"
#include <tyr/planning/declarations.hpp>

namespace runir::serialization
{

template<tyr::TaskKind Kind>
using ExecutionViews = ygg::TypeList<kr::ps::ext::ModuleStateView<Kind>,
                                     kr::ps::ext::CallStackView<Kind>,
                                     kr::ps::ext::ProgramStateView<Kind>,
                                     kr::ps::icp::ProgramStateView<Kind>,
                                     kr::ps::icp::HistoriesView<Kind>>;

}  // namespace runir::serialization

#endif
