#include "fixtures.hpp"
#include "module_fixtures.hpp"
#include "planning_fixtures.hpp"

#include <algorithm>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <runir/kr/ps/ext/dl/module_factory.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/module_program_executor.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/ext/successor_expander.hpp>
#include <runir/kr/task_context.hpp>
#include <runir/kr/uns/dl/parser.hpp>
#include <runir/kr/uns/repository.hpp>
#include <set>
#include <stdexcept>
#include <yggdrasil/serialization/json.hpp>

namespace runir::tests
{

TEST(RunirTests, ModuleProgramStatusesToString)
{
    using kr::ps::ext::ModuleProgramProofStatus;
    EXPECT_EQ(kr::ps::ext::to_string(ModuleProgramProofStatus::SUCCESS), "success");
    EXPECT_EQ(kr::ps::ext::to_string(ModuleProgramProofStatus::FAILURE), "failure");
    EXPECT_EQ(kr::ps::ext::to_string(ModuleProgramProofStatus::OUT_OF_TIME), "out_of_time");
    EXPECT_EQ(kr::ps::ext::to_string(ModuleProgramProofStatus::OUT_OF_STATES), "out_of_states");
    EXPECT_THROW((void) kr::ps::ext::to_string(static_cast<ModuleProgramProofStatus>(255)), std::invalid_argument);

    using kr::ps::ext::detail::ModuleProgramOutcome;
    EXPECT_EQ(kr::ps::ext::detail::to_string(ModuleProgramOutcome::SUCCESS), "success");
    EXPECT_EQ(kr::ps::ext::detail::to_string(ModuleProgramOutcome::APPLIED), "applied");
    EXPECT_EQ(kr::ps::ext::detail::to_string(ModuleProgramOutcome::RESTORED_CALLER), "restored_caller");
    EXPECT_EQ(kr::ps::ext::detail::to_string(ModuleProgramOutcome::FAILURE), "failure");
    EXPECT_EQ(kr::ps::ext::detail::to_string(ModuleProgramOutcome::NO_APPLICABLE_ACTION), "no_applicable_action");
    EXPECT_EQ(kr::ps::ext::detail::to_string(ModuleProgramOutcome::MALFORMED_CALL), "malformed_call");
    EXPECT_EQ(kr::ps::ext::detail::to_string(ModuleProgramOutcome::SEARCH_FAILURE), "search_failure");
    EXPECT_EQ(kr::ps::ext::detail::to_string(ModuleProgramOutcome::OUT_OF_TIME), "out_of_time");
    EXPECT_EQ(kr::ps::ext::detail::to_string(ModuleProgramOutcome::OUT_OF_STATES), "out_of_states");
    EXPECT_EQ(kr::ps::ext::detail::to_string(ModuleProgramOutcome::CYCLE), "cycle");
    EXPECT_THROW((void) kr::ps::ext::detail::to_string(static_cast<ModuleProgramOutcome>(255)), std::invalid_argument);
}

TEST(RunirTests, ExtFindSolutionTreatsClassifierMatchesAsTerminalFailures)
{
    namespace p = tyr::planning;

    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
    auto task_context = kr::TaskContext<p::GroundTag>::create(search_context);

    auto dl_repository = task_context->ext_dl_repository;
    auto repository = task_context->ext_repository;
    const auto module =
        kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/executor/ext_find_solution_treats_classifier_matches_as_terminal_failures/module.module"),
                                      task->get_domain().get_domain(),
                                      *repository);
    const auto program = create_module_program(*repository, module, { module });

    auto classifier_dl_repository = task_context->uns_dl_repository;
    auto classifier_repository = task_context->uns_repository;
    const auto classifier = kr::uns::dl::parse_classifier(read_fixture("kr/uns/always.classifier"), task->get_domain().get_domain(), *classifier_repository);

    auto options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag> {};
    options.classifier = classifier;
    const auto result = kr::ps::ext::find_solution(task_context, program, options);

    EXPECT_EQ(result.status, kr::ps::ext::ModuleProgramProofStatus::FAILURE);
    ASSERT_TRUE(result.graph);
    ASSERT_EQ(result.graph->get_num_vertices(), 1);
    EXPECT_EQ(result.graph->get_num_edges(), 0);
    ASSERT_EQ(result.deadend_states.size(), 1);
    EXPECT_TRUE(result.open_states.empty());
    const auto& label = result.graph->get_vertex(result.deadend_states.front()).get_property();
    EXPECT_FALSE(label.is_goal);
    EXPECT_FALSE(label.is_alive);
    EXPECT_TRUE(label.is_unsolvable);
    EXPECT_EQ(label.execution_state.get_call_stack().get_memory_state().get_name(), "source");
    EXPECT_GT(task_context->dl_denotation_repository->size<kr::dl::BooleanTag>(), 0);
}

TEST(RunirTests, ExtPaperModulesExecuteOnSmallBlocksworldInstance)
{
    namespace p = tyr::planning;

    const auto domain = benchmark_path("classical/profiling/blocksworld-large-simple/domain.pddl");
    const auto task_file = benchmark_path("classical/profiling/blocksworld-large-simple/p-100-2.pddl");
    auto search_context = make_ground_context(domain, task_file);
    auto task = search_context->task;
    auto task_context = kr::TaskContext<p::GroundTag>::create(search_context);

    auto dl_repository = task_context->ext_dl_repository;
    auto repository = task_context->ext_repository;

    const auto program = kr::ps::ext::dl::ModuleFactory::create_bonet_et_al_icaps2024_program(task->get_domain().get_domain(), *repository);
    ASSERT_EQ(program.get_modules().size(), 5);

    auto search_options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag>();

    const auto search_result = kr::ps::ext::find_solution(task_context, program, search_options);
    EXPECT_TRUE(search_result.is_successful());
    ASSERT_TRUE(search_result.plan.has_value());
    EXPECT_EQ(search_result.plan->get_length(), 4);

    auto proof_options = search_options;
    proof_options.universal = true;
    const auto proof = kr::ps::ext::find_solution(task_context, program, proof_options);
    EXPECT_EQ(proof.status, kr::ps::ext::ModuleProgramProofStatus::FAILURE) << fmt::format("{}", proof);
    ASSERT_TRUE(proof.graph);
    ASSERT_TRUE(search_result.graph);
    EXPECT_GT(proof.graph->get_num_vertices(), search_result.graph->get_num_vertices());

    auto has_internal_memory_state = false;
    auto has_external_memory_state = false;
    for (const auto vertex : proof.graph->get_vertex_indices())
    {
        const auto phase = proof.graph->get_vertex(vertex).get_property().execution_state.get_phase();
        has_internal_memory_state |= phase == kr::ps::ext::ExecutionPhase::INTERNAL;
        has_external_memory_state |= phase == kr::ps::ext::ExecutionPhase::EXTERNAL;
    }
    EXPECT_TRUE(has_internal_memory_state);
    EXPECT_TRUE(has_external_memory_state);

    EXPECT_FALSE(proof.cycle.empty());
}

TEST(RunirTests, ExtSketchUsesOnlyImmediateOutcomesAndUniversalPreservesParallelEdges)
{
    namespace p = tyr::planning;

    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
    auto task_context = kr::TaskContext<p::GroundTag>::create(search_context);

    auto dl_repository = task_context->ext_dl_repository;
    auto repository = task_context->ext_repository;
    const auto module = kr::ps::ext::dl::parse_module(
        read_fixture("kr/ps/ext/executor/ext_sketch_uses_only_immediate_outcomes_and_universal_preserves_parallel_edges/module.module"),
        task->get_domain().get_domain(),
        *repository);
    const auto program = create_module_program(*repository, module, { module });
    auto expander = kr::ps::ext::SuccessorExpander<p::GroundTag>(task_context, program);
    const auto initial_state = expander.initial_state();
    const auto immediate = expander.labeled_successors(initial_state);
    const auto steps = expander.control_steps(initial_state, immediate);
    ASSERT_GT(immediate.size(), 1);
    ASSERT_EQ(steps.size(), immediate.size() * 2);
    for (const auto& step : steps)
        EXPECT_EQ(step.plan_suffix.size(), 1);

    auto greedy_options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag> {};
    const auto greedy = kr::ps::ext::find_solution(task_context, program, greedy_options);
    auto universal_options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag> {};
    universal_options.universal = true;
    const auto universal = kr::ps::ext::find_solution(task_context, program, universal_options);
    ASSERT_TRUE(greedy.graph);
    ASSERT_TRUE(universal.graph);
    EXPECT_EQ(greedy.graph->get_out_degree(0), 1);
    EXPECT_EQ(universal.graph->get_out_degree(0), steps.size());

    auto targets = std::set<runir::graphs::VertexIndex> {};
    for (const auto edge : universal.graph->get_out_edge_indices(0))
        targets.insert(universal.graph->get_target(edge));
    EXPECT_LT(targets.size(), universal.graph->get_out_degree(0));

    const auto two_step_module = kr::ps::ext::dl::parse_module(
        read_fixture("kr/ps/ext/executor/ext_sketch_uses_only_immediate_outcomes_and_universal_preserves_parallel_edges/two_step_module.module"),
        task->get_domain().get_domain(),
        *repository);
    const auto two_step_program = create_module_program(*repository, two_step_module, { two_step_module });
    auto two_step_expander = kr::ps::ext::SuccessorExpander<p::GroundTag>(task_context, two_step_program);
    const auto two_step_state = two_step_expander.initial_state();
    const auto two_step_outcomes = two_step_expander.control_steps(two_step_state);
    ASSERT_EQ(two_step_outcomes.size(), 1);
    EXPECT_EQ(two_step_outcomes.front().status, kr::ps::ext::detail::ModuleProgramOutcome::NO_APPLICABLE_ACTION);

    auto rejected_options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag> {};
    const auto rejected = kr::ps::ext::find_solution(task_context, two_step_program, rejected_options);
    EXPECT_EQ(rejected.status, kr::ps::ext::ModuleProgramProofStatus::FAILURE);
    ASSERT_TRUE(rejected.graph);
    EXPECT_EQ(rejected.graph->get_num_vertices(), 1);
    EXPECT_EQ(rejected.graph->get_num_edges(), 0);
}

TEST(RunirTests, ExtFindSolutionReportsTheCompleteThreeStateCycle)
{
    namespace p = tyr::planning;

    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
    auto task_context = kr::TaskContext<p::GroundTag>::create(search_context);

    auto dl_repository = task_context->ext_dl_repository;
    auto repository = task_context->ext_repository;
    const auto module = kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/executor/ext_find_solution_reports_the_complete_three_state_cycle/module.module"),
                                                      task->get_domain().get_domain(),
                                                      *repository);
    const auto program = create_module_program(*repository, module, { module });

    auto options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag> {};
    const auto result = kr::ps::ext::find_solution(task_context, program, options);
    EXPECT_EQ(result.status, kr::ps::ext::ModuleProgramProofStatus::FAILURE);
    ASSERT_TRUE(result.graph);
    EXPECT_EQ(result.graph->get_num_vertices(), 3);
    EXPECT_EQ(result.graph->get_num_edges(), 3);
    ASSERT_EQ(result.cycle.size(), 4);
    EXPECT_EQ(result.cycle.front(), result.cycle.back());
    EXPECT_EQ(std::set(result.cycle.begin(), result.cycle.end()).size(), 3);
}

TEST(RunirTests, ExtExecutorFixtureOutcomesMatch)
{
    namespace p = tyr::planning;

    const auto domain = benchmark_path("classical/profiling/blocksworld-large-simple/domain.pddl");
    const auto task_file = benchmark_path("classical/profiling/blocksworld-large-simple/p-100-2.pddl");
    auto search_context = make_ground_context(domain, task_file);
    auto task = search_context->task;
    auto task_context = kr::TaskContext<p::GroundTag>::create(search_context);
    const auto suite = load_fixture_json("kr/ps/ext/execution.json");
    const auto& cases = ygg::common::as_array(ygg::common::require_member(ygg::common::as_object(suite, "suite"), "cases", "suite"), "suite.cases");

    for (const auto& value : cases)
    {
        const auto& test_case = ygg::common::as_object(value, "case");
        const auto program = kr::ps::ext::dl::parse_module_program(read_fixture(ygg::common::as_string(test_case, "program_file", "case")),
                                                                   task->get_domain().get_domain(),
                                                                   *task_context->ext_repository);
        auto options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag>();
        options.universal = ygg::common::as_bool(test_case, "universal", "case");

        const auto result = kr::ps::ext::find_solution(task_context, program, options);

        const auto status = ygg::common::as_string(test_case, "status", "case");
        EXPECT_EQ(kr::ps::ext::to_string(result.status), status);
        ASSERT_TRUE(result.graph);
        EXPECT_EQ(result.graph->get_num_vertices(), ygg::common::as_size(test_case, "num_vertices", "case"));
        EXPECT_EQ(result.graph->get_num_edges(), ygg::common::as_size(test_case, "num_edges", "case"));
        EXPECT_EQ(result.deadend_states.size(), ygg::common::as_size(test_case, "num_deadends", "case"));
        EXPECT_EQ(!result.open_states.empty(), ygg::common::as_bool(test_case, "has_open_states", "case"));
        EXPECT_EQ(result.cycle.size(), ygg::common::as_size(test_case, "cycle_length", "case"));
    }
}

}  // namespace runir::tests
