#ifndef RUNIR_KR_PS_EXT_DETAIL_EXECUTION_STATE_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_EXECUTION_STATE_HPP_

#include "runir/datasets/task_class.hpp"
#include "runir/kr/dl/semantics/builder.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/ps/ext/evaluation_context.hpp"

#include <tyr/planning/node.hpp>
#include <vector>

namespace runir::kr::ps::ext::detail
{

template<tyr::planning::TaskKind Kind>
class ModuleProgramExecutionState
{
private:
    runir::kr::dl::semantics::Builder m_dl_builder;
    runir::kr::dl::semantics::DenotationRepositoryFactory m_dl_denotation_repository_factory;
    runir::kr::dl::semantics::DenotationRepository m_dl_denotation_repository;
    tyr::planning::Node<Kind> m_initial_node;
    std::vector<ModuleView> m_modules;
    EvaluationContext<Kind> m_context;

    static std::vector<ModuleView> collect_modules(ModuleProgramView program)
    {
        auto result = std::vector<ModuleView> {};
        for (auto module : program.get_modules())
            result.push_back(module);
        return result;
    }

public:
    ModuleProgramExecutionState(const runir::datasets::TaskSearchContext<Kind>& search_context, ModuleProgramView program) :
        m_dl_builder(),
        m_dl_denotation_repository_factory(),
        m_dl_denotation_repository(m_dl_denotation_repository_factory.create()),
        m_initial_node(search_context.successor_generator->get_initial_node()),
        m_modules(collect_modules(program)),
        m_context(m_initial_node.get_state(), program.get_entry_module(), m_dl_builder, m_dl_denotation_repository, {}, {}, {}, {}, m_modules)
    {
    }

    const auto& get_initial_node() const noexcept { return m_initial_node; }
    auto& get_context() noexcept { return m_context; }
    const auto& get_context() const noexcept { return m_context; }
};

}  // namespace runir::kr::ps::ext::detail

#endif
