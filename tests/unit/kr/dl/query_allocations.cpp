#include "incremental_root_evaluators.hpp"
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
#include <runir/kr/dl/semantics/detail/repository_instantiations.hpp>
#include <runir/kr/dl/semantics/incremental/delta.hpp>
#include <runir/kr/dl/semantics/incremental/detail/atomic_query.hpp>
#include <runir/kr/dl/semantics/incremental/evaluation.hpp>
#include <runir/kr/dl/semantics/state_evaluation.hpp>
#include <runir/kr/ps/ext/detail/proof_search.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <runir/kr/ps/ext/program_executor.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/task_context.hpp>
#include <set>
#include <string>
#include <tuple>
#include <tyr/planning/ground/successor_generator.hpp>
#include <tyr/planning/lifted/successor_generator.hpp>
#include <utility>
#include <variant>
#include <vector>
#include <yggdrasil/database/semantics/incremental/projection.hpp>

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
using ObjectIndex = ygg::Index<tyr::formalism::Object>;

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

    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../fixtures/kr/dl/query";
    const auto search = make_ground_context(directory / "domain.pddl", directory / "task.pddl");
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    auto repository = kr::dl::ConstructorRepositoryFactory().create(search->task->get_repository());
    auto builder = kr::dl::semantics::Builder();
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage(denotations);
    auto arguments = ygg::Data<kr::dl::semantics::CallArguments>();
    auto registers = ygg::Data<kr::dl::semantics::RegisterValues>();
    registers.concept_values.resize(8);
    registers.role_values.resize(8);
    const auto object = search->task->get_domain().get_domain().get_constants()[0];
    registers.concept_values[5] = object.get_index();
    registers.role_values[7] = ::cista::pair(object.get_index(), object.get_index());
    const auto empty_arguments = kr::dl::semantics::insert(denotations, arguments).first;
    const auto register_values = kr::dl::semantics::insert(denotations, registers).first;
    auto context = kr::dl::semantics::StateEvaluationContext<tyr::GroundTag>(initial.get_state(), builder, storage, empty_arguments, register_values);
    const auto nominal = kr::ps::ext::dl::parse_concept(R"((c_nominal "a"))", search->task->get_domain().get_domain(), *repository);
    auto persistent_memo = kr::dl::semantics::DenotationCaches {};
    auto persistent = kr::dl::semantics::StateEvaluationContext<tyr::GroundTag>(initial.get_state(),
                                                                                builder,
                                                                                persistent_memo,
                                                                                denotations,
                                                                                storage,
                                                                                empty_arguments,
                                                                                register_values);
    arguments.concept_arguments.push_back(kr::dl::semantics::evaluate<tyr::GroundTag>(nominal, persistent).get_index());
    const auto argument_values = kr::dl::semantics::insert(denotations, arguments).first;
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
        ASSERT_EQ(kr::dl::semantics::evaluate<tyr::GroundTag>(expression, context).get(), 3);
    }

    bool valid = true;
    allocation_tracking::Scope measured;
    for (size_t i = 0; i < 1000; ++i)
    {
        auto borrowed = kr::dl::semantics::StateEvaluationContext<tyr::GroundTag>(initial.get_state(), builder, storage, argument_values, register_values);
        auto copied = borrowed;
        valid &= &copied.get_workspace() == &builder.get_workspace();
        valid &= &copied.registers().get_data() == &register_values.get_data();
        valid &= &copied.arguments().get_data() == &argument_values.get_data();
        valid &= copied.registers().at(kr::dl::RegisterIdentifier<kr::dl::ConceptTag>(5)).value().get_index() == object.get_index();
        valid &= copied.registers().at(kr::dl::RegisterIdentifier<kr::dl::RoleTag>(7)).value().get_second().get_index() == object.get_index();
        valid &= copied.arguments().at(kr::dl::ArgumentIdentifier<kr::dl::ConceptTag>(0)).get().count() == 1;
        storage.reset_dynamic();
        valid &= kr::dl::semantics::evaluate<tyr::GroundTag>(expression, copied).get() == 3;
    }
    const auto counts = measured.finish();

    EXPECT_TRUE(valid);
    EXPECT_EQ(counts.allocated, 0);
    EXPECT_EQ(counts.deallocated, 0);
}

TEST(RunirQueries, WarmedIncrementalQueryEvaluationAllocatesAndFreesNothing)
{
    using Fluent = tyr::formalism::FluentTag;
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../fixtures/kr/dl/query";
    const auto search = make_ground_context(directory / "domain.pddl", directory / "task.pddl");
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    auto repository = kr::dl::ConstructorRepositoryFactory().create(search->task->get_repository());
    const auto count = kr::ps::ext::dl::parse_numerical(R"((n_count (q_project (x) (q_atomic_state "triple" (x y z)))))",
                                                        search->task->get_domain().get_domain(),
                                                        *repository);
    const auto query = count.get_variant().get<ygg::Index<kr::dl::Numerical<kr::dl::CountTag>>>().get_arg().get<ygg::Index<kr::dl::Query<>>>();
    const auto projected = query.get_variant().get<ygg::Index<kr::dl::Query<kr::dl::QueryProjectTag>>>();
    const auto atomic = projected.get_arg().get_variant().get<ygg::Index<kr::dl::Query<kr::dl::AtomicStateTag<Fluent>>>>();
    auto leaf = kr::dl::semantics::incremental::detail::AtomicQueryEvaluator<Fluent>(atomic);
    auto projection = ygg::database::incremental::ProjectionEvaluator(projected.get_data().plan);
    auto workspace = ygg::database::Workspace {};
    leaf.initialize<tyr::GroundTag>(initial.get_state());
    projection.initialize(leaf.get_result(), workspace);
    ASSERT_EQ(leaf.get_result().size(), 4);
    ASSERT_EQ(projection.get_result().size(), 2);

    auto registers = ygg::Data<kr::dl::semantics::RegisterValues> {};
    const auto register_view = ygg::make_view(registers, *search->task->get_repository());
    const auto graph_count = kr::ps::ext::dl::parse_numerical(
        R"((n_count (q_project (x)
            (q_join
                (q_join (q_atomic_state "triple" (x y z)) (q_atomic_state "copied" (x y z)))
                (q_join (q_atomic_state "triple" (x y z)) (q_project () (q_atomic_state "fixed" (x y z))))))))",
        search->task->get_domain().get_domain(),
        *repository);
    const auto graph_query = graph_count.get_variant().get<ygg::Index<kr::dl::Numerical<kr::dl::CountTag>>>().get_arg().get<ygg::Index<kr::dl::Query<>>>();
    auto graph = QueryRootEvaluator<tyr::GroundTag>(*search->task, graph_query);
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage(denotations);
    auto builder = kr::dl::semantics::Builder {};
    auto argument_data = ygg::Data<kr::dl::semantics::CallArguments> {};
    const auto arguments = kr::dl::semantics::insert(denotations, argument_data).first;
    const auto register_values = kr::dl::semantics::insert(denotations, registers).first;
    auto context = kr::dl::semantics::StateEvaluationContext<tyr::GroundTag>(initial.get_state(), builder, storage, arguments, register_values);
    ASSERT_EQ(graph.initialize(context).get_result().size(), 2);

    // Build completed planning transitions before tracking evaluation allocations.
    auto changes = std::array<kr::dl::semantics::incremental::Delta, 4> {};
    auto projected_sizes = std::array<size_t, 5> { 2 };
    auto contexts = std::vector { context };  // The context of each state the changes pass through, built before tracking.
    auto current = initial;
    for (size_t i = 0; i < changes.size(); ++i)
    {
        const auto successors = search->successor_generator->get_successor_nodes(current, *search->state_repository, *search->axiom_evaluator);
        ASSERT_FALSE(successors.empty());
        const auto next = successors.front();
        changes[i].assign<tyr::GroundTag>(current.get_state(), register_view, next.get_state(), register_view);
        contexts.push_back(kr::dl::semantics::StateEvaluationContext<tyr::GroundTag>(next.get_state(), builder, storage, arguments, register_values));
        auto objects = std::set<ObjectIndex> {};
        for (const auto atom : next.get_state().get_atoms_view(atomic.get_predicate()))
            objects.insert(atom.get_row().get_data()[0]);
        projected_sizes[i + 1] = objects.size();
        current = next;
    }
    ASSERT_EQ(projected_sizes.back(), 0);

    const auto same_rows = [](const auto& lhs, const auto& rhs)
    {
        if (lhs.size() != rhs.size())
            return false;
        for (size_t i = 0; i < lhs.size(); ++i)
            if (!rhs.contains(lhs.row(i)))
                return false;
        return true;
    };
    const auto apply = [&](const auto& change, size_t target, size_t rows, size_t projected_rows, bool adding)
    {
        const auto previous_size = projection.get_result().size();
        leaf.update(change.added.fluent_atoms, change.removed.fluent_atoms);
        const auto& delta = leaf.get_delta();
        projection.update(delta.change(), workspace);
        auto graph_state = graph.update(contexts[target], change, workspace);
        const auto& projected_delta = projection.get_delta();
        return leaf.get_result().size() == rows && delta.added.size() == static_cast<size_t>(adding) && delta.removed.size() == static_cast<size_t>(!adding)
               && projection.get_result().size() == projected_rows && projected_delta.added.size() == (adding ? projected_rows - previous_size : 0)
               && projected_delta.removed.size() == (adding ? 0 : previous_size - projected_rows)
               && same_rows(graph_state.get_result(), projection.get_result()) && same_rows(graph_state.get_delta().added, projected_delta.added)
               && same_rows(graph_state.get_delta().removed, projected_delta.removed);
    };
    const auto cycle = [&]
    {
        bool valid = true;
        for (size_t i = 0; i < changes.size(); ++i)
            valid &= apply(changes[i], i + 1, changes.size() - i - 1, projected_sizes[i + 1], false);
        for (size_t i = changes.size(); i-- > 0;)
        {
            changes[i].reverse();
            valid &= apply(changes[i], i, changes.size() - i, projected_sizes[i], true);
            changes[i].reverse();
        }
        return valid;
    };

    // Warm erasure headroom, shared children, both join inputs, and first/last projection witnesses.
    for (size_t repeat = 0; repeat < 8; ++repeat)
        ASSERT_TRUE(cycle());
    bool valid = true;
    allocation_tracking::Scope measured;
    for (size_t repeat = 0; repeat < 1000; ++repeat)
        valid &= cycle();
    const auto counts = measured.finish();
    EXPECT_TRUE(valid);
    EXPECT_EQ(counts.allocated, 0);
    EXPECT_EQ(counts.deallocated, 0);
}

TEST(RunirQueries, WarmedIncrementalDlEvaluationAllocatesAndFreesNothing)
{
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../fixtures/kr/dl/incremental";
    const auto search = make_ground_context(directory / "domain.pddl", directory / "task.pddl");
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    const auto domain = search->task->get_domain().get_domain();
    auto repository = kr::dl::ConstructorRepositoryFactory().create(search->task->get_repository());
    auto concepts = std::vector<RootEvaluator<tyr::GroundTag, kr::dl::ConceptTag>> {};
    auto roles = std::vector<RootEvaluator<tyr::GroundTag, kr::dl::RoleTag>> {};
    auto numericals = std::vector<RootEvaluator<tyr::GroundTag, kr::dl::NumericalTag>> {};
    auto booleans = std::vector<RootEvaluator<tyr::GroundTag, kr::dl::BooleanTag>> {};
    for (const auto expression : { R"((c_atomic_state "present"))", R"((c_atomic_state "copied-present"))", "(c_register 0)", R"((c_atomic_state "fixed"))" })
        concepts.emplace_back(*search->task, kr::ps::ext::dl::parse_concept(expression, domain, *repository));
    for (const auto expression : { R"((r_atomic_state "edge"))", R"((r_atomic_state "copied-edge"))", "(r_register 0)" })
        roles.emplace_back(*search->task, kr::ps::ext::dl::parse_role(expression, domain, *repository));
    for (const auto expression : { R"((c_atomic_state "present"))", R"((r_atomic_state "edge"))", "(c_register 0)", "(r_register 0)" })
    {
        numericals.emplace_back(*search->task, kr::ps::ext::dl::parse_numerical(std::string("(n_count ") + expression + ")", domain, *repository));
        booleans.emplace_back(*search->task, kr::ps::ext::dl::parse_boolean(std::string("(b_nonempty ") + expression + ")", domain, *repository));
    }

    const auto source = kr::ps::ext::dl::parse_concept("(c_register 0)", domain, *repository);
    const auto edge = kr::ps::ext::dl::parse_role(R"((r_atomic_state "edge"))", domain, *repository);
    const auto target = kr::ps::ext::dl::parse_concept("(c_register 1)", domain, *repository);
    const auto source_count = kr::ps::ext::dl::parse_numerical("(n_count (c_register 0))", domain, *repository);
    const auto edge_count = kr::ps::ext::dl::parse_numerical(R"((n_count (r_atomic_state "edge")))", domain, *repository);
    const auto distance = kr::ps::ext::dl::parse_numerical(R"((n_distance (c_register 0) (r_atomic_state "edge") (c_register 1)))", domain, *repository);
    const auto self_distance = kr::ps::ext::dl::parse_numerical(R"((n_distance (c_register 0) (r_atomic_state "edge") (c_register 0)))", domain, *repository);
    const auto closure = kr::ps::ext::dl::parse_role(R"((r_transitive_closure (r_atomic_state "edge")))", domain, *repository);
    const auto nonempty = kr::ps::ext::dl::parse_boolean(R"((b_nonempty (c_some (r_atomic_state "edge") (c_atomic_state "present"))))", domain, *repository);
    const auto projected = kr::ps::ext::dl::parse_concept(R"((c_project x (q_role (x y) (r_atomic_state "edge"))))", domain, *repository);
    const auto projected_role = kr::ps::ext::dl::parse_role(R"((r_project y x (q_role (x y) (r_atomic_state "edge"))))", domain, *repository);
    const auto query_count = kr::ps::ext::dl::parse_numerical(
        R"((n_count (q_project (x) (q_join (q_role (x y) (r_atomic_state "edge")) (q_concept z (c_atomic_state "present"))))))",
        domain,
        *repository);
    auto roots = std::vector<kr::dl::semantics::incremental::EvaluationRoot> { source,        edge,    target,   source_count, edge_count,     distance,
                                                                               self_distance, closure, nonempty, projected,    projected_role, query_count };
    for (const auto expression : {
             R"((c_and (c_not (c_atomic_state "present")) (c_register 0)))",
             R"((c_at_least 1 (r_atomic_state "edge") (c_atomic_state "present")))",
         })
        roots.emplace_back(kr::ps::ext::dl::parse_concept(expression, domain, *repository));
    for (const auto expression : {
             R"((r_composition (r_atomic_state "edge") (r_atomic_state "edge")))",
             R"((r_complement (r_or (r_atomic_state "edge") (r_register 0))))",
         })
        roots.emplace_back(kr::ps::ext::dl::parse_role(expression, domain, *repository));
    roots.emplace_back(
        kr::ps::ext::dl::parse_numerical(R"((n_add (n_distance (c_register 0) (r_atomic_state "edge") (c_register 1)) (n_const 1)))", domain, *repository));

    auto evaluation = kr::dl::semantics::incremental::IncrementalEvaluation<tyr::GroundTag>(*search->task, roots);
    const auto& graph = evaluation.graph();
    const auto source_id = graph.get_index(source);
    const auto edge_id = graph.get_index(edge);
    const auto source_count_id = graph.get_index(source_count);
    const auto edge_count_id = graph.get_index(edge_count);
    const auto distance_id = graph.get_index(distance);
    const auto self_distance_id = graph.get_index(self_distance);
    const auto closure_id = graph.get_index(closure);
    const auto nonempty_id = graph.get_index(nonempty);
    const auto projected_id = graph.get_index(projected);
    const auto projected_role_id = graph.get_index(projected_role);
    const auto query_count_id = graph.get_index(query_count);

    auto registers = std::array<ygg::Data<kr::dl::semantics::RegisterValues>, 3> {};
    const auto objects = domain.get_constants();
    ASSERT_GE(objects.size(), 3);
    for (size_t i = 0; i < registers.size(); ++i)
    {
        registers[i].concept_values.resize(2);
        registers[i].role_values.resize(1);
        if (i < 2)
        {
            registers[i].concept_values[0] = objects[i].get_index();
            registers[i].concept_values[1] = objects[2 - i].get_index();
            registers[i].role_values[0] = ::cista::pair(objects[i].get_index(), objects[i + 1].get_index());
        }
    }
    auto changes = std::array<kr::dl::semantics::incremental::Delta, 2> {};
    auto current = initial;
    auto states = std::vector { initial.get_state() };
    for (size_t i = 0; i < changes.size(); ++i)
    {
        const auto successors = search->successor_generator->get_successor_nodes(current, *search->state_repository, *search->axiom_evaluator);
        ASSERT_FALSE(successors.empty());
        const auto next = successors.front();
        changes[i].assign<tyr::GroundTag>(current.get_state(),
                                          ygg::make_view(registers[i], *search->task->get_repository()),
                                          next.get_state(),
                                          ygg::make_view(registers[i + 1], *search->task->get_repository()));
        current = next;
        states.push_back(next.get_state());
    }

    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage(denotations);
    auto builder = kr::dl::semantics::Builder {};
    auto argument_data = ygg::Data<kr::dl::semantics::CallArguments> {};
    const auto arguments = kr::dl::semantics::insert(denotations, argument_data).first;
    const auto context_for = [&](size_t i)
    {
        return kr::dl::semantics::StateEvaluationContext<tyr::GroundTag, tyr::planning::StateView<tyr::GroundTag>>(
            states[i],
            builder,
            storage,
            arguments,
            ygg::make_view(registers[i], *search->task->get_repository()));
    };
    // The inputs of each state of the cycle, built before the measured scope.
    const auto contexts = std::array { context_for(0), context_for(1), context_for(2) };
    const auto& context = contexts.front();
    const auto initialize = [&]
    {
        for (auto& evaluator : concepts)
            evaluator.initialize(context);
        for (auto& evaluator : roles)
            evaluator.initialize(context);
        for (auto& evaluator : numericals)
            evaluator.initialize(context);
        for (auto& evaluator : booleans)
            evaluator.initialize(context);
        auto state = evaluation.enter(context);
        for (const auto& root : roots)
            std::visit([&](auto expression) { (void) state.evaluate(graph.get_index(expression)); }, root);
    };
    initialize();
    auto workspace = ygg::database::Workspace {};
    // Enters state `target` by `delta`. Every read root is updated from the delta: get_delta(), delta() and changed()
    // throw otherwise.
    const auto apply = [&](const auto& delta, size_t target, size_t members, size_t register_members)
    {
        const auto& target_context = contexts[target];
        const auto concept_sizes = std::array { members, members, register_members, static_cast<size_t>(2) };
        const auto role_sizes = std::array { members, members, register_members };
        bool valid = true;
        for (size_t i = 0; i < concepts.size(); ++i)
        {
            auto state = concepts[i].update(target_context, delta, workspace);
            (void) state.get_delta();
            valid &= state.size() == concept_sizes[i];
        }
        for (size_t i = 0; i < roles.size(); ++i)
        {
            auto state = roles[i].update(target_context, delta, workspace);
            (void) state.get_delta();
            valid &= state.size() == role_sizes[i];
        }
        for (size_t i = 0; i < numericals.size(); ++i)
        {
            const auto expected = i < 2 ? members : register_members;
            auto numerical = numericals[i].update(target_context, delta, workspace);
            auto boolean = booleans[i].update(target_context, delta, workspace);
            (void) numerical.changed();
            (void) boolean.changed();
            valid &= numerical.get_result().get() == expected;
            valid &= boolean.get_result().get() == (expected != 0);
        }
        auto graph = evaluation.enter(target_context, delta, workspace);
        const auto infinity = std::numeric_limits<ygg::uint_t>::max();
        const auto expected_distance = members == 2 ? static_cast<ygg::uint_t>(2) : members == 1 ? static_cast<ygg::uint_t>(0) : infinity;
        valid &= graph.size(source_id) == register_members && graph.size(edge_id) == members;
        valid &= graph.evaluate(source_count_id).get() == register_members;
        valid &= graph.evaluate(edge_count_id).get() == members;
        valid &= graph.evaluate(distance_id).get() == expected_distance;
        valid &= graph.evaluate(self_distance_id).get() == (register_members ? 0 : infinity);
        valid &= graph.size(closure_id) == (members == 2 ? 3 : members);
        valid &= graph.evaluate(nonempty_id).get() == (members == 2);
        valid &= graph.size(projected_id) == members && graph.size(projected_role_id) == members;
        valid &= graph.evaluate(query_count_id).get() == members;
        for (const auto id : { source_count_id, edge_count_id, distance_id, self_distance_id, query_count_id })
            (void) graph.changed(id);
        (void) graph.changed(nonempty_id);
        for (const auto id : { source_id, projected_id })
            (void) graph.delta(id);
        for (const auto id : { edge_id, closure_id, projected_role_id })
            (void) graph.delta(id);
        return valid;
    };
    const auto cycle = [&]
    {
        bool valid = true;
        for (size_t i = 0; i < changes.size(); ++i)
            valid &= apply(changes[i], i + 1, changes.size() - i - 1, i == 0 ? 1 : 0);
        for (size_t i = changes.size(); i-- > 0;)
        {
            changes[i].reverse();
            valid &= apply(changes[i], i, changes.size() - i, 1);
            changes[i].reverse();
        }
        initialize();
        return valid;
    };

    // Warm both delta directions, first/last memberships, and invocation reinitialization.
    for (size_t repeat = 0; repeat < 8; ++repeat)
        ASSERT_TRUE(cycle());
    bool valid = true;
    allocation_tracking::Scope measured;
    for (size_t repeat = 0; repeat < 1000; ++repeat)
        valid &= cycle();
    const auto counts = measured.finish();
    EXPECT_TRUE(valid);
    EXPECT_EQ(counts.allocated, 0);
    EXPECT_EQ(counts.deallocated, 0);
}

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
    auto result = builder.get_builder<ygg::database::Relation>(columns);
    for (ygg::uint_t i = 0; i < 32; ++i)
        result->insert(std::tuple { static_cast<ygg::uint_t>(i), static_cast<ygg::uint_t>(i + 1) });
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
        auto next = builder.get_builder<ygg::database::Relation>(repeat % 2 ? renamed : columns);
        // Cista schema storage uses malloc, which the global new/delete counter does not cover.
        reused &= next.get() == slot;
        reused &= next->columns().data() == schema_buffer;
        reused &= next->memory_usage() - next->storage().memory_usage() == schema_capacity;
        reused &= next->storage().memory_usage() == row_capacity;
        for (ygg::uint_t i = 0; i < 32; ++i)
            next->insert(std::tuple { static_cast<ygg::uint_t>(i), static_cast<ygg::uint_t>(i + 1) });
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
            const auto object = ObjectIndex(static_cast<ygg::uint_t>(i + generation * 512));
            auto owner = builder.get_builder<ygg::database::Relation>(columns);
            owner->insert(std::tuple { object.get_value(), static_cast<ygg::uint_t>(1) });
            owner->insert(std::tuple { object.get_value(), static_cast<ygg::uint_t>(2) });
            const auto value = ygg::database::insert(repository, *owner, generation).first;
            const auto alias = repository.rename(value, aliases);
            valid &= value.size() == 2 && alias.size() == 2 && ObjectIndex(value[0].template get<ygg::uint_t>(size_t { 0 })) == object;
            valid &= value.get_storage_address() == alias.get_storage_address();
            auto duplicate = builder.get_builder<ygg::database::Relation>(columns);
            duplicate->insert(std::tuple { object.get_value(), static_cast<ygg::uint_t>(2) });
            duplicate->insert(std::tuple { object.get_value(), static_cast<ygg::uint_t>(1) });
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

TEST(RunirSearch, WarmedProgramSearchAllocationsGrowWithContainerCapacity)
{
    using Path = kr::ps::ext::detail::SearchPath<tyr::GroundTag, kr::ps::ext::InternedExecutionStorage<tyr::GroundTag>>;
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
                const auto length = scale == 0 ? static_cast<size_t>(64) : static_cast<size_t>(512);
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
                const auto program = kr::ps::ext::dl::parse_program(text, search->task->get_domain().get_domain(), *context->domain_context->ext_repository);
                auto options = kr::ps::ext::ProgramSearchOptions<tyr::GroundTag> {};
                options.universal = universal;
                auto expander = kr::ps::ext::SuccessorExpander<tyr::GroundTag>(context, program);
                const auto initial_node = search->successor_generator->get_packed_initial_node(*search->state_repository, *search->axiom_evaluator);
                const auto initial = expander.initial_state(initial_node.get_state().unpack());
                auto classifier = kr::ps::NoUnsolvability<tyr::GroundTag> {};
                auto path_pool = ygg::SharedObjectPool<Path> {};
                // Warm path and binding pools to the live DFS depth. Fresh search storage
                // on each pass ensures memoization cannot skip any of the measured work.
                for (const auto warmup : { true, false })
                {
                    SCOPED_TRACE(warmup);
                    auto storage = kr::ps::ext::detail::SearchStorage<tyr::GroundTag, kr::ps::ext::StateMemorization::ALL>(options);
                    auto statistics = kr::ps::ext::ProgramSearchStatistics {};
                    auto first_goal = ygg::SharedObjectPoolPtr<Path> {};
                    auto witness = ygg::SharedObjectPoolPtr<Path> {};
                    allocation_tracking::Scope measured;
                    const auto status = kr::ps::ext::detail::depth_first_search<
                        tyr::GroundTag>(expander, initial_node.get_state(), initial, options, classifier, storage, statistics, path_pool, first_goal, witness);
                    const auto counts = measured.finish();
                    if (!warmup)
                        allocations[scale] = counts.allocated;
                    ASSERT_EQ(status, kr::ps::ext::ProgramProofStatus::SUCCESS);
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

namespace
{
template<tyr::TaskKind Kind>
allocation_tracking::Counts measure_warmed_binding_enumeration(const kr::TaskContextPtr<Kind>& context, bool borrowed)
{
    auto& search = *context->search_context;
    auto& generator = *search.successor_generator;
    const auto node = generator.get_initial_node(*search.state_repository, *search.axiom_evaluator);
    bool valid = true;
    const auto enumerate = [&]
    {
        size_t emitted = 0;
        const auto visit = [&](auto)
        {
            ++emitted;
            return true;
        };
        valid &= borrowed ? generator.for_each_borrowed_applicable_action_binding(node, std::ref(visit)) :
                            generator.for_each_applicable_action_binding(node, std::ref(visit));
        valid &= emitted == 2;
    };
    for (size_t i = 0; i < 8; ++i)
        enumerate();
    EXPECT_TRUE(valid);

    allocation_tracking::Scope measured;
    for (size_t i = 0; i < 512; ++i)
        enumerate();
    const auto counts = measured.finish();
    EXPECT_TRUE(valid);
    return counts;
}

template<tyr::TaskKind Kind, kr::ps::ext::ExecutionStorageConcept<Kind> Storage>
void expect_warmed_action_expansions(const kr::TaskContextPtr<Kind>& context, kr::ps::ext::ProgramView program, allocation_tracking::Counts expected)
{
    auto expander = kr::ps::ext::SuccessorExpander<Kind, Storage>(context, program);
    auto& search = *context->search_context;
    const auto node = search.successor_generator->get_initial_node(*search.state_repository, *search.axiom_evaluator);
    const auto initial = expander.initial_state(node.get_state());
    auto statistics = kr::ps::ext::ProgramSearchStatistics {};
    bool valid = true;
    const auto expand = [&]
    {
        size_t emitted = 0;
        valid &= expander.for_each_successor(
            expander.view(initial),
            statistics,
            [&](const auto& step)
            {
                if constexpr (std::same_as<std::remove_cvref_t<decltype(step)>, kr::ps::ext::detail::ProgramStep<Kind, Storage>>)
                {
                    valid &= step.status == kr::ps::ext::detail::ProgramOutcome::APPLIED && step.state_transition.has_value();
                    ++emitted;
                }
                else
                    valid = false;
                return true;
            },
            [] { return false; });
        valid &= emitted == 2;
    };
    for (size_t i = 0; i < 8; ++i)
        expand();
    ASSERT_TRUE(valid);

    allocation_tracking::Scope measured;
    for (size_t i = 0; i < 512; ++i)
        expand();
    const auto counts = measured.finish();
    EXPECT_TRUE(valid);
    EXPECT_EQ(counts.allocated, expected.allocated);
    EXPECT_EQ(counts.deallocated, expected.deallocated);
}

template<tyr::TaskKind Kind>
void expect_warmed_action_storage()
{
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../fixtures/kr/ps/ext/choose";
    const auto search = [&]
    {
        if constexpr (std::same_as<Kind, tyr::GroundTag>)
            return make_ground_context(directory / "domain.pddl", directory / "task.pddl");
        else
            return make_lifted_context(directory / "domain.pddl", directory / "task.pddl");
    }();
    const auto context = kr::TaskContext<Kind>::create(kr::DomainContext::create(search->task->get_domain()), search);
    // Lifted Datalog cost buckets allocate during existing indexed enumeration. Borrowed
    // traversal and policy expansion must add nothing to that measured baseline.
    const auto baseline = measure_warmed_binding_enumeration(context, false);
    const auto borrowed = measure_warmed_binding_enumeration(context, true);
    EXPECT_EQ(borrowed.allocated, baseline.allocated);
    EXPECT_EQ(borrowed.deallocated, baseline.deallocated);
    // This fixture has one action, so the all-schema baseline also covers scoped Do.
    for (const auto& [rule, enumerates] : { std::pair { R"((:action (:conditions) (:action "move") (:query Moves) (:effects (unchanged N))))", false },
                                            std::pair { R"((:do (:conditions) (:action "move") (:arguments From To) (:effects (unchanged N))))", true },
                                            std::pair { R"((:sketch (:conditions) (:effects (unchanged N))))", true } })
    {
        SCOPED_TRACE(rule);
        const auto source = std::string(R"(
(:program (:entry actions)
  (:module (:symbol actions) (:arguments) (:registers)
    (:entry source) (:memory source target)
    (:features
      (:query (:symbol Moves) (:expression
        (q_join (q_atomic_state "edge" (from to)) (q_atomic_state "at" (from)))))
      (:concept (:symbol From) (:expression (c_atomic_state "at")))
      (:concept (:symbol To) (:expression (c_top)))
      (:numerical (:symbol N) (:expression (n_count (c_atomic_state "at")))))
    (:rules (:rule (:symbol move) (:expression
      (:source-memory source) (:target-memory target)
)") + rule + ")))))";
        const auto program = kr::ps::ext::dl::parse_program(source, search->task->get_domain().get_domain(), *context->domain_context->ext_repository);
        const auto expected = enumerates ? baseline : allocation_tracking::Counts {};
        expect_warmed_action_expansions<Kind, kr::ps::ext::InternedExecutionStorage<Kind>>(context, program, expected);
        expect_warmed_action_expansions<Kind, kr::ps::ext::TransientExecutionStorage<Kind>>(context, program, expected);
    }
}
}  // namespace

TEST(RunirSearch, WarmedActionExpansionsReuseGroundStorage) { expect_warmed_action_storage<tyr::GroundTag>(); }
TEST(RunirSearch, WarmedActionExpansionsReuseLiftedStorage) { expect_warmed_action_storage<tyr::LiftedTag>(); }

}  // namespace runir::tests
