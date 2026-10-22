#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_FEATURE_NODE_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_FEATURE_NODE_HPP_

#include "runir/kr/dl/semantics/incremental/detail/evaluators/argument.hpp"
#include "runir/kr/dl/semantics/incremental/detail/evaluators/atomic.hpp"
#include "runir/kr/dl/semantics/incremental/detail/evaluators/count.hpp"
#include "runir/kr/dl/semantics/incremental/detail/evaluators/distance.hpp"
#include "runir/kr/dl/semantics/incremental/detail/evaluators/fillers.hpp"
#include "runir/kr/dl/semantics/incremental/detail/evaluators/nonempty.hpp"
#include "runir/kr/dl/semantics/incremental/detail/evaluators/number_restriction.hpp"
#include "runir/kr/dl/semantics/incremental/detail/evaluators/projection.hpp"
#include "runir/kr/dl/semantics/incremental/detail/evaluators/register.hpp"
#include "runir/kr/dl/semantics/incremental/detail/evaluators/scalar.hpp"
#include "runir/kr/dl/semantics/incremental/detail/evaluators/set.hpp"
#include "runir/kr/dl/semantics/incremental/detail/evaluators/static.hpp"

#include <concepts>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <variant>
#include <vector>

namespace runir::kr::dl::semantics::incremental::detail
{

/// A prepared constructor owns its output and its typed dependencies. The graph
/// visits this evaluator directly; expression dispatch happens only at preparation.
template<tyr::TaskKind Kind, CategoryTag Category>
class FeatureNode
{
public:
    FeatureNode(FamilyConstructorView<Category> expression, GraphBuilder<Kind>& graph);
    FamilyConstructorView<Category> get_expression() const noexcept { return m_expression; }
    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context& context);
    void update(EvaluationGraph<Kind>& graph, const Delta& delta, ygg::database::Workspace& workspace);
    std::span<const ygg::uint_t> get_dependencies() const noexcept { return m_dependencies; }
    BorrowedDenotationView<Category> get_result(const tyr::formalism::planning::Repository& repository) const { return state().get_result(repository); }
    const auto& get_delta() const
        requires ConceptOrRoleTag<Category>
    {
        return state().get_delta();
    }
    size_t size() const
        requires ConceptOrRoleTag<Category>
    {
        return state().size();
    }
    bool changed() const
        requires BooleanOrNumericalTag<Category>
    {
        return state().changed();
    }

private:
    using ConceptOperations = ygg::TypeList<AtomicEvaluator<Kind, Category, tyr::formalism::FluentTag>,
                                            AtomicEvaluator<Kind, Category, tyr::formalism::DerivedTag>,
                                            UnarySetEvaluator<Kind, Category, NegationTag, ConceptTag>,
                                            BinarySetEvaluator<Kind, Category, IntersectionTag, ConceptTag, ConceptTag>,
                                            BinarySetEvaluator<Kind, Category, UnionTag, ConceptTag, ConceptTag>,
                                            BinarySetEvaluator<Kind, Category, ValueRestrictionTag, RoleTag, ConceptTag>,
                                            BinarySetEvaluator<Kind, Category, ExistentialQuantificationTag, RoleTag, ConceptTag>,
                                            BinarySetEvaluator<Kind, Category, RoleValueMapTag, RoleTag, RoleTag>,
                                            BinarySetEvaluator<Kind, Category, AgreementTag, RoleTag, RoleTag>,
                                            NumberRestrictionEvaluator<Kind, AtLeastNumberRestrictionTag>,
                                            NumberRestrictionEvaluator<Kind, AtMostNumberRestrictionTag>,
                                            NumberRestrictionEvaluator<Kind, ExactNumberRestrictionTag>,
                                            QualifiedNumberRestrictionEvaluator<Kind, QualifiedAtLeastNumberRestrictionTag>,
                                            QualifiedNumberRestrictionEvaluator<Kind, QualifiedAtMostNumberRestrictionTag>,
                                            QualifiedNumberRestrictionEvaluator<Kind, QualifiedExactNumberRestrictionTag>,
                                            FillersEvaluator<Kind>,
                                            ProjectionEvaluator<Kind, Category>>;
    using RoleOperations = ygg::TypeList<AtomicEvaluator<Kind, Category, tyr::formalism::FluentTag>,
                                         AtomicEvaluator<Kind, Category, tyr::formalism::DerivedTag>,
                                         UnarySetEvaluator<Kind, Category, ComplementTag, RoleTag>,
                                         UnarySetEvaluator<Kind, Category, InverseTag, RoleTag>,
                                         UnarySetEvaluator<Kind, Category, IdentityTag, ConceptTag>,
                                         UnarySetEvaluator<Kind, Category, TransitiveClosureTag, RoleTag>,
                                         UnarySetEvaluator<Kind, Category, ReflexiveTransitiveClosureTag, RoleTag>,
                                         BinarySetEvaluator<Kind, Category, IntersectionTag, RoleTag, RoleTag>,
                                         BinarySetEvaluator<Kind, Category, UnionTag, RoleTag, RoleTag>,
                                         BinarySetEvaluator<Kind, Category, RestrictionTag, RoleTag, ConceptTag>,
                                         BinarySetEvaluator<Kind, Category, CompositionTag, RoleTag, RoleTag>,
                                         ProjectionEvaluator<Kind, Category>>;
    using BooleanOperations = ygg::TypeList<AtomicEvaluator<Kind, Category, tyr::formalism::FluentTag>,
                                            AtomicEvaluator<Kind, Category, tyr::formalism::DerivedTag>,
                                            NonemptyEvaluator<Kind>,
                                            LogicalNotEvaluator<Kind>,
                                            ScalarBinaryEvaluator<Kind, Category, AndTag, BooleanTag>,
                                            ScalarBinaryEvaluator<Kind, Category, OrTag, BooleanTag>,
                                            ScalarBinaryEvaluator<Kind, Category, BooleanEqTag, BooleanTag>,
                                            ScalarBinaryEvaluator<Kind, Category, BooleanNeTag, BooleanTag>,
                                            ScalarBinaryEvaluator<Kind, Category, BooleanLtTag, BooleanTag>,
                                            ScalarBinaryEvaluator<Kind, Category, BooleanLeTag, BooleanTag>,
                                            ScalarBinaryEvaluator<Kind, Category, BooleanGtTag, BooleanTag>,
                                            ScalarBinaryEvaluator<Kind, Category, BooleanGeTag, BooleanTag>,
                                            ScalarBinaryEvaluator<Kind, Category, NumericalEqTag, NumericalTag>,
                                            ScalarBinaryEvaluator<Kind, Category, NumericalNeTag, NumericalTag>,
                                            ScalarBinaryEvaluator<Kind, Category, NumericalLtTag, NumericalTag>,
                                            ScalarBinaryEvaluator<Kind, Category, NumericalLeTag, NumericalTag>,
                                            ScalarBinaryEvaluator<Kind, Category, NumericalGtTag, NumericalTag>,
                                            ScalarBinaryEvaluator<Kind, Category, NumericalGeTag, NumericalTag>>;
    using NumericalOperations = ygg::TypeList<CountEvaluator<Kind>,
                                              RoleEdgeDistanceEvaluator<Kind>,
                                              QueryEdgeDistanceEvaluator<Kind>,
                                              ScalarBinaryEvaluator<Kind, Category, AddTag, NumericalTag>,
                                              ScalarBinaryEvaluator<Kind, Category, SubTag, NumericalTag>,
                                              ScalarBinaryEvaluator<Kind, Category, MulTag, NumericalTag>,
                                              ScalarBinaryEvaluator<Kind, Category, DivTag, NumericalTag>,
                                              ScalarBinaryEvaluator<Kind, Category, MinTag, NumericalTag>,
                                              ScalarBinaryEvaluator<Kind, Category, MaxTag, NumericalTag>>;
    using Operations = std::conditional_t<std::same_as<Category, ConceptTag>,
                                          ConceptOperations,
                                          std::conditional_t<std::same_as<Category, RoleTag>,
                                                             RoleOperations,
                                                             std::conditional_t<std::same_as<Category, BooleanTag>, BooleanOperations, NumericalOperations>>>;
    using InvocationOperations = std::conditional_t<true,
                                                    std::conditional_t<ConceptOrRoleTag<Category>,
                                                                       ygg::TypeList<RegisterEvaluator<Kind, Category>, ArgumentEvaluator<Kind, Category>>,
                                                                       ygg::TypeList<ArgumentEvaluator<Kind, Category>>>,
                                                    ygg::TypeList<>>;
    using Evaluators =
        ygg::ApplyTypeListT<std::variant, ygg::ConcatTypeListsT<ygg::TypeList<StaticEvaluator<Kind, Category>>, Operations, InvocationOperations>>;

    FamilyConstructorView<Category> m_expression;
    std::vector<ygg::uint_t> m_dependencies;
    Evaluators m_evaluator;

    template<ConceptOrRoleTag Projected, typename C>
    Evaluators prepare(ygg::View<ygg::Index<QueryProjection<Projected>>, C> expression, GraphBuilder<Kind>& graph)
    {
        return ProjectionEvaluator<Kind, Category>(graph.prepare(expression.get_arg(), m_dependencies), expression.get_data().plan);
    }

    Evaluators prepare_distance(FamilyNumericalView<DistanceTag> expression, FamilyConstructorView<RoleTag>, GraphBuilder<Kind>& graph)
    {
        return RoleEdgeDistanceEvaluator<Kind>(expression, graph, m_dependencies);
    }
    /// Query edges can range over pair vertices; maintaining them would materialize every edge.
    Evaluators prepare_distance(FamilyNumericalView<DistanceTag> expression, FamilyQueryView<> edges, GraphBuilder<Kind>& graph)
    {
        return QueryEdgeDistanceEvaluator<Kind>(expression, edges, graph, m_dependencies);
    }

    template<template<typename> typename Expression, typename Tag, typename C>
    Evaluators prepare(ygg::View<ygg::Index<Expression<Tag>>, C> expression, GraphBuilder<Kind>& graph)
    {
        if constexpr (std::same_as<Tag, AtomicStateTag<tyr::formalism::FluentTag>>)
            return AtomicEvaluator<Kind, Category, tyr::formalism::FluentTag>(expression.get_predicate(), expression.get_polarity());
        else if constexpr (std::same_as<Tag, AtomicStateTag<tyr::formalism::DerivedTag>>)
            return AtomicEvaluator<Kind, Category, tyr::formalism::DerivedTag>(expression.get_predicate(), expression.get_polarity());
        else if constexpr (std::same_as<Tag, RegisterTag>)
            return RegisterEvaluator<Kind, Category>(expression.get_register().get_identifier());
        else if constexpr (std::same_as<Tag, ArgumentTag<Category>>)
            return ArgumentEvaluator<Kind, Category>(expression.get_argument().get_identifier());
        else if constexpr (std::same_as<Tag, DistanceTag>)
            return ygg::visit([&](auto edges) { return prepare_distance(expression, edges, graph); }, expression.get_mid());
        else if constexpr (std::same_as<Tag, CountTag>)
            return ygg::visit([&](auto child) -> Evaluators { return CountEvaluator<Kind>(graph.prepare(child, m_dependencies)); }, expression.get_arg());
        else if constexpr (std::same_as<Tag, NonemptyTag>)
            return ygg::visit([&](auto child) -> Evaluators { return NonemptyEvaluator<Kind>(graph.prepare(child, m_dependencies)); }, expression.get_arg());
        else if constexpr (std::same_as<Tag, QualifiedAtLeastNumberRestrictionTag> || std::same_as<Tag, QualifiedAtMostNumberRestrictionTag>
                           || std::same_as<Tag, QualifiedExactNumberRestrictionTag>)
            return QualifiedNumberRestrictionEvaluator<Kind, Tag>(graph.prepare(expression.get_role(), m_dependencies),
                                                                  graph.prepare(expression.get_concept(), m_dependencies),
                                                                  expression.get_n());
        else if constexpr (std::same_as<Tag, AtLeastNumberRestrictionTag> || std::same_as<Tag, AtMostNumberRestrictionTag>
                           || std::same_as<Tag, ExactNumberRestrictionTag>)
            return NumberRestrictionEvaluator<Kind, Tag>(graph.prepare(expression.get_role(), m_dependencies), expression.get_n());
        else if constexpr (std::same_as<Tag, RoleFillersTag>)
            return FillersEvaluator<Kind>(graph.prepare(expression.get_role(), m_dependencies), expression);
        else if constexpr (NumericalBinaryTag<Tag>)
            return ScalarBinaryEvaluator<Kind, Category, Tag, NumericalTag>(graph.prepare(expression.get_lhs(), m_dependencies),
                                                                            graph.prepare(expression.get_rhs(), m_dependencies));
        else if constexpr (LogicalBinaryTag<Tag>)
            return ScalarBinaryEvaluator<Kind, Category, Tag, BooleanTag>(graph.prepare(expression.get_lhs(), m_dependencies),
                                                                          graph.prepare(expression.get_rhs(), m_dependencies));
        else if constexpr (ComparisonTag<Tag>)
            return ScalarBinaryEvaluator<Kind, Category, Tag, comparison_operand_t<Tag>>(graph.prepare(expression.get_lhs(), m_dependencies),
                                                                                         graph.prepare(expression.get_rhs(), m_dependencies));
        else if constexpr (std::same_as<Tag, NotTag>)
            return LogicalNotEvaluator<Kind>(graph.prepare(expression.get_arg(), m_dependencies));
        else if constexpr (std::same_as<Tag, IntersectionTag> || std::same_as<Tag, UnionTag>)
            return BinarySetEvaluator<Kind, Category, Tag, Category, Category>(graph.prepare(expression.get_lhs(), m_dependencies),
                                                                               graph.prepare(expression.get_rhs(), m_dependencies));
        else if constexpr (std::same_as<Tag, NegationTag> || std::same_as<Tag, ComplementTag> || std::same_as<Tag, InverseTag>
                           || std::same_as<Tag, TransitiveClosureTag> || std::same_as<Tag, ReflexiveTransitiveClosureTag>)
            return UnarySetEvaluator<Kind, Category, Tag, Category>(graph.prepare(expression.get_arg(), m_dependencies));
        else if constexpr (std::same_as<Tag, IdentityTag>)
            return UnarySetEvaluator<Kind, Category, Tag, ConceptTag>(graph.prepare(expression.get_arg(), m_dependencies));
        else if constexpr (std::same_as<Tag, RestrictionTag> || std::same_as<Tag, ValueRestrictionTag> || std::same_as<Tag, ExistentialQuantificationTag>)
            return BinarySetEvaluator<Kind, Category, Tag, RoleTag, ConceptTag>(graph.prepare(expression.get_lhs(), m_dependencies),
                                                                                graph.prepare(expression.get_rhs(), m_dependencies));
        else if constexpr (std::same_as<Tag, CompositionTag> || std::same_as<Tag, RoleValueMapTag> || std::same_as<Tag, AgreementTag>)
            return BinarySetEvaluator<Kind, Category, Tag, RoleTag, RoleTag>(graph.prepare(expression.get_lhs(), m_dependencies),
                                                                             graph.prepare(expression.get_rhs(), m_dependencies));
        else
            throw std::logic_error("Incremental evaluation: static constructor reached dynamic preparation.");
    }

    const DenotationState<Category>& state() const
    {
        return std::visit([](const auto& evaluator) -> const DenotationState<Category>& { return evaluator.value; }, m_evaluator);
    }
};

template<tyr::TaskKind Kind, CategoryTag Category>
FeatureNode<Kind, Category>::FeatureNode(FamilyConstructorView<Category> expression, GraphBuilder<Kind>& graph) :
    m_expression(expression),
    m_evaluator(expression.is_static() ? Evaluators(StaticEvaluator<Kind, Category>(expression)) :
                                         ygg::visit([&](auto concrete) { return prepare(concrete, graph); }, expression.get_variant()))
{
}

template<tyr::TaskKind Kind, CategoryTag Category>
template<StateEvaluationContextConcept<Kind> Context>
void FeatureNode<Kind, Category>::initialize(EvaluationGraph<Kind>& graph, Context& context)
{
    std::visit(
        [&](auto& evaluator)
        {
            evaluator.initialize(graph, context);
            evaluator.value.clear_delta();
        },
        m_evaluator);
}

template<tyr::TaskKind Kind, CategoryTag Category>
void FeatureNode<Kind, Category>::update(EvaluationGraph<Kind>& graph, const Delta& delta, ygg::database::Workspace& workspace)
{
    std::visit(
        [&](auto& evaluator)
        {
            evaluator.value.clear_delta();
            evaluator.update(graph, delta, workspace);
        },
        m_evaluator);
}

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
