#ifndef RUNIR_KR_PS_EXT_EVALUATION_ENVIRONMENT_HPP_
#define RUNIR_KR_PS_EXT_EVALUATION_ENVIRONMENT_HPP_

#include "runir/kr/dl/semantics/builder.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/dl/semantics/evaluation_workspace.hpp"
#include "runir/kr/dl/semantics/ext/evaluation_context.hpp"
#include "runir/kr/ps/ext/dl/evaluation_context.hpp"
#include "runir/kr/ps/ext/evaluation_context.hpp"

#include <cstddef>
#include <utility>
#include <vector>

namespace runir::kr::ps::ext
{

template<tyr::planning::TaskKind Kind>
class EvaluationEnvironment
{
private:
    using StateDlContext = runir::kr::dl::semantics::EvaluationContext<runir::kr::ExtFamilyTag, Kind>;
    using TransitionDlContext = runir::kr::ps::dl::EvaluationContext<runir::kr::ExtFamilyTag, Kind>;

    runir::kr::dl::semantics::Builder m_dl_builder;
    runir::kr::dl::semantics::DenotationRepositoryFactory m_dl_denotation_repository_factory;
    runir::kr::dl::semantics::DenotationRepository m_dl_denotation_repository;
    runir::kr::dl::semantics::EvaluationWorkspace m_dl_workspace;
    std::vector<runir::kr::dl::semantics::ConceptDenotationView> m_do_argument_denotations;
    ModuleProgramView m_program;

public:
    explicit EvaluationEnvironment(ModuleProgramView program) :
        m_dl_builder(),
        m_dl_denotation_repository_factory(),
        m_dl_denotation_repository(m_dl_denotation_repository_factory.create()),
        m_dl_workspace(),
        m_do_argument_denotations(),
        m_program(program)
    {
    }

    auto get_program() const noexcept { return m_program; }
    auto& get_dl_repository() noexcept { return m_program.get_context().get_dl_repository(); }
    auto& get_dl_workspace() noexcept { return m_dl_workspace; }

    auto& prepare_do_argument_denotations(size_t size)
    {
        m_do_argument_denotations.clear();
        m_do_argument_denotations.reserve(size);
        return m_do_argument_denotations;
    }

    StateDlContext make_dl_context(const EvaluationContext<Kind>& context) { return make_dl_context(context, context.get_state()); }

    TransitionDlContext make_dl_transition_context(const EvaluationContext<Kind>& context, tyr::planning::StateView<Kind> target_state)
    {
        return TransitionDlContext(context.get_state(),
                                   std::move(target_state),
                                   m_dl_builder,
                                   m_dl_denotation_repository,
                                   m_dl_workspace,
                                   runir::kr::dl::semantics::Arguments(context.get_call_stack().arguments()),
                                   context.get_call_stack().registers());
    }

private:
    StateDlContext make_dl_context(const EvaluationContext<Kind>& context, tyr::planning::StateView<Kind> state)
    {
        return StateDlContext(std::move(state),
                              m_dl_builder,
                              m_dl_denotation_repository,
                              runir::kr::dl::semantics::Arguments(context.get_call_stack().arguments()),
                              context.get_call_stack().registers());
    }
};

}  // namespace runir::kr::ps::ext

#endif
