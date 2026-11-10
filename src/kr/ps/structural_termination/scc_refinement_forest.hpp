#ifndef RUNIR_KR_PS_STRUCTURAL_TERMINATION_SCC_REFINEMENT_FOREST_HPP_
#define RUNIR_KR_PS_STRUCTURAL_TERMINATION_SCC_REFINEMENT_FOREST_HPP_
//
// The forest makes the monotone refinement of residual memory SCCs explicit:
//
//   * the root initially contains all memory states;
//   * residual SCC decomposition can replace a leaf by child SCCs;
//   * marks established at a node are inherited by all descendants; and
//   * current opponent sets are restricted to the leaf containing a rule.
//
// A leaf's effective marks are the union of its local marks and the local
// marks of all its ancestors.

#include <algorithm>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <urd/repr/values/dense_set.hpp>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace runir::kr::ps::dl
{
struct QualitativePolicy;
}

namespace runir::kr::ps::detail
{

class SccRefinementForest
{
public:
    using NodeIndex = std::size_t;

    struct Marks
    {
        std::uint64_t booleans = 0;
        std::uint64_t numericals = 0;
        /// Per feature, the positions of the rules that established its mark, in ascending order.
        urd::Store<urd::sequence<urd::Store<urd::set<std::size_t>, urd::dense>>> boolean_witnessing_rule_positions;
        urd::Store<urd::sequence<urd::Store<urd::set<std::size_t>, urd::dense>>> numerical_witnessing_rule_positions;

        Marks(std::size_t num_booleans, std::size_t num_numericals) :
            boolean_witnessing_rule_positions(num_booleans),
            numerical_witnessing_rule_positions(num_numericals)
        {
        }

        Marks& operator|=(const Marks& other)
        {
            booleans |= other.booleans;
            numericals |= other.numericals;
            for (std::size_t position = 0; position < boolean_witnessing_rule_positions.size(); ++position)
                boolean_witnessing_rule_positions[position] |= other.boolean_witnessing_rule_positions[position];
            for (std::size_t position = 0; position < numerical_witnessing_rule_positions.size(); ++position)
                numerical_witnessing_rule_positions[position] |= other.numerical_witnessing_rule_positions[position];
            return *this;
        }

        void mark_boolean(std::size_t feature_position, std::size_t witnessing_rule_position)
        {
            auto& witnesses = boolean_witnessing_rule_positions.at(feature_position);
            booleans |= std::uint64_t { 1 } << feature_position;
            witnesses.insert(witnessing_rule_position);
        }

        void mark_numerical(std::size_t feature_position, std::size_t witnessing_rule_position)
        {
            auto& witnesses = numerical_witnessing_rule_positions.at(feature_position);
            numericals |= std::uint64_t { 1 } << feature_position;
            witnesses.insert(witnessing_rule_position);
        }
    };

    struct Node
    {
        std::optional<NodeIndex> parent;
        urd::Store<urd::sequence<NodeIndex>> children;

        // Stable global positions of memory states contained in this SCC.
        urd::Store<urd::sequence<std::size_t>> memory_positions;

        // Only marks established while this node was a leaf. Inherited marks
        // remain represented by ancestors rather than being copied.
        Marks local_marks;

        Node(std::optional<NodeIndex> parent_, urd::Store<urd::sequence<std::size_t>> memory_positions_, std::size_t num_booleans, std::size_t num_numericals) :
            parent(parent_),
            memory_positions(std::move(memory_positions_)),
            local_marks(num_booleans, num_numericals)
        {
        }

        [[nodiscard]] bool is_leaf() const noexcept { return children.empty(); }
    };

    // `component_of[m]` identifies the initial SCC containing memory state m.
    explicit SccRefinementForest(std::span<const std::size_t> component_of, std::size_t num_components, std::size_t num_booleans, std::size_t num_numericals) :
        num_booleans_(num_booleans),
        num_numericals_(num_numericals),
        leaf_of_memory_(component_of.size())
    {
        auto memories_by_component = urd::Store<urd::sequence<urd::Store<urd::sequence<std::size_t>>>>(num_components);
        for (std::size_t memory_position = 0; memory_position < component_of.size(); ++memory_position)
        {
            if (component_of[memory_position] >= num_components)
                throw std::invalid_argument("SCC component index is out of bounds");
            memories_by_component[component_of[memory_position]].push_back(memory_position);
        }

        roots_.reserve(num_components);
        for (auto& memory_positions : memories_by_component)
        {
            if (memory_positions.empty())
                continue;

            const auto root = append_node(std::nullopt, std::move(memory_positions));
            roots_.push_back(root);
            assign_leaf(root);
        }
    }

    [[nodiscard]] const urd::Store<urd::sequence<NodeIndex>>& roots() const noexcept { return roots_; }
    [[nodiscard]] const urd::Store<urd::sequence<Node>>& nodes() const noexcept { return nodes_; }

    [[nodiscard]] NodeIndex leaf_of_memory(std::size_t memory_position) const { return leaf_of_memory_.at(memory_position); }

    [[nodiscard]] const Node& node(NodeIndex index) const { return nodes_.at(index); }

    void mark_boolean(NodeIndex leaf, std::size_t feature_position, std::size_t witnessing_rule_position)
    {
        auto& current = require_leaf(leaf);
        current.local_marks.mark_boolean(feature_position, witnessing_rule_position);
    }

    void mark_numerical(NodeIndex leaf, std::size_t feature_position, std::size_t witnessing_rule_position)
    {
        auto& current = require_leaf(leaf);
        current.local_marks.mark_numerical(feature_position, witnessing_rule_position);
    }

    [[nodiscard]] Marks effective_marks(NodeIndex node_index) const
    {
        auto result = Marks(num_booleans_, num_numericals_);
        auto current = std::optional<NodeIndex> { node_index };
        while (current)
        {
            const auto& entry = nodes_.at(*current);
            result |= entry.local_marks;
            current = entry.parent;
        }
        return result;
    }

    // Replaces one current SCC by the SCCs obtained after residual-rule
    // removal. `child_memory_positions` must be a disjoint partition of the
    // parent's memory states. A one-part "split" is deliberately rejected:
    // callers should leave the existing leaf unchanged in that case.
    urd::Store<urd::sequence<NodeIndex>> split(NodeIndex leaf, std::span<const urd::Store<urd::sequence<std::size_t>>> child_memory_positions)
    {
        require_leaf(leaf);
        if (child_memory_positions.size() < 2)
            throw std::invalid_argument("an SCC refinement requires at least two children");
        validate_partition(nodes_[leaf].memory_positions, child_memory_positions);

        auto children = urd::Store<urd::sequence<NodeIndex>> {};
        children.reserve(child_memory_positions.size());
        for (const auto& memory_positions : child_memory_positions)
        {
            const auto child = append_node(leaf, memory_positions);
            children.push_back(child);
            assign_leaf(child);
        }
        nodes_[leaf].children = children;
        return children;
    }

private:
    Node& require_leaf(NodeIndex index)
    {
        auto& result = nodes_.at(index);
        if (!result.is_leaf())
            throw std::invalid_argument("SCC refinement operation requires a current leaf");
        return result;
    }

    NodeIndex append_node(std::optional<NodeIndex> parent, urd::Store<urd::sequence<std::size_t>> memory_positions)
    {
        const auto result = nodes_.size();
        nodes_.emplace_back(parent, std::move(memory_positions), num_booleans_, num_numericals_);
        return result;
    }

    void assign_leaf(NodeIndex leaf)
    {
        for (const auto memory_position : nodes_[leaf].memory_positions)
            leaf_of_memory_.at(memory_position) = leaf;
    }

    static void validate_partition(std::span<const std::size_t> parent, std::span<const urd::Store<urd::sequence<std::size_t>>> children)
    {
        using Positions = urd::Store<urd::set<std::size_t>, urd::dense>;
        const auto expected = Positions(parent.begin(), parent.end());
        auto actual = Positions();
        for (const auto& child : children)
        {
            if (child.empty())
                throw std::invalid_argument("an SCC refinement child cannot be empty");
            for (const auto memory_position : child)
                if (!expected.contains(memory_position) || !actual.insert(memory_position).second)
                    throw std::invalid_argument("SCC refinement children are not a disjoint subset of their parent");
        }
        if (actual != expected)
            throw std::invalid_argument("SCC refinement children do not partition their parent");
    }

    std::size_t num_booleans_;
    std::size_t num_numericals_;
    urd::Store<urd::sequence<NodeIndex>> roots_;
    urd::Store<urd::sequence<Node>> nodes_;
    urd::Store<urd::sequence<NodeIndex>> leaf_of_memory_;
};

// Policy-facing facade for memory-aware incomplete elimination. Its API uses
// rule positions so incomplete.cpp does not have to translate rules to memory
// endpoints or manipulate the refinement tree directly.
//
// Intended use:
//
//     ResidualMemorySccs memory_sccs(policy, use_memory_scc_scope);
//     memory_sccs.refine(remaining_rule_positions);
//
//     if (!memory_sccs.share_opponent_scope(candidate, opponent))
//         continue;
//
//     const auto marks = memory_sccs.marks_for(candidate);
//     memory_sccs.establish_r1_mark(candidate, numerical_position);
//     memory_sccs.establish_r2_mark(candidate, boolean_position);
//
// With use_memory_scc_scope enabled, refine decomposes the current leaves into
// residual memory SCCs. Callers first saturate rule elimination in the current
// leaves, so marks established there are inherited by every child. When
// disabled, all memory states remain in one component, so opponent scope and
// markings are global and no rule is cross-SCC. R3 only reads marks_for(); it
// never establishes a mark.
class ResidualMemorySccs
{
public:
    explicit ResidualMemorySccs(const dl::QualitativePolicy& policy, bool use_memory_scc_scope);

    bool refine(std::span<const std::size_t> remaining_rule_positions);

    [[nodiscard]] bool share_opponent_scope(std::size_t lhs_rule_position, std::size_t rhs_rule_position) const;
    [[nodiscard]] bool is_cross_scc_rule(std::size_t rule_position) const;

    [[nodiscard]] SccRefinementForest::Marks marks_for(std::size_t rule_position) const;

    void establish_r1_mark(std::size_t witnessing_rule_position, std::size_t numerical_position);
    void establish_r2_mark(std::size_t witnessing_rule_position, std::size_t boolean_position);

    [[nodiscard]] const SccRefinementForest& refinement_forest() const { return forest_; }

private:
    [[nodiscard]] SccRefinementForest::NodeIndex scc_for_rule(std::size_t rule_position) const;

    const dl::QualitativePolicy* policy_;
    bool use_memory_scc_scope_;
    SccRefinementForest forest_;
};

}  // namespace runir::kr::ps::detail

#endif
