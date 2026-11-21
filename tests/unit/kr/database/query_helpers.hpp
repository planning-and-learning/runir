#ifndef YGG_TESTS_UNIT_DATABASE_QUERY_HELPERS_HPP_
#define YGG_TESTS_UNIT_DATABASE_QUERY_HELPERS_HPP_

#include <algorithm>
#include <array>
#include <cstddef>
#include <initializer_list>
#include <limits>
#include <optional>
#include <ranges>
#include <runir/kr/database/semantics/distance.hpp>
#include <runir/kr/database/semantics/operations.hpp>
#include <runir/kr/database/semantics/relation_view.hpp>
#include <runir/kr/database/syntax/query.hpp>
#include <span>
#include <stdexcept>
#include <urd/repr/maps/unordered_flat_map.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/formalism/builder_impl.hpp>  // the tests' own storage, RelationRecords, is no library's
#include <yggdrasil/formalism/canonicalization.hpp>
#include <yggdrasil/formalism/free_relations.hpp>
#include <yggdrasil/formalism/scratch_relation.hpp>

/// The scratch relations of the tests, and a relation drawn from them: what the tests build, read and pass as an
/// operation's output (it converts to the ygg::FreeScratchRelation it holds). Its rows are in tuple order.
namespace runir::tests
{
inline ygg::FreeScratchRelations& scratch()
{
    static ygg::FreeScratchRelations relations;
    return relations;
}

class Relation
{
public:
    Relation() : Relation(std::span<const urd::data<ygg::ColumnLayout>>()) {}

    explicit Relation(std::span<const urd::data<ygg::ColumnLayout>> columns) : m_scratch(scratch().draw(columns)) {}

    // NOLINTNEXTLINE(google-explicit-constructor): passed where an operation takes its output
    operator ygg::FreeScratchRelation&() noexcept { return *m_scratch; }

    // NOLINTNEXTLINE(google-explicit-constructor): read where an operation checks its output against an input
    operator const ygg::FreeScratchRelation&() const noexcept { return *m_scratch; }

    [[nodiscard]] ygg::FreeScratchRelation& get() noexcept { return *m_scratch; }

    [[nodiscard]] ygg::FreeRelationBuilderView view() const { return m_scratch->view(); }

    [[nodiscard]] std::span<const urd::data<ygg::ColumnLayout>> columns() const { return view().columns(); }

    [[nodiscard]] size_t size() const { return view().get_rows().size(); }

    [[nodiscard]] size_t arity() const { return view().arity(); }

    [[nodiscard]] bool empty() const { return view().get_rows().empty(); }

    template<ygg::RowValues V>
    [[nodiscard]] bool contains(const V& values) const
    { return view().contains(values); }

    /// The position of the row of these values in tuple order, if it holds one.
    template<ygg::RowValues V>
    [[nodiscard]] std::optional<size_t> find(const V& values) const
    { return view().find(values); }

    /// Its position in tuple order.
    template<ygg::RowValues V>
    size_t insert(const V& values)
    { return m_scratch->insert(values).first; }

    template<ygg::RowValues V>
    bool erase(const V& values)
    { return m_scratch->erase(values); }

    void clear() { m_scratch->clear(); }

    void initialize(std::span<const urd::data<ygg::ColumnLayout>> columns)
    { m_scratch->relation.initialize(m_scratch->rows, m_scratch->rows.schema(columns), columns); }

    void rename(std::span<const urd::data<ygg::ColumnLayout>> columns) { m_scratch->relation.rename(m_scratch->rows, columns); }

    [[nodiscard]] size_t memory_usage() const { return urd::memory_usage(m_scratch->relation); }

private:
    urd::handle<ygg::FreeScratchRelation> m_scratch;
};

inline ygg::FreeRelationBuilderView borrow(const Relation& relation) { return relation.view(); }

/// A change of a relation, as a test describes one: its added and its removed rows.
struct Change
{
    Relation added;
    Relation removed;

    explicit Change(std::span<const urd::data<ygg::ColumnLayout>> columns) : added(columns), removed(columns) {}

    void clear()
    {
        added.clear();
        removed.clear();
    }

    /// The (added, removed) change, as views.
    [[nodiscard]] std::pair<ygg::FreeRelationBuilderView, ygg::FreeRelationBuilderView> change() const { return { added.view(), removed.view() }; }

    [[nodiscard]] size_t memory_usage() const { return added.memory_usage() + removed.memory_usage(); }
};

/// A row's values, as it is held: a row view, or a builder's row child resolved.
template<typename R>
std::vector<ygg::Value> values_of(const R& row)
{
    return ygg::with_values(row, [](const auto& values) { return std::vector<ygg::Value>(std::ranges::begin(values), std::ranges::end(values)); });
}

/// The record of a scratch relation's row (its rows are records, held by their handles): the row stays the same row
/// while this stays.
template<typename R>
const void* record_of(const R& row)
{ return &std::get<2>(row).get_values(); }

}  // namespace runir::tests

/// Test-only query construction on top of checkout + insert_query.
namespace runir::tests::qb
{
/// A schema whose columns all have type T (by default ygg::uint_t).
template<ygg::ColumnValueType T = ygg::uint_t>
std::vector<urd::data<ygg::ColumnLayout>> columns_of(std::span<const urd::index<ygg::Column>> labels)
{
    std::vector<urd::data<ygg::ColumnLayout>> columns;
    for (const auto label : labels)
        columns.push_back({ label, ygg::type_id<T> });
    return columns;
}

template<ygg::ColumnValueType T = ygg::uint_t>
std::vector<urd::data<ygg::ColumnLayout>> columns_of(std::initializer_list<urd::index<ygg::Column>> labels)
{ return columns_of<T>(std::span<const urd::index<ygg::Column>>(labels)); }

inline kr::database::QueryRepository repository() { return kr::database::QueryRepository(); }

inline kr::database::QueryBuilder& builder()
{
    static kr::database::QueryBuilder storage;
    return storage;
}

inline kr::database::QueryView<> require(const kr::database::QueryRepository& repository, kr::database::QueryView<> query)
{
    if (&get_repository(query.context()) != &repository)
        throw std::invalid_argument("Query: operand belongs to another repository.");
    return query;
}

inline kr::database::QueryView<> input(kr::database::QueryRepository& repository, size_t slot, std::span<const urd::data<ygg::ColumnLayout>> columns)
{
    auto data = kr::database::checkout<kr::database::Query<kr::database::QueryInputTag>>(builder());
    data->input_slot = slot;
    data->columns.assign(columns.begin(), columns.end());
    return kr::database::insert_query(repository, builder(), *data);
}

inline kr::database::QueryView<> input(kr::database::QueryRepository& repository, size_t slot, std::span<const urd::index<ygg::Column>> columns)
{ return input(repository, slot, columns_of(columns)); }

inline kr::database::QueryView<> input(kr::database::QueryRepository& repository, size_t slot, std::initializer_list<urd::index<ygg::Column>> columns)
{ return input(repository, slot, std::span<const urd::index<ygg::Column>>(columns)); }

inline kr::database::QueryView<> empty(kr::database::QueryRepository& repository, std::span<const urd::data<ygg::ColumnLayout>> columns)
{
    auto data = kr::database::checkout<kr::database::Query<kr::database::QueryEmptyTag>>(builder());
    data->columns.assign(columns.begin(), columns.end());
    return kr::database::insert_query(repository, builder(), *data);
}

inline kr::database::QueryView<> empty(kr::database::QueryRepository& repository, std::span<const urd::index<ygg::Column>> columns)
{ return empty(repository, columns_of(columns)); }

inline kr::database::QueryView<> empty(kr::database::QueryRepository& repository, std::initializer_list<urd::index<ygg::Column>> columns)
{ return empty(repository, std::span<const urd::index<ygg::Column>>(columns)); }

template<typename Tag>
kr::database::QueryView<> binary(kr::database::QueryRepository& repository, kr::database::QueryView<> left_operand, kr::database::QueryView<> right_operand)
{
    auto data = kr::database::checkout<kr::database::Query<Tag>>(builder());
    urd::store(data->left_operand, require(repository, left_operand));
    urd::store(data->right_operand, require(repository, right_operand));
    return kr::database::insert_query(repository, builder(), *data);
}

inline kr::database::QueryView<>
join(kr::database::QueryRepository& repository, kr::database::QueryView<> left_operand, kr::database::QueryView<> right_operand)
{ return binary<kr::database::QueryJoinTag>(repository, left_operand, right_operand); }

inline kr::database::QueryView<>
union_(kr::database::QueryRepository& repository, kr::database::QueryView<> left_operand, kr::database::QueryView<> right_operand)
{ return binary<kr::database::QueryUnionTag>(repository, left_operand, right_operand); }

inline kr::database::QueryView<>
difference(kr::database::QueryRepository& repository, kr::database::QueryView<> left_operand, kr::database::QueryView<> right_operand)
{ return binary<kr::database::QueryDifferenceTag>(repository, left_operand, right_operand); }

inline kr::database::QueryView<>
semijoin(kr::database::QueryRepository& repository, kr::database::QueryView<> left_operand, kr::database::QueryView<> right_operand)
{ return binary<kr::database::QuerySemijoinTag>(repository, left_operand, right_operand); }

inline kr::database::QueryView<>
antijoin(kr::database::QueryRepository& repository, kr::database::QueryView<> left_operand, kr::database::QueryView<> right_operand)
{ return binary<kr::database::QueryAntijoinTag>(repository, left_operand, right_operand); }

template<typename Tag>
kr::database::QueryView<> relabel(kr::database::QueryRepository& repository, kr::database::QueryView<> arg, std::span<const urd::index<ygg::Column>> labels)
{
    auto data = kr::database::checkout<kr::database::Query<Tag>>(builder());
    urd::store(data->operand, require(repository, arg));
    data->labels.assign(labels.begin(), labels.end());
    return kr::database::insert_query(repository, builder(), *data);
}

inline kr::database::QueryView<>
project(kr::database::QueryRepository& repository, kr::database::QueryView<> arg, std::span<const urd::index<ygg::Column>> labels)
{ return relabel<kr::database::QueryProjectTag>(repository, arg, labels); }

inline kr::database::QueryView<>
project(kr::database::QueryRepository& repository, kr::database::QueryView<> arg, std::initializer_list<urd::index<ygg::Column>> labels)
{ return project(repository, arg, std::span<const urd::index<ygg::Column>>(labels)); }

inline kr::database::QueryView<>
rename(kr::database::QueryRepository& repository, kr::database::QueryView<> arg, std::span<const urd::index<ygg::Column>> labels)
{ return relabel<kr::database::QueryRenameTag>(repository, arg, labels); }

inline kr::database::QueryView<>
rename(kr::database::QueryRepository& repository, kr::database::QueryView<> arg, std::initializer_list<urd::index<ygg::Column>> labels)
{ return rename(repository, arg, std::span<const urd::index<ygg::Column>>(labels)); }

inline kr::database::QueryView<>
select_equal(kr::database::QueryRepository& repository, kr::database::QueryView<> arg, urd::index<ygg::Column> left, urd::index<ygg::Column> right)
{
    auto data = kr::database::checkout<kr::database::Query<kr::database::QuerySelectEqualTag>>(builder());
    urd::store(data->operand, require(repository, arg));
    data->left_column = left;
    data->right_column = right;
    return kr::database::insert_query(repository, builder(), *data);
}

inline kr::database::QueryView<>
select_encoded(kr::database::QueryRepository& repository, kr::database::QueryView<> arg, urd::index<ygg::Column> column, ygg::Value value, ygg::TypeId type)
{
    auto data = kr::database::checkout<kr::database::Query<kr::database::QuerySelectValueTag>>(builder());
    urd::store(data->operand, require(repository, arg));
    data->column = column;
    data->constant = value;
    data->constant_type = type;
    return kr::database::insert_query(repository, builder(), *data);
}

template<ygg::ColumnValueType T>
kr::database::QueryView<> select_value(kr::database::QueryRepository& repository, kr::database::QueryView<> arg, urd::index<ygg::Column> column, T value)
{
    const auto columns = columns_of(arg);
    const auto found = std::ranges::find(columns, column, &urd::data<ygg::ColumnLayout>::label);
    if (found == columns.end())
        throw std::out_of_range("Columns: unknown column label.");
    if (found->type != ygg::type_id<T>)
        throw std::invalid_argument("Query: constant type does not match selected column.");
    return select_encoded(repository, arg, column, ygg::encode_value(value), ygg::type_id<T>);
}

inline kr::database::QueryView<> generic_join(kr::database::QueryRepository& repository,
                                              std::span<const kr::database::QueryView<>> inputs,
                                              std::span<const urd::index<ygg::Column>> variable_order,
                                              std::span<const urd::index<ygg::Column>> output_order = {})
{
    auto data = kr::database::checkout<kr::database::Query<kr::database::QueryGenericJoinTag>>(builder());
    for (const auto input : inputs)
        urd::append(data->inputs, require(repository, input));
    data->variable_order.assign(variable_order.begin(), variable_order.end());
    data->output_order.assign(output_order.begin(), output_order.end());
    return kr::database::insert_query(repository, builder(), *data);
}

inline kr::database::QueryView<> generic_join(kr::database::QueryRepository& repository,
                                              std::initializer_list<kr::database::QueryView<>> inputs,
                                              std::initializer_list<urd::index<ygg::Column>> variable_order,
                                              std::initializer_list<urd::index<ygg::Column>> output_order = {})
{
    return generic_join(repository,
                        std::span<const kr::database::QueryView<>>(inputs),
                        std::span<const urd::index<ygg::Column>>(variable_order),
                        std::span<const urd::index<ygg::Column>>(output_order));
}
}  // namespace runir::tests::qb

/// Test-only evaluation of one operation: its plan built from the inputs' columns, into a new output of the plan's
/// columns (the library takes the plan, the output and the workspace explicitly).
namespace runir::tests::ops
{
/// uint_t columns labeled `labels`.
inline std::vector<urd::data<ygg::ColumnLayout>> cols(std::initializer_list<urd::index<ygg::Column>> labels) { return qb::columns_of(labels); }

/// Interns the columns and the relation of `relation` (its pool and rows set) in `repository`.
inline std::pair<ygg::FreeRelationView, bool>
intern_relation(ygg::FreeRelationRepository& repository, urd::data<ygg::Relation<ygg::Schema>>& relation, std::span<const urd::data<ygg::ColumnLayout>> labels)
{
    auto columns = urd::data<ygg::Columns>();
    columns.values.set(labels.begin(), labels.end());
    relation.columns = repository.insert(columns).first.index();
    return repository.insert(relation);
}

/// Interns the relation `source` reads (a builder's view, or a relation of any layer) in `repository`: each row, then the
/// columns, then the relation.
template<typename V>
std::pair<ygg::FreeRelationView, bool> intern(ygg::FreeRelationRepository& repository, const V& source)
{
    auto relation = urd::data<ygg::Relation<ygg::Schema>>();
    relation.pool = repository.schema(source.columns());
    auto row = urd::data<ygg::Row<ygg::Schema>>();
    row.relation = relation.pool;
    for (const auto& held : source.get_rows())
        ygg::with_values(held,
                         [&](const auto& values)
                         {
                             row.values.set(std::ranges::begin(values), std::ranges::end(values));
                             urd::append(relation.rows, repository.insert(row).first);
                         });
    ygg::canonicalize(relation, repository);
    return intern_relation(repository, relation, source.columns());
}

inline std::pair<ygg::FreeRelationView, bool> intern(ygg::FreeRelationRepository& repository, const Relation& source)
{ return intern(repository, source.view()); }

/// `source` (of `repository`) under new columns: the same pool and rows.
inline ygg::FreeRelationView rename(ygg::FreeRelationRepository& repository, ygg::FreeRelationView source, std::span<const urd::data<ygg::ColumnLayout>> labels)
{
    auto relation = urd::data<ygg::Relation<ygg::Schema>>();
    relation.pool = repository[source.index()].pool;
    urd::store(relation.rows, source.get_rows());
    return intern_relation(repository, relation, labels).first;
}

template<typename V>
Relation projected(const V& input, std::span<const urd::index<ygg::Column>> labels)
{
    const kr::database::ProjectionPlan plan(input.columns(), labels);
    Relation out(plan.output_columns());
    kr::database::Workspace workspace;
    kr::database::project(input, plan, out, workspace);
    return out;
}

template<typename V>
Relation projected(const V& input, std::initializer_list<urd::index<ygg::Column>> labels)
{ return projected(input, std::span<const urd::index<ygg::Column>>(labels)); }

template<typename L, typename R>
Relation joined(const L& left_operand, const R& right_operand)
{
    const kr::database::JoinPlan plan(left_operand.columns(), right_operand.columns());
    Relation out(plan.output_columns());
    kr::database::Workspace workspace;
    kr::database::join(left_operand, right_operand, plan, out, workspace);
    return out;
}

template<typename L, typename R>
Relation unioned(const L& left_operand, const R& right_operand)
{
    const kr::database::SetPlan plan(left_operand.columns(), right_operand.columns());
    Relation out(plan.output_columns());
    kr::database::union_(left_operand, right_operand, plan, out);
    return out;
}

template<typename L, typename R>
Relation subtracted(const L& left_operand, const R& right_operand)
{
    const kr::database::SetPlan plan(left_operand.columns(), right_operand.columns());
    Relation out(plan.output_columns());
    kr::database::difference(left_operand, right_operand, plan, out);
    return out;
}

template<typename S, typename E, typename T>
Relation distances(const S& sources, const E& edges, const T& targets, const kr::database::DistancePlan& plan)
{
    Relation out(plan.output_columns());
    kr::database::DistanceWorkspace workspace(plan);
    kr::database::distance(sources, edges, targets, plan, out, workspace);
    return out;
}
}  // namespace runir::tests::ops

/// Shared by the min-distance tests: relations borrowed as views, rows decoded to uint_t values, reference distances.
namespace runir::tests::distances
{
inline urd::Store<urd::sequence<ygg::FreeRelationBuilderView>> borrow_all(const urd::Store<urd::sequence<Relation>>& relations)
{
    urd::Store<urd::sequence<ygg::FreeRelationBuilderView>> views;
    views.reserve(relations.size());
    for (const auto& relation : relations)
        views.push_back(relation.view());
    return views;
}

inline urd::Store<urd::sequence<ygg::uint_t>> decode(std::span<const ygg::Value> row)
{
    urd::Store<urd::sequence<ygg::uint_t>> values;
    values.reserve(row.size());
    for (const auto value : row)
        values.push_back(ygg::decode_value<ygg::uint_t>(value));
    return values;
}

/// A row's values, as it is held (a row view, or a builder's row child resolved), decoded.
template<typename R>
    requires(!std::convertible_to<const R&, std::span<const ygg::Value>>)
urd::Store<urd::sequence<ygg::uint_t>> decode(const R& row)
{
    const auto values = values_of(row);
    return decode(std::span<const ygg::Value>(values));
}

/// Half the distance to the targets in a supergraph of every state: consistent in all of them.
struct HalfDistance
{
    urd::Store<urd::map<urd::Store<urd::sequence<ygg::uint_t>>, ygg::uint_t>> distances;
    std::span<const urd::data<ygg::ColumnLayout>> columns;

    [[nodiscard]] ygg::uint_t estimate(std::span<const ygg::Value> vertex) const
    {
        const auto found = distances.find(decode(vertex));
        return found == distances.end() ? std::numeric_limits<ygg::uint_t>::max() : found->second / 2;
    }
};

/// Backward breadth-first distances to the targets over materialized edges.
inline urd::Store<urd::map<urd::Store<urd::sequence<ygg::uint_t>>, ygg::uint_t>>
distances_to(ygg::FreeRelationBuilderView edges, ygg::FreeRelationBuilderView targets, size_t k)
{
    urd::Store<urd::map<urd::Store<urd::sequence<ygg::uint_t>>, ygg::uint_t>> result;
    urd::Store<urd::sequence<urd::Store<urd::sequence<ygg::uint_t>>>> frontier;
    for (size_t i = 0; i < targets.get_rows().size(); ++i)
    {
        const auto vertex = decode(targets.row(i));
        if (result.emplace(vertex, 0).second)
            frontier.push_back(vertex);
    }
    for (ygg::uint_t level = 1; !frontier.empty(); ++level)
    {
        urd::Store<urd::sequence<urd::Store<urd::sequence<ygg::uint_t>>>> next;
        for (size_t i = 0; i < edges.get_rows().size(); ++i)
        {
            const auto edge = decode(edges.row(i));
            const auto from = urd::Store<urd::sequence<ygg::uint_t>>(edge.begin(), edge.begin() + static_cast<std::ptrdiff_t>(k));
            const auto to = urd::Store<urd::sequence<ygg::uint_t>>(edge.begin() + static_cast<std::ptrdiff_t>(k), edge.end());
            if (std::ranges::find(frontier, to) != frontier.end() && result.emplace(from, level).second)
                next.push_back(from);
        }
        frontier = std::move(next);
    }
    return result;
}
}  // namespace runir::tests::distances

#endif
