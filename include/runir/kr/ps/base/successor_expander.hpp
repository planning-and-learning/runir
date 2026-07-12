#ifndef RUNIR_KR_PS_BASE_SUCCESSOR_EXPANDER_HPP_
#define RUNIR_KR_PS_BASE_SUCCESSOR_EXPANDER_HPP_

#include "runir/kr/ps/base/compatibility.hpp"
#include "runir/kr/ps/base/dl/evaluation_context.hpp"
#include "runir/kr/ps/base/sketch_view.hpp"
#include "runir/kr/task_context.hpp"

#include <optional>
#include <tuple>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_view.hpp>
#include <utility>

namespace runir::kr::ps::base
{

template<tyr::planning::TaskKind Kind>
class EvaluationContext
{
private:
    using TransitionContext = runir::kr::ps::dl::EvaluationContext<runir::kr::BaseFamilyTag, Kind>;

    tyr::planning::StateView<Kind> m_state;
    runir::kr::dl::semantics::Builder* m_dl_builder;
    runir::kr::dl::semantics::DenotationRepository* m_dl_denotation_repository;

public:
    EvaluationContext(tyr::planning::StateView<Kind> state,
                      runir::kr::dl::semantics::Builder& dl_builder,
                      runir::kr::dl::semantics::DenotationRepository& dl_denotation_repository) noexcept :
        m_state(std::move(state)),
        m_dl_builder(&dl_builder),
        m_dl_denotation_repository(&dl_denotation_repository)
    {
    }

    const auto& get_state() const noexcept { return m_state; }
    auto identifying_members() const noexcept { return std::tie(m_state); }
    auto& get_dl_builder() const noexcept { return *m_dl_builder; }
    auto& get_dl_denotation_repository() const noexcept { return *m_dl_denotation_repository; }

    template<typename F>
    decltype(auto) with_dl_transition_context(tyr::planning::StateView<Kind> target_state, F&& f) const
    {
        auto context = TransitionContext(m_state, std::move(target_state), get_dl_builder(), get_dl_denotation_repository());
        return std::forward<F>(f)(context);
    }
};

template<tyr::planning::TaskKind Kind>
class SuccessorExpander
{
private:
    runir::kr::TaskContext<Kind>* m_task_context;
    SketchView m_sketch;

public:
    SuccessorExpander(runir::kr::TaskContext<Kind>& task_context, SketchView sketch) : m_task_context(&task_context), m_sketch(sketch) {}

    EvaluationContext<Kind> context_at(tyr::planning::StateView<Kind> state)
    {
        return EvaluationContext<Kind>(std::move(state), m_task_context->dl_builder, *m_task_context->dl_denotation_repository);
    }

    std::optional<RuleView> matching_rule(EvaluationContext<Kind>& context, const tyr::planning::StateView<Kind>& target_state)
    {
        if (context.get_state().get_index() == target_state.get_index())
            return std::nullopt;

        return context.with_dl_transition_context(target_state,
                                                  [&](auto& transition_context) -> std::optional<RuleView>
                                                  {
                                                      for (auto rule : m_sketch.get_rules())
                                                          if (runir::kr::ps::base::is_compatible_with(rule, transition_context))
                                                              return rule;
                                                      return std::nullopt;
                                                  });
    }
};

}  // namespace runir::kr::ps::base

#endif
