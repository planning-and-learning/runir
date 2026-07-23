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

#include <boost/dynamic_bitset.hpp>

#include <algorithm>
#include <cstddef>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace runir::kr::ps::detail
{

struct QualitativePolicy;

class SccRefinementForest
{
public:
    using NodeIndex = std::size_t;

    struct Marks
    {
        boost::dynamic_bitset<> booleans;
        boost::dynamic_bitset<> numericals;

        Marks(std::size_t num_booleans, std::size_t num_numericals) :
            booleans(num_booleans),
            numericals(num_numericals)
        {
        }

        Marks& operator|=(const Marks& other)
        {
            booleans |= other.booleans;
            numericals |= other.numericals;
            return *this;
        }
    };

    struct Node
    {
        std::optional<NodeIndex> parent;
        std::vector<NodeIndex> children;

        // Stable global positions of memory states contained in this SCC.
        std::vector<std::size_t> memory_positions;

        // Only marks established while this node was a leaf. Inherited marks
        // remain represented by ancestors rather than being copied.
        Marks local_marks;

        Node(std::optional<NodeIndex> parent_,
             std::vector<std::size_t> memory_positions_,
             std::size_t num_booleans,
             std::size_t num_numericals) :
            parent(parent_),
            memory_positions(std::move(memory_positions_)),
            local_marks(num_booleans, num_numericals)
        {
        }

        bool is_leaf() const noexcept { return children.empty(); }
    };

    // `component_of[m]` identifies the initial SCC containing memory state m.
    explicit SccRefinementForest(std::span<const std::size_t> component_of,
                                 std::size_t num_components,
                                 std::size_t num_booleans,
                                 std::size_t num_numericals) :
        num_booleans_(num_booleans),
        num_numericals_(num_numericals),
        leaf_of_memory_(component_of.size())
    {
        auto memories_by_component = std::vector<std::vector<std::size_t>>(num_components);
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

    const std::vector<NodeIndex>& roots() const noexcept { return roots_; }
    const std::vector<Node>& nodes() const noexcept { return nodes_; }

    NodeIndex leaf_of_memory(std::size_t memory_position) const
    {
        return leaf_of_memory_.at(memory_position);
    }

    const Node& node(NodeIndex index) const { return nodes_.at(index); }

    void mark_boolean(NodeIndex leaf, std::size_t feature_position)
    {
        auto& current = require_leaf(leaf);
        current.local_marks.booleans.set(feature_position);
    }

    void mark_numerical(NodeIndex leaf, std::size_t feature_position)
    {
        auto& current = require_leaf(leaf);
        current.local_marks.numericals.set(feature_position);
    }

    Marks effective_marks(NodeIndex node_index) const
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
    std::vector<NodeIndex> split(NodeIndex leaf, std::span<const std::vector<std::size_t>> child_memory_positions)
    {
        require_leaf(leaf);
        if (child_memory_positions.size() < 2)
            throw std::invalid_argument("an SCC refinement requires at least two children");
        validate_partition(nodes_[leaf].memory_positions, child_memory_positions);

        auto children = std::vector<NodeIndex> {};
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

    NodeIndex append_node(std::optional<NodeIndex> parent, std::vector<std::size_t> memory_positions)
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

    static void validate_partition(std::span<const std::size_t> parent,
                                   std::span<const std::vector<std::size_t>> children)
    {
        auto expected = boost::dynamic_bitset<>(parent.empty() ? 0 : (*std::max_element(parent.begin(), parent.end()) + 1));
        for (const auto memory_position : parent)
            expected.set(memory_position);

        auto actual = boost::dynamic_bitset<>(expected.size());
        for (const auto& child : children)
        {
            if (child.empty())
                throw std::invalid_argument("an SCC refinement child cannot be empty");
            for (const auto memory_position : child)
            {
                if (memory_position >= actual.size() || actual.test(memory_position))
                    throw std::invalid_argument("SCC refinement children are not a disjoint subset of their parent");
                actual.set(memory_position);
            }
        }
        if (actual != expected)
            throw std::invalid_argument("SCC refinement children do not partition their parent");
    }

    std::size_t num_booleans_;
    std::size_t num_numericals_;
    std::vector<NodeIndex> roots_;
    std::vector<Node> nodes_;
    std::vector<NodeIndex> leaf_of_memory_;
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
// disabled, every remaining rule stays in one global opponent scope. R3 only
// reads marks_for(); it never establishes a mark.
class ResidualMemorySccs
{
public:
    explicit ResidualMemorySccs(const QualitativePolicy& policy, bool use_memory_scc_scope);

    bool refine(std::span<const std::size_t> remaining_rule_positions);

    bool share_opponent_scope(std::size_t lhs_rule_position, std::size_t rhs_rule_position) const;
    bool is_cross_scc_rule(std::size_t rule_position) const;

    SccRefinementForest::Marks marks_for(std::size_t rule_position) const;

    void establish_r1_mark(std::size_t witnessing_rule_position, std::size_t numerical_position);
    void establish_r2_mark(std::size_t witnessing_rule_position, std::size_t boolean_position);

    const SccRefinementForest& refinement_forest() const { return *forest_; }

private:
    SccRefinementForest::NodeIndex scc_for_rule(std::size_t rule_position) const;

    const QualitativePolicy& policy_;
    bool use_memory_scc_scope_;
    std::optional<SccRefinementForest> forest_;
};

}  // namespace runir::kr::ps::detail

#endif
