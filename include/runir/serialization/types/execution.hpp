#ifndef RUNIR_SERIALIZATION_TYPES_EXECUTION_HPP_
#define RUNIR_SERIALIZATION_TYPES_EXECUTION_HPP_

#include "runir/kr/ps/ext/execution_view.hpp"
#include "runir/kr/ps/icp/execution_view.hpp"
#include "runir/serialization/types/views.hpp"

#include <tyr/planning/declarations.hpp>

namespace runir::serialization
{

template<tyr::TaskKind Kind>
using ExecutionViews = boost::mp11::mp_list<kr::ps::ext::ModuleStateView,
                                            kr::ps::ext::CallStackView,
                                            kr::ps::ext::ProgramStateView<Kind>,
                                            kr::ps::icp::ProgramStateView<Kind>,
                                            kr::ps::icp::HistoriesView>;

}  // namespace runir::serialization

#endif
