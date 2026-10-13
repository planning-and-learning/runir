#ifndef RUNIR_TESTS_INCREMENTAL_ROOT_EVALUATORS_HPP_
#define RUNIR_TESTS_INCREMENTAL_ROOT_EVALUATORS_HPP_

#include <runir/kr/dl/semantics/incremental/evaluation.hpp>

namespace runir::tests
{

/// The root of a one-root IncrementalEvaluation in one entered state; stale once its evaluator enters another state.
template<kr::FamilyTag Family, tyr::TaskKind Kind, kr::dl::CategoryTag Category, kr::dl::semantics::StateEvaluationContextConcept<Family, Kind> Context>
struct RootState
{
    kr::dl::semantics::incremental::StateEvaluation<Family, Kind, Context> state;
    kr::dl::semantics::incremental::EvaluationIndex<Category> root;

    auto get_result() { return state.evaluate(root); }
    const auto& get_delta()
        requires kr::dl::ConceptOrRoleTag<Category>
    {
        return state.delta(root);
    }
    size_t size()
        requires kr::dl::ConceptOrRoleTag<Category>
    {
        return state.size(root);
    }
    bool changed()
        requires kr::dl::BooleanOrNumericalTag<Category>
    {
        return state.changed(root);
    }
};

/// The root of a one-root IncrementalEvaluation, evaluated eagerly in each entered state.
template<kr::FamilyTag Family, tyr::TaskKind Kind, kr::dl::CategoryTag Category>
class RootEvaluator
{
    kr::dl::semantics::incremental::IncrementalEvaluation<Family, Kind> m_evaluation;
    kr::dl::semantics::incremental::EvaluationIndex<Category> m_root;

    template<kr::dl::semantics::StateEvaluationContextConcept<Family, Kind> Context>
    RootState<Family, Kind, Category, Context> evaluated(kr::dl::semantics::incremental::StateEvaluation<Family, Kind, Context> state)
    {
        auto result = RootState<Family, Kind, Category, Context> { std::move(state), m_root };
        (void) result.get_result();
        return result;
    }

public:
    RootEvaluator(const tyr::planning::Task<Kind>& task, kr::dl::FamilyConstructorView<Family, Category> expression) :
        m_evaluation(task, { expression }),
        m_root(m_evaluation.graph().get_index(expression))
    {
    }
    /// Enters the state of `context` unrelated to the previous one.
    template<kr::dl::semantics::StateEvaluationContextConcept<Family, Kind> Context>
    RootState<Family, Kind, Category, Context> initialize(const Context& context)
    {
        return evaluated(m_evaluation.enter(context));
    }
    /// Enters the state of `context`, reached from the previous one by `delta`. The root is updated from the delta if
    /// it can be, otherwise initialized; its get_delta() / changed() throw std::logic_error in the latter case.
    template<kr::dl::semantics::StateEvaluationContextConcept<Family, Kind> Context>
    RootState<Family, Kind, Category, Context>
    update(const Context& context, const kr::dl::semantics::incremental::Delta<Family>& delta, ygg::database::Workspace<kr::dl::QueryValues>& workspace)
    {
        return evaluated(m_evaluation.enter(context, delta, workspace));
    }
};

/// The root query of a one-root IncrementalEvaluation in one entered state; stale once its evaluator enters another
/// state.
template<kr::FamilyTag Family, tyr::TaskKind Kind, kr::dl::semantics::StateEvaluationContextConcept<Family, Kind> Context>
struct QueryRootState
{
    kr::dl::semantics::incremental::StateEvaluation<Family, Kind, Context> state;
    kr::dl::semantics::incremental::QueryEvaluationIndex root;

    const auto& get_result() { return state.evaluate(root); }
    const auto& get_delta() { return state.delta(root); }
    size_t size() { return state.size(root); }
};

/// The root query of a one-root IncrementalEvaluation, evaluated eagerly in each entered state.
template<kr::FamilyTag Family, tyr::TaskKind Kind>
class QueryRootEvaluator
{
    kr::dl::semantics::incremental::IncrementalEvaluation<Family, Kind> m_evaluation;
    kr::dl::semantics::incremental::QueryEvaluationIndex m_root;

    template<kr::dl::semantics::StateEvaluationContextConcept<Family, Kind> Context>
    QueryRootState<Family, Kind, Context> evaluated(kr::dl::semantics::incremental::StateEvaluation<Family, Kind, Context> state)
    {
        auto result = QueryRootState<Family, Kind, Context> { std::move(state), m_root };
        (void) result.get_result();
        return result;
    }

public:
    QueryRootEvaluator(const tyr::planning::Task<Kind>& task, kr::dl::FamilyQueryView<Family> expression) :
        m_evaluation(task, { expression }),
        m_root(m_evaluation.graph().get_index(expression))
    {
    }
    template<kr::dl::semantics::StateEvaluationContextConcept<Family, Kind> Context>
    QueryRootState<Family, Kind, Context> initialize(const Context& context)
    {
        return evaluated(m_evaluation.enter(context));
    }
    template<kr::dl::semantics::StateEvaluationContextConcept<Family, Kind> Context>
    QueryRootState<Family, Kind, Context>
    update(const Context& context, const kr::dl::semantics::incremental::Delta<Family>& delta, ygg::database::Workspace<kr::dl::QueryValues>& workspace)
    {
        return evaluated(m_evaluation.enter(context, delta, workspace));
    }
};

}  // namespace runir::tests

#endif
