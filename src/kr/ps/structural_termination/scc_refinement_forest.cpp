#include "scc_refinement_forest.hpp"

#include "detail.hpp"

namespace runir::kr::ps::detail
{

ResidualMemorySccs::ResidualMemorySccs(const QualitativePolicy& policy, bool use_memory_scc_scope) :
    policy_(policy),
    use_memory_scc_scope_(use_memory_scc_scope)
{
    const auto component_of = std::vector<std::size_t>(policy_.num_memory_states, 0);
    forest_.emplace(component_of, 1, policy_.num_booleans, policy_.num_numericals);
}

bool ResidualMemorySccs::refine(std::span<const std::size_t> remaining_rule_positions)
{
    if (!use_memory_scc_scope_)
        return false;

    auto memory_edges = std::vector<PolicyEdge> {};
    memory_edges.reserve(remaining_rule_positions.size());
    for (const auto rule_position : remaining_rule_positions)
    {
        const auto& rule = policy_.rule_profiles[rule_position];
        memory_edges.push_back(PolicyEdge { rule.source_memory_position, rule.target_memory_position, rule_position });
    }

    const auto components = find_strong_components(memory_edges, policy_.num_memory_states);
    auto memories_by_component = std::vector<std::vector<std::size_t>>(components.count);
    for (std::size_t memory = 0; memory < policy_.num_memory_states; ++memory)
        memories_by_component[components.component_of[memory]].push_back(memory);

    const auto previous_node_count = forest_->nodes().size();
    auto children_by_parent = std::vector<std::vector<std::vector<std::size_t>>>(previous_node_count);
    for (auto& memories : memories_by_component)
    {
        const auto parent = forest_->leaf_of_memory(memories.front());
        for (const auto memory : memories)
            if (forest_->leaf_of_memory(memory) != parent)
                throw std::logic_error("removing residual rules merged memory SCCs");
        children_by_parent[parent].push_back(std::move(memories));
    }

    auto refined = false;
    for (SccRefinementForest::NodeIndex parent = 0; parent < previous_node_count; ++parent)
    {
        if (!forest_->node(parent).is_leaf())
            continue;

        const auto& children = children_by_parent[parent];
        if (children.size() == 1 && children.front() == forest_->node(parent).memory_positions)
            continue;
        forest_->split(parent, children);
        refined = true;
    }
    return refined;
}

bool ResidualMemorySccs::share_opponent_scope(std::size_t lhs_rule_position, std::size_t rhs_rule_position) const
{
    return !use_memory_scc_scope_ || scc_for_rule(lhs_rule_position) == scc_for_rule(rhs_rule_position);
}

bool ResidualMemorySccs::is_cross_scc_rule(std::size_t rule_position) const
{
    if (!use_memory_scc_scope_)
        return false;
    const auto& rule = policy_.rule_profiles[rule_position];
    return forest_->leaf_of_memory(rule.source_memory_position) != forest_->leaf_of_memory(rule.target_memory_position);
}

SccRefinementForest::Marks ResidualMemorySccs::marks_for(std::size_t rule_position) const
{
    return forest_->effective_marks(scc_for_rule(rule_position));
}

void ResidualMemorySccs::establish_r1_mark(std::size_t witnessing_rule_position, std::size_t numerical_position)
{
    forest_->mark_numerical(scc_for_rule(witnessing_rule_position), numerical_position);
}

void ResidualMemorySccs::establish_r2_mark(std::size_t witnessing_rule_position, std::size_t boolean_position)
{
    forest_->mark_boolean(scc_for_rule(witnessing_rule_position), boolean_position);
}

SccRefinementForest::NodeIndex ResidualMemorySccs::scc_for_rule(std::size_t rule_position) const
{
    return forest_->leaf_of_memory(policy_.rule_profiles[rule_position].source_memory_position);
}

}  // namespace runir::kr::ps::detail
