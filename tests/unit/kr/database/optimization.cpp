#include "query_helpers.hpp"

#include <array>
#include <cmath>
#include <gtest/gtest.h>
#include <limits>
#include <random>
#include <runir/kr/database/optimization/details/structural.hpp>
#include <runir/kr/database/optimization/optimization.hpp>
#include <runir/kr/database/semantics/bound/query_evaluation.hpp>
#include <runir/kr/database/semantics/incremental/query.hpp>
#include <runir/kr/database/semantics/query_evaluation.hpp>
#include <runir/kr/database/semantics/relation_view.hpp>
#include <runir/kr/database/syntax/formatter.hpp>
#include <set>
#include <string>
#include <type_traits>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <yggdrasil/formalism/relation_view.hpp>

namespace
{
using Column = urd::index<ygg::Column>;
using runir::tests::borrow;
using runir::tests::Relation;
using runir::tests::scratch;
using runir::tests::values_of;
using PlanId = urd::index<runir::kr::database::Query<>>;
using Delta = runir::tests::Change;
using Change = std::pair<ygg::FreeRelationBuilderView, ygg::FreeRelationBuilderView>;

const Column w(3), x(0), y(1), z(2);

auto rows(const auto& relation)
{
    urd::Store<urd::sequence<urd::Store<urd::sequence<ygg::Value>>>> result;
    result.reserve(relation.rows().size());
    for (const auto& row : relation.rows())
    {
        auto& values = result.emplace_back();
        for (const auto value : values_of(row))
            values.push_back(value);
    }
    std::ranges::sort(result);
    return result;
}

auto difference(const auto& left_operand, const auto& right_operand)
{
    std::remove_cvref_t<decltype(left_operand)> result;
    std::ranges::set_difference(left_operand, right_operand, std::back_inserter(result));
    return result;
}

template<class Tag>
size_t count(const runir::kr::database::QueryPlan& plan)
{
    size_t result = 0;
    for (size_t position = 0; position < plan.node_count(); ++position)
        result += runir::kr::database::is<Tag>(plan[PlanId(ygg::to_uint_t(position))]);
    return result;
}

urd::Store<urd::sequence<Column>> labels(std::initializer_list<Column> values)
{
    urd::Store<urd::sequence<Column>> result(values);
    std::ranges::sort(result);
    return result;
}

TEST(DatabaseOptimization, CollectsCanonicalTypedDistinctCounts)
{
    const std::vector<urd::data<ygg::ColumnLayout>> columns { { x, ygg::type_id<double> }, { y, ygg::type_id<bool> } };
    Relation relation(columns);
    relation.insert(std::array { ygg::encode_value(0.0), ygg::encode_value(false) });
    relation.insert(std::array { ygg::encode_value(-0.0), ygg::encode_value(true) });
    relation.insert(std::array { ygg::encode_value(std::numeric_limits<double>::quiet_NaN()), ygg::encode_value(false) });
    const auto stats = runir::kr::database::collect_statistics(std::array { borrow(relation) });
    EXPECT_EQ(stats.inputs.at(0).rows, 3);
    EXPECT_EQ(stats.inputs.at(0).distinct.at(x), 2);
    EXPECT_EQ(stats.inputs.at(0).distinct.at(y), 2);
}

TEST(DatabaseOptimization, AveragesStatisticsOverSamplesAndWarnsAboutAlwaysEmptyKeys)
{
    const std::vector<urd::data<ygg::ColumnLayout>> columns { { x, ygg::type_id<double> }, { y, ygg::type_id<double> } };
    Relation two(columns), four(columns), empty(columns);
    two.insert(std::array { ygg::encode_value(0.0), ygg::encode_value(0.0) });
    two.insert(std::array { ygg::encode_value(0.0), ygg::encode_value(1.0) });
    for (int i = 0; i < 4; ++i)
        four.insert(std::array { ygg::encode_value(static_cast<double>(i)), ygg::encode_value(0.0) });

    auto accumulator = runir::kr::database::StatisticsAccumulator<std::string>();
    for (const auto* sample : { &two, &four, &empty })
    {
        accumulator.add("sometimes", borrow(*sample));
        accumulator.add("always", borrow(empty));
    }
    auto warnings = ygg::diagnostics::Warnings();
    const auto stats = accumulator.finish(warnings, [](const std::string& key) { return key; });

    EXPECT_EQ(stats.at("sometimes").rows, 2);                  // (2 + 4 + 0) / 3
    EXPECT_EQ(stats.at("sometimes").distinct.at(x), 5.0 / 3);  // (1 + 4 + 0) / 3
    EXPECT_EQ(stats.at("sometimes").distinct.at(y), 1);        // (2 + 1 + 0) / 3, clamped into [1, rows]
    EXPECT_EQ(stats.at("always").rows, 0);
    ASSERT_EQ(warnings.get().size(), 1);
    EXPECT_NE(warnings.get()[0].message.find("always was empty in all 3 samples"), std::string::npos);
}

TEST(DatabaseOptimization, RejectsInvalidStatistics)
{
    auto queries = runir::tests::qb::repository();
    const auto input = runir::tests::qb::input(queries, 0, { x });
    EXPECT_NO_THROW(runir::kr::database::optimize(input));
    runir::kr::database::Statistics statistics;
    statistics.inputs[0].rows = 3;
    EXPECT_NO_THROW(runir::kr::database::optimize(input, statistics));
    const auto valid = statistics;
    for (const double rows : { -1.0, std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN() })
    {
        statistics = valid;
        statistics.inputs[0].rows = rows;
        EXPECT_THROW(runir::kr::database::optimize(input, statistics), std::invalid_argument);
    }
    statistics = valid;
    statistics.inputs[0].distinct[x] = 4;
    EXPECT_THROW(runir::kr::database::optimize(input, statistics), std::invalid_argument);
    statistics = valid;
    statistics.inputs[0].distinct[z] = 1;
    EXPECT_THROW(runir::kr::database::optimize(input, statistics), std::invalid_argument);
}

TEST(DatabaseOptimization, GyoRecognizesAcyclicAndCyclicHypergraphs)
{
    using runir::kr::database::detail::join_tree;
    const auto path = join_tree({ labels({ x, y }), labels({ y, z }), labels({ z, w }) });
    ASSERT_TRUE(path);
    EXPECT_EQ(std::ranges::count_if(path.value(), [](const auto& parent) { return !parent; }), 1);
    EXPECT_TRUE(join_tree({ labels({ x, y }), labels({ x, z }), labels({ x, w }) }));
    EXPECT_TRUE(join_tree({ labels({ x, y, z }), labels({ x, y }), labels({ y, z }), labels({ z, x }) }));
    EXPECT_FALSE(join_tree({ labels({ x, y }), labels({ y, z }), labels({ z, x }) }));
    EXPECT_TRUE(join_tree({ labels({ x }), labels({ y }) }));
}

TEST(DatabaseOptimization, YannakakisBoundsIntermediatesByInputTimesOutput)
{
    auto queries = runir::tests::qb::repository();
    const auto r = runir::tests::qb::input(queries, 0, { x, y });
    const auto s = runir::tests::qb::input(queries, 1, { y, z });
    const auto t = runir::tests::qb::input(queries, 2, { z, w });
    const auto root = runir::tests::qb::project(queries, runir::tests::qb::join(queries, runir::tests::qb::join(queries, r, s), t), { x });
    std::array<Relation, 3> inputs { Relation(runir::tests::ops::cols({ x, y })),
                                     Relation(runir::tests::ops::cols({ y, z })),
                                     Relation(runir::tests::ops::cols({ z, w })) };
    for (ygg::uint_t i = 0; i < 10; ++i)
    {
        inputs[0].insert(std::array { ygg::encode_value(i), ygg::encode_value(static_cast<ygg::uint_t>(0)) });
        inputs[1].insert(std::array { ygg::encode_value(static_cast<ygg::uint_t>(0)), ygg::encode_value(i) });
        inputs[2].insert(std::array { ygg::encode_value(i), ygg::encode_value((i % 2)) });
        inputs[2].insert(std::array { ygg::encode_value(i + 100), ygg::encode_value(static_cast<ygg::uint_t>(0)) });  // dangling
    }
    const auto refs = std::array { borrow(inputs[0]), borrow(inputs[1]), borrow(inputs[2]) };
    const auto plan = runir::kr::database::optimize(root);
    EXPECT_EQ(count<runir::kr::database::QueryGenericJoinTag>(plan), 0);
    // Evaluate every operator of the plan as its own root.
    urd::Store<urd::sequence<runir::kr::database::QueryView<>>> nodes;
    nodes.reserve(plan.node_count());
    for (size_t position = 0; position < plan.node_count(); ++position)
        nodes.push_back(plan[PlanId(ygg::to_uint_t(position))]);
    runir::kr::database::QueryEvaluator evaluator(scratch(), runir::kr::database::compile_plan(std::span<const runir::kr::database::QueryView<>>(nodes)));
    evaluator.evaluate(refs);
    runir::kr::database::QueryEvaluator reference(scratch(), runir::kr::database::compile_plan({ root })), optimized(scratch(), plan);
    reference.evaluate(refs);
    optimized.evaluate(refs);
    EXPECT_EQ(rows(optimized.get_result()), rows(reference.get_result()));
    // Yannakakis' bound for projected acyclic joins: O(|input| · |output|).
    const auto bound = (inputs[0].size() + inputs[1].size() + inputs[2].size()) * reference.get_result().rows().size();
    for (size_t position = 0; position < nodes.size(); ++position)
        EXPECT_LE(evaluator.get_result(position).rows().size(), bound);
}

TEST(DatabaseOptimization, BoundRootsKeepTheirJoinStructure)
{
    // π_{x,w}(R(x,y) ⋈ S(y,z) ⋈ T(z,w)), evaluated with x bound as the edges of a distance are. Yannakakis' semijoins
    // would flatten into extra atoms of every binding's plan; a bound root keeps its three.
    auto queries = runir::tests::qb::repository();
    const auto r = runir::tests::qb::input(queries, 0, { x, y });
    const auto s = runir::tests::qb::input(queries, 1, { y, z });
    const auto t = runir::tests::qb::input(queries, 2, { z, w });
    const auto root = runir::tests::qb::project(queries, runir::tests::qb::join(queries, runir::tests::qb::join(queries, r, s), t), { x, w });
    const auto atoms = [](runir::kr::database::QueryView<> query)
    {
        size_t result = 0;
        for (const auto& branch : runir::kr::database::bound::union_normal_form(query).branches)
            result += branch.atoms.size();
        return result;
    };
    const auto roots = std::array { root };
    const auto bound = std::array { runir::kr::database::Evaluation::bound };
    const auto plan =
        runir::kr::database::optimize(std::span<const runir::kr::database::QueryView<>>(roots), std::span<const runir::kr::database::Evaluation>(bound));
    const auto planned = plan[plan.roots().front()];
    EXPECT_EQ(atoms(planned), 3);
    const auto full = runir::kr::database::optimize(root);
    EXPECT_GT(atoms(full[full.roots().front()]), 3);
    std::array<Relation, 3> inputs { Relation(runir::tests::ops::cols({ x, y })),
                                     Relation(runir::tests::ops::cols({ y, z })),
                                     Relation(runir::tests::ops::cols({ z, w })) };
    for (ygg::uint_t i = 0; i < 10; ++i)
    {
        inputs[0].insert(std::array { ygg::encode_value(i % 3), ygg::encode_value(i) });
        inputs[1].insert(std::array { ygg::encode_value(i), ygg::encode_value(i % 4) });
        inputs[2].insert(std::array { ygg::encode_value(i % 4), ygg::encode_value(i) });
    }
    const auto successors = [&](runir::kr::database::QueryView<> query)
    {
        auto evaluator = runir::kr::database::bound::QueryEvaluator(scratch(), query);
        for (size_t slot = 0; slot < inputs.size(); ++slot)
            evaluator.bind_slot(slot, borrow(inputs[slot]));
        const auto binding = evaluator.prepare({ 0 });
        auto result = urd::Store<urd::sequence<urd::Store<urd::sequence<ygg::Value>>>>();
        evaluator.evaluate(binding,
                           std::array { ygg::encode_value(static_cast<ygg::uint_t>(1)) },
                           [&](std::span<const ygg::Value> row)
                           {
                               result.emplace_back(row.begin(), row.end());
                               return true;
                           });
        std::ranges::sort(result);
        return result;
    };
    EXPECT_FALSE(successors(root).empty());
    EXPECT_EQ(successors(planned), successors(root));
    EXPECT_THROW((runir::kr::database::optimize(std::span<const runir::kr::database::QueryView<>>(roots), std::span<const runir::kr::database::Evaluation>())),
                 std::invalid_argument);
}

TEST(DatabaseOptimization, StructureChoosesGenericJoinOnlyForCyclicBlocks)
{
    auto queries = runir::tests::qb::repository();
    const auto a = runir::tests::qb::input(queries, 0, { x, y });
    const auto b = runir::tests::qb::input(queries, 1, { y, z });
    const auto c = runir::tests::qb::input(queries, 2, { z, x });
    const auto triangle = runir::tests::qb::join(queries, runir::tests::qb::join(queries, a, b), c);
    const auto path = runir::tests::qb::join(queries, a, b);
    EXPECT_EQ(count<runir::kr::database::QueryGenericJoinTag>(runir::kr::database::optimize(triangle)), 1);
    EXPECT_EQ(count<runir::kr::database::QueryGenericJoinTag>(runir::kr::database::optimize(path)), 0);
    // A measured input alone does not make the block measured.
    runir::kr::database::Statistics partial;
    partial.inputs[0] = { 10, {} };
    EXPECT_EQ(count<runir::kr::database::QueryGenericJoinTag>(runir::kr::database::optimize(triangle, partial)), 1);
}

TEST(DatabaseOptimization, CostBasedPlanChoosesTheSelectiveSubjoin)
{
    auto queries = runir::tests::qb::repository();
    const auto a = runir::tests::qb::input(queries, 0, { x, y });
    const auto b = runir::tests::qb::input(queries, 1, { y, z });
    const auto c = runir::tests::qb::input(queries, 2, { z });
    const auto root = runir::tests::qb::join(queries, runir::tests::qb::join(queries, a, b), c);
    runir::kr::database::Statistics statistics;
    statistics.inputs[0] = { 10000, { { x, 10000 }, { y, 10000 } } };
    statistics.inputs[1] = { 10000, { { y, 10000 }, { z, 10000 } } };
    statistics.inputs[2] = { 1, { { z, 1 } } };
    const auto plan = runir::kr::database::optimize(root, statistics);
    EXPECT_EQ(count<runir::kr::database::QueryGenericJoinTag>(plan), 0);
    bool found = false;
    for (size_t position = 0; position < plan.node_count(); ++position)
        std::visit(
            [&]<typename Concrete>(Concrete query)
            {
                if constexpr (std::same_as<Concrete, runir::kr::database::QueryView<runir::kr::database::QueryJoinTag>>)
                {
                    std::set<size_t> slots;
                    for (const auto child : { query.left_operand(), query.right_operand() })
                        for (const auto node : runir::kr::database::reachable(std::span<const runir::kr::database::QueryView<>>(&child, 1)))
                            std::visit(
                                [&]<typename Leaf>(Leaf leaf)
                                {
                                    if constexpr (std::same_as<Leaf, runir::kr::database::QueryView<runir::kr::database::QueryInputTag>>)
                                        slots.insert(leaf.input_slot());
                                },
                                node.variant());
                    found |= slots == std::set<size_t> { 1, 2 };
                }
            },
            plan[PlanId(ygg::to_uint_t(position))].variant());
    EXPECT_TRUE(found);
}

TEST(DatabaseOptimization, CostBasedPlanUsesGenericJoinWhereJoinsGrow)
{
    auto queries = runir::tests::qb::repository();
    const auto a = runir::tests::qb::input(queries, 0, { x, y });
    const auto b = runir::tests::qb::input(queries, 1, { y, z });
    const auto c = runir::tests::qb::input(queries, 2, { z, x });
    const auto root = runir::tests::qb::join(queries, runir::tests::qb::join(queries, a, b), c);
    // Every pairwise join grows: 10^4 · 10^4 / 100 rows from inputs of 10^4 rows.
    runir::kr::database::Statistics statistics;
    statistics.inputs[0] = { 10000, { { x, 100 }, { y, 100 } } };
    statistics.inputs[1] = { 10000, { { y, 100 }, { z, 100 } } };
    statistics.inputs[2] = { 10000, { { z, 100 }, { x, 100 } } };
    EXPECT_EQ(count<runir::kr::database::QueryGenericJoinTag>(runir::kr::database::optimize(root, statistics)), 1);
    // With key-like columns no join grows, and binary joins remain.
    for (const auto& [slot, input] : statistics.inputs)
        for (const auto& [column, count] : input.distinct)
            statistics.inputs.at(slot).distinct.at(column) = 10000;
    EXPECT_EQ(count<runir::kr::database::QueryGenericJoinTag>(runir::kr::database::optimize(root, statistics)), 0);
}

TEST(DatabaseOptimization, NullaryGenericJoinHasOneRow)
{
    auto queries = runir::tests::qb::repository();
    const auto root = runir::tests::qb::generic_join(queries, {}, {});
    for (const auto& plan : { runir::kr::database::optimize(root), runir::kr::database::optimize(root, runir::kr::database::Statistics { 4, {}, {} }) })
    {
        runir::kr::database::QueryEvaluator evaluator(scratch(), plan);
        evaluator.evaluate(std::array<ygg::FreeRelationBuilderView, 0> {});
        EXPECT_EQ(evaluator.get_result().rows().size(), 1);
    }
}

TEST(DatabaseOptimization, BothModesPreserveDifferencesRenamesAndExistentialVariables)
{
    auto queries = runir::tests::qb::repository();
    const auto a = runir::tests::qb::input(queries, 0, { x, y });
    const auto b = runir::tests::qb::input(queries, 1, { x, y });
    const auto edges = runir::tests::qb::input(queries, 2, { y, z });
    const auto projection = runir::tests::qb::project(queries, a, { x });
    const auto renamed = runir::tests::qb::rename(queries, a, { y, x });
    const auto selected = runir::tests::qb::select_value(queries, renamed, y, static_cast<ygg::uint_t>(1));
    const auto roots = std::array { runir::tests::qb::join(queries, projection, edges),
                                    runir::tests::qb::project(queries, runir::tests::qb::difference(queries, a, b), { x }),
                                    runir::tests::qb::difference(queries, projection, runir::tests::qb::project(queries, b, { x })),
                                    selected,
                                    runir::tests::qb::project(queries, runir::tests::qb::union_(queries, a, b), { y, x }),
                                    runir::tests::qb::join(queries, runir::tests::qb::union_(queries, a, b), edges),
                                    runir::tests::qb::join(queries, a, runir::tests::qb::project(queries, a, { x })),
                                    runir::tests::qb::project(queries, runir::tests::qb::join(queries, a, edges), { z, x }),
                                    runir::tests::qb::project(queries, a, { x }),
                                    runir::tests::qb::project(queries, a, { x }) };
    std::array<Relation, 3> inputs { Relation(runir::tests::ops::cols({ x, y })),
                                     Relation(runir::tests::ops::cols({ x, y })),
                                     Relation(runir::tests::ops::cols({ y, z })) };
    inputs[0].insert(std::array { ygg::encode_value(static_cast<ygg::uint_t>(1)), ygg::encode_value(static_cast<ygg::uint_t>(2)) });
    inputs[0].insert(std::array { ygg::encode_value(static_cast<ygg::uint_t>(1)), ygg::encode_value(static_cast<ygg::uint_t>(3)) });
    inputs[1].insert(std::array { ygg::encode_value(static_cast<ygg::uint_t>(1)), ygg::encode_value(static_cast<ygg::uint_t>(2)) });
    inputs[2].insert(std::array { ygg::encode_value(static_cast<ygg::uint_t>(0)), ygg::encode_value(static_cast<ygg::uint_t>(2)) });
    const auto refs = std::array { borrow(inputs[0]), borrow(inputs[1]), borrow(inputs[2]) };
    const auto statistics = runir::kr::database::collect_statistics(refs);
    const auto unmeasured = runir::kr::database::optimize(std::span<const runir::kr::database::QueryView<>>(roots));
    const auto measured = runir::kr::database::optimize(std::span<const runir::kr::database::QueryView<>>(roots), statistics);
    runir::kr::database::QueryEvaluator reference(scratch(), runir::kr::database::compile_plan(roots));
    reference.evaluate(refs);
    EXPECT_EQ(reference.get_result(0).rows().size(), 1);
    EXPECT_EQ(reference.get_result(1).rows().size(), 1);
    EXPECT_TRUE(reference.get_result(2).rows().empty());
    for (const auto* chosen : { &unmeasured, &measured })
    {
        EXPECT_EQ(chosen->roots()[8], chosen->roots()[9]);
        runir::kr::database::QueryEvaluator full(scratch(), *chosen);
        runir::kr::database::incremental::QueryEvaluator incremental(scratch(), *chosen);
        full.evaluate(refs);
        incremental.initialize(refs);
        for (size_t root = 0; root < roots.size(); ++root)
        {
            EXPECT_EQ(rows(full.get_result(root)), rows(reference.get_result(root)));
            EXPECT_EQ(rows(incremental.get_result(root)), rows(reference.get_result(root)));
            EXPECT_TRUE(std::ranges::equal(full.get_result(root).columns(), columns_of(roots[root])));
        }
    }
    runir::kr::database::incremental::QueryEvaluator incremental(scratch(), measured);
    incremental.initialize(refs);
    // NOLINTNEXTLINE(bugprone-random-generator-seed): a fixed seed keeps the test reproducible
    std::mt19937 random(1597);
    for (size_t iteration = 0; iteration < 35; ++iteration)
    {
        urd::Store<urd::sequence<decltype(rows(borrow(inputs[0])))>> before;
        before.reserve(roots.size());
        for (size_t root = 0; root < roots.size(); ++root)
            before.push_back(rows(incremental.get_result(root)));
        urd::Store<urd::sequence<Delta>> deltas;
        for (auto& input : inputs)
        {
            deltas.emplace_back(input.columns());
            for (ygg::uint_t first = 0; first < 4; ++first)
                for (ygg::uint_t second = 0; second < 4; ++second)
                {
                    if (random() % 4 != 0)
                        continue;
                    const auto row = std::array { ygg::encode_value(first), ygg::encode_value(second) };
                    if (input.contains(row))
                    {
                        deltas.back().removed.insert(row);
                        input.erase(row);
                    }
                    else
                    {
                        deltas.back().added.insert(row);
                        input.insert(row);
                    }
                }
        }
        urd::Store<urd::sequence<Change>> changes;
        changes.reserve(deltas.size());
        for (const auto& change : deltas)
            changes.push_back(change.change());
        incremental.update(changes);
        reference.evaluate(refs);
        for (size_t root = 0; root < roots.size(); ++root)
        {
            SCOPED_TRACE(iteration);
            SCOPED_TRACE(root);
            const auto after = rows(reference.get_result(root));
            EXPECT_EQ(rows(incremental.get_result(root)), after);
            EXPECT_EQ(rows(incremental.get_delta(root).change().first), difference(after, before[root]));
            EXPECT_EQ(rows(incremental.get_delta(root).change().second), difference(before[root], after));
        }
    }
}

TEST(DatabaseOptimization, BothModesMatchRandomRelationalExpressions)
{
    auto queries = runir::tests::qb::repository();
    const auto a = runir::tests::qb::input(queries, 0, { x, y });
    const auto b = runir::tests::qb::input(queries, 1, { x, y });
    const auto c = runir::tests::qb::input(queries, 2, { y, z });
    const auto e = runir::tests::qb::input(queries, 3, { x, z });
    const auto leaves = std::array { a, b, c, e };
    std::array<Relation, 4> inputs { Relation(runir::tests::ops::cols({ x, y })),
                                     Relation(runir::tests::ops::cols({ x, y })),
                                     Relation(runir::tests::ops::cols({ y, z })),
                                     Relation(runir::tests::ops::cols({ x, z })) };
    const auto refs = std::array { borrow(inputs[0]), borrow(inputs[1]), borrow(inputs[2]), borrow(inputs[3]) };
    // NOLINTNEXTLINE(bugprone-random-generator-seed): a fixed seed keeps the test reproducible
    std::mt19937 random(12553);
    const auto labels = [](runir::kr::database::QueryView<> query)
    {
        urd::Store<urd::sequence<Column>> result;
        for (const auto& column : columns_of(query))
            result.push_back(column.label);
        return result;
    };
    const auto peer = [&](runir::kr::database::QueryView<> query)
    {
        const auto target = labels(query);
        auto result = target.size() == 3 ? runir::tests::qb::join(queries, a, c) : leaves[random() % leaves.size()];
        auto columns = labels(result);
        columns.resize(target.size());
        result = runir::tests::qb::project(queries, result, columns);
        return runir::tests::qb::rename(queries, result, target);
    };
    for (size_t expression = 0; expression < 64; ++expression)
    {
        auto root = leaves[random() % leaves.size()];
        for (size_t depth = 0; depth < 4; ++depth)
        {
            auto columns = labels(root);
            switch (random() % 9)
            {
                case 0:
                    root = runir::tests::qb::join(queries, root, leaves[random() % leaves.size()]);
                    break;
                case 1:
                    std::shuffle(columns.begin(), columns.end(), random);
                    columns.resize(random() % (columns.size() + 1));
                    root = runir::tests::qb::project(queries, root, columns);
                    break;
                case 2:
                {
                    std::array names { x, y, z };
                    std::shuffle(names.begin(), names.end(), random);
                    root = runir::tests::qb::rename(queries, root, std::span<const Column>(names.data(), columns.size()));
                    break;
                }
                case 3:
                    if (!columns.empty())
                        root = runir::tests::qb::select_equal(queries, root, columns[random() % columns.size()], columns[random() % columns.size()]);
                    break;
                case 4:
                    if (!columns.empty())
                        root = runir::tests::qb::select_value(queries, root, columns[random() % columns.size()], static_cast<ygg::uint_t>(random() % 4));
                    break;
                case 5:
                    root = runir::tests::qb::union_(queries, root, peer(root));
                    break;
                case 6:
                    root = runir::tests::qb::difference(queries, root, peer(root));
                    break;
                case 7:
                    root = runir::tests::qb::semijoin(queries, root, leaves[random() % leaves.size()]);
                    break;
                case 8:
                    root = runir::tests::qb::antijoin(queries, root, leaves[random() % leaves.size()]);
                    break;
                default:
                    ADD_FAILURE() << "random() % 9 is below 9";
                    break;
            }
        }
        for (auto& input : inputs)
        {
            input.clear();
            for (ygg::uint_t left = 0; left < 4; ++left)
                for (ygg::uint_t right = 0; right < 4; ++right)
                    if (random() % 3 != 0)
                        input.insert(std::array { ygg::encode_value(left), ygg::encode_value(right) });
        }
        const auto baseline = runir::kr::database::compile_plan({ root });
        SCOPED_TRACE(expression);
        SCOPED_TRACE(runir::kr::database::explain(baseline));
        const auto statistics = runir::kr::database::collect_statistics(refs);
        const auto unmeasured = runir::kr::database::optimize(root);
        const auto measured = runir::kr::database::optimize(root, statistics);
        runir::kr::database::QueryEvaluator reference(scratch(), baseline);
        reference.evaluate(refs);
        for (const auto* optimized : { &unmeasured, &measured })
        {
            runir::kr::database::QueryEvaluator evaluator(scratch(), *optimized);
            evaluator.evaluate(refs);
            EXPECT_TRUE(std::ranges::equal(evaluator.get_result().columns(), columns_of(root)));
            EXPECT_EQ(rows(evaluator.get_result()), rows(reference.get_result()));
        }
    }
}

TEST(DatabaseOptimization, BothModesPreserveFactoredUnionsAndFilteredDifferences)
{
    auto queries = runir::tests::qb::repository();
    const auto a = runir::tests::qb::input(queries, 0, { x, y });
    const auto b = runir::tests::qb::input(queries, 1, { x, y });
    const auto c = runir::tests::qb::input(queries, 2, { y, z });
    const auto selected_difference = runir::tests::qb::select_value(queries,
                                                                    runir::tests::qb::rename(queries, runir::tests::qb::difference(queries, a, b), { y, x }),
                                                                    y,
                                                                    static_cast<ygg::uint_t>(1));
    const auto roots =
        std::array { runir::tests::qb::union_(queries, runir::tests::qb::join(queries, a, c), runir::tests::qb::join(queries, b, c)),
                     runir::tests::qb::join(queries, runir::tests::qb::union_(queries, a, b), c),
                     selected_difference,
                     runir::tests::qb::select_equal(queries, runir::tests::qb::rename(queries, runir::tests::qb::difference(queries, a, b), { y, x }), y, x),
                     runir::tests::qb::union_(queries, a, runir::tests::qb::join(queries, a, b)),
                     runir::tests::qb::join(queries, a, runir::tests::qb::union_(queries, a, b)) };
    Relation ar(runir::tests::ops::cols({ x, y })), br(runir::tests::ops::cols({ x, y })), cr(runir::tests::ops::cols({ y, z }));
    ar.insert(std::array { ygg::encode_value(static_cast<ygg::uint_t>(1)), ygg::encode_value(static_cast<ygg::uint_t>(1)) });
    ar.insert(std::array { ygg::encode_value(static_cast<ygg::uint_t>(1)), ygg::encode_value(static_cast<ygg::uint_t>(2)) });
    br.insert(std::array { ygg::encode_value(static_cast<ygg::uint_t>(1)), ygg::encode_value(static_cast<ygg::uint_t>(2)) });
    br.insert(std::array { ygg::encode_value(static_cast<ygg::uint_t>(2)), ygg::encode_value(static_cast<ygg::uint_t>(2)) });
    cr.insert(std::array { ygg::encode_value(static_cast<ygg::uint_t>(1)), ygg::encode_value(static_cast<ygg::uint_t>(3)) });
    cr.insert(std::array { ygg::encode_value(static_cast<ygg::uint_t>(2)), ygg::encode_value(static_cast<ygg::uint_t>(2)) });
    const auto inputs = std::array { borrow(ar), borrow(br), borrow(cr) };
    const auto statistics = runir::kr::database::collect_statistics(inputs);
    const auto original = runir::kr::database::compile_plan(roots);
    const auto unmeasured = runir::kr::database::optimize(std::span<const runir::kr::database::QueryView<>>(roots));
    const auto measured = runir::kr::database::optimize(std::span<const runir::kr::database::QueryView<>>(roots), statistics);
    runir::kr::database::QueryEvaluator reference(scratch(), original);
    reference.evaluate(inputs);
    EXPECT_EQ(rows(reference.get_result(0)), rows(reference.get_result(1)));
    EXPECT_EQ(reference.get_result(2).rows().size(), 1);
    EXPECT_EQ(reference.get_result(3).rows().size(), 1);
    EXPECT_EQ(rows(reference.get_result(4)), rows(borrow(ar)));
    EXPECT_EQ(rows(reference.get_result(5)), rows(borrow(ar)));
    for (const auto* chosen : { &unmeasured, &measured })
    {
        runir::kr::database::QueryEvaluator full(scratch(), *chosen);
        runir::kr::database::incremental::QueryEvaluator incremental(scratch(), *chosen);
        full.evaluate(inputs);
        incremental.initialize(inputs);
        for (size_t root = 0; root < roots.size(); ++root)
        {
            EXPECT_EQ(rows(full.get_result(root)), rows(reference.get_result(root)));
            EXPECT_EQ(rows(incremental.get_result(root)), rows(reference.get_result(root)));
        }
    }
}
}  // namespace
