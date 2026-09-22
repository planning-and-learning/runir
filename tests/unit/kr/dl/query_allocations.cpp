#include "planning_fixtures.hpp"

#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <gtest/gtest.h>
#include <new>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/ext/evaluation.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <tyr/planning/ground/successor_generator.hpp>

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
    auto caches = sem::DenotationCaches<kr::ExtFamilyTag>();
    auto arguments = ygg::Data<sem::CallArguments>();
    auto registers = ygg::Data<sem::RegisterValues>();
    registers.concept_values.resize(8);
    registers.role_values.resize(8);
    const auto object = search->task->get_domain().get_domain().get_constants()[0];
    registers.concept_values[5] = object.get_index();
    registers.role_values[7] = ::cista::pair(object.get_index(), object.get_index());
    const auto empty_arguments = sem::get_or_create(denotations, arguments).first;
    const auto register_values = sem::get_or_create(denotations, registers).first;
    auto context = sem::StateEvaluationContext<kr::ExtFamilyTag, tyr::GroundTag>(initial.get_state(),
                                                                                 builder,
                                                                                 denotations,
                                                                                 builder.get_workspace(),
                                                                                 caches,
                                                                                 empty_arguments,
                                                                                 register_values);
    const auto nominal = kr::ps::ext::dl::parse_concept(R"((c_nominal "a"))", search->task->get_domain().get_domain(), *repository);
    arguments.concept_arguments.push_back(sem::evaluate(nominal, context).get_index());
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
        caches.clear(false);
        ASSERT_EQ(sem::evaluate(expression, context).get(), 3);
    }

    bool valid = true;
    allocation_tracking::Scope measured;
    for (size_t i = 0; i < 1000; ++i)
    {
        auto borrowed = sem::StateEvaluationContext<kr::ExtFamilyTag, tyr::GroundTag>(initial.get_state(),
                                                                                      builder,
                                                                                      denotations,
                                                                                      builder.get_workspace(),
                                                                                      caches,
                                                                                      argument_values,
                                                                                      register_values);
        auto copied = borrowed;
        valid &= &copied.registers().get_data() == &register_values.get_data();
        valid &= &copied.arguments().get_data() == &argument_values.get_data();
        valid &= copied.registers().at(dl::RegisterIdentifier<dl::ConceptTag>(5)).value().get_index() == object.get_index();
        valid &= copied.registers().at(dl::RegisterIdentifier<dl::RoleTag>(7)).value().get_second().get_index() == object.get_index();
        valid &= copied.arguments().at(dl::ArgumentIdentifier<dl::ConceptTag>(0)).get().count() == 1;
        caches.clear(false);
        valid &= sem::evaluate(expression, copied).get() == 3;
    }
    const auto counts = measured.finish();

    EXPECT_TRUE(valid);
    EXPECT_EQ(counts.allocated, 0);
    EXPECT_EQ(counts.deallocated, 0);
}

}  // namespace runir::tests
