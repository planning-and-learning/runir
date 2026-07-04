#ifndef RUNIR_KR_PS_EXT_EVALUATION_ENVIRONMENT_HPP_
#define RUNIR_KR_PS_EXT_EVALUATION_ENVIRONMENT_HPP_

#include "runir/kr/dl/semantics/builder.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/dl/semantics/ext/evaluation_context.hpp"
#include "runir/kr/ps/ext/dl/evaluation_context.hpp"
#include "runir/kr/ps/ext/evaluation_context.hpp"

#include <cstddef>
#include <utility>

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
    ModuleProgramView m_program;

public:
    explicit EvaluationEnvironment(ModuleProgramView program) :
        m_dl_builder(),
        m_dl_denotation_repository_factory(),
        m_dl_denotation_repository(m_dl_denotation_repository_factory.create()),
        m_program(program)
    {
    }

    auto get_program() const noexcept { return m_program; }
    auto& get_dl_repository() noexcept { return m_program.get_context().get_dl_repository(); }

    EvaluationContext<Kind> make_context(tyr::planning::StateView<Kind> state, ModuleView module) const
    {
        return EvaluationContext<Kind>(std::move(state), m_program, module);
    }

    EvaluationContext<Kind> make_context(tyr::planning::StateView<Kind> state,
                                         ModuleView module,
                                         MemoryStateView memory_state,
                                         typename EvaluationContext<Kind>::ConceptRegisters concept_registers,
                                         typename EvaluationContext<Kind>::RoleRegisters role_registers) const
    {
        auto context = make_context(std::move(state), module);
        context.set_memory_state(memory_state);
        context.set_registers(std::move(concept_registers), std::move(role_registers));
        return context;
    }

    StateDlContext make_dl_context(const EvaluationContext<Kind>& context) { return make_dl_context(context, context.get_state()); }

    TransitionDlContext make_dl_transition_context(const EvaluationContext<Kind>& context, tyr::planning::StateView<Kind> target_state)
    {
        auto dl_context = TransitionDlContext(context.get_state(),
                                              std::move(target_state),
                                              m_dl_builder,
                                              m_dl_denotation_repository,
                                              context.template arguments<runir::kr::dl::ConceptTag>(),
                                              context.template arguments<runir::kr::dl::RoleTag>(),
                                              context.template arguments<runir::kr::dl::BooleanTag>(),
                                              context.template arguments<runir::kr::dl::NumericalTag>());
        initialize_registers(context, dl_context.get_source_context());
        initialize_registers(context, dl_context.get_target_context());
        return dl_context;
    }

private:
    StateDlContext make_dl_context(const EvaluationContext<Kind>& context, tyr::planning::StateView<Kind> state)
    {
        auto dl_context = StateDlContext(std::move(state),
                                         m_dl_builder,
                                         m_dl_denotation_repository,
                                         context.template arguments<runir::kr::dl::ConceptTag>(),
                                         context.template arguments<runir::kr::dl::RoleTag>(),
                                         context.template arguments<runir::kr::dl::BooleanTag>(),
                                         context.template arguments<runir::kr::dl::NumericalTag>());
        initialize_registers(context, dl_context);
        return dl_context;
    }

    static void initialize_registers(const EvaluationContext<Kind>& source, StateDlContext& target)
    {
        const auto& concept_registers = source.concept_registers();
        for (size_t index = 0; index < concept_registers.size(); ++index)
            if (const auto object = concept_registers[index])
                target.set(runir::kr::dl::RegisterIdentifier<runir::kr::dl::ConceptTag>(static_cast<ygg::uint_t>(index)), ygg::make_view(*object, target));

        const auto& role_registers = source.role_registers();
        for (size_t index = 0; index < role_registers.size(); ++index)
            if (const auto role = role_registers[index])
                target.set(runir::kr::dl::RegisterIdentifier<runir::kr::dl::RoleTag>(static_cast<ygg::uint_t>(index)),
                           std::pair(ygg::make_view(role->first, target), ygg::make_view(role->second, target)));
    }
};

}  // namespace runir::kr::ps::ext

#endif
