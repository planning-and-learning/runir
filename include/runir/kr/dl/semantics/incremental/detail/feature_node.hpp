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
template<FamilyTag Family, tyr::TaskKind Kind, CategoryTag Category>
class FeatureNode
{
public:
    FeatureNode(FamilyConstructorView<Family, Category> expression, EvaluationGraph<Family, Kind>& graph);
    FamilyConstructorView<Family, Category> get_expression() const noexcept { return m_expression; }
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(EvaluationGraph<Family, Kind>& graph, Context& context);
    void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>& delta, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>& workspace);
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
    using ConceptOperations = ygg::TypeList<AtomicEvaluator<Family, Kind, Category, tyr::formalism::FluentTag>,
                                            AtomicEvaluator<Family, Kind, Category, tyr::formalism::DerivedTag>,
                                            UnarySetEvaluator<Family, Kind, Category, NegationTag, ConceptTag>,
                                            BinarySetEvaluator<Family, Kind, Category, IntersectionTag, ConceptTag, ConceptTag>,
                                            BinarySetEvaluator<Family, Kind, Category, UnionTag, ConceptTag, ConceptTag>,
                                            BinarySetEvaluator<Family, Kind, Category, ValueRestrictionTag, RoleTag, ConceptTag>,
                                            BinarySetEvaluator<Family, Kind, Category, ExistentialQuantificationTag, RoleTag, ConceptTag>,
                                            BinarySetEvaluator<Family, Kind, Category, RoleValueMapTag, RoleTag, RoleTag>,
                                            BinarySetEvaluator<Family, Kind, Category, AgreementTag, RoleTag, RoleTag>,
                                            NumberRestrictionEvaluator<Family, Kind, AtLeastNumberRestrictionTag>,
                                            NumberRestrictionEvaluator<Family, Kind, AtMostNumberRestrictionTag>,
                                            NumberRestrictionEvaluator<Family, Kind, ExactNumberRestrictionTag>,
                                            QualifiedNumberRestrictionEvaluator<Family, Kind, QualifiedAtLeastNumberRestrictionTag>,
                                            QualifiedNumberRestrictionEvaluator<Family, Kind, QualifiedAtMostNumberRestrictionTag>,
                                            QualifiedNumberRestrictionEvaluator<Family, Kind, QualifiedExactNumberRestrictionTag>,
                                            FillersEvaluator<Family, Kind>,
                                            ProjectionEvaluator<Family, Kind, Category>>;
    using RoleOperations = ygg::TypeList<AtomicEvaluator<Family, Kind, Category, tyr::formalism::FluentTag>,
                                         AtomicEvaluator<Family, Kind, Category, tyr::formalism::DerivedTag>,
                                         UnarySetEvaluator<Family, Kind, Category, ComplementTag, RoleTag>,
                                         UnarySetEvaluator<Family, Kind, Category, InverseTag, RoleTag>,
                                         UnarySetEvaluator<Family, Kind, Category, IdentityTag, ConceptTag>,
                                         UnarySetEvaluator<Family, Kind, Category, TransitiveClosureTag, RoleTag>,
                                         UnarySetEvaluator<Family, Kind, Category, ReflexiveTransitiveClosureTag, RoleTag>,
                                         BinarySetEvaluator<Family, Kind, Category, IntersectionTag, RoleTag, RoleTag>,
                                         BinarySetEvaluator<Family, Kind, Category, UnionTag, RoleTag, RoleTag>,
                                         BinarySetEvaluator<Family, Kind, Category, RestrictionTag, RoleTag, ConceptTag>,
                                         BinarySetEvaluator<Family, Kind, Category, CompositionTag, RoleTag, RoleTag>,
                                         ProjectionEvaluator<Family, Kind, Category>>;
    using BooleanOperations = ygg::TypeList<AtomicEvaluator<Family, Kind, Category, tyr::formalism::FluentTag>,
                                            AtomicEvaluator<Family, Kind, Category, tyr::formalism::DerivedTag>,
                                            NonemptyEvaluator<Family, Kind>,
                                            LogicalNotEvaluator<Family, Kind>,
                                            ScalarBinaryEvaluator<Family, Kind, Category, AndTag, BooleanTag>,
                                            ScalarBinaryEvaluator<Family, Kind, Category, OrTag, BooleanTag>,
                                            ScalarBinaryEvaluator<Family, Kind, Category, BooleanEqTag, BooleanTag>,
                                            ScalarBinaryEvaluator<Family, Kind, Category, BooleanNeTag, BooleanTag>,
                                            ScalarBinaryEvaluator<Family, Kind, Category, BooleanLtTag, BooleanTag>,
                                            ScalarBinaryEvaluator<Family, Kind, Category, BooleanLeTag, BooleanTag>,
                                            ScalarBinaryEvaluator<Family, Kind, Category, BooleanGtTag, BooleanTag>,
                                            ScalarBinaryEvaluator<Family, Kind, Category, BooleanGeTag, BooleanTag>,
                                            ScalarBinaryEvaluator<Family, Kind, Category, NumericalEqTag, NumericalTag>,
                                            ScalarBinaryEvaluator<Family, Kind, Category, NumericalNeTag, NumericalTag>,
                                            ScalarBinaryEvaluator<Family, Kind, Category, NumericalLtTag, NumericalTag>,
                                            ScalarBinaryEvaluator<Family, Kind, Category, NumericalLeTag, NumericalTag>,
                                            ScalarBinaryEvaluator<Family, Kind, Category, NumericalGtTag, NumericalTag>,
                                            ScalarBinaryEvaluator<Family, Kind, Category, NumericalGeTag, NumericalTag>>;
    using NumericalOperations = ygg::TypeList<CountEvaluator<Family, Kind>,
                                              DistanceFeatureEvaluator<Family, Kind>,
                                              ScalarBinaryEvaluator<Family, Kind, Category, AddTag, NumericalTag>,
                                              ScalarBinaryEvaluator<Family, Kind, Category, SubTag, NumericalTag>,
                                              ScalarBinaryEvaluator<Family, Kind, Category, MulTag, NumericalTag>,
                                              ScalarBinaryEvaluator<Family, Kind, Category, DivTag, NumericalTag>,
                                              ScalarBinaryEvaluator<Family, Kind, Category, MinTag, NumericalTag>,
                                              ScalarBinaryEvaluator<Family, Kind, Category, MaxTag, NumericalTag>>;
    using Operations = std::conditional_t<std::same_as<Category, ConceptTag>,
                                          ConceptOperations,
                                          std::conditional_t<std::same_as<Category, RoleTag>,
                                                             RoleOperations,
                                                             std::conditional_t<std::same_as<Category, BooleanTag>, BooleanOperations, NumericalOperations>>>;
    using InvocationOperations =
        std::conditional_t<std::same_as<Family, runir::kr::ExtFamilyTag>,
                           std::conditional_t<ConceptOrRoleTag<Category>,
                                              ygg::TypeList<RegisterEvaluator<Family, Kind, Category>, ArgumentEvaluator<Family, Kind, Category>>,
                                              ygg::TypeList<ArgumentEvaluator<Family, Kind, Category>>>,
                           ygg::TypeList<>>;
    using Evaluators =
        ygg::ApplyTypeListT<std::variant, ygg::ConcatTypeListsT<ygg::TypeList<StaticEvaluator<Family, Kind, Category>>, Operations, InvocationOperations>>;

    FamilyConstructorView<Family, Category> m_expression;
    std::vector<ygg::uint_t> m_dependencies;
    Evaluators m_evaluator;

    template<ConceptOrRoleTag Projected, typename C>
    Evaluators prepare(ygg::View<ygg::Index<QueryProjection<Family, Projected>>, C> expression, EvaluationGraph<Family, Kind>& graph)
    {
        return ProjectionEvaluator<Family, Kind, Category>(graph.prepare(expression.get_arg(), m_dependencies), expression.get_data().plan);
    }

    template<template<typename, typename> typename Expression, typename Tag, typename C>
    Evaluators prepare(ygg::View<ygg::Index<Expression<Family, Tag>>, C> expression, EvaluationGraph<Family, Kind>& graph)
    {
        if constexpr (std::same_as<Tag, AtomicStateTag<tyr::formalism::FluentTag>>)
            return AtomicEvaluator<Family, Kind, Category, tyr::formalism::FluentTag>(expression.get_predicate(), expression.get_polarity());
        else if constexpr (std::same_as<Tag, AtomicStateTag<tyr::formalism::DerivedTag>>)
            return AtomicEvaluator<Family, Kind, Category, tyr::formalism::DerivedTag>(expression.get_predicate(), expression.get_polarity());
        else if constexpr (std::same_as<Tag, RegisterTag>)
            return RegisterEvaluator<Family, Kind, Category>(expression.get_register().get_identifier());
        else if constexpr (std::same_as<Tag, ArgumentTag<Category>>)
            return ArgumentEvaluator<Family, Kind, Category>(expression.get_argument().get_identifier());
        else if constexpr (std::same_as<Tag, DistanceTag>)
            return DistanceFeatureEvaluator<Family, Kind>(graph.prepare(expression.get_lhs(), m_dependencies),
                                                          graph.prepare(expression.get_mid(), m_dependencies),
                                                          graph.prepare(expression.get_rhs(), m_dependencies));
        else if constexpr (std::same_as<Tag, CountTag>)
            return ygg::visit([&](auto child) -> Evaluators { return CountEvaluator<Family, Kind>(graph.prepare(child, m_dependencies)); },
                              expression.get_arg());
        else if constexpr (std::same_as<Tag, NonemptyTag>)
            return ygg::visit([&](auto child) -> Evaluators { return NonemptyEvaluator<Family, Kind>(graph.prepare(child, m_dependencies)); },
                              expression.get_arg());
        else if constexpr (std::same_as<Tag, QualifiedAtLeastNumberRestrictionTag> || std::same_as<Tag, QualifiedAtMostNumberRestrictionTag>
                           || std::same_as<Tag, QualifiedExactNumberRestrictionTag>)
            return QualifiedNumberRestrictionEvaluator<Family, Kind, Tag>(graph.prepare(expression.get_role(), m_dependencies),
                                                                          graph.prepare(expression.get_concept(), m_dependencies),
                                                                          expression.get_n());
        else if constexpr (std::same_as<Tag, AtLeastNumberRestrictionTag> || std::same_as<Tag, AtMostNumberRestrictionTag>
                           || std::same_as<Tag, ExactNumberRestrictionTag>)
            return NumberRestrictionEvaluator<Family, Kind, Tag>(graph.prepare(expression.get_role(), m_dependencies), expression.get_n());
        else if constexpr (std::same_as<Tag, RoleFillersTag>)
            return FillersEvaluator<Family, Kind>(graph.prepare(expression.get_role(), m_dependencies), expression);
        else if constexpr (NumericalBinaryTag<Tag>)
            return ScalarBinaryEvaluator<Family, Kind, Category, Tag, NumericalTag>(graph.prepare(expression.get_lhs(), m_dependencies),
                                                                                    graph.prepare(expression.get_rhs(), m_dependencies));
        else if constexpr (LogicalBinaryTag<Tag>)
            return ScalarBinaryEvaluator<Family, Kind, Category, Tag, BooleanTag>(graph.prepare(expression.get_lhs(), m_dependencies),
                                                                                  graph.prepare(expression.get_rhs(), m_dependencies));
        else if constexpr (ComparisonTag<Tag>)
            return ScalarBinaryEvaluator<Family, Kind, Category, Tag, comparison_operand_t<Tag>>(graph.prepare(expression.get_lhs(), m_dependencies),
                                                                                                 graph.prepare(expression.get_rhs(), m_dependencies));
        else if constexpr (std::same_as<Tag, NotTag>)
            return LogicalNotEvaluator<Family, Kind>(graph.prepare(expression.get_arg(), m_dependencies));
        else if constexpr (std::same_as<Tag, IntersectionTag> || std::same_as<Tag, UnionTag>)
            return BinarySetEvaluator<Family, Kind, Category, Tag, Category, Category>(graph.prepare(expression.get_lhs(), m_dependencies),
                                                                                       graph.prepare(expression.get_rhs(), m_dependencies));
        else if constexpr (std::same_as<Tag, NegationTag> || std::same_as<Tag, ComplementTag> || std::same_as<Tag, InverseTag>
                           || std::same_as<Tag, TransitiveClosureTag> || std::same_as<Tag, ReflexiveTransitiveClosureTag>)
            return UnarySetEvaluator<Family, Kind, Category, Tag, Category>(graph.prepare(expression.get_arg(), m_dependencies));
        else if constexpr (std::same_as<Tag, IdentityTag>)
            return UnarySetEvaluator<Family, Kind, Category, Tag, ConceptTag>(graph.prepare(expression.get_arg(), m_dependencies));
        else if constexpr (std::same_as<Tag, RestrictionTag> || std::same_as<Tag, ValueRestrictionTag> || std::same_as<Tag, ExistentialQuantificationTag>)
            return BinarySetEvaluator<Family, Kind, Category, Tag, RoleTag, ConceptTag>(graph.prepare(expression.get_lhs(), m_dependencies),
                                                                                        graph.prepare(expression.get_rhs(), m_dependencies));
        else if constexpr (std::same_as<Tag, CompositionTag> || std::same_as<Tag, RoleValueMapTag> || std::same_as<Tag, AgreementTag>)
            return BinarySetEvaluator<Family, Kind, Category, Tag, RoleTag, RoleTag>(graph.prepare(expression.get_lhs(), m_dependencies),
                                                                                     graph.prepare(expression.get_rhs(), m_dependencies));
        else
            throw std::logic_error("Incremental evaluation: static constructor reached dynamic preparation.");
    }

    const DenotationState<Category>& state() const
    {
        return std::visit([](const auto& evaluator) -> const DenotationState<Category>& { return evaluator.value; }, m_evaluator);
    }
};

template<FamilyTag Family, tyr::TaskKind Kind, CategoryTag Category>
FeatureNode<Family, Kind, Category>::FeatureNode(FamilyConstructorView<Family, Category> expression, EvaluationGraph<Family, Kind>& graph) :
    m_expression(expression),
    m_evaluator(expression.is_static() ? Evaluators(StaticEvaluator<Family, Kind, Category>(expression)) :
                                         ygg::visit([&](auto concrete) { return prepare(concrete, graph); }, expression.get_variant()))
{
}

template<FamilyTag Family, tyr::TaskKind Kind, CategoryTag Category>
template<StateEvaluationContextConcept<Family, Kind> Context>
void FeatureNode<Family, Kind, Category>::initialize(EvaluationGraph<Family, Kind>& graph, Context& context)
{
    std::visit(
        [&](auto& evaluator)
        {
            evaluator.initialize(graph, context);
            evaluator.value.clear_delta();
        },
        m_evaluator);
}

template<FamilyTag Family, tyr::TaskKind Kind, CategoryTag Category>
void FeatureNode<Family, Kind, Category>::update(EvaluationGraph<Family, Kind>& graph,
                                                 const Delta<Family>& delta,
                                                 ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>& workspace)
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
