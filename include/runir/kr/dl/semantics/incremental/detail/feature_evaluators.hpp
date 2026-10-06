#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_FEATURE_EVALUATORS_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_FEATURE_EVALUATORS_HPP_

#include "runir/kr/dl/semantics/ext/evaluation.hpp"
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/distance.hpp"
#include "runir/kr/dl/semantics/incremental/detail/set_operations.hpp"

#include <variant>
#include <yggdrasil/containers/span.hpp>
#include <yggdrasil/database/incremental/projection.hpp>

namespace runir::kr::dl::semantics::incremental::detail
{

template<ConceptOrRoleTag Category, tyr::formalism::FactKind Fact>
DenotationElementView<Category> atom_element(tyr::formalism::planning::AtomView<tyr::GroundTag, Fact> atom)
{
    const auto objects = atom.get_row().get_objects();
    if constexpr (std::same_as<Category, ConceptTag>)
        return objects[0];
    else
        return std::pair(objects[0], objects[1]);
}

template<CategoryTag Category, tyr::formalism::FactKind Fact>
void update_atomic(DenotationState<Category>& output,
                   tyr::formalism::planning::PredicateView<Fact> predicate,
                   bool polarity,
                   std::span<const tyr::formalism::planning::AtomView<tyr::GroundTag, Fact>> added,
                   std::span<const tyr::formalism::planning::AtomView<tyr::GroundTag, Fact>> removed)
{
    const auto present = [&](auto atom)
    {
        if constexpr (std::same_as<Category, BooleanTag>)
            return output.get_value() == polarity;
        else
            return output.contains(atom_element<Category>(atom)) == polarity;
    };
    const auto change = [&](auto atom, bool present)
    {
        if constexpr (std::same_as<Category, BooleanTag>)
            output.set(present == polarity);
        else
            output.set(atom_element<Category>(atom), present == polarity);
    };
    for (const auto atom : added)
        if (atom.get_predicate() == predicate && present(atom))
            throw std::invalid_argument("Incremental DL: added atom is already present.");
    for (const auto atom : removed)
        if (atom.get_predicate() == predicate)
        {
            if (!present(atom))
                throw std::invalid_argument("Incremental DL: removed atom is absent.");
            change(atom, false);
        }
    for (const auto atom : added)
        if (atom.get_predicate() == predicate)
            change(atom, true);
}

template<ConceptOrRoleTag Category>
void update_register(DenotationState<Category>& output,
                     RegisterIdentifier<Category> slot,
                     const std::vector<std::pair<RegisterIdentifier<Category>, DenotationElementView<Category>>>& added,
                     const std::vector<std::pair<RegisterIdentifier<Category>, DenotationElementView<Category>>>& removed)
{
    for (const auto& [identifier, value] : added)
        if (identifier == slot && output.contains(value))
            throw std::invalid_argument("Incremental DL: added register value is already present.");
    for (const auto& [identifier, value] : removed)
        if (identifier == slot)
        {
            if (!output.contains(value))
                throw std::invalid_argument("Incremental DL: removed register value is absent.");
            output.set(value, false);
        }
    for (const auto& [identifier, value] : added)
        if (identifier == slot)
            output.set(value, true);
    if (output.size() > 1)
        throw std::invalid_argument("Incremental DL: a register can hold at most one value.");
}

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
    struct Static
    {
        FamilyConstructorView<Family, Category> expression;
        DenotationState<Category> value;
        bool initialized = false;

        explicit Static(FamilyConstructorView<Family, Category> expression) : expression(expression) {}
        template<StateEvaluationContextConcept<Family, Kind> Context>
        void initialize(EvaluationGraph<Family, Kind>&, Context& context)
        {
            if (!initialized)
            {
                value.assign(semantics::evaluate<Kind>(expression, context));
                initialized = true;
            }
        }
        void update(EvaluationGraph<Family, Kind>&, const Delta<Family>&, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>&) {}
    };

    template<tyr::formalism::FactKind Fact>
    struct Atomic
    {
        tyr::formalism::planning::PredicateView<Fact> predicate;
        bool polarity;
        DenotationState<Category> value;

        Atomic(tyr::formalism::planning::PredicateView<Fact> predicate, bool polarity) : predicate(predicate), polarity(polarity) {}
        template<StateEvaluationContextConcept<Family, Kind> Context>
        void initialize(EvaluationGraph<Family, Kind>& graph, Context& context)
        {
            if (!graph.repository().contains(predicate))
                throw std::invalid_argument("Incremental DL: predicate does not belong to the task repository.");
            if constexpr (std::same_as<Category, BooleanTag>)
            {
                bool present = false;
                for ([[maybe_unused]] const auto atom : tyr::planning::get_atoms_view<Kind>(context.get_state(), predicate))
                    present = true;
                value.set(present == polarity);
            }
            else
            {
                value.initialize(semantics::detail::num_objects<Kind, Family>(context));
                for (const auto atom : tyr::planning::get_atoms_view<Kind>(context.get_state(), predicate))
                    value.set(atom_element<Category>(atom), true);
                if (!polarity)
                    value.flip();
            }
        }
        void update(EvaluationGraph<Family, Kind>&, const Delta<Family>& delta, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>&)
        {
            if constexpr (std::same_as<Fact, tyr::formalism::FluentTag>)
                update_atomic<Category, Fact>(value, predicate, polarity, delta.added.fluent_atoms, delta.removed.fluent_atoms);
            else
                update_atomic<Category, Fact>(value, predicate, polarity, delta.added.derived_atoms, delta.removed.derived_atoms);
        }
    };

    struct Register
    {
        RegisterIdentifier<Category> identifier;
        DenotationState<Category> value;

        explicit Register(RegisterIdentifier<Category> identifier) : identifier(identifier) {}
        template<StateEvaluationContextConcept<Family, Kind> Context>
        void initialize(EvaluationGraph<Family, Kind>&, Context& context)
        {
            value.initialize(semantics::detail::num_objects<Kind, Family>(context));
            const auto slot = context.registers().at(identifier);
            if (slot)
            {
                if constexpr (std::same_as<Category, ConceptTag>)
                    value.set(slot.value(), true);
                else
                    value.set(std::pair(slot.value().get_first(), slot.value().get_second()), true);
            }
        }
        void update(EvaluationGraph<Family, Kind>&, const Delta<Family>& delta, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>&)
        {
            if constexpr (std::same_as<Category, ConceptTag>)
                update_register(value, identifier, delta.added.concept_registers, delta.removed.concept_registers);
            else
                update_register(value, identifier, delta.added.role_registers, delta.removed.role_registers);
        }
    };

    struct Argument
    {
        ArgumentIdentifier<Category> identifier;
        DenotationState<Category> value;

        explicit Argument(ArgumentIdentifier<Category> identifier) : identifier(identifier) {}
        template<StateEvaluationContextConcept<Family, Kind> Context>
        void initialize(EvaluationGraph<Family, Kind>&, Context& context)
        {
            value.assign(context.arguments().at(identifier));
        }
        void update(EvaluationGraph<Family, Kind>&, const Delta<Family>&, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>&) {}
    };

    template<typename Tag, ConceptOrRoleTag Input>
    struct UnarySet
    {
        EvaluationIndex<Input> child;
        DenotationState<Category> value;

        explicit UnarySet(EvaluationIndex<Input> child) : child(child) {}
        template<StateEvaluationContextConcept<Family, Kind> Context>
        void initialize(EvaluationGraph<Family, Kind>& graph, Context&)
        {
            initialize_set(Tag {}, value, graph.result(child), graph.change(child), graph.set_workspace());
        }
        void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>&)
        {
            update_set(Tag {}, value, graph.result(child), graph.change(child), graph.set_workspace());
        }
    };

    template<typename Tag, ConceptOrRoleTag Left, ConceptOrRoleTag Right>
    struct BinarySet
    {
        EvaluationIndex<Left> lhs;
        EvaluationIndex<Right> rhs;
        DenotationState<Category> value;

        BinarySet(EvaluationIndex<Left> lhs, EvaluationIndex<Right> rhs) : lhs(lhs), rhs(rhs) {}
        template<StateEvaluationContextConcept<Family, Kind> Context>
        void initialize(EvaluationGraph<Family, Kind>& graph, Context&)
        {
            initialize_set(Tag {}, value, graph.result(lhs), graph.change(lhs), graph.result(rhs), graph.change(rhs), graph.set_workspace());
        }
        void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>&)
        {
            update_set(Tag {}, value, graph.result(lhs), graph.change(lhs), graph.result(rhs), graph.change(rhs), graph.set_workspace());
        }
    };

    template<typename Tag>
    struct NumberRestriction
    {
        EvaluationIndex<RoleTag> role;
        ygg::uint_t threshold;
        DenotationState<Category> value;

        NumberRestriction(EvaluationIndex<RoleTag> role, ygg::uint_t threshold) : role(role), threshold(threshold) {}
        template<StateEvaluationContextConcept<Family, Kind> Context>
        void initialize(EvaluationGraph<Family, Kind>& graph, Context&)
        {
            initialize_set(Tag {}, value, graph.result(role), graph.change(role), threshold, graph.set_workspace());
        }
        void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>&)
        {
            update_set(Tag {}, value, graph.result(role), graph.change(role), threshold, graph.set_workspace());
        }
    };

    template<typename Tag>
    struct QualifiedNumberRestriction
    {
        EvaluationIndex<RoleTag> role;
        EvaluationIndex<ConceptTag> qualifying_concept;
        ygg::uint_t threshold;
        DenotationState<Category> value;

        QualifiedNumberRestriction(EvaluationIndex<RoleTag> role, EvaluationIndex<ConceptTag> qualifying_concept, ygg::uint_t threshold) :
            role(role),
            qualifying_concept(qualifying_concept),
            threshold(threshold)
        {
        }
        template<StateEvaluationContextConcept<Family, Kind> Context>
        void initialize(EvaluationGraph<Family, Kind>& graph, Context&)
        {
            initialize_set(Tag {},
                           value,
                           graph.result(role),
                           graph.change(role),
                           graph.result(qualifying_concept),
                           graph.change(qualifying_concept),
                           threshold,
                           graph.set_workspace());
        }
        void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>&)
        {
            update_set(Tag {},
                       value,
                       graph.result(role),
                       graph.change(role),
                       graph.result(qualifying_concept),
                       graph.change(qualifying_concept),
                       threshold,
                       graph.set_workspace());
        }
    };

    struct Fillers
    {
        EvaluationIndex<RoleTag> role;
        FamilyConceptView<Family, RoleFillersTag> expression;
        DenotationState<Category> value;

        Fillers(EvaluationIndex<RoleTag> role, FamilyConceptView<Family, RoleFillersTag> expression) : role(role), expression(expression) {}
        template<StateEvaluationContextConcept<Family, Kind> Context>
        void initialize(EvaluationGraph<Family, Kind>& graph, Context&)
        {
            initialize_set(RoleFillersTag {}, value, graph.result(role), graph.change(role), expression.get_objects(), graph.set_workspace());
        }
        void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>&)
        {
            update_set(RoleFillersTag {}, value, graph.result(role), graph.change(role), expression.get_objects(), graph.set_workspace());
        }
    };

    template<typename Tag, BooleanOrNumericalTag Input>
    struct ScalarBinary
    {
        EvaluationIndex<Input> lhs;
        EvaluationIndex<Input> rhs;
        DenotationState<Category> value;

        ScalarBinary(EvaluationIndex<Input> lhs, EvaluationIndex<Input> rhs) : lhs(lhs), rhs(rhs) {}
        void refresh(EvaluationGraph<Family, Kind>& graph)
        {
            if constexpr (NumericalBinaryTag<Tag>)
                value.set(semantics::detail::apply_numerical_binary<Tag>(graph.result(lhs).get(), graph.result(rhs).get()));
            else if constexpr (LogicalBinaryTag<Tag>)
                value.set(semantics::detail::apply_logical_binary<Tag>(graph.result(lhs).get(), graph.result(rhs).get()));
            else
                value.set(semantics::detail::apply_comparison<Tag>(graph.result(lhs).get(), graph.result(rhs).get()));
        }
        template<StateEvaluationContextConcept<Family, Kind> Context>
        void initialize(EvaluationGraph<Family, Kind>& graph, Context&)
        {
            refresh(graph);
        }
        void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>&)
        {
            refresh(graph);
        }
    };

    struct LogicalNot
    {
        EvaluationIndex<BooleanTag> child;
        DenotationState<Category> value;

        explicit LogicalNot(EvaluationIndex<BooleanTag> child) : child(child) {}
        template<StateEvaluationContextConcept<Family, Kind> Context>
        void initialize(EvaluationGraph<Family, Kind>& graph, Context&)
        {
            value.set(!graph.result(child).get());
        }
        void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>&)
        {
            value.set(!graph.result(child).get());
        }
    };

    struct Cardinality
    {
        std::variant<EvaluationIndex<ConceptTag>, EvaluationIndex<RoleTag>, QueryEvaluationIndex> child;
        DenotationState<Category> value;

        explicit Cardinality(std::variant<EvaluationIndex<ConceptTag>, EvaluationIndex<RoleTag>, QueryEvaluationIndex> child) : child(child) {}
        void refresh(EvaluationGraph<Family, Kind>& graph)
        {
            const auto count = std::visit([&](auto index) { return graph.cardinality(index); }, child);
            if constexpr (std::same_as<Category, BooleanTag>)
                value.set(count != 0);
            else
                value.set(ygg::to_uint_t(count));
        }
        template<StateEvaluationContextConcept<Family, Kind> Context>
        void initialize(EvaluationGraph<Family, Kind>& graph, Context&)
        {
            refresh(graph);
        }
        void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>&)
        {
            refresh(graph);
        }
    };

    struct Distance
    {
        EvaluationIndex<ConceptTag> sources;
        EvaluationIndex<RoleTag> edges;
        EvaluationIndex<ConceptTag> targets;
        DistanceEvaluator operation;
        DenotationState<Category> value;

        Distance(EvaluationIndex<ConceptTag> sources, EvaluationIndex<RoleTag> edges, EvaluationIndex<ConceptTag> targets) :
            sources(sources),
            edges(edges),
            targets(targets)
        {
        }
        template<StateEvaluationContextConcept<Family, Kind> Context>
        void initialize(EvaluationGraph<Family, Kind>& graph, Context&)
        {
            operation.initialize(graph.result(sources), graph.result(edges), graph.result(targets));
            value.set(operation.get_result());
        }
        void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>&)
        {
            operation
                .update(graph.result(sources), graph.result(edges), graph.result(targets), graph.change(sources), graph.change(edges), graph.change(targets));
            value.set(operation.get_result());
        }
    };

    struct Projection
    {
        QueryEvaluationIndex child;
        ygg::database::incremental::ProjectionEvaluator<ygg::Index<tyr::formalism::Object>> operation;
        DenotationState<Category> value;

        Projection(QueryEvaluationIndex child, const ygg::database::ProjectionPlan& plan) : child(child), operation(plan) {}
        void set_row(EvaluationGraph<Family, Kind>& graph, std::span<const ygg::Index<tyr::formalism::Object>> row, bool present)
        {
            const auto objects = ygg::make_view(row, graph.repository());
            if constexpr (std::same_as<Category, ConceptTag>)
                value.set(objects[0], present);
            else
                value.set(std::pair(objects[0], objects[1]), present);
        }
        template<StateEvaluationContextConcept<Family, Kind> Context>
        void initialize(EvaluationGraph<Family, Kind>& graph, Context& context)
        {
            value.initialize(semantics::detail::num_objects<Kind, Family>(context));
            operation.initialize(graph.result(child), context.get_workspace().get_database_workspace());
            const auto& rows = operation.get_result();
            for (size_t i = 0; i < rows.size(); ++i)
                set_row(graph, rows.row(i), true);
        }
        void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>& workspace)
        {
            const auto& input = graph.change(child);
            operation.update(input.added, input.removed, workspace);
            const auto& change = operation.get_delta();
            for (size_t i = 0; i < change.removed.size(); ++i)
                set_row(graph, change.removed.row(i), false);
            for (size_t i = 0; i < change.added.size(); ++i)
                set_row(graph, change.added.row(i), true);
        }
    };

    using ConceptOperations = ygg::TypeList<Atomic<tyr::formalism::FluentTag>,
                                            Atomic<tyr::formalism::DerivedTag>,
                                            UnarySet<NegationTag, ConceptTag>,
                                            BinarySet<IntersectionTag, ConceptTag, ConceptTag>,
                                            BinarySet<UnionTag, ConceptTag, ConceptTag>,
                                            BinarySet<ValueRestrictionTag, RoleTag, ConceptTag>,
                                            BinarySet<ExistentialQuantificationTag, RoleTag, ConceptTag>,
                                            BinarySet<RoleValueMapTag, RoleTag, RoleTag>,
                                            BinarySet<AgreementTag, RoleTag, RoleTag>,
                                            NumberRestriction<AtLeastNumberRestrictionTag>,
                                            NumberRestriction<AtMostNumberRestrictionTag>,
                                            NumberRestriction<ExactNumberRestrictionTag>,
                                            QualifiedNumberRestriction<QualifiedAtLeastNumberRestrictionTag>,
                                            QualifiedNumberRestriction<QualifiedAtMostNumberRestrictionTag>,
                                            QualifiedNumberRestriction<QualifiedExactNumberRestrictionTag>,
                                            Fillers,
                                            Projection>;
    using RoleOperations = ygg::TypeList<Atomic<tyr::formalism::FluentTag>,
                                         Atomic<tyr::formalism::DerivedTag>,
                                         UnarySet<ComplementTag, RoleTag>,
                                         UnarySet<InverseTag, RoleTag>,
                                         UnarySet<IdentityTag, ConceptTag>,
                                         UnarySet<TransitiveClosureTag, RoleTag>,
                                         UnarySet<ReflexiveTransitiveClosureTag, RoleTag>,
                                         BinarySet<IntersectionTag, RoleTag, RoleTag>,
                                         BinarySet<UnionTag, RoleTag, RoleTag>,
                                         BinarySet<RestrictionTag, RoleTag, ConceptTag>,
                                         BinarySet<CompositionTag, RoleTag, RoleTag>,
                                         Projection>;
    using BooleanOperations = ygg::TypeList<Atomic<tyr::formalism::FluentTag>,
                                            Atomic<tyr::formalism::DerivedTag>,
                                            Cardinality,
                                            LogicalNot,
                                            ScalarBinary<AndTag, BooleanTag>,
                                            ScalarBinary<OrTag, BooleanTag>,
                                            ScalarBinary<BooleanEqTag, BooleanTag>,
                                            ScalarBinary<BooleanNeTag, BooleanTag>,
                                            ScalarBinary<BooleanLtTag, BooleanTag>,
                                            ScalarBinary<BooleanLeTag, BooleanTag>,
                                            ScalarBinary<BooleanGtTag, BooleanTag>,
                                            ScalarBinary<BooleanGeTag, BooleanTag>,
                                            ScalarBinary<NumericalEqTag, NumericalTag>,
                                            ScalarBinary<NumericalNeTag, NumericalTag>,
                                            ScalarBinary<NumericalLtTag, NumericalTag>,
                                            ScalarBinary<NumericalLeTag, NumericalTag>,
                                            ScalarBinary<NumericalGtTag, NumericalTag>,
                                            ScalarBinary<NumericalGeTag, NumericalTag>>;
    using NumericalOperations = ygg::TypeList<Cardinality,
                                              Distance,
                                              ScalarBinary<AddTag, NumericalTag>,
                                              ScalarBinary<SubTag, NumericalTag>,
                                              ScalarBinary<MulTag, NumericalTag>,
                                              ScalarBinary<DivTag, NumericalTag>,
                                              ScalarBinary<MinTag, NumericalTag>,
                                              ScalarBinary<MaxTag, NumericalTag>>;
    using Operations = std::conditional_t<std::same_as<Category, ConceptTag>,
                                          ConceptOperations,
                                          std::conditional_t<std::same_as<Category, RoleTag>,
                                                             RoleOperations,
                                                             std::conditional_t<std::same_as<Category, BooleanTag>, BooleanOperations, NumericalOperations>>>;
    using InvocationOperations = std::conditional_t<std::same_as<Family, runir::kr::ExtFamilyTag>,
                                                    std::conditional_t<ConceptOrRoleTag<Category>, ygg::TypeList<Register, Argument>, ygg::TypeList<Argument>>,
                                                    ygg::TypeList<>>;
    using Evaluators = ygg::ApplyTypeListT<std::variant, ygg::ConcatTypeListsT<ygg::TypeList<Static>, Operations, InvocationOperations>>;

    FamilyConstructorView<Family, Category> m_expression;
    Evaluators m_evaluator;

    template<ConceptOrRoleTag Projected, typename C>
    Evaluators prepare(ygg::View<ygg::Index<QueryProjection<Family, Projected>>, C> expression, EvaluationGraph<Family, Kind>& graph)
    {
        return Projection(graph.prepare(expression.get_arg()), expression.get_data().plan);
    }

    template<template<typename, typename> typename Expression, typename Tag, typename C>
    Evaluators prepare(ygg::View<ygg::Index<Expression<Family, Tag>>, C> expression, EvaluationGraph<Family, Kind>& graph)
    {
        if constexpr (std::same_as<Tag, AtomicStateTag<tyr::formalism::FluentTag>>)
            return Atomic<tyr::formalism::FluentTag>(expression.get_predicate(), expression.get_polarity());
        else if constexpr (std::same_as<Tag, AtomicStateTag<tyr::formalism::DerivedTag>>)
            return Atomic<tyr::formalism::DerivedTag>(expression.get_predicate(), expression.get_polarity());
        else if constexpr (std::same_as<Tag, RegisterTag>)
            return Register(expression.get_register().get_identifier());
        else if constexpr (std::same_as<Tag, ArgumentTag<Category>>)
            return Argument(expression.get_argument().get_identifier());
        else if constexpr (std::same_as<Tag, DistanceTag>)
            return Distance(graph.prepare(expression.get_lhs()), graph.prepare(expression.get_mid()), graph.prepare(expression.get_rhs()));
        else if constexpr (std::same_as<Tag, CountTag> || std::same_as<Tag, NonemptyTag>)
            return ygg::visit([&](auto child) -> Evaluators { return Cardinality(graph.prepare(child)); }, expression.get_arg());
        else if constexpr (std::same_as<Tag, QualifiedAtLeastNumberRestrictionTag> || std::same_as<Tag, QualifiedAtMostNumberRestrictionTag>
                           || std::same_as<Tag, QualifiedExactNumberRestrictionTag>)
            return QualifiedNumberRestriction<Tag>(graph.prepare(expression.get_role()), graph.prepare(expression.get_concept()), expression.get_n());
        else if constexpr (std::same_as<Tag, AtLeastNumberRestrictionTag> || std::same_as<Tag, AtMostNumberRestrictionTag>
                           || std::same_as<Tag, ExactNumberRestrictionTag>)
            return NumberRestriction<Tag>(graph.prepare(expression.get_role()), expression.get_n());
        else if constexpr (std::same_as<Tag, RoleFillersTag>)
            return Fillers(graph.prepare(expression.get_role()), expression);
        else if constexpr (NumericalBinaryTag<Tag>)
            return ScalarBinary<Tag, NumericalTag>(graph.prepare(expression.get_lhs()), graph.prepare(expression.get_rhs()));
        else if constexpr (LogicalBinaryTag<Tag>)
            return ScalarBinary<Tag, BooleanTag>(graph.prepare(expression.get_lhs()), graph.prepare(expression.get_rhs()));
        else if constexpr (ComparisonTag<Tag>)
            return ScalarBinary<Tag, comparison_operand_t<Tag>>(graph.prepare(expression.get_lhs()), graph.prepare(expression.get_rhs()));
        else if constexpr (std::same_as<Tag, NotTag>)
            return LogicalNot(graph.prepare(expression.get_arg()));
        else if constexpr (std::same_as<Tag, IntersectionTag> || std::same_as<Tag, UnionTag>)
            return BinarySet<Tag, Category, Category>(graph.prepare(expression.get_lhs()), graph.prepare(expression.get_rhs()));
        else if constexpr (std::same_as<Tag, NegationTag> || std::same_as<Tag, ComplementTag> || std::same_as<Tag, InverseTag>
                           || std::same_as<Tag, TransitiveClosureTag> || std::same_as<Tag, ReflexiveTransitiveClosureTag>)
            return UnarySet<Tag, Category>(graph.prepare(expression.get_arg()));
        else if constexpr (std::same_as<Tag, IdentityTag>)
            return UnarySet<Tag, ConceptTag>(graph.prepare(expression.get_arg()));
        else if constexpr (std::same_as<Tag, RestrictionTag> || std::same_as<Tag, ValueRestrictionTag> || std::same_as<Tag, ExistentialQuantificationTag>)
            return BinarySet<Tag, RoleTag, ConceptTag>(graph.prepare(expression.get_lhs()), graph.prepare(expression.get_rhs()));
        else if constexpr (std::same_as<Tag, CompositionTag> || std::same_as<Tag, RoleValueMapTag> || std::same_as<Tag, AgreementTag>)
            return BinarySet<Tag, RoleTag, RoleTag>(graph.prepare(expression.get_lhs()), graph.prepare(expression.get_rhs()));
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
    m_evaluator(expression.is_static() ? Evaluators(Static(expression)) :
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
