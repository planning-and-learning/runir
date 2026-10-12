#ifndef RUNIR_KR_DL_SEMANTICS_HEURISTICS_QUERY_RELAXATION_HPP_
#define RUNIR_KR_DL_SEMANTICS_HEURISTICS_QUERY_RELAXATION_HPP_

#include "runir/kr/dl/query_view.hpp"
#include "runir/kr/dl/translations/ygg_query.hpp"

#include <algorithm>
#include <array>
#include <memory>
#include <span>
#include <utility>
#include <vector>
#include <yggdrasil/database/semantics/operations.hpp>

namespace runir::kr::dl::semantics::detail
{

/// Static relaxations of DL edge queries, a superset of the edges in every state: dynamic leaves are dropped, and
/// differences and complements keep their left side. A join's conjuncts are relaxed and joined pairwise with early
/// projection: each intermediate result is projected onto the labels still needed, as in existential quantifier
/// scheduling (variable elimination; cf. Dechter, "Bucket Elimination: A Unifying Framework for Reasoning", AIJ 1999,
/// which eliminates one variable at a time instead).
/// `Leaves` returns a static leaf's rows: leaves(query index) -> a relation view.
template<FamilyTag Family, typename Leaves>
class QueryRelaxation
{
    using Relation = ygg::Builder<ygg::database::Relation<QueryValues>>;
    using RelationPtr = std::unique_ptr<Relation>;
    using Layout = std::span<const ygg::database::ColumnLayout>;
    using Label = ygg::Index<ygg::database::Column>;

    Leaves m_leaves;
    ygg::database::Workspace<QueryValues> m_workspace;

    static bool contains(Layout layout, Label label)
    {
        return std::ranges::find(layout, label, &ygg::database::ColumnLayout::label) != layout.end();
    }
    static size_t position(Layout layout, Label label)
    {
        return size_t(std::ranges::find(layout, label, &ygg::database::ColumnLayout::label) - layout.begin());
    }
    /// Appends a column, recomputing its offset for the layout being built.
    static void append(std::vector<ygg::database::ColumnLayout>& layout, ygg::database::ColumnLayout column)
    {
        column.offset = layout.empty() ? 0 : layout.back().offset + layout.back().size;
        layout.push_back(column);
    }

    static RelationPtr make(Layout layout) { return std::make_unique<Relation>(layout); }
    RelationPtr project(const Relation& input, std::span<const Label> labels)
    {
        std::vector<ygg::database::ColumnLayout> layout;
        for (const auto label : labels)
            append(layout, input.columns()[input.column_index(label)]);
        auto out = make(layout);
        ygg::database::project(input, labels, *out, m_workspace);
        return out;
    }
    /// Natural join on shared labels: lhs columns followed by rhs-only columns.
    RelationPtr join(const Relation& lhs, const Relation& rhs)
    {
        std::vector<ygg::database::ColumnLayout> layout;
        for (const auto& column : lhs.columns())
            append(layout, column);
        for (const auto& column : rhs.columns())
            if (!contains(lhs.columns().span(), column.label))
                append(layout, column);
        auto out = make(layout);
        ygg::database::join(lhs, rhs, *out, m_workspace);
        return out;
    }
    template<typename C>
    RelationPtr full(ygg::View<ygg::Index<Query<Family>>, C> query)
    {
        const auto& rows = m_leaves(ygg::uint_t(query.get_index()));
        auto out = make(rows.columns().span());
        for (size_t i = 0; i < rows.size(); ++i)
            out->insert(rows.row(i));
        return out;
    }

public:
    explicit QueryRelaxation(Leaves leaves) : m_leaves(std::move(leaves)) {}

    /// A static relaxation of a query restricted to some labels: Free (no constraint), Empty (provably no rows in any
    /// state), or Rows over a subset of the requested labels, whose missing labels are unconstrained.
    struct Relaxed
    {
        enum class Kind
        {
            FREE,
            EMPTY,
            ROWS
        };
        Kind kind = Kind::FREE;
        RelationPtr rows;
    };

    /// A superset of `query` in every state, projected existentially onto `needed`: dynamic leaves become Free, and
    /// differences and complements keep their left side, which only adds rows. Joins
    /// eliminate variables (see eliminate).
    template<typename C>
    Relaxed relax(ygg::View<ygg::Index<Query<Family>>, C> query, std::span<const Label> needed)
    {
        return ygg::visit([&](auto concrete) { return relax_node(query, concrete, needed); }, query.get_variant());
    }

private:
    static Relaxed free_relaxed() { return Relaxed {}; }
    static Relaxed empty_relaxed() { return Relaxed { Relaxed::Kind::EMPTY, RelationPtr() }; }
    static bool contains_label(std::span<const Label> labels, Label label) { return std::ranges::find(labels, label) != labels.end(); }
    /// Projects onto the relation's labels that occur in `needed`; with none left, only its emptiness remains.
    Relaxed restrict_rows(RelationPtr relation, std::span<const Label> needed)
    {
        std::vector<Label> labels;
        for (const auto& column : relation->columns())
            if (contains_label(needed, column.label))
                labels.push_back(column.label);
        if (labels.empty())
            return relation->empty() ? empty_relaxed() : free_relaxed();
        if (labels.size() == relation->arity())
            return Relaxed { Relaxed::Kind::ROWS, std::move(relation) };
        return Relaxed { Relaxed::Kind::ROWS, project(*relation, labels) };
    }
    /// Variable elimination over relaxed conjuncts: Free conjuncts are dropped,
    /// conjuncts sharing a label are joined first, and every result keeps only the labels in `needed` or in
    /// another conjunct.
    Relaxed eliminate(std::vector<Relaxed> parts, std::span<const Label> needed)
    {
        const auto kept = [&](size_t a, size_t b)
        {
            std::vector<Label> labels(needed.begin(), needed.end());
            for (size_t p = 0; p < parts.size(); ++p)
                if (p != a && p != b && parts[p].kind == Relaxed::Kind::ROWS)
                    for (const auto& column : parts[p].rows->columns())
                        labels.push_back(column.label);
            return labels;
        };
        for (size_t i = 0; i < parts.size(); ++i)
            if (parts[i].kind == Relaxed::Kind::ROWS)
                parts[i] = restrict_rows(std::move(parts[i].rows), kept(i, i));
        while (true)
        {
            if (std::ranges::any_of(parts, [](const Relaxed& part)
                                    { return part.kind == Relaxed::Kind::EMPTY || (part.kind == Relaxed::Kind::ROWS && part.rows->empty()); }))
                return empty_relaxed();
            std::erase_if(parts, [](const Relaxed& part) { return part.kind == Relaxed::Kind::FREE; });
            if (parts.empty())
                return free_relaxed();
            if (parts.size() == 1)
                return restrict_rows(std::move(parts[0].rows), needed);
            const auto shares = [&](size_t a, size_t b)
            { return std::ranges::any_of(parts[b].rows->columns(), [&](const auto& column) { return contains(parts[a].rows->columns().span(), column.label); }); };
            // Without a shared label, the remaining conjuncts are a product over needed labels only.
            auto i = size_t(0);
            auto j = size_t(1);
            [&]
            {
                for (size_t a = 0; a < parts.size(); ++a)
                    for (size_t b = a + 1; b < parts.size(); ++b)
                        if (shares(a, b))
                        {
                            i = a;
                            j = b;
                            return;
                        }
            }();
            auto merged = restrict_rows(join(*parts[i].rows, *parts[j].rows), kept(i, j));
            parts.erase(parts.begin() + ptrdiff_t(j));
            parts[i] = std::move(merged);
        }
    }
    /// Relaxes the conjuncts of nested joins, each on all its own labels.
    template<typename C, typename Tag>
    void relax_conjuncts(ygg::View<ygg::Index<Query<Family>>, C> query,
                         ygg::View<ygg::Index<Query<Family, Tag>>, C> concrete,
                         std::vector<Relaxed>& parts)
    {
        const auto conjuncts_of = [&](auto child) { ygg::visit([&](auto inner) { relax_conjuncts(child, inner, parts); }, child.get_variant()); };
        if constexpr (std::same_as<Tag, QueryJoinTag>)
        {
            conjuncts_of(concrete.get_lhs());
            conjuncts_of(concrete.get_rhs());
        }
        else if constexpr (std::same_as<Tag, QueryGenericJoinTag>)
        {
            for (const auto arg : concrete.get_args())
                conjuncts_of(arg);
        }
        else
        {
            std::vector<Label> labels;
            for (const auto& column : concrete.get_schema())
                labels.push_back(column.label);
            parts.push_back(relax(query, labels));
        }
    }
    template<typename Q>
    static Layout schema_of(Q query)
    {
        return ygg::visit([](auto view) { return view.get_schema().span(); }, query.get_variant());
    }

    template<typename C, typename Tag>
    Relaxed relax_node(ygg::View<ygg::Index<Query<Family>>, C> query,
                       ygg::View<ygg::Index<Query<Family, Tag>>, C> concrete,
                       std::span<const Label> needed)
    {
        const auto schema = concrete.get_schema().span();
        if constexpr (std::same_as<Tag, QueryJoinTag> || std::same_as<Tag, QueryGenericJoinTag>)
        {
            auto parts = std::vector<Relaxed>();
            relax_conjuncts(query, concrete, parts);
            return eliminate(std::move(parts), needed);
        }
        else if constexpr (std::same_as<Tag, QueryUnionTag>)
        {
            auto lhs = relax(concrete.get_lhs(), needed);
            auto rhs = relax(concrete.get_rhs(), needed);
            if (lhs.kind == Relaxed::Kind::EMPTY)
                return rhs;
            if (rhs.kind == Relaxed::Kind::EMPTY)
                return lhs;
            if (lhs.kind == Relaxed::Kind::FREE || rhs.kind == Relaxed::Kind::FREE)
                return free_relaxed();
            // Only labels constrained on both sides stay constrained.
            std::vector<Label> common;
            for (const auto& column : lhs.rows->columns())
                if (contains(rhs.rows->columns().span(), column.label))
                    common.push_back(column.label);
            if (common.empty())
                return free_relaxed();
            // `common` follows the lhs column order, so only the rhs needs reordering.
            const auto left = restrict_rows(std::move(lhs.rows), common);
            const auto right = project(*restrict_rows(std::move(rhs.rows), common).rows, common);
            auto out = make(left.rows->columns().span());
            ygg::database::union_(*left.rows, *right, *out);
            return Relaxed { Relaxed::Kind::ROWS, std::move(out) };
        }
        else if constexpr (std::same_as<Tag, QueryDifferenceTag>)
            return relax(concrete.get_lhs(), needed);
        else if constexpr (std::same_as<Tag, QueryProjectTag>)
            // Labels projected away are local to the child and quantified existentially.
            return relax(concrete.get_arg(), needed);
        else if constexpr (std::same_as<Tag, QueryRenameTag>)
        {
            const auto child_schema = schema_of(concrete.get_arg());
            std::vector<Label> child_needed;
            for (size_t i = 0; i < schema.size(); ++i)
            {
                if (contains_label(needed, schema[i].label))
                    child_needed.push_back(child_schema[i].label);
            }
            auto child = relax(concrete.get_arg(), child_needed);
            if (child.kind != Relaxed::Kind::ROWS)
                return child;
            std::vector<ygg::database::ColumnLayout> layout;
            for (const auto& column : child.rows->columns())
            {
                auto renamed = column;
                renamed.label = schema[position(child_schema, column.label)].label;
                layout.push_back(renamed);
            }
            auto out = make(layout);
            for (size_t i = 0; i < child.rows->size(); ++i)
                out->insert(child.rows->row(i));
            return Relaxed { Relaxed::Kind::ROWS, std::move(out) };
        }
        else if constexpr (std::same_as<Tag, QuerySelectEqualTag> || std::same_as<Tag, QuerySelectValueTag>)
        {
            const auto& data = concrete.get_data();
            std::vector<Label> child_needed(needed.begin(), needed.end());
            if constexpr (std::same_as<Tag, QuerySelectEqualTag>)
            {
                child_needed.push_back(Label(data.lhs_column.get_value()));
                child_needed.push_back(Label(data.rhs_column.get_value()));
            }
            else
                child_needed.push_back(Label(data.column.get_value()));
            auto child = relax(concrete.get_arg(), child_needed);
            if (child.kind != Relaxed::Kind::ROWS)
                return child;
            const auto columns = child.rows->columns().span();
            auto out = make(columns);
            if constexpr (std::same_as<Tag, QuerySelectEqualTag>)
            {
                const auto lhs = Label(data.lhs_column.get_value());
                const auto rhs = Label(data.rhs_column.get_value());
                if (!contains(columns, lhs) || !contains(columns, rhs))
                    return restrict_rows(std::move(child.rows), needed);
                ygg::database::select_equal_columns(*child.rows, lhs, rhs, *out);
            }
            else
            {
                const auto column = Label(data.column.get_value());
                if (!contains(columns, column))
                    return restrict_rows(std::move(child.rows), needed);
                ygg::database::select_equal_value(*child.rows, column, data.object, *out);
            }
            return restrict_rows(std::move(out), needed);
        }
        else
        {
            // Leaves: atomic state and goal queries, concepts, roles, and the empty query. Dynamic leaves are
            // dropped, so the relaxation holds in every state.
            if (!query.is_static())
                return free_relaxed();
            // A complement is the universe minus its argument; like a difference it keeps its left side, Free.
            if (dl::detail::is_complement_leaf(query))
                return free_relaxed();
            return restrict_rows(full(query), needed);
        }
    }

};

}  // namespace runir::kr::dl::semantics::detail

#endif
