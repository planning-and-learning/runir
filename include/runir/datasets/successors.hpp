/*
 * Copyright (C) 2025-2026 Dominik Drexler
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef RUNIR_DATASETS_SUCCESSORS_HPP_
#define RUNIR_DATASETS_SUCCESSORS_HPP_

#include "runir/datasets/task_class.hpp"

#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <tyr/formalism/declarations.hpp>
#include <tyr/formalism/planning/declarations.hpp>
#include <tyr/formalism/repository.hpp>
#include <tyr/planning/axioms/axiom_evaluator.hpp>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/nodes/labeled_node_view.hpp>
#include <tyr/planning/nodes/node_view.hpp>
#include <tyr/planning/plans/plan_view.hpp>
#include <tyr/planning/states/state_builder.hpp>
#include <tyr/planning/states/state_repository.hpp>
#include <tyr/planning/successors/successor_generator.hpp>
#include <tyr/planning/successors/successors.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/formalism/row_data.hpp>

// What a task's search context gives of a node, composed from tyr's successor functions
// (tyr/planning/successors/successors.hpp): a node of a state, a successor registered or not, an applicable binding.

namespace runir::datasets
{

/// A planning state as a search reaches it: a view of the state (S: registered and shared, or one being built and
/// borrowed) and its metric. What the code over tyr's states passes around; tyr's successor generator reads a node of its
/// own records instead (node_of).
template<tyr::TaskKind Kind, tyr::planning::StateBuilderViewConcept<Kind> S = tyr::planning::SharedStateBuilderView<Kind>>
class PlanningNode
{
public:
    PlanningNode(S state, ygg::float_t metric) : m_state(std::move(state)), m_metric(metric) {}

    [[nodiscard]] const S& state() const noexcept { return m_state; }

    [[nodiscard]] ygg::float_t metric() const noexcept { return m_metric; }

    /// Its record through its state's repository: of a node whose state is registered.
    [[nodiscard]] tyr::planning::OwnedNodeRecordView<Kind> pack() const
        requires std::same_as<S, tyr::planning::SharedStateBuilderView<Kind>>
    { return tyr::planning::OwnedNodeRecordView<Kind>(urd::data<tyr::planning::Node<Kind>>(m_state.pack().index(), m_metric), &m_state.context()); }

    friend bool operator==(const PlanningNode& lhs, const PlanningNode& rhs) { return lhs.m_state == rhs.m_state && lhs.m_metric == rhs.m_metric; }

private:
    S m_state;
    ygg::float_t m_metric;
};

/// A planning node and the action binding that led to it (Binding: interned, or borrowed from the successor generator
/// while it lends it).
template<tyr::TaskKind Kind,
         tyr::planning::StateBuilderViewConcept<Kind> S = tyr::planning::SharedStateBuilderView<Kind>,
         tyr::formalism::ActionRowViewConcept Binding = tyr::formalism::ActionRowView>
class LabeledPlanningNode
{
public:
    LabeledPlanningNode(Binding label, PlanningNode<Kind, S> node) : m_label(std::move(label)), m_node(std::move(node)) {}

    [[nodiscard]] const Binding& label() const noexcept { return m_label; }

    [[nodiscard]] const PlanningNode<Kind, S>& node() const noexcept { return m_node; }

    /// Its record through its state's repository: of an interned binding and a node whose state is registered.
    [[nodiscard]] tyr::planning::OwnedLabeledNodeRecordView<Kind> pack() const
        requires std::same_as<S, tyr::planning::SharedStateBuilderView<Kind>> && std::same_as<Binding, tyr::formalism::ActionRowView>
    {
        const auto& state = m_node.state();
        return tyr::planning::OwnedLabeledNodeRecordView<Kind>(
            urd::data<tyr::planning::LabeledNode<Kind>>(m_label.index(), urd::data<tyr::planning::Node<Kind>>(state.pack().index(), m_node.metric())),
            &state.context());
    }

private:
    Binding m_label;
    PlanningNode<Kind, S> m_node;
};

/// A labeled node's record read back as a labeled planning node: its state unpacked from its repository and shared.
template<tyr::TaskKind Kind>
LabeledPlanningNode<Kind> unpacked(const tyr::planning::OwnedLabeledNodeRecordView<Kind>& packed)
{
    const auto node = packed.node();
    return LabeledPlanningNode<Kind>(packed.label(), PlanningNode<Kind>(node.state().unpack(), node.metric()));
}

/// A labeled node of tyr's records whose label is interned and whose state is registered, as a labeled planning node:
/// its state shared.
template<tyr::TaskKind Kind>
LabeledPlanningNode<Kind> labeled_planning_node(const std::type_identity_t<tyr::planning::LabeledNodeBuilderView<Kind>>& labeled)
{
    const auto node = labeled.node();
    return LabeledPlanningNode<Kind>(urd::interned_view(labeled.label()), PlanningNode<Kind>(node.state().pack().unpack(), node.metric()));
}

/// A registered state, read through any view of it, shared: unpacked from its repository, so that it outlives the view
/// (a node's borrowed one, as Python hands it over).
template<tyr::TaskKind Kind>
tyr::planning::SharedStateBuilderView<Kind> shared_state(const std::type_identity_t<tyr::planning::StateBuilderView<Kind>>& state)
{
    const auto packed = state.pack();
    if (packed.index().is_none())
        throw std::invalid_argument("shared_state: the state is not registered in its repository.");
    return packed.unpack();
}

/// A node of `state` (a view of it: registered, or being built) with metric `metric`: the state copied into a builder of
/// its repository's pool, its index as it is. What tyr's successor generator reads a source from.
// ponytail: one state copy per call. Hold the node across the calls on one state if a profile shows it.
template<tyr::TaskKind Kind>
tyr::planning::OwnedNodeBuilderView<Kind> node_of(const std::type_identity_t<tyr::planning::StateBuilderView<Kind>>& state, ygg::float_t metric = 0)
{
    auto builder = tyr::planning::get_state_builder(state.context());
    builder->assign(state);
    return tyr::planning::OwnedNodeBuilderView<Kind>(tyr::planning::NodeBuilder<Kind> { std::move(builder), metric }, &state.context());
}

/// The context's initial node as its record, read through the state repository: its state registered, its metric.
template<tyr::TaskKind Kind>
tyr::planning::OwnedNodeRecordView<Kind> packed_initial_node(const TaskSearchContext<Kind>& search)
{
    const auto initial = tyr::planning::get_initial_node(*search.state_repository, *search.axiom_evaluator);
    return tyr::planning::OwnedNodeRecordView<Kind>(initial.pack(), search.state_repository.get());
}

/// The plan of `actions` from `initial`, replayed in the context's state repository, as its record.
template<tyr::TaskKind Kind>
tyr::planning::OwnedPlanRecordView<tyr::planning::DeterministicTag, Kind>
replayed_plan(const TaskSearchContext<Kind>& search,
              const std::type_identity_t<tyr::planning::OwnedNodeRecordView<Kind>>& initial,
              std::span<const tyr::formalism::ActionRowView> actions)
{
    auto& repository = *search.state_repository;
    const auto plan = tyr::planning::replay_plan<Kind>(urd::record(initial), actions, *search.successor_generator, repository, *search.axiom_evaluator);
    return tyr::planning::OwnedPlanRecordView<tyr::planning::DeterministicTag, Kind>(urd::resolve(plan, std::as_const(repository)).pack(), &repository);
}

/// The successor of `node` by `binding`, registered in the context's state repository: generated, looked up, closed
/// under the axioms unless a state of its identity is registered already, registered. Its changes are written into the
/// generator's scratch delta, so a warm call allocates nothing for them.
template<tyr::TaskKind Kind, tyr::formalism::ActionRowViewConcept Binding>
tyr::planning::NodeBuilder<Kind>
registered_successor(const TaskSearchContext<Kind>& search, const std::type_identity_t<tyr::planning::NodeBuilderView<Kind>>& node, Binding binding)
{
    auto& repository = *search.state_repository;
    auto& generator = *search.successor_generator;
    auto successor = tyr::planning::SuccessorState<Kind> {};
    std::swap(successor.delta, generator.scratch_delta());
    successor.state = tyr::planning::get_state_builder(repository);
    successor.auxiliary_value = generator.generate_successor_state(node, binding, *successor.state, successor.delta);
    tyr::planning::find_successor(repository, node, successor);
    tyr::planning::extend_successor(*search.axiom_evaluator, node, successor);
    auto result = tyr::planning::register_successor(repository, node, successor);
    std::swap(successor.delta, generator.scratch_delta());
    return result;
}

/// The successor of `node` by `binding`, closed under the axioms and registered nowhere, written into `out`: its state
/// builder is reused in place (its buffers kept), so a warm call allocates nothing.
template<tyr::TaskKind Kind, tyr::formalism::ActionRowViewConcept Binding>
void write_unregistered_successor(const TaskSearchContext<Kind>& search,
                                  const std::type_identity_t<tyr::planning::NodeBuilderView<Kind>>& node,
                                  Binding binding,
                                  std::type_identity_t<tyr::planning::NodeBuilder<Kind>>& out)
{
    auto& generator = *search.successor_generator;
    auto successor = tyr::planning::SuccessorState<Kind> {};
    std::swap(successor.delta, generator.scratch_delta());
    successor.state = std::move(out.state);
    // A state generated into is new: it has no index, whatever the builder held before.
    successor.state->set(urd::index<tyr::planning::State<Kind>> {});
    successor.auxiliary_value = generator.generate_successor_state(node, binding, *successor.state, successor.delta);
    tyr::planning::extend_successor(*search.axiom_evaluator, node, successor);
    out = tyr::planning::successor_node(node, successor);
    std::swap(successor.delta, generator.scratch_delta());
}

/// The successor of `node` by `binding` as a planning node: registered in the context's state repository, its state
/// shared.
template<tyr::TaskKind Kind, tyr::formalism::ActionRowViewConcept Binding>
PlanningNode<Kind>
registered_successor_node(const TaskSearchContext<Kind>& search, const std::type_identity_t<tyr::planning::NodeBuilderView<Kind>>& node, Binding binding)
{
    auto successor = registered_successor<Kind>(search, node, binding);
    const auto metric = successor.metric;
    return PlanningNode<Kind>(tyr::planning::share(*search.state_repository, std::move(successor.state)), metric);
}

/// The context's initial node as a planning node: its state registered and shared, its metric.
template<tyr::TaskKind Kind>
PlanningNode<Kind> initial_node(const TaskSearchContext<Kind>& search)
{
    const auto initial = tyr::planning::get_initial_node(*search.state_repository, *search.axiom_evaluator);
    return PlanningNode<Kind>(initial.state().pack().unpack(), initial.metric());
}

/// The successor of a planning node by `binding`, registered in the context's state repository.
template<tyr::TaskKind Kind, tyr::planning::StateBuilderViewConcept<Kind> S, tyr::formalism::ActionRowViewConcept Binding>
PlanningNode<Kind> successor_node(const TaskSearchContext<Kind>& search, const PlanningNode<Kind, S>& node, Binding binding)
{ return registered_successor_node<Kind>(search, node_of<Kind>(node.state(), node.metric()), binding); }

/// The successors of a planning node, registered in the context's state repository, each labeled by its binding,
/// interned.
template<tyr::TaskKind Kind, tyr::planning::StateBuilderViewConcept<Kind> S>
std::vector<LabeledPlanningNode<Kind>> labeled_successor_nodes(const TaskSearchContext<Kind>& search, const PlanningNode<Kind, S>& node)
{
    auto result = std::vector<LabeledPlanningNode<Kind>> {};
    auto& repository = *search.state_repository;
    auto builder = tyr::formalism::Builder {};
    const auto source = node_of<Kind>(node.state(), node.metric());
    tyr::planning::for_each_successor<tyr::planning::DeterministicTag>(
        *search.successor_generator,
        source,
        [&](const tyr::planning::TransitionLabel<Kind>& transition, tyr::planning::SuccessorState<Kind>& successor)
        {
            tyr::planning::find_successor(repository, source, successor);
            tyr::planning::extend_successor(*search.axiom_evaluator, source, successor);
            auto registered = tyr::planning::register_successor(repository, source, successor);
            const auto metric = registered.metric;
            result.emplace_back(tyr::planning::intern_action_binding(*search.successor_generator, builder, transition.binding.value()),
                                PlanningNode<Kind>(tyr::planning::share(repository, std::move(registered.state)), metric));
            return true;
        });
    return result;
}

/// The successors of a planning node, registered in the context's state repository, without their labels.
template<tyr::TaskKind Kind, tyr::planning::StateBuilderViewConcept<Kind> S>
std::vector<PlanningNode<Kind>> successor_nodes(const TaskSearchContext<Kind>& search, const PlanningNode<Kind, S>& node)
{
    auto result = std::vector<PlanningNode<Kind>> {};
    for (const auto& labeled : labeled_successor_nodes(search, node))
        result.push_back(labeled.node());
    return result;
}

/// `binding`, a row of the task's repository: an interned one as it is, a borrowed one (valid only while its giver
/// lends it) interned through `builder`. Of a binding the successor generator gave: it is not validated again.
template<tyr::TaskKind Kind>
tyr::formalism::ActionRowView
interned_binding(const tyr::planning::StateRepository<Kind>& repository, tyr::formalism::Builder& builder, tyr::planning::ActionBindingView<Kind> binding)
{
    if constexpr (std::same_as<Kind, tyr::GroundTag>)
    {
        return binding;
    }
    else
    {
        return repository.get_task()->get_repository()->insert(binding, builder).first;
    }
}

/// `binding`, which the context's successor generator gave, interned through the context's scratch: kept beyond the
/// time it is lent for.
template<tyr::TaskKind Kind>
tyr::formalism::ActionRowView interned_binding(TaskSearchContext<Kind>& search, std::type_identity_t<tyr::planning::ActionBindingView<Kind>> binding)
{ return tyr::planning::intern_action_binding(*search.successor_generator, search.builder, binding); }

/// The action bindings applicable at a planning node, interned.
template<tyr::TaskKind Kind, tyr::planning::StateBuilderViewConcept<Kind> S>
std::vector<tyr::formalism::ActionRowView> applicable_action_bindings(const TaskSearchContext<Kind>& search, const PlanningNode<Kind, S>& node)
{
    auto result = std::vector<tyr::formalism::ActionRowView> {};
    auto builder = tyr::formalism::Builder {};
    auto& generator = *search.successor_generator;
    generator.for_each_applicable_action_binding(node_of<Kind>(node.state(), node.metric()),
                                                 {},
                                                 [&](tyr::planning::ActionBindingView<Kind> binding)
                                                 {
                                                     result.push_back(tyr::planning::intern_action_binding(generator, builder, binding));
                                                     return true;
                                                 });
    return result;
}

/// The status of `binding` at `node` (check_action_binding) and, where it is applicable, the binding interned: through
/// the context's scratch, so that a warm call allocates nothing.
template<tyr::TaskKind Kind, tyr::formalism::ActionRowViewConcept Binding>
tyr::planning::ActionBindingResult
applicable_action_binding(TaskSearchContext<Kind>& search, const std::type_identity_t<tyr::planning::NodeBuilderView<Kind>>& node, Binding binding)
{
    auto& generator = *search.successor_generator;
    const auto status = generator.check_action_binding(node, binding);
    if (status != tyr::planning::ActionBindingStatus::APPLICABLE)
    {
        return { status, std::nullopt };
    }
    else if constexpr (std::same_as<Binding, tyr::formalism::ActionRowView>)
    {
        return { status, binding };
    }
    else if constexpr (std::same_as<Kind, tyr::GroundTag>)
    {
        // An applicable ground binding names a ground action of the task: its row is interned already.
        return { status, search.task->get_repository()->find(binding, search.builder) };
    }
    else
    {
        return { status, tyr::planning::intern_action_binding(generator, search.builder, binding) };
    }
}

}  // namespace runir::datasets

#endif
