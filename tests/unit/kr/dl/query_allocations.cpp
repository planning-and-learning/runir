#include "planning_fixtures.hpp"

#include <array>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <gtest/gtest.h>
#include <new>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/ext/evaluation.hpp>
#include <runir/kr/ps/ext/detail/proof_search.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <runir/kr/ps/ext/program_executor.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/task_context.hpp>
#include <string>
#include <tyr/planning/ground/successor_generator.hpp>
#include <variant>

#if defined(_MSC_VER)
#include <malloc.h>
#endif

// This executable replaces C++ allocation functions only. That covers the
// default std/GTL allocators and RawArraySet's geometric byte storage, including
// aligned allocations. It does not intercept direct malloc/free calls, including
// Cista column/plan storage. Tracking is restricted to the evaluating thread and excludes GTest,
// setup, warmup, and destruction of the retained evaluation objects.
namespace allocation_tracking
{
struct Counts
{
    size_t allocated = 0;
    size_t deallocated = 0;
};

thread_local bool enabled = false;
thread_local Counts counts;

void* allocate(size_t bytes)
{
    auto* result = std::malloc(bytes == 0 ? 1 : bytes);
    if (!result)
        throw std::bad_alloc();
    if (enabled)
        ++counts.allocated;
    return result;
}

void deallocate(void* pointer) noexcept
{
    if (pointer && enabled)
        ++counts.deallocated;
    std::free(pointer);
}

void* allocate_aligned(size_t bytes, size_t alignment)
{
    void* result = nullptr;
#if defined(_MSC_VER)
    result = _aligned_malloc(bytes == 0 ? 1 : bytes, alignment);
#else
    if (alignment < sizeof(void*))
        alignment = sizeof(void*);
    if (posix_memalign(&result, alignment, bytes == 0 ? 1 : bytes) != 0)
        result = nullptr;
#endif
    if (!result)
        throw std::bad_alloc();
    if (enabled)
        ++counts.allocated;
    return result;
}

void deallocate_aligned(void* pointer) noexcept
{
    if (pointer && enabled)
        ++counts.deallocated;
#if defined(_MSC_VER)
    _aligned_free(pointer);
#else
    std::free(pointer);
#endif
}

class Scope
{
public:
    Scope()
    {
        counts = {};
        enabled = true;
    }
    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;
    ~Scope() { enabled = false; }

    Counts finish() noexcept
    {
        enabled = false;
        return counts;
    }
};
}  // namespace allocation_tracking

void* operator new(size_t bytes) { return allocation_tracking::allocate(bytes); }
void* operator new[](size_t bytes) { return allocation_tracking::allocate(bytes); }
void operator delete(void* pointer) noexcept { allocation_tracking::deallocate(pointer); }
void operator delete[](void* pointer) noexcept { allocation_tracking::deallocate(pointer); }
void operator delete(void* pointer, size_t) noexcept { allocation_tracking::deallocate(pointer); }
void operator delete[](void* pointer, size_t) noexcept { allocation_tracking::deallocate(pointer); }
void* operator new(size_t bytes, std::align_val_t alignment) { return allocation_tracking::allocate_aligned(bytes, static_cast<size_t>(alignment)); }
void* operator new[](size_t bytes, std::align_val_t alignment) { return allocation_tracking::allocate_aligned(bytes, static_cast<size_t>(alignment)); }
void operator delete(void* pointer, std::align_val_t) noexcept { allocation_tracking::deallocate_aligned(pointer); }
void operator delete[](void* pointer, std::align_val_t) noexcept { allocation_tracking::deallocate_aligned(pointer); }
void operator delete(void* pointer, size_t, std::align_val_t) noexcept { allocation_tracking::deallocate_aligned(pointer); }
void operator delete[](void* pointer, size_t, std::align_val_t) noexcept { allocation_tracking::deallocate_aligned(pointer); }
void* operator new(size_t bytes, const std::nothrow_t&) noexcept
{
    try
    {
        return ::operator new(bytes);
    }
    catch (...)
    {
        return nullptr;
    }
}
void* operator new[](size_t bytes, const std::nothrow_t&) noexcept
{
    try
    {
        return ::operator new[](bytes);
    }
    catch (...)
    {
        return nullptr;
    }
}
void* operator new(size_t bytes, std::align_val_t alignment, const std::nothrow_t&) noexcept
{
    try
    {
        return ::operator new(bytes, alignment);
    }
    catch (...)
    {
        return nullptr;
    }
}
void* operator new[](size_t bytes, std::align_val_t alignment, const std::nothrow_t&) noexcept
{
    try
    {
        return ::operator new[](bytes, alignment);
    }
    catch (...)
    {
        return nullptr;
    }
}
void operator delete(void* pointer, const std::nothrow_t&) noexcept { allocation_tracking::deallocate(pointer); }
void operator delete[](void* pointer, const std::nothrow_t&) noexcept { allocation_tracking::deallocate(pointer); }
void operator delete(void* pointer, std::align_val_t, const std::nothrow_t&) noexcept { allocation_tracking::deallocate_aligned(pointer); }
void operator delete[](void* pointer, std::align_val_t, const std::nothrow_t&) noexcept { allocation_tracking::deallocate_aligned(pointer); }

namespace runir::tests
{

TEST(RunirQueries, WarmedExtFeatureEvaluationAllocatesAndFreesNothing)
{
    // Explicit calls check the counter without allocation elision by the compiler.
    allocation_tracking::Scope counter_check;
    auto* ordinary = ::operator new(32);
    auto* array = ::operator new[](32);
    auto* aligned = ::operator new(64, std::align_val_t(64));
    auto* aligned_array = ::operator new[](64, std::align_val_t(64));
    ::operator delete(ordinary);
    ::operator delete[](array);
    ::operator delete(aligned, std::align_val_t(64));
    ::operator delete[](aligned_array, std::align_val_t(64));
    const auto checked = counter_check.finish();
    ASSERT_EQ(checked.allocated, 4);
    ASSERT_EQ(checked.deallocated, 4);

    namespace dl = kr::dl;
    namespace sem = dl::semantics;
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../fixtures/kr/dl/query";
    const auto search = make_ground_context(directory / "domain.pddl", directory / "task.pddl");
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    auto repository = dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(search->task->get_repository());
    auto builder = sem::Builder();
    auto denotations = sem::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = sem::EvaluationStorage<kr::ExtFamilyTag>(denotations);
    auto arguments = ygg::Data<sem::CallArguments>();
    auto registers = ygg::Data<sem::RegisterValues>();
    registers.concept_values.resize(8);
    registers.role_values.resize(8);
    const auto object = search->task->get_domain().get_domain().get_constants()[0];
    registers.concept_values[5] = object.get_index();
    registers.role_values[7] = ::cista::pair(object.get_index(), object.get_index());
    const auto empty_arguments = sem::get_or_create(denotations, arguments).first;
    const auto register_values = sem::get_or_create(denotations, registers).first;
    auto context = sem::StateEvaluationContext<kr::ExtFamilyTag, tyr::GroundTag>(initial.get_state(), builder, storage, empty_arguments, register_values);
    const auto nominal = kr::ps::ext::dl::parse_concept(R"((c_nominal "a"))", search->task->get_domain().get_domain(), *repository);
    auto persistent_memo = sem::DenotationCaches<kr::ExtFamilyTag> {};
    auto persistent = sem::StateEvaluationContext<kr::ExtFamilyTag, tyr::GroundTag>(initial.get_state(),
                                                                                    builder,
                                                                                    persistent_memo,
                                                                                    denotations,
                                                                                    storage,
                                                                                    empty_arguments,
                                                                                    register_values);
    arguments.concept_arguments.push_back(sem::evaluate(nominal, persistent).get_index());
    const auto argument_values = sem::get_or_create(denotations, arguments).first;
    const auto expression = kr::ps::ext::dl::parse_numerical(
        R"((n_count (q_rename (source target)
                (q_project (x z)
                    (q_join (q_atomic_state "triple" (x y z)) (q_atomic_state "edge" (x y)))))))",
        search->task->get_domain().get_domain(),
        *repository);

    // Clearing dynamic results measures query recomputation and pooled returns.
    for (size_t i = 0; i < 8; ++i)
    {
        storage.reset_dynamic();
        ASSERT_EQ(sem::evaluate(expression, context).get(), 3);
    }

    bool valid = true;
    allocation_tracking::Scope measured;
    for (size_t i = 0; i < 1000; ++i)
    {
        auto borrowed = sem::StateEvaluationContext<kr::ExtFamilyTag, tyr::GroundTag>(initial.get_state(), builder, storage, argument_values, register_values);
        auto copied = borrowed;
        valid &= &copied.get_workspace() == &builder.get_workspace();
        valid &= &copied.registers().get_data() == &register_values.get_data();
        valid &= &copied.arguments().get_data() == &argument_values.get_data();
        valid &= copied.registers().at(dl::RegisterIdentifier<dl::ConceptTag>(5)).value().get_index() == object.get_index();
        valid &= copied.registers().at(dl::RegisterIdentifier<dl::RoleTag>(7)).value().get_second().get_index() == object.get_index();
        valid &= copied.arguments().at(dl::ArgumentIdentifier<dl::ConceptTag>(0)).get().count() == 1;
        storage.reset_dynamic();
        valid &= sem::evaluate(expression, copied).get() == 3;
    }
    const auto counts = measured.finish();

    EXPECT_TRUE(valid);
    EXPECT_EQ(counts.allocated, 0);
    EXPECT_EQ(counts.deallocated, 0);
}

TEST(RunirQueries, QueryResultResetReusesCistaSchemaAndRowCapacity)
{
    namespace sem = kr::dl::semantics;
    using ColumnIndex = ygg::Index<ygg::database::Column>;
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../fixtures/kr/dl/query";
    const auto search = make_ground_context(directory / "domain.pddl", directory / "task.pddl");
    auto denotations = sem::DenotationRepositoryFactory().create(search->task->get_repository());
    auto& repository = denotations.get_relation_repository();
    auto builder = sem::Builder {};
    const auto columns = std::array<ColumnIndex, 2> { ColumnIndex(0), ColumnIndex(1) };
    const auto renamed = std::array<ColumnIndex, 2> { ColumnIndex(2), ColumnIndex(3) };
    auto result = builder.get_builder<ygg::database::Relation<>>(columns);
    for (ygg::uint_t i = 0; i < 32; ++i)
        result->insert({ i, i + 1 });
    const auto* slot = result.get();
    const auto* schema_buffer = result->columns().data();
    const auto schema_capacity = result->memory_usage() - result->storage().memory_usage();
    const auto* row_buffer = (*result)[0].data();
    const auto row_capacity = result->storage().memory_usage();
    const auto stored = ygg::database::intern_relation(*result, repository).first;
    const auto* stored_schema_buffer = stored.columns().data();
    const auto* stored_row_buffer = stored[0].data();
    const auto* stored_row_indices = stored.row_indices().data();
    result = {};

    bool reused = true;
    allocation_tracking::Scope measured;
    for (size_t repeat = 0; repeat < 1000; ++repeat)
    {
        denotations.clear();
        reused &= repository.empty();
        auto next = builder.get_builder<ygg::database::Relation<>>(repeat % 2 ? renamed : columns);
        // Cista schema storage uses malloc, which the global new/delete counter does not cover.
        reused &= next.get() == slot;
        reused &= next->columns().data() == schema_buffer;
        reused &= next->memory_usage() - next->storage().memory_usage() == schema_capacity;
        reused &= next->storage().memory_usage() == row_capacity;
        for (ygg::uint_t i = 0; i < 32; ++i)
            next->insert({ i, i + 1 });
        reused &= (*next)[0].data() == row_buffer;
        const auto value = ygg::database::intern_relation(*next, repository).first;
        reused &= value.columns().data() == stored_schema_buffer;
        reused &= value[0].data() == stored_row_buffer;
        reused &= value.row_indices().data() == stored_row_indices;
    }
    const auto counts = measured.finish();
    EXPECT_TRUE(reused);
    EXPECT_EQ(counts.allocated, 0);
    EXPECT_EQ(counts.deallocated, 0);
}

TEST(RunirQueries, LargeWarmedQueryResultTablesResetWithoutAllocations)
{
    namespace sem = kr::dl::semantics;
    using ColumnIndex = ygg::Index<ygg::database::Column>;
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../fixtures/kr/dl/query";
    const auto search = make_ground_context(directory / "domain.pddl", directory / "task.pddl");
    auto denotations = sem::DenotationRepositoryFactory().create(search->task->get_repository());
    auto& repository = denotations.get_relation_repository();
    auto builder = sem::Builder {};
    const auto columns = std::array<ColumnIndex, 2> { ColumnIndex(0), ColumnIndex(1) };
    const auto aliases = std::array<ColumnIndex, 2> { ColumnIndex(100), ColumnIndex(101) };
    bool valid = true;
    const auto cycle = [&](size_t generation)
    {
        denotations.clear();
        valid &= repository.empty();
        for (ygg::uint_t i = 0; i < 512; ++i)
        {
            const auto object = ygg::uint_t(i + generation * 512);
            auto owner = builder.get_builder<ygg::database::Relation<>>(columns);
            owner->insert({ object, 1 });
            owner->insert({ object, 2 });
            const auto value = ygg::database::intern_relation(*owner, repository, generation).first;
            const auto alias = repository.rename(value, aliases);
            valid &= value.size() == 2 && alias.size() == 2 && value[0][0] == object;
            valid &= value.get_storage_address() == alias.get_storage_address();
            auto duplicate = builder.get_builder<ygg::database::Relation<>>(columns);
            duplicate->insert({ object, 2 });
            duplicate->insert({ object, 1 });
            valid &= ygg::database::intern_relation(*duplicate, repository, generation).first == value;
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

TEST(RunirSearch, WarmedProgramSearchAllocationsGrowWithContainerCapacity)
{
    namespace ext = kr::ps::ext;
    using Path = ext::detail::SearchPath<tyr::GroundTag, ext::ProgramStateView<tyr::GroundTag>>;
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../fixtures/kr/ps/ext/choose";
    const auto search = make_ground_context(directory / "domain.pddl", directory / "task.pddl");
    auto context = kr::TaskContext<tyr::GroundTag>::create(kr::DomainContext::create(search->task->get_domain()), search);

    for (const auto choose : { false, true })
        for (const auto universal : { false, true })
        {
            SCOPED_TRACE(choose);
            SCOPED_TRACE(universal);
            auto allocations = std::array<size_t, 2> {};
            for (size_t scale = 0; scale < allocations.size(); ++scale)
            {
                const auto length = scale == 0 ? size_t(64) : size_t(512);
                auto text = std::string(R"((:program (:entry chain)
                    (:module (:symbol chain) (:arguments) (:registers (:concept selected))
                        (:entry m0) (:memory)");
                for (size_t i = 0; i <= length + 2; ++i)
                    text += " m" + std::to_string(i);
                text += R"()
                        (:features
                            (:concept (:symbol Candidates) (:expression (c_atomic_state "candidate")))
                            (:concept (:symbol Good) (:expression (c_and (c_atomic_state "candidate") (c_not (c_atomic_state "bad")))))
                            (:concept (:symbol Here) (:expression (c_atomic_state "at")))
                            (:concept (:symbol Goal) (:expression (c_atomic_goal "at" true))))
                        (:rules )";
                for (size_t i = 0; i < length + 2; ++i)
                {
                    text += "(:rule (:symbol r" + std::to_string(i) + ") (:expression (:source-memory m" + std::to_string(i) + ") (:target-memory m"
                            + std::to_string(i + 1) + ") ";
                    if (i < length)
                        text += choose ? "(:choose (:conditions) (:concept Candidates) (:register (:concept selected)))" :
                                         "(:load (:conditions) (:concept Goal) (:register (:concept selected)))";
                    else
                        text += std::string(R"((:do (:conditions) (:action "move") (:arguments Here )") + (i == length ? "Good" : "Goal") + ") (:effects))";
                    text += "))";
                }
                text += ")))";
                const auto program = ext::dl::parse_program(text, search->task->get_domain().get_domain(), *context->domain_context->ext_repository);
                auto options = ext::ProgramSearchOptions<tyr::GroundTag> {};
                options.universal = universal;
                auto expander = ext::SuccessorExpander<tyr::GroundTag>(context, program);
                const auto initial_node = search->successor_generator->get_packed_initial_node(*search->state_repository, *search->axiom_evaluator);
                const auto initial = expander.initial_state(initial_node.get_state().unpack());
                auto classifier = kr::ps::NoUnsolvability {};
                auto path_pool = ygg::SharedObjectPool<Path> {};
                // Warm path and binding pools to the live DFS depth. Fresh search storage
                // on each pass ensures memoization cannot skip any of the measured work.
                for (const auto warmup : { true, false })
                {
                    SCOPED_TRACE(warmup);
                    auto storage = ext::detail::SearchStorage<tyr::GroundTag, ext::StateMemorization::ALL>(options);
                    auto statistics = ext::ProgramSearchStatistics {};
                    auto first_goal = ygg::SharedObjectPoolPtr<Path> {};
                    auto witness = ygg::SharedObjectPoolPtr<Path> {};
                    allocation_tracking::Scope measured;
                    const auto status = ext::detail::depth_first_search<
                        tyr::GroundTag>(expander, initial_node.get_state(), initial, options, classifier, storage, statistics, path_pool, first_goal, witness);
                    const auto counts = measured.finish();
                    if (!warmup)
                        allocations[scale] = counts.allocated;
                    ASSERT_EQ(status, ext::ProgramProofStatus::SUCCESS);
                    EXPECT_EQ(statistics.num_expanded, length + 2);
                    EXPECT_EQ(statistics.num_generated, length + 2);
                    ASSERT_TRUE(first_goal);
                    EXPECT_EQ(first_goal->choice_depth, choose ? length : 0);
                }
            }
            // After warmup, eight times as many steps must not cause per-step or per-Choose allocations.
            // The allowance covers geometric growth of traversal and memoization containers across allocators.
            EXPECT_LE(allocations[1], allocations[0] + 128) << "small=" << allocations[0] << ", large=" << allocations[1];
        }
}

}  // namespace runir::tests
