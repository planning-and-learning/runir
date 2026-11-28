#include "query_helpers.hpp"

#include <algorithm>
#include <array>
#include <cista/serialization.h>
#include <gtest/gtest.h>
#include <memory>
#include <random>
#include <runir/kr/database/semantics/incremental/query.hpp>
#include <runir/kr/database/semantics/query_evaluation.hpp>
#include <runir/kr/database/semantics/relation_view.hpp>
#include <runir/kr/database/syntax/query_repository.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <vector>

namespace
{
using Column = urd::index<ygg::Column>;
using runir::tests::borrow;
using runir::tests::Relation;
using runir::tests::scratch;
using runir::tests::values_of;
using Delta = runir::tests::Change;
using Change = std::pair<ygg::FreeRelationBuilderView, ygg::FreeRelationBuilderView>;

const Column x(0), y(1), z(2);

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

auto minus(const auto& left_operand, const auto& right_operand)
{
    std::remove_cvref_t<decltype(left_operand)> result;
    std::ranges::set_difference(left_operand, right_operand, std::back_inserter(result));
    return result;
}

void apply(Relation& relation, const Delta& change)
{
    const auto [added, removed] = change.change();
    for (const auto& row : removed.rows())
        EXPECT_TRUE(relation.erase(values_of(row)));
    for (const auto& row : added.rows())
        relation.insert(values_of(row));
}

void reverse(Delta& change) { std::swap(change.added, change.removed); }

template<typename T>
concept HasArgument = requires(T view) { view.operand(); };
template<typename T>
concept HasOperands = requires(T view) {
    view.left_operand();
    view.right_operand();
};
using JoinView = runir::kr::database::QueryView<runir::kr::database::QueryJoinTag>;
using ProjectView = runir::kr::database::QueryView<runir::kr::database::QueryProjectTag>;
static_assert(HasOperands<JoinView> && !HasArgument<JoinView>);
static_assert(HasArgument<ProjectView> && !HasOperands<ProjectView>);
static_assert(!HasArgument<runir::kr::database::QueryView<>> && !HasOperands<runir::kr::database::QueryView<>>);
static_assert(!std::copy_constructible<runir::kr::database::QueryRepository>);
static_assert(!std::convertible_to<urd::index<runir::kr::database::Query<runir::kr::database::QueryInputTag>>,
                                   urd::index<runir::kr::database::Query<runir::kr::database::QueryJoinTag>>>);

TEST(DatabaseQuery, TypedLogicalAlternativesKeepOperandsAndRootProvenance)
{
    auto repository = runir::tests::qb::repository();
    const auto a = runir::tests::qb::input(repository, 0, { x, y });
    const auto b = runir::tests::qb::input(repository, 1, { y, z });
    const auto empty = runir::tests::qb::empty(repository, { x, y });
    const auto joined = runir::tests::qb::join(repository, a, b);
    const auto projected = runir::tests::qb::project(repository, a, { x });
    const auto renamed = runir::tests::qb::rename(repository, a, { y, z });
    const auto equal = runir::tests::qb::select_equal(repository, a, x, y);
    const auto selected = runir::tests::qb::select_value(repository, a, x, static_cast<ygg::uint_t>(7));
    const auto united = runir::tests::qb::union_(repository, a, empty);
    const auto difference = runir::tests::qb::difference(repository, a, empty);
    const auto roots = std::array { a, empty, joined, projected, renamed, equal, selected, united, difference };
    EXPECT_EQ(runir::kr::database::as<runir::kr::database::QueryInputTag>(a).input_slot(), 0);
    EXPECT_TRUE(std::ranges::equal(runir::kr::database::as<runir::kr::database::QueryEmptyTag>(empty).columns(), columns_of(a)));
    EXPECT_EQ(runir::kr::database::as<runir::kr::database::QueryJoinTag>(joined).left_operand().index(), a.index());
    EXPECT_EQ(runir::kr::database::as<runir::kr::database::QueryJoinTag>(joined).right_operand().index(), b.index());
    EXPECT_EQ(runir::kr::database::as<runir::kr::database::QueryProjectTag>(projected).operand().index(), a.index());
    EXPECT_EQ(runir::kr::database::as<runir::kr::database::QueryProjectTag>(projected).labels().front(), x);
    EXPECT_EQ(runir::kr::database::as<runir::kr::database::QueryRenameTag>(renamed).labels().front(), y);
    EXPECT_EQ(runir::kr::database::as<runir::kr::database::QuerySelectEqualTag>(equal).left_column(), x);
    EXPECT_EQ(runir::kr::database::as<runir::kr::database::QuerySelectEqualTag>(equal).right_column(), y);
    EXPECT_EQ(runir::kr::database::as<runir::kr::database::QuerySelectValueTag>(selected).column(), x);
    EXPECT_EQ(ygg::decode_value<ygg::uint_t>(runir::kr::database::as<runir::kr::database::QuerySelectValueTag>(selected).constant()), 7);
    EXPECT_EQ(runir::kr::database::as<runir::kr::database::QueryUnionTag>(united).right_operand().index(), empty.index());
    EXPECT_EQ(runir::kr::database::as<runir::kr::database::QueryDifferenceTag>(difference).left_operand().index(), a.index());
    const auto count = repository.size();
    EXPECT_EQ(count, 10);
    const auto duplicate = runir::tests::qb::join(repository, a, b);
    EXPECT_EQ(duplicate.index(), joined.index());
    EXPECT_EQ(runir::kr::database::as<runir::kr::database::QueryJoinTag>(duplicate).index(),
              runir::kr::database::as<runir::kr::database::QueryJoinTag>(joined).index());
    EXPECT_EQ(repository.size(), count);
    EXPECT_EQ(runir::kr::database::as<runir::kr::database::QueryInputTag>(a).index().value,
              runir::kr::database::as<runir::kr::database::QueryJoinTag>(joined).index().value);
    EXPECT_NE(a.index(), joined.index());
    const auto plan = runir::kr::database::compile_plan(roots);
    for (size_t i = 0; i < roots.size(); ++i)
    {
        std::visit([&](auto operation) { EXPECT_TRUE(std::ranges::equal(operation.columns(), columns_of(roots[i]))); }, roots[i].variant());
        EXPECT_TRUE(std::ranges::equal(columns_of(plan[plan.roots()[i]]), columns_of(roots[i])));
    }
}

TEST(DatabaseQuery, RecordConstructorsMatchFieldsAndValidateConstantTypes)
{
    auto repository = runir::tests::qb::repository();
    const auto a = runir::tests::qb::input(repository, 0, { x, y });
    const auto b = runir::tests::qb::input(repository, 1, { y, z });
    const auto insert = [&]<typename Tag>(urd::data<runir::kr::database::Query<Tag>> data)
    {
        const auto concrete = repository.insert(data).first;
        auto erased = urd::data<runir::kr::database::Query<>>(concrete.index());
        return repository.insert(erased).first;
    };

    auto by_index = urd::data<runir::kr::database::Query<runir::kr::database::QueryJoinTag>>(a.index(), b.index(), {});
    EXPECT_EQ(by_index.left_operand, a.index());
    EXPECT_EQ(by_index.right_operand, b.index());
    EXPECT_EQ(insert(std::move(by_index)).index(), runir::tests::qb::join(repository, a, b).index());

    const std::array labels { x };
    EXPECT_EQ(insert(urd::data<runir::kr::database::Query<runir::kr::database::QueryProjectTag>>(a.index(), labels, {})).index(),
              runir::tests::qb::project(repository, a, { x }).index());
    EXPECT_EQ(insert(urd::data<runir::kr::database::Query<runir::kr::database::QuerySelectValueTag>>(a.index(),
                                                                                                     x,
                                                                                                     ygg::encode_value(ygg::uint_t(7)),
                                                                                                     ygg::type_id<ygg::uint_t>,
                                                                                                     {}))
                  .index(),
              runir::tests::qb::select_value(repository, a, x, ygg::uint_t(7)).index());
    const auto generic = insert(
        urd::data<runir::kr::database::Query<runir::kr::database::QueryGenericJoinTag>>(std::array { a.index(), b.index() }, std::array { y, x, z }, {}, {}));
    EXPECT_TRUE(std::ranges::equal(columns_of(generic) | std::views::transform(&urd::data<ygg::ColumnLayout>::label), std::array { x, y, z }));

    auto mismatched = urd::data<runir::kr::database::Query<runir::kr::database::QuerySelectValueTag>>(a.index(),
                                                                                                      x,
                                                                                                      ygg::encode_value(static_cast<std::int32_t>(7)),
                                                                                                      ygg::type_id<std::int32_t>,
                                                                                                      {});
    EXPECT_EQ(mismatched.constant_type, (ygg::type_id<std::int32_t>) );
    EXPECT_THROW(static_cast<void>(repository.insert(mismatched)), std::invalid_argument);
}

TEST(DatabaseQuery, InternsValidatedNodesAndCompilesIndependentReachablePlan)
{
    runir::kr::database::QueryPlan plan;
    {
        auto repository = runir::tests::qb::repository();
        auto other = runir::tests::qb::repository();
        const auto a = runir::tests::qb::input(repository, 0, { x, y });
        EXPECT_EQ(a.index(), runir::tests::qb::input(repository, 0, { x, y }).index());
        const auto same = runir::tests::qb::input(repository, 0, { x, y });
        const auto foreign = runir::tests::qb::input(other, 0, { x, y });
        EXPECT_NE(repository.layer(), other.layer());
        EXPECT_TRUE(urd::EqualTo<decltype(a)> {}(a, same));
        EXPECT_FALSE(urd::EqualTo<decltype(a)> {}(a, foreign));
        EXPECT_EQ(urd::Hash<decltype(a)> {}(a), urd::Hash<decltype(a)> {}(same));
        std::unordered_map<runir::kr::database::QueryView<>, int, urd::Hash<runir::kr::database::QueryView<>>, urd::EqualTo<runir::kr::database::QueryView<>>>
            keys;
        keys.emplace(a, 1);
        keys.emplace(same, 2);
        keys.emplace(foreign, 3);
        EXPECT_EQ(keys.size(), 2);
        EXPECT_THROW(runir::tests::qb::project(repository, a, { z }), std::out_of_range);
        EXPECT_THROW(runir::tests::qb::rename(repository, a, { x, x }), std::invalid_argument);
        const auto p = runir::tests::qb::project(repository, a, { x });
        EXPECT_EQ(p.index(), runir::tests::qb::project(repository, a, { x }).index());
        runir::tests::qb::input(repository, 9, { z });  // Unreachable inputs must not require bindings.
        const auto renamed = runir::tests::qb::rename(repository, p, { z });
        plan = runir::kr::database::compile_plan({ p, renamed, p });
        ASSERT_EQ(plan.node_count(), 3);
        EXPECT_EQ(plan.roots()[0], plan.roots()[2]);
        EXPECT_EQ(columns_of(plan[plan.roots()[1]])[0].label, z);
    }
    Relation input { runir::tests::ops::cols({ x, y }) };
    input.insert(std::array { ygg::encode_value(static_cast<ygg::uint_t>(2)), ygg::encode_value(static_cast<ygg::uint_t>(3)) });
    input.insert(std::array { ygg::encode_value(static_cast<ygg::uint_t>(2)), ygg::encode_value(static_cast<ygg::uint_t>(4)) });
    runir::kr::database::QueryEvaluator evaluator(scratch(), plan);
    EXPECT_THROW((void) evaluator.get_result(), std::logic_error);
    evaluator.evaluate(std::array { borrow(input) });
    EXPECT_EQ(evaluator.get_result(0).rows().size(), 1);
    EXPECT_EQ(evaluator.get_result(1).columns()[0].label, z);
    input.clear();
    EXPECT_EQ(evaluator.get_result().rows().size(), 1);
    EXPECT_THROW(evaluator.evaluate(std::array { evaluator.get_result() }), std::invalid_argument);
    EXPECT_EQ(evaluator.get_result().rows().size(), 1);
}

TEST(DatabaseQuery, IncrementalDagMatchesFullAndExactDeltasForwardBackward)
{
    auto repository = runir::tests::qb::repository();
    const auto a = runir::tests::qb::input(repository, 0, { x, y });
    const auto b = runir::tests::qb::input(repository, 1, { x, y });
    const auto c = runir::tests::qb::input(repository, 2, { y, z });
    const auto joined = runir::tests::qb::join(repository, a, c);
    const auto projection = runir::tests::qb::project(repository, joined, { x });
    const auto roots = std::array { joined,
                                    projection,
                                    runir::tests::qb::rename(repository, projection, { z }),
                                    runir::tests::qb::select_equal(repository, a, x, y),
                                    runir::tests::qb::select_value(repository, a, x, static_cast<ygg::uint_t>(1)),
                                    runir::tests::qb::union_(repository, a, b),
                                    runir::tests::qb::difference(repository, a, b),
                                    runir::tests::qb::join(repository, a, a),
                                    runir::tests::qb::project(repository, joined, {}),
                                    runir::tests::qb::empty(repository, { x }),
                                    runir::tests::qb::semijoin(repository, a, c),
                                    runir::tests::qb::antijoin(repository, a, c) };
    const auto plan = runir::kr::database::compile_plan(roots);
    runir::kr::database::QueryEvaluator full(scratch(), plan);
    runir::kr::database::incremental::QueryEvaluator incremental(scratch(), plan);
    Relation first(runir::tests::ops::cols({ x, y })), second(runir::tests::ops::cols({ x, y })), third(runir::tests::ops::cols({ y, z }));
    first.insert(std::array { ygg::encode_value(static_cast<ygg::uint_t>(1)), ygg::encode_value(static_cast<ygg::uint_t>(1)) });
    first.insert(std::array { ygg::encode_value(static_cast<ygg::uint_t>(2)), ygg::encode_value(static_cast<ygg::uint_t>(2)) });
    second.insert(std::array { ygg::encode_value(static_cast<ygg::uint_t>(1)), ygg::encode_value(static_cast<ygg::uint_t>(1)) });
    third.insert(std::array { ygg::encode_value(static_cast<ygg::uint_t>(1)), ygg::encode_value(static_cast<ygg::uint_t>(5)) });
    third.insert(std::array { ygg::encode_value(static_cast<ygg::uint_t>(2)), ygg::encode_value(static_cast<ygg::uint_t>(6)) });
    const auto inputs = std::array { borrow(first), borrow(second), borrow(third) };
    incremental.initialize(inputs);
    const auto compare = [&]
    {
        full.evaluate(inputs);
        for (size_t i = 0; i < roots.size(); ++i)
        {
            EXPECT_EQ(rows(incremental.get_result(i)), rows(full.get_result(i)));
            EXPECT_TRUE(std::ranges::equal(incremental.get_result(i).columns(), columns_of(roots[i])));
        }
    };
    compare();
    Delta da(first.columns()), dbb(second.columns()), dc(third.columns());
    da.removed.insert(std::array { ygg::encode_value(static_cast<ygg::uint_t>(1)), ygg::encode_value(static_cast<ygg::uint_t>(1)) });
    da.added.insert(std::array { ygg::encode_value(static_cast<ygg::uint_t>(1)), ygg::encode_value(static_cast<ygg::uint_t>(2)) });
    dbb.removed.insert(std::array { ygg::encode_value(static_cast<ygg::uint_t>(1)), ygg::encode_value(static_cast<ygg::uint_t>(1)) });
    dbb.added.insert(std::array { ygg::encode_value(static_cast<ygg::uint_t>(1)), ygg::encode_value(static_cast<ygg::uint_t>(2)) });
    dc.added.insert(std::array { ygg::encode_value(static_cast<ygg::uint_t>(2)), ygg::encode_value(static_cast<ygg::uint_t>(7)) });
    for (int iteration = 0; iteration < 2; ++iteration)
    {
        // Read each iteration: reverse swaps a change's relations, not their contents.
        const auto changes = std::array { std::pair { borrow(da.added), borrow(da.removed) },
                                          std::pair { borrow(dbb.added), borrow(dbb.removed) },
                                          std::pair { borrow(dc.added), borrow(dc.removed) } };
        urd::Store<urd::sequence<decltype(rows(borrow(first)))>> before;
        before.reserve(roots.size());
        for (size_t i = 0; i < roots.size(); ++i)
            before.push_back(rows(incremental.get_result(i)));
        incremental.update(changes);
        apply(first, da);
        apply(second, dbb);
        apply(third, dc);
        compare();
        for (size_t i = 0; i < roots.size(); ++i)
        {
            const auto after = rows(incremental.get_result(i));
            EXPECT_EQ(rows(incremental.get_delta(i).change().first), minus(after, before[i]));
            EXPECT_EQ(rows(incremental.get_delta(i).change().second), minus(before[i], after));
        }
        reverse(da);
        reverse(dbb);
        reverse(dc);
    }
    // A replaced projection witness keeps membership and must emit no change.
    EXPECT_TRUE(incremental.get_delta(1).added->view().rows().empty());
    EXPECT_TRUE(incremental.get_delta(1).removed->view().rows().empty());
    EXPECT_GT(incremental.memory_usage(), 0);
    EXPECT_GT(full.memory_usage(), 0);
}

TEST(DatabaseQuery, MixedTypesAndConstants)
{
    using TypedRelation = Relation;
    const std::vector<urd::data<ygg::ColumnLayout>> schema { { x, ygg::type_id<std::uint32_t> }, { y, ygg::type_id<double> } };
    auto mixed = runir::tests::qb::repository();
    auto input = runir::tests::qb::input(mixed, 0, schema);
    EXPECT_THROW(runir::tests::qb::select_equal(mixed, input, x, y), std::invalid_argument);
    EXPECT_THROW(runir::tests::qb::select_value(mixed, input, y, std::uint32_t(1)), std::invalid_argument);
    auto selected = runir::tests::qb::select_value(mixed, input, y, -0.0);
    const auto selected_view = std::get<runir::kr::database::QueryView<runir::kr::database::QuerySelectValueTag>>(selected.variant());
    EXPECT_EQ(selected_view.plan().position, 1);
    EXPECT_EQ(selected.index(), runir::tests::qb::select_value(mixed, input, y, 0.0).index());
    TypedRelation relation(schema);
    relation.insert(std::array { ygg::encode_value(static_cast<std::uint32_t>(4)), ygg::encode_value(0.0) });
    relation.insert(std::array { ygg::encode_value(static_cast<std::uint32_t>(5)), ygg::encode_value(2.0) });
    runir::kr::database::QueryEvaluator evaluator(scratch(), runir::kr::database::compile_plan({ runir::tests::qb::project(mixed, selected, { y, x }) }));
    evaluator.evaluate(std::array { borrow(relation) });
    EXPECT_TRUE(evaluator.get_result().contains(std::array { ygg::encode_value(0.0), ygg::encode_value(std::uint32_t(4)) }));
}

TEST(DatabaseQuery, RejectsInvalidBatchesBeforeMutatingAndSupportsOtherTypeFamilies)
{
    const auto schema = runir::tests::qb::columns_of<double>({ x });
    auto repository = runir::tests::qb::repository();
    const auto input = runir::tests::qb::input(repository, 0, schema);
    Relation relation(schema);
    relation.insert(std::array { ygg::encode_value(1.0) });
    const auto plan = runir::kr::database::compile_plan({ runir::tests::qb::select_value(repository, input, x, 1.0) });
    runir::kr::database::QueryEvaluator full(scratch(), plan);
    runir::kr::database::incremental::QueryEvaluator incremental(scratch(), plan);
    const auto inputs = std::array { borrow(relation) };
    full.evaluate(inputs);
    incremental.initialize(inputs);
    Delta invalid(relation.columns());
    invalid.added.insert(std::array { ygg::encode_value(1.0) });
    const auto changes = std::array { std::pair { borrow(invalid.added), borrow(invalid.removed) } };
    EXPECT_THROW(incremental.update(changes), std::invalid_argument);
    EXPECT_EQ(rows(incremental.get_result()), rows(full.get_result()));
    EXPECT_THROW(incremental.update(std::span<const Change> {}), std::invalid_argument);
    EXPECT_EQ(incremental.get_result().rows().size(), 1);
}

TEST(DatabaseQuery, RandomizedSharedSetDagMaintainsExactNetChanges)
{
    auto repository = runir::tests::qb::repository();
    const auto a = runir::tests::qb::input(repository, 0, { x, y });
    const auto b = runir::tests::qb::input(repository, 1, { x, y });
    const auto edges = runir::tests::qb::input(repository, 2, { y, z });
    const auto ready = runir::tests::qb::input(repository, 3, {});
    const auto u = runir::tests::qb::union_(repository, a, b);
    const auto difference = runir::tests::qb::difference(repository, a, b);
    const auto project_union = runir::tests::qb::project(repository, u, { x });
    const auto project_a = runir::tests::qb::project(repository, a, { x });
    const auto project_b = runir::tests::qb::project(repository, b, { x });
    const auto roots = std::array { runir::tests::qb::join(repository, u, edges),
                                    runir::tests::qb::join(repository, difference, edges),
                                    runir::tests::qb::project(repository, difference, { x }),
                                    project_union,
                                    runir::tests::qb::union_(repository, project_a, project_b),
                                    runir::tests::qb::difference(repository, project_a, project_b),
                                    runir::tests::qb::join(repository, project_a, project_b),
                                    runir::tests::qb::select_equal(repository, u, x, y),
                                    runir::tests::qb::select_value(repository, difference, x, static_cast<ygg::uint_t>(1)),
                                    runir::tests::qb::project(repository, runir::tests::qb::join(repository, a, edges), {}),
                                    runir::tests::qb::join(repository, ready, project_union),
                                    runir::tests::qb::rename(repository, project_union, { z }),
                                    runir::tests::qb::semijoin(repository, u, edges),
                                    runir::tests::qb::antijoin(repository, u, edges),
                                    runir::tests::qb::semijoin(repository, a, ready),
                                    runir::tests::qb::antijoin(repository, difference, ready),
                                    runir::tests::qb::antijoin(repository, project_union, project_b),
                                    // References: u ⋉ edges = π_{x,y}(u ⋈ edges) and u ▷ edges = u \ (u ⋉ edges).
                                    runir::tests::qb::project(repository, runir::tests::qb::join(repository, u, edges), { x, y }),
                                    u };
    constexpr size_t semijoin_root = 12, antijoin_root = 13, reference_root = 17, union_root = 18;
    const auto plan = runir::kr::database::compile_plan(roots);
    std::array<Relation, 4> inputs { Relation(runir::tests::ops::cols({ x, y })),
                                     Relation(runir::tests::ops::cols({ x, y })),
                                     Relation(runir::tests::ops::cols({ y, z })),
                                     Relation() };
    const auto refs = std::array { borrow(inputs[0]), borrow(inputs[1]), borrow(inputs[2]), borrow(inputs[3]) };
    runir::kr::database::QueryEvaluator full(scratch(), plan);
    runir::kr::database::incremental::QueryEvaluator incremental(scratch(), plan);
    incremental.initialize(refs);
    // NOLINTNEXTLINE(bugprone-random-generator-seed): a fixed seed keeps the test reproducible
    std::mt19937 random(741);
    for (size_t iteration = 0; iteration < 120; ++iteration)
    {
        urd::Store<urd::sequence<decltype(rows(borrow(inputs[0])))>> before;
        before.reserve(roots.size());
        for (size_t i = 0; i < roots.size(); ++i)
            before.push_back(rows(incremental.get_result(i)));
        urd::Store<urd::sequence<Delta>> changes;
        for (size_t input = 0; input < inputs.size(); ++input)
        {
            changes.emplace_back(inputs[input].columns());
            auto& relation = inputs[input];
            auto& change = changes.back();
            const auto toggle = [&](const auto& tuple)
            {
                if (random() % 3 != 0)
                    return;
                if (relation.contains(tuple))
                {
                    change.removed.insert(tuple);
                    relation.erase(tuple);
                }
                else
                {
                    change.added.insert(tuple);
                    relation.insert(tuple);
                }
            };
            if (input == 3)
                toggle(std::array<ygg::Value, 0> {});
            else
                for (ygg::uint_t i = 0; i < 3; ++i)
                    for (ygg::uint_t j = 0; j < 3; ++j)
                        toggle(std::array { ygg::encode_value(i), ygg::encode_value(j) });
        }
        urd::Store<urd::sequence<Change>> delta_refs;
        delta_refs.reserve(changes.size());
        for (const auto& change : changes)
            delta_refs.push_back(change.change());
        incremental.update(delta_refs);
        full.evaluate(refs);
        for (size_t root = 0; root < roots.size(); ++root)
        {
            SCOPED_TRACE(iteration);
            SCOPED_TRACE(root);
            const auto after = rows(full.get_result(root));
            EXPECT_EQ(rows(incremental.get_result(root)), after);
            EXPECT_EQ(rows(incremental.get_delta(root).change().first), minus(after, before[root]));
            EXPECT_EQ(rows(incremental.get_delta(root).change().second), minus(before[root], after));
        }
        const auto reference = rows(full.get_result(reference_root));
        EXPECT_EQ(rows(full.get_result(semijoin_root)), reference);
        EXPECT_EQ(rows(full.get_result(antijoin_root)), minus(rows(full.get_result(union_root)), reference));
    }
}

TEST(DatabaseQuery, GenericJoinPlanExecutesInBothModesAndPrunesAlternatives)
{
    auto repository = runir::tests::qb::repository();
    const auto ai = runir::tests::qb::input(repository, 0, { x, y });
    const auto bi = runir::tests::qb::input(repository, 1, { y, z });
    const auto ci = runir::tests::qb::input(repository, 2, { z, x });
    runir::tests::qb::input(repository, 8, { x, y });
    const auto root = runir::tests::qb::generic_join(repository, { ai, bi, ci }, { z, x, y });
    const auto plan = runir::kr::database::compile_plan({ root });
    ASSERT_EQ(plan.node_count(), 4);
    runir::kr::database::QueryEvaluator full(scratch(), plan);
    runir::kr::database::incremental::QueryEvaluator incremental(scratch(), plan);
    Relation ar(runir::tests::ops::cols({ x, y })), br(runir::tests::ops::cols({ y, z })), cr(runir::tests::ops::cols({ z, x }));
    ar.insert(std::array { ygg::encode_value(static_cast<ygg::uint_t>(1)), ygg::encode_value(static_cast<ygg::uint_t>(2)) });
    br.insert(std::array { ygg::encode_value(static_cast<ygg::uint_t>(2)), ygg::encode_value(static_cast<ygg::uint_t>(3)) });
    cr.insert(std::array { ygg::encode_value(static_cast<ygg::uint_t>(3)), ygg::encode_value(static_cast<ygg::uint_t>(1)) });
    const auto inputs = std::array { borrow(ar), borrow(br), borrow(cr) };
    full.evaluate(inputs);
    incremental.initialize(inputs);
    ASSERT_EQ(incremental.get_result().rows().size(), 1);
    EXPECT_EQ(rows(full.get_result()), rows(incremental.get_result()));
    Delta da(ar.columns()), dbb(br.columns()), dc(cr.columns());
    dc.removed.insert(std::array { ygg::encode_value(static_cast<ygg::uint_t>(3)), ygg::encode_value(static_cast<ygg::uint_t>(1)) });
    incremental.update(std::array { std::pair { borrow(da.added), borrow(da.removed) },
                                    std::pair { borrow(dbb.added), borrow(dbb.removed) },
                                    std::pair { borrow(dc.added), borrow(dc.removed) } });
    EXPECT_TRUE(incremental.get_result().rows().empty());
    EXPECT_EQ(incremental.get_delta().removed->view().rows().size(), 1);
    apply(cr, dc);
    full.evaluate(inputs);
    EXPECT_EQ(rows(full.get_result()), rows(incremental.get_result()));
}

TEST(DatabaseQuery, NullaryGenericJoinIsIdentityInBothModes)
{
    auto repository = runir::tests::qb::repository();
    const auto root = runir::tests::qb::generic_join(repository, {}, {});
    const auto plan = runir::kr::database::compile_plan({ root, root });
    ASSERT_EQ(plan.node_count(), 1);
    EXPECT_EQ(plan.roots().front(), plan.roots().back());
    runir::kr::database::QueryEvaluator full(scratch(), plan);
    runir::kr::database::incremental::QueryEvaluator incremental(scratch(), plan);
    const std::array<ygg::FreeRelationBuilderView, 0> no_inputs;
    full.evaluate(no_inputs);
    incremental.initialize(no_inputs);
    ASSERT_EQ(full.get_result().rows().size(), 1);
    EXPECT_TRUE(full.get_result().contains(std::array<ygg::Value, 0> {}));
    EXPECT_EQ(rows(full.get_result()), rows(incremental.get_result()));
    incremental.update(std::array<Change, 0> {});
    EXPECT_EQ(rows(full.get_result()), rows(incremental.get_result(1)));
    EXPECT_TRUE(incremental.get_delta().added->view().rows().empty());
    EXPECT_TRUE(incremental.get_delta().removed->view().rows().empty());
}

TEST(DatabaseQuery, InterningUsesArgumentsAndCanonicalOrders)
{
    auto repository = runir::tests::qb::repository();
    const auto input = runir::tests::qb::input(repository, 0, { x, y });
    EXPECT_EQ(runir::tests::qb::input(repository, 0, { x, y }).index(), input.index());
    const auto project = runir::tests::qb::project(repository, input, { x, y });
    EXPECT_EQ(runir::tests::qb::project(repository, input, { x, y }).index(), project.index());
    EXPECT_NE(runir::tests::qb::project(repository, input, { y, x }).index(), project.index());
    const auto renamed = runir::tests::qb::rename(repository, input, { x, y });
    EXPECT_NE(runir::tests::qb::rename(repository, input, { y, x }).index(), renamed.index());
    const auto joined = runir::tests::qb::generic_join(repository, { input }, { x, y });
    EXPECT_EQ(runir::tests::qb::generic_join(repository, { input }, { x, y }, { x, y }).index(), joined.index());
    EXPECT_NE(runir::tests::qb::generic_join(repository, { input }, { y, x }, { x, y }).index(), joined.index());
    EXPECT_NE(runir::tests::qb::generic_join(repository, { input }, { x, y }, { y, x }).index(), joined.index());
}

TEST(DatabaseQuery, CachedPlansSurviveRepositoryGrowthAndCistaRelocation)
{
    auto repository = runir::tests::qb::repository();
    const auto a = runir::tests::qb::input(repository, 0, { x, y });
    const auto b = runir::tests::qb::input(repository, 1, { y, z });
    const auto joined = runir::tests::qb::join(repository, a, b);
    const auto projected = runir::tests::qb::project(repository, joined, { z, x });
    const auto generic = runir::tests::qb::generic_join(repository, { a, b }, { z, y, x }, { x, z, y });
    const auto view = runir::kr::database::as<runir::kr::database::QueryJoinTag>(joined);
    const auto* original = &view.plan();
    for (size_t slot = 2; slot < 260; ++slot)
        runir::tests::qb::join(repository, runir::tests::qb::input(repository, slot, { x, y }), b);
    EXPECT_EQ(&view.plan(), original);
    EXPECT_TRUE(std::ranges::equal(view.plan().output_columns, columns_of(joined)));

    const auto round_trip = []<typename Concrete>(Concrete query)
    {
        using Record = urd::record_of<Concrete>;
        auto original_bytes = cista::serialize(*query.context()[query.index()]);
        auto relocated = original_bytes;
        original_bytes.clear();
        original_bytes.shrink_to_fit();
        const auto* restored = cista::deserialize<Record>(relocated);
        EXPECT_TRUE(std::ranges::equal(restored->plan.output_columns, query.columns()));
        if constexpr (std::same_as<Concrete, runir::kr::database::QueryView<runir::kr::database::QueryGenericJoinTag>>)
        {
            const auto& expected = query.plan();
            EXPECT_TRUE(std::ranges::equal(restored->plan.variable_order, expected.variable_order));
            for (size_t input = 0; input < expected.input_columns.size(); ++input)
            {
                EXPECT_TRUE(std::ranges::equal(restored->plan.input_columns[input], expected.input_columns[input]));
                EXPECT_EQ(restored->plan.input_keys[input].size(), expected.input_keys[input].size());
            }
            for (size_t depth = 0; depth < expected.variable_order.size(); ++depth)
                EXPECT_TRUE(std::ranges::equal(restored->plan.variable_inputs[depth], expected.variable_inputs[depth]));
        }
    };
    round_trip(view);
    round_trip(runir::kr::database::as<runir::kr::database::QueryProjectTag>(projected));
    round_trip(runir::kr::database::as<runir::kr::database::QueryGenericJoinTag>(generic));
}

TEST(DatabaseQuery, FactoriesRejectMalformedQueriesBeforePublishingPlans)
{
    auto repository = runir::tests::qb::repository();
    auto other = runir::tests::qb::repository();
    const auto input = runir::tests::qb::input(repository, 0, { x });
    const auto foreign = runir::tests::qb::input(other, 0, { x });
    // A plan copies its roots whatever repository holds them (urd::copy): equal queries are one there.
    const auto copied = runir::kr::database::compile_plan({ input, foreign });
    ASSERT_EQ(copied.root_count(), 2U);
    EXPECT_EQ(copied.roots()[0], copied.roots()[1]);
    EXPECT_THROW(runir::tests::qb::generic_join(repository, { input }, { y }), std::invalid_argument);
    EXPECT_THROW(runir::tests::qb::generic_join(repository, { input }, { x, x }), std::invalid_argument);
    EXPECT_THROW(runir::tests::qb::generic_join(repository, { input }, { x }, { y }), std::invalid_argument);
    EXPECT_THROW(runir::tests::qb::select_encoded(repository, input, x, 0, ygg::type_id<double>), std::invalid_argument);
    const auto schema = runir::tests::qb::columns_of({ x });
    urd::Store<urd::sequence<urd::data<ygg::ColumnLayout>>> malformed(schema.begin(), schema.end());
    malformed[0].type = static_cast<ygg::TypeId>(std::variant_size_v<ygg::ColumnValue>);
    EXPECT_THROW(runir::tests::qb::input(repository, 1, malformed), std::invalid_argument);
    EXPECT_THROW(runir::tests::qb::input(repository, 0, malformed), std::invalid_argument);
    const auto plan = runir::kr::database::compile_plan({ input });
    EXPECT_THROW(plan[urd::index<runir::kr::database::Query<>>(99)], std::out_of_range);
    EXPECT_EQ(repository.size(), 1);
}
}  // namespace
