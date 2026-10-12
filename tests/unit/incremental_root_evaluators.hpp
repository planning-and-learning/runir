#ifndef RUNIR_TESTS_INCREMENTAL_ROOT_EVALUATORS_HPP_
#define RUNIR_TESTS_INCREMENTAL_ROOT_EVALUATORS_HPP_

#include <runir/kr/dl/semantics/incremental/evaluation.hpp>

namespace runir::tests
{

/// The root of a one-root IncrementalEvaluation, evaluated eagerly in each entered state.
template<kr::FamilyTag Family, tyr::TaskKind Kind, kr::dl::CategoryTag Category>
class RootEvaluator
{
    kr::dl::semantics::incremental::IncrementalEvaluation<Family, Kind> m_evaluation;
    kr::dl::semantics::incremental::EvaluationIndex<Category> m_root;

public:
    RootEvaluator(const tyr::planning::Task<Kind>& task, kr::dl::FamilyConstructorView<Family, Category> expression) :
        m_evaluation(task, { expression }),
        m_root(m_evaluation.graph().get_index(expression))
    {
    }
    template<kr::dl::semantics::StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(Context& context)
    {
        m_evaluation.enter(context).evaluate(m_root);
    }
    /// Updates from the delta alone; throws std::logic_error if the root must be initialized instead.
    void update(const kr::dl::semantics::incremental::Delta<Family>& delta, ygg::database::Workspace<kr::dl::QueryValues>& workspace) { m_evaluation.enter(delta, workspace).evaluate(m_root); }
    auto get_result() const& { return m_evaluation.get_result(m_root); }
    auto get_result() const&& = delete;
    const auto& get_delta() const&
        requires kr::dl::ConceptOrRoleTag<Category>
    {
        return m_evaluation.get_delta(m_root);
    }
    const auto& get_delta() const&& = delete;
    size_t size() const
        requires kr::dl::ConceptOrRoleTag<Category>
    {
        return m_evaluation.size(m_root);
    }
    bool changed() const
        requires kr::dl::BooleanOrNumericalTag<Category>
    {
        return m_evaluation.changed(m_root);
    }
};

/// The root query of a one-root IncrementalEvaluation, evaluated eagerly in each entered state.
template<kr::FamilyTag Family, tyr::TaskKind Kind>
class QueryRootEvaluator
{
    kr::dl::semantics::incremental::IncrementalEvaluation<Family, Kind> m_evaluation;
    kr::dl::semantics::incremental::QueryEvaluationIndex m_root;

public:
    QueryRootEvaluator(const tyr::planning::Task<Kind>& task, kr::dl::FamilyQueryView<Family> expression) :
        m_evaluation(task, { expression }),
        m_root(m_evaluation.graph().get_index(expression))
    {
    }
    template<kr::dl::semantics::StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(Context& context)
    {
        m_evaluation.enter(context).evaluate(m_root);
    }
    void update(const kr::dl::semantics::incremental::Delta<Family>& delta, ygg::database::Workspace<kr::dl::QueryValues>& workspace) { m_evaluation.enter(delta, workspace).evaluate(m_root); }
    const auto& get_result() const& { return m_evaluation.get_result(m_root); }
    const auto& get_result() const&& = delete;
    const auto& get_delta() const& { return m_evaluation.get_delta(m_root); }
    const auto& get_delta() const&& = delete;
};

}  // namespace runir::tests

#endif
