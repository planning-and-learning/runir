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
#include <runir/kr/dl/semantics/detail/repository_instantiations.hpp>
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
#include <malloc.h>

namespace runir::tests
{
using ObjectIndex = ygg::Index<tyr::formalism::Object>;
using QueryValues = kr::dl::QueryValues;

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
            [&](auto step)
            {
                if constexpr (std::same_as<decltype(step), kr::ps::ext::detail::ProgramStep<Kind, Storage>>)
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
