#include "allocation_tracking.hpp"
#include "planning_fixtures.hpp"
#include <array>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <gtest/gtest.h>
#include <limits>
#include <new>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/builder.hpp>
#include <runir/kr/dl/semantics/detail/repository_instantiations.hpp>
#include <runir/kr/dl/semantics/evaluation_storage.hpp>
#include <runir/kr/dl/semantics/ext/evaluation.hpp>
#include <set>
#include <string>
#include <tuple>
#include <tyr/planning/ground/task.hpp>
#include <utility>
#include <variant>
#include <vector>
#include <malloc.h>

namespace runir::tests
{
using ObjectIndex = ygg::Index<tyr::formalism::Object>;
using QueryValues = kr::dl::QueryValues;

TEST(RunirQueries, QueryResultResetReusesCistaSchemaAndRowCapacity)
{
    using ColumnIndex = ygg::Index<ygg::database::Column>;
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../fixtures/kr/dl/query";
    const auto search = make_ground_context(directory / "domain.pddl", directory / "task.pddl");
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto& repository = denotations.get_relation_repository();
    auto builder = kr::dl::semantics::Builder {};
    const auto columns = std::array<ColumnIndex, 2> { ColumnIndex(0), ColumnIndex(1) };
    const auto renamed = std::array<ColumnIndex, 2> { ColumnIndex(2), ColumnIndex(3) };
    auto result = builder.get_builder<ygg::database::Relation<QueryValues>>(columns);
    for (ygg::uint_t i = 0; i < 32; ++i)
        result->insert(std::tuple { ObjectIndex(i), ObjectIndex(i + 1) });
    const auto* slot = result.get();
    const auto* schema_buffer = result->columns().data();
    const auto schema_capacity = result->memory_usage() - result->storage().memory_usage();
    const auto* row_buffer = result->row(0).data();
    const auto row_capacity = result->storage().memory_usage();
    const auto stored = ygg::database::insert(repository, *result).first;
    const auto* stored_schema_buffer = stored.columns().data();
    const auto* stored_row_buffer = stored.row(0).data();
    const auto* stored_row_indices = stored.row_indices().data();
    result = {};

    bool reused = true;
    allocation_tracking::Scope measured;
    for (size_t repeat = 0; repeat < 1000; ++repeat)
    {
        denotations.clear();
        reused &= repository.empty();
        auto next = builder.get_builder<ygg::database::Relation<QueryValues>>(repeat % 2 ? renamed : columns);
        // Cista schema storage uses malloc, which the global new/delete counter does not cover.
        reused &= next.get() == slot;
        reused &= next->columns().data() == schema_buffer;
        reused &= next->memory_usage() - next->storage().memory_usage() == schema_capacity;
        reused &= next->storage().memory_usage() == row_capacity;
        for (ygg::uint_t i = 0; i < 32; ++i)
            next->insert(std::tuple { ObjectIndex(i), ObjectIndex(i + 1) });
        reused &= next->row(0).data() == row_buffer;
        const auto value = ygg::database::insert(repository, *next).first;
        reused &= value.columns().data() == stored_schema_buffer;
        reused &= value.row(0).data() == stored_row_buffer;
        reused &= value.row_indices().data() == stored_row_indices;
    }
    const auto counts = measured.finish();
    EXPECT_TRUE(reused);
    EXPECT_EQ(counts.allocated, 0);
    EXPECT_EQ(counts.deallocated, 0);
}

TEST(RunirQueries, LargeWarmedQueryResultTablesResetWithoutAllocations)
{
    using ColumnIndex = ygg::Index<ygg::database::Column>;
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../fixtures/kr/dl/query";
    const auto search = make_ground_context(directory / "domain.pddl", directory / "task.pddl");
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto& repository = denotations.get_relation_repository();
    auto builder = kr::dl::semantics::Builder {};
    const auto columns = std::array<ColumnIndex, 2> { ColumnIndex(0), ColumnIndex(1) };
    const auto aliases = std::array<ColumnIndex, 2> { ColumnIndex(100), ColumnIndex(101) };
    bool valid = true;
    const auto cycle = [&](size_t generation)
    {
        denotations.clear();
        valid &= repository.empty();
        for (ygg::uint_t i = 0; i < 512; ++i)
        {
            const auto object = ObjectIndex(ygg::uint_t(i + generation * 512));
            auto owner = builder.get_builder<ygg::database::Relation<QueryValues>>(columns);
            owner->insert(std::tuple { object, ObjectIndex(1) });
            owner->insert(std::tuple { object, ObjectIndex(2) });
            const auto value = ygg::database::insert(repository, *owner, generation).first;
            const auto alias = repository.rename(value, aliases);
            valid &= value.size() == 2 && alias.size() == 2 && value[0].get<ObjectIndex>(size_t { 0 }) == object;
            valid &= value.get_storage_address() == alias.get_storage_address();
            auto duplicate = builder.get_builder<ygg::database::Relation<QueryValues>>(columns);
            duplicate->insert(std::tuple { object, ObjectIndex(2) });
            duplicate->insert(std::tuple { object, ObjectIndex(1) });
            valid &= ygg::database::insert(repository, *duplicate, generation).first == value;
            valid &= repository.rename(value, aliases) == alias;
        }
        valid &= repository.size() == 1024;
    };
    for (size_t repeat = 0; repeat < 8; ++repeat)
        cycle(repeat);

    allocation_tracking::Scope measured;
    for (size_t repeat = 0; repeat < 100; ++repeat)
        cycle(repeat);
    const auto counts = measured.finish();
    EXPECT_TRUE(valid);
    EXPECT_EQ(counts.allocated, 0);
    EXPECT_EQ(counts.deallocated, 0);
}

}  // namespace runir::tests
