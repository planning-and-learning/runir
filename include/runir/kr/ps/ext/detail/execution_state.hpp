#ifndef RUNIR_KR_PS_EXT_DETAIL_EXECUTION_STATE_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_EXECUTION_STATE_HPP_

#include "runir/datasets/task_class.hpp"
#include "runir/kr/ps/ext/evaluation_context.hpp"

#include <tyr/planning/node.hpp>

namespace runir::kr::ps::ext::detail
{

template<tyr::planning::TaskKind Kind>
class ModuleProgramExecutionState
{
private:
    tyr::planning::Node<Kind> m_initial_node;
    EvaluationContext<Kind> m_context;

public:
    ModuleProgramExecutionState(const runir::datasets::TaskSearchContext<Kind>& search_context, ModuleProgramView program) :
        m_initial_node(search_context.successor_generator->get_initial_node()),
        m_context(m_initial_node.get_state(), program, program.get_entry_module())
    {
    }

    const auto& get_initial_node() const noexcept { return m_initial_node; }
    auto& get_context() noexcept { return m_context; }
    const auto& get_context() const noexcept { return m_context; }
};

}  // namespace runir::kr::ps::ext::detail

#endif
