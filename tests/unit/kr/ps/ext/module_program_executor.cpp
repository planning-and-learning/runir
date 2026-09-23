#include "fixtures.hpp"
#include "module_fixtures.hpp"
#include "planning_fixtures.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
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
    auto task_context = kr::TaskContext<tyr::GroundTag>::create(kr::DomainContext::create(task->get_domain()), search_context);

    auto dl_repository = task_context->domain_context->ext_repository->get_dl_repository_ptr();
    auto repository = task_context->domain_context->ext_repository;
    const auto module =
        kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/executor/ext_find_solution_treats_classifier_matches_as_terminal_failures/module.module"),
                                      task->get_domain().get_domain(),
                                      *repository);
    const auto program = create_module_program(*repository, module, { module });

    auto classifier_dl_repository = task_context->domain_context->uns_repository->get_dl_repository_ptr();
    auto classifier_repository = task_context->domain_context->uns_repository;
    const auto classifier = kr::uns::dl::parse_classifier(read_fixture("kr/uns/always.classifier"), task->get_domain().get_domain(), *classifier_repository);

    auto options = kr::ps::ext::ModuleProgramSearchOptions<tyr::GroundTag> {};
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
    EXPECT_EQ(result.statistics.num_expanded, 0);
    EXPECT_EQ(result.statistics.num_generated, 0);
    EXPECT_GT(task_context->dl_denotation_repository->size<kr::dl::semantics::Denotation<kr::dl::BooleanTag>>(), 0);
}

TEST(RunirTests, ExtPaperModulesExecuteOnSmallBlocksworldInstance)
{
    namespace p = tyr::planning;

    const auto domain = benchmark_path("classical/profiling/blocksworld-large-simple/domain.pddl");
    const auto task_file = benchmark_path("classical/profiling/blocksworld-large-simple/p-100-2.pddl");
    auto search_context = make_ground_context(domain, task_file);
    auto task = search_context->task;
    auto task_context = kr::TaskContext<tyr::GroundTag>::create(kr::DomainContext::create(task->get_domain()), search_context);

    auto dl_repository = task_context->domain_context->ext_repository->get_dl_repository_ptr();
    auto repository = task_context->domain_context->ext_repository;

    const auto program = kr::ps::ext::dl::ModuleFactory::create_bonet_et_al_icaps2024_program(task->get_domain().get_domain(), *repository);
    ASSERT_EQ(program.get_modules().size(), 5);

    auto search_options = kr::ps::ext::ModuleProgramSearchOptions<tyr::GroundTag>();

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
    auto task_context = kr::TaskContext<tyr::GroundTag>::create(kr::DomainContext::create(task->get_domain()), search_context);

    auto dl_repository = task_context->domain_context->ext_repository->get_dl_repository_ptr();
    auto repository = task_context->domain_context->ext_repository;
    const auto module = kr::ps::ext::dl::parse_module(
        read_fixture("kr/ps/ext/executor/ext_sketch_uses_only_immediate_outcomes_and_universal_preserves_parallel_edges/module.module"),
        task->get_domain().get_domain(),
        *repository);
    const auto program = create_module_program(*repository, module, { module });
    auto expander = kr::ps::ext::SuccessorExpander<tyr::GroundTag>(task_context, program);
    const auto initial_state = expander.initial_state();
    const auto immediate = expander.labeled_successors(initial_state);
    const auto steps = expander.control_steps(initial_state, immediate);
    ASSERT_GT(immediate.size(), 1);
    ASSERT_EQ(steps.size(), immediate.size() * 2);
    for (const auto& step : steps)
        EXPECT_EQ(step.plan_suffix.size(), 1);

    auto greedy_options = kr::ps::ext::ModuleProgramSearchOptions<tyr::GroundTag> {};
    const auto greedy = kr::ps::ext::find_solution(task_context, program, greedy_options);
    auto universal_options = kr::ps::ext::ModuleProgramSearchOptions<tyr::GroundTag> {};
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
    auto two_step_expander = kr::ps::ext::SuccessorExpander<tyr::GroundTag>(task_context, two_step_program);
    const auto two_step_state = two_step_expander.initial_state();
    const auto two_step_outcomes = two_step_expander.control_steps(two_step_state);
    ASSERT_EQ(two_step_outcomes.size(), 1);
    EXPECT_EQ(two_step_outcomes.front().status, kr::ps::ext::detail::ModuleProgramOutcome::NO_APPLICABLE_ACTION);

    auto rejected_options = kr::ps::ext::ModuleProgramSearchOptions<tyr::GroundTag> {};
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
    auto task_context = kr::TaskContext<tyr::GroundTag>::create(kr::DomainContext::create(task->get_domain()), search_context);

    auto dl_repository = task_context->domain_context->ext_repository->get_dl_repository_ptr();
    auto repository = task_context->domain_context->ext_repository;
    const auto module = kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/executor/ext_find_solution_reports_the_complete_three_state_cycle/module.module"),
                                                      task->get_domain().get_domain(),
                                                      *repository);
    const auto program = create_module_program(*repository, module, { module });

    auto options = kr::ps::ext::ModuleProgramSearchOptions<tyr::GroundTag> {};
    const auto result = kr::ps::ext::find_solution(task_context, program, options);
    EXPECT_EQ(result.status, kr::ps::ext::ModuleProgramProofStatus::FAILURE);
    ASSERT_TRUE(result.graph);
    EXPECT_EQ(result.graph->get_num_vertices(), 3);
    EXPECT_EQ(result.graph->get_num_edges(), 3);
    EXPECT_EQ(result.statistics.num_expanded, 3);
    EXPECT_EQ(result.statistics.num_generated, 3);  // Generating an already visited target still counts.
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
    auto task_context = kr::TaskContext<tyr::GroundTag>::create(kr::DomainContext::create(task->get_domain()), search_context);
    const auto suite = load_fixture_json("kr/ps/ext/execution.json");
    const auto& cases = ygg::common::as_array(ygg::common::require_member(ygg::common::as_object(suite, "suite"), "cases", "suite"), "suite.cases");

    for (const auto& value : cases)
    {
        const auto& test_case = ygg::common::as_object(value, "case");
        const auto program = kr::ps::ext::dl::parse_module_program(read_fixture(ygg::common::as_string(test_case, "program_file", "case")),
                                                                   task->get_domain().get_domain(),
                                                                   *task_context->domain_context->ext_repository);
        auto options = kr::ps::ext::ModuleProgramSearchOptions<tyr::GroundTag>();
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

namespace
{

std::string choice_module(const std::string& name, const std::string& rules)
{
    return "(:module (:symbol " + name + R"()
        (:arguments) (:registers (:concept r0) (:concept r1))
        (:entry m0) (:memory m0 m1 m2 m3 m4 m5 m6)
        (:features
            (:concept (:symbol Candidates) (:expression (c_atomic_state "candidate")))
            (:concept (:symbol Empty) (:expression (c_bot)))
            (:concept (:symbol Here) (:expression (c_atomic_state "at")))
            (:concept (:symbol Goal) (:expression (c_atomic_goal "at" true)))
            (:concept (:symbol R) (:expression (c_register r0)))
            (:boolean (:symbol Bad) (:expression (b_nonempty (c_and (c_register r0) (c_atomic_state "bad")))))
            (:boolean (:symbol Same) (:expression (b_nonempty (c_and (c_register r0) (c_register r1))))))
        (:rules )"
           + rules + "))";
}

std::string choice_rule(const std::string& name, const std::string& source, const std::string& target, const std::string& body)
{
    return "(:rule (:symbol " + name + ") (:expression (:source-memory " + source + ") (:target-memory " + target + ") " + body + "))";
}

const auto choose_candidates = std::string("(:choose (:conditions) (:concept Candidates) (:register (:concept r0)))");
const auto choose_empty = std::string("(:choose (:conditions) (:concept Empty) (:register (:concept r0)))");
const auto move_to_register = std::string(R"((:do (:conditions) (:action "move") (:arguments Here R) (:effects)))");
const auto move_to_goal = std::string(R"((:do (:conditions) (:action "move") (:arguments Here Goal) (:effects)))");
const auto move_rules = choice_rule("move-selected", "m1", "m2", move_to_register) + choice_rule("finish", "m2", "m3", move_to_goal);

template<tyr::TaskKind Kind>
void check_choice_execution()
{
    namespace ext = kr::ps::ext;
    using Status = ext::ModuleProgramProofStatus;
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../../fixtures/kr/ps/ext/choose";
    auto search_context = [&]()
    {
        if constexpr (std::same_as<Kind, tyr::GroundTag>)
            return make_ground_context(directory / "domain.pddl", directory / "task.pddl");
        else
            return make_lifted_context(directory / "domain.pddl", directory / "task.pddl");
    }();
    const auto task = search_context->task;
    auto context = kr::TaskContext<Kind>::create(kr::DomainContext::create(task->get_domain()), search_context);
    auto& repository = *context->domain_context->ext_repository;
    const auto parse = [&](const std::string& text) { return ext::dl::parse_module(text, task->get_domain().get_domain(), repository); };
    const auto make_program = [&](const std::string& text)
    {
        const auto module = parse(text);
        return create_module_program(repository, module, { module });
    };
    const auto program = make_program(choice_module("choose", choice_rule("select", "m0", "m1", choose_candidates) + move_rules));
    auto expander = ext::SuccessorExpander<Kind>(context, program);
    const auto bindings = expander.choose_steps(expander.initial_state());
    ASSERT_EQ(bindings.size(), 2);
    EXPECT_EQ(bindings[0].target.get_call_stack().get_registers().get_concept_values()[0].value().get_name(), "bad");

    for (const auto universal : { false, true })
    {
        SCOPED_TRACE(universal);
        auto options = ext::ModuleProgramSearchOptions<Kind> {};
        options.universal = universal;
        const auto result = ext::find_solution(context, program, options);
        ASSERT_EQ(result.status, Status::SUCCESS);
        EXPECT_EQ(result.statistics.choice_depth, 1);
        EXPECT_EQ(result.statistics.max_choice_depth, 1);
        EXPECT_EQ(result.statistics.num_choice_points, 1);
        EXPECT_EQ(result.statistics.num_binding_attempts, 2);
        EXPECT_EQ(result.statistics.num_backtracks, 1);
        EXPECT_EQ(result.statistics.num_expanded, 5);
        EXPECT_EQ(result.statistics.num_generated, 5);
        EXPECT_FALSE(result.open_states.empty());  // The bad binding really was attempted.
        if (!universal)
        {
            ASSERT_TRUE(result.plan);
            EXPECT_EQ(result.plan->get_length(), 2);
            EXPECT_TRUE(expander.is_goal(result.plan->get_labeled_succ_nodes().back().node.get_state().unpack()));
            EXPECT_EQ(result.plan->get_labeled_succ_nodes().front().label.get_objects()[1].get_name(), "good");
        }

        const auto cyclic = make_program(choice_module(
            "cyclic",
            choice_rule("select", "m0", "m1", choose_candidates) + choice_rule("loop", "m1", "m1", "(:sketch (:conditions (positive Bad)) (:effects))")
                + choice_rule("move-selected", "m1", "m2", R"((:do (:conditions (negative Bad)) (:action "move") (:arguments Here R) (:effects)))")
                + choice_rule("finish", "m2", "m3", move_to_goal)));
        const auto cyclic_result = ext::find_solution(context, cyclic, options);
        EXPECT_EQ(cyclic_result.status, Status::SUCCESS);
        EXPECT_EQ(cyclic_result.statistics.choice_depth, 1);
        EXPECT_EQ(cyclic_result.statistics.max_choice_depth, 1);
        EXPECT_EQ(cyclic_result.statistics.num_binding_attempts, 2);
        EXPECT_EQ(cyclic_result.statistics.num_backtracks, 1);
        EXPECT_FALSE(cyclic_result.cycle.empty());

        for (const auto& body : { choose_empty,
                                  std::string("(:choose (:conditions) (:concept Candidates) (:register (:concept r0)) (:effects (positive Same)))") })
        {
            const auto empty = make_program(choice_module("empty", choice_rule("select", "m0", "m1", body)));
            const auto empty_result = ext::find_solution(context, empty, options);
            EXPECT_EQ(empty_result.status, Status::FAILURE);
            EXPECT_EQ(empty_result.statistics.choice_depth, 0);
            EXPECT_EQ(empty_result.statistics.max_choice_depth, 0);
            EXPECT_EQ(empty_result.statistics.num_choice_points, 0);
            EXPECT_EQ(empty_result.statistics.num_binding_attempts, 0);
            EXPECT_EQ(empty_result.statistics.num_backtracks, 0);
            EXPECT_EQ(empty_result.statistics.num_expanded, 1);
            EXPECT_EQ(empty_result.statistics.num_generated, 0);
            EXPECT_FALSE(empty_result.deadend_states.empty());
        }

        for (const auto good : { false, true })
        {
            SCOPED_TRACE(good);
            const auto filtered = make_program(choice_module(
                "singleton",
                choice_rule("select", "m0", "m1",
                            std::string("(:choose (:conditions) (:concept Candidates) (:register (:concept r0)) (:effects (")
                                + (good ? "negative" : "positive") + " Bad)))")
                    + move_rules));
            const auto filtered_result = ext::find_solution(context, filtered, options);
            EXPECT_EQ(filtered_result.status, good ? Status::SUCCESS : Status::FAILURE);
            EXPECT_EQ(filtered_result.statistics.choice_depth, 0);
            EXPECT_EQ(filtered_result.statistics.max_choice_depth, 0);
            EXPECT_EQ(filtered_result.statistics.num_choice_points, 0);
            EXPECT_EQ(filtered_result.statistics.num_binding_attempts, 1);
            EXPECT_EQ(filtered_result.statistics.num_backtracks, good ? 0 : 1);
        }

        const auto ordinary = make_program(choice_module(
            "ordinary-success",
            choice_rule("select", "m0", "m1", "(:load (:conditions) (:concept Candidates) (:register (:concept r0)) (:effects (negative Bad)))")
                + move_rules));
        const auto ordinary_result = ext::find_solution(context, ordinary, options);
        EXPECT_EQ(ordinary_result.status, Status::SUCCESS);
        EXPECT_EQ(ordinary_result.statistics.choice_depth, 0);
        EXPECT_EQ(ordinary_result.statistics.max_choice_depth, 0);
        EXPECT_EQ(ordinary_result.statistics.num_choice_points, 0);
        EXPECT_EQ(ordinary_result.statistics.num_binding_attempts, 0);
        EXPECT_EQ(ordinary_result.statistics.num_backtracks, 0);
        EXPECT_EQ(ordinary_result.statistics.num_expanded, 3);
        EXPECT_EQ(ordinary_result.statistics.num_generated, 3);

        const auto exhausted = make_program(choice_module("exhausted", choice_rule("select", "m0", "m1", choose_candidates)));
        const auto exhausted_result = ext::find_solution(context, exhausted, options);
        EXPECT_EQ(exhausted_result.status, Status::FAILURE);
        EXPECT_EQ(exhausted_result.statistics.choice_depth, 0);
        EXPECT_EQ(exhausted_result.statistics.max_choice_depth, 1);
        EXPECT_EQ(exhausted_result.statistics.num_choice_points, 1);
        EXPECT_EQ(exhausted_result.statistics.num_binding_attempts, 2);
        EXPECT_EQ(exhausted_result.statistics.num_backtracks, 2);
        EXPECT_EQ(exhausted_result.statistics.num_expanded, 3);
        EXPECT_EQ(exhausted_result.statistics.num_generated, 2);

        const auto nested = make_program(choice_module(
            "nested",
            choice_rule("outer", "m0", "m1", choose_candidates)
                + choice_rule("inner", "m1", "m4", "(:choose (:conditions) (:concept Candidates) (:register (:concept r1)))")
                + choice_rule("move-selected", "m4", "m2", R"((:do (:conditions (positive Same)) (:action "move") (:arguments Here R) (:effects)))")
                + choice_rule("finish", "m2", "m3", move_to_goal)));
        const auto nested_result = ext::find_solution(context, nested, options);
        EXPECT_EQ(nested_result.status, Status::SUCCESS);
        EXPECT_EQ(nested_result.statistics.choice_depth, 2);
        EXPECT_EQ(nested_result.statistics.max_choice_depth, 2);
        EXPECT_EQ(nested_result.statistics.num_choice_points, 3);
        EXPECT_EQ(nested_result.statistics.num_binding_attempts, 6);
        EXPECT_EQ(nested_result.statistics.num_backtracks, 4);
        if (!universal)
        {
            ASSERT_TRUE(nested_result.plan);
            EXPECT_EQ(nested_result.plan->get_length(), 2);
        }

        // The rejected branch crosses three choices; only the outer choice remains on the winning path.
        const auto deeper_failure = make_program(choice_module(
            "deeper-failure",
            choice_rule("outer", "m0", "m1", choose_candidates)
                + choice_rule("bad-inner", "m1", "m4", "(:choose (:conditions (positive Bad)) (:concept Candidates) (:register (:concept r1)))")
                + choice_rule("bad-deepest", "m4", "m5", "(:choose (:conditions) (:concept Candidates) (:register (:concept r1)))")
                + choice_rule("move-good", "m1", "m2", R"((:do (:conditions (negative Bad)) (:action "move") (:arguments Here R) (:effects)))")
                + choice_rule("finish", "m2", "m3", move_to_goal)));
        const auto deeper_failure_result = ext::find_solution(context, deeper_failure, options);
        EXPECT_EQ(deeper_failure_result.status, Status::SUCCESS);
        EXPECT_EQ(deeper_failure_result.statistics.choice_depth, 1);
        EXPECT_EQ(deeper_failure_result.statistics.max_choice_depth, 3);
        EXPECT_EQ(deeper_failure_result.statistics.num_choice_points, 4);
        EXPECT_EQ(deeper_failure_result.statistics.num_binding_attempts, 8);
        EXPECT_EQ(deeper_failure_result.statistics.num_backtracks, 7);
        // The deepest failed states are revisited through both inner bindings, but only expanded once.
        EXPECT_EQ(deeper_failure_result.statistics.num_expanded, 8);
        EXPECT_EQ(deeper_failure_result.statistics.num_generated, 10);

        const auto callee =
            parse(choice_module("callee", choice_rule("select", "m0", "m1", choose_candidates) + choice_rule("move-selected", "m1", "m3", move_to_register)));
        const auto caller = parse(choice_module("caller",
                                                choice_rule("save-goal", "m0", "m1", "(:load (:conditions) (:concept Goal) (:register (:concept r0)))")
                                                    + choice_rule("call", "m1", "m2", "(:call (:conditions) (:callee callee) (:arguments))")
                                                    + choice_rule("finish", "m2", "m3", move_to_register)));
        const auto call_program = create_module_program(repository, caller, { caller, callee });
        const auto call_result = ext::find_solution(context, call_program, options);
        EXPECT_EQ(call_result.status, Status::SUCCESS);
        EXPECT_EQ(call_result.statistics.choice_depth, 1);
        EXPECT_EQ(call_result.statistics.max_choice_depth, 1);
        EXPECT_EQ(call_result.statistics.num_binding_attempts, 2);
        EXPECT_EQ(call_result.statistics.num_backtracks, 1);
        EXPECT_EQ(call_result.statistics.num_expanded, 9);
        EXPECT_EQ(call_result.statistics.num_generated, 9);  // Includes entering the callee and both caller returns.
        EXPECT_FALSE(call_result.open_states.empty());
        if (!universal)
        {
            ASSERT_TRUE(call_result.plan);
            EXPECT_EQ(call_result.plan->get_length(), 2);
            EXPECT_EQ(call_result.plan->get_labeled_succ_nodes().front().label.get_objects()[1].get_name(), "good");
        }

        // A normal return here would let the caller reach the goal: the empty choose must prevent it.
        const auto empty_callee = parse(choice_module(
            "callee",
            choice_rule("load-good", "m0", "m1", "(:load (:conditions) (:concept Candidates) (:register (:concept r0)) (:effects (negative Bad)))")
                + choice_rule("move-good", "m1", "m2", move_to_register) + choice_rule("empty", "m2", "m3", choose_empty)));
        const auto blocked_call = create_module_program(repository, caller, { caller, empty_callee });
        EXPECT_EQ(ext::find_solution(context, blocked_call, options).status, Status::FAILURE);

        auto limited = options;
        limited.max_num_states = 1;
        const auto state_limited = ext::find_solution(context, program, limited);
        EXPECT_EQ(state_limited.status, Status::OUT_OF_STATES);
        EXPECT_EQ(state_limited.statistics.choice_depth, 0);
        EXPECT_EQ(state_limited.statistics.max_choice_depth, 1);
        EXPECT_EQ(state_limited.statistics.num_expanded, 1);
        EXPECT_EQ(state_limited.statistics.num_generated, 2);  // Both bindings are generated before the graph hits its limit.
        limited.max_num_states = result.graph->get_num_vertices() - 1;
        EXPECT_EQ(ext::find_solution(context, program, limited).status, Status::OUT_OF_STATES);
        limited = options;
        limited.max_time = std::chrono::steady_clock::duration::zero();
        const auto time_limited = ext::find_solution(context, program, limited);
        EXPECT_EQ(time_limited.status, Status::OUT_OF_TIME);
        EXPECT_EQ(time_limited.statistics.choice_depth, 0);
        EXPECT_EQ(time_limited.statistics.max_choice_depth, 0);
        EXPECT_EQ(time_limited.statistics.num_expanded, 0);
        EXPECT_EQ(time_limited.statistics.num_generated, 0);

        auto shuffled = options;
        shuffled.shuffle_choice_points = true;
        shuffled.random_seed = 42;
        const auto first = ext::find_solution(context, program, shuffled);
        const auto second = ext::find_solution(context, program, shuffled);
        EXPECT_EQ(first.status, Status::SUCCESS);
        EXPECT_EQ(first.status, second.status);
        EXPECT_EQ(first.graph->get_num_vertices(), second.graph->get_num_vertices());
        EXPECT_EQ(first.graph->get_num_edges(), second.graph->get_num_edges());
    }

    auto universal = ext::ModuleProgramSearchOptions<Kind> {};
    universal.universal = true;
    const auto load_goal = std::string("(:load (:conditions) (:concept Goal) (:register (:concept r0)))");
    const auto load_good = std::string("(:load (:conditions) (:concept Candidates) (:register (:concept r0)) (:effects (negative Bad)))");
    const auto parallel = make_program(choice_module(
        "parallel-choices",
        choice_rule("left", "m0", "m1", choose_candidates) + choice_rule("right", "m0", "m2", choose_candidates)
            + choice_rule("join-left", "m1", "m3", load_good) + choice_rule("join-right", "m2", "m3", load_good)
            + choice_rule("move", "m3", "m4", move_to_register) + choice_rule("finish", "m4", "m5", move_to_goal)));
    const auto parallel_result = ext::find_solution(context, parallel, universal);
    EXPECT_EQ(parallel_result.status, Status::SUCCESS);
    EXPECT_EQ(parallel_result.statistics.choice_depth, 1);  // Maximum path depth, not the sum of parallel obligations.
    EXPECT_EQ(parallel_result.statistics.max_choice_depth, 1);
    EXPECT_EQ(parallel_result.statistics.num_choice_points, 2);
    EXPECT_EQ(parallel_result.statistics.num_binding_attempts, 2);
    EXPECT_EQ(parallel_result.statistics.num_backtracks, 0);
    EXPECT_EQ(parallel_result.statistics.num_expanded, 5);
    EXPECT_EQ(parallel_result.statistics.num_generated, 8);  // Includes the two untried bindings, not just proof edges.

    const auto reconverged_rules =
        // The LIFO frontier expands the short route and its suffix before the longer route reaches the join.
        choice_rule("long-route", "m0", "m1", load_goal) + choice_rule("short-route", "m0", "m3", load_goal)
            + choice_rule("long-choice", "m1", "m2", choose_candidates) + choice_rule("normalize-prefix", "m2", "m3", load_goal)
            + choice_rule("shared-choice", "m3", "m4", choose_candidates) + choice_rule("normalize-suffix", "m4", "m5", load_good)
            + choice_rule("move", "m5", "m6", move_to_register) + choice_rule("finish", "m6", "m0", move_to_goal);
    const auto reconverged = make_program(choice_module("unequal-depths", reconverged_rules));
    const auto reconverged_result = ext::find_solution(context, reconverged, universal);
    EXPECT_EQ(reconverged_result.status, Status::SUCCESS);
    EXPECT_EQ(reconverged_result.statistics.choice_depth, 2);
    EXPECT_EQ(reconverged_result.statistics.max_choice_depth, 2);
    EXPECT_EQ(reconverged_result.statistics.num_choice_points, 2);
    EXPECT_EQ(reconverged_result.statistics.num_binding_attempts, 2);
    EXPECT_EQ(reconverged_result.statistics.num_backtracks, 0);

    // The next ordinary obligation hits the state limit just after the longer route reaches the shared suffix.
    const auto partial = make_program(choice_module("partial-unequal-depths", reconverged_rules + choice_rule("exceed-limit", "m2", "m6", load_goal)));
    auto limited = universal;
    limited.max_num_states = reconverged_result.graph->get_num_vertices();
    const auto partial_result = ext::find_solution(context, partial, limited);
    EXPECT_EQ(partial_result.status, Status::OUT_OF_STATES);
    EXPECT_EQ(partial_result.statistics.choice_depth, 0);
    EXPECT_EQ(partial_result.statistics.max_choice_depth, 2);

    const auto revisited = make_program(
        choice_module("revisited",
                      choice_rule("init", "m0", "m1", load_goal) + choice_rule("first-A", "m1", "m2", load_goal) + choice_rule("then-B", "m1", "m3", load_goal)
                          + choice_rule("A", "m2", "m4", choose_candidates) + choice_rule("B", "m3", "m2", load_goal)
                          + choice_rule("bad-to-B", "m4", "m3", "(:load (:conditions (positive Bad)) (:concept Goal) (:register (:concept r0)))")
                          + choice_rule("good-move", "m4", "m5", R"((:do (:conditions (negative Bad)) (:action "move") (:arguments Here R) (:effects)))")
                          + choice_rule("finish", "m5", "m0", move_to_goal)));
    const auto revisited_result = ext::find_solution(context, revisited, universal);
    EXPECT_EQ(revisited_result.status, Status::SUCCESS);
    EXPECT_FALSE(revisited_result.cycle.empty());

    // Each rule retains its own obligation: a successful choose cannot hide a bad load or another empty choose.
    for (const auto& other : { std::string("(:load (:conditions) (:concept Candidates) (:register (:concept r0)))"), choose_empty })
    {
        const auto mixed =
            make_program(choice_module("mixed", choice_rule("select", "m0", "m1", choose_candidates) + choice_rule("other", "m0", "m1", other) + move_rules));
        EXPECT_EQ(ext::find_solution(context, mixed, universal).status, Status::FAILURE);
    }
}

}  // namespace

TEST(RunirTests, ExtChooseBacktracksInGroundExecution) { check_choice_execution<tyr::GroundTag>(); }
TEST(RunirTests, ExtChooseBacktracksInLiftedExecution) { check_choice_execution<tyr::LiftedTag>(); }

namespace
{

template<tyr::TaskKind Kind>
void check_ordinary_execution_keeps_complete_graph()
{
    namespace ext = kr::ps::ext;
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../../fixtures/kr/ps/ext/choose";
    auto search_context = [&]()
    {
        if constexpr (std::same_as<Kind, tyr::GroundTag>)
            return make_ground_context(directory / "domain.pddl", directory / "task.pddl");
        else
            return make_lifted_context(directory / "domain.pddl", directory / "task.pddl");
    }();
    const auto task = search_context->task;
    auto context = kr::TaskContext<Kind>::create(kr::DomainContext::create(task->get_domain()), search_context);
    auto& repository = *context->domain_context->ext_repository;
    const auto classifier = kr::uns::dl::parse_classifier(
        R"((:classifier (:symbol bad-location)
            (:features (:boolean (:symbol bad) (:expression (b_nonempty (c_and (c_atomic_state "at") (c_atomic_state "bad"))))))
            (:expression (or (and bad)))))",
        task->get_domain().get_domain(),
        *context->domain_context->uns_repository);
    const auto skip = std::string("(:sketch (:conditions) (:effects))");
    const auto load = std::string("(:load (:conditions) (:concept Candidates) (:register (:concept r0)))");

    for (const auto diamond : { false, true })
    {
        SCOPED_TRACE(diamond);
        const auto rules = diamond
                               ? choice_rule("left", "m0", "m1", skip) + choice_rule("right", "m0", "m2", skip)
                                     + choice_rule("join-left", "m1", "m3", skip) + choice_rule("join-right", "m2", "m3", skip)
                                     + choice_rule("load", "m3", "m4", load) + choice_rule("move", "m4", "m5", move_to_register)
                                     + choice_rule("finish", "m5", "m0", move_to_goal)
                               : choice_rule("first", "m0", "m1", skip) + choice_rule("parallel", "m0", "m1", skip)
                                     + choice_rule("load", "m1", "m2", load) + choice_rule("move", "m2", "m3", move_to_register)
                                     + choice_rule("finish", "m3", "m4", move_to_goal);
        const auto module = ext::dl::parse_module(choice_module("ordinary", rules), task->get_domain().get_domain(), repository);
        const auto program = create_module_program(repository, module, { module });

        auto options = ext::ModuleProgramSearchOptions<Kind> {};
        const auto greedy = ext::find_solution(context, program, options);
        EXPECT_EQ(greedy.status, ext::ModuleProgramProofStatus::FAILURE);
        ASSERT_TRUE(greedy.graph);
        EXPECT_EQ(greedy.graph->get_num_vertices(), diamond ? 5 : 4);
        EXPECT_EQ(greedy.graph->get_num_edges(), diamond ? 4 : 3);
        EXPECT_EQ(greedy.open_states.size(), 1);
        EXPECT_TRUE(greedy.deadend_states.empty());
        EXPECT_FALSE(greedy.plan);

        options.universal = true;
        for (const auto classify : { false, true })
        {
            SCOPED_TRACE(classify);
            options.classifier = classify ? std::optional(classifier) : std::nullopt;
            options.shuffle_choice_points = false;
            const auto complete = ext::find_solution(context, program, options);
            EXPECT_EQ(complete.status, ext::ModuleProgramProofStatus::FAILURE);
            ASSERT_TRUE(complete.graph);
            EXPECT_EQ(complete.graph->get_num_vertices(), diamond ? 9 : 7);
            EXPECT_EQ(complete.graph->get_num_edges(), diamond ? 9 : 7);
            EXPECT_EQ(complete.graph->get_out_degree(0), 2);
            EXPECT_EQ(complete.open_states.size(), classify ? 0 : 1);
            EXPECT_EQ(complete.deadend_states.size(), classify ? 1 : 0);
            EXPECT_TRUE(complete.cycle.empty());
            EXPECT_FALSE(complete.plan);
            auto goals = 0;
            for (const auto vertex : complete.graph->get_vertex_indices())
                goals += complete.graph->get_vertex(vertex).get_property().is_goal;
            EXPECT_EQ(goals, 1);  // A failing load outcome must not prevent exploration of the other outcome.
            const auto& failures = classify ? complete.deadend_states : complete.open_states;
            ASSERT_EQ(failures.size(), 1);
            const auto& failure = complete.graph->get_vertex(failures.front()).get_property();
            EXPECT_FALSE(failure.is_goal);
            EXPECT_EQ(failure.is_alive, !classify);
            EXPECT_EQ(failure.is_unsolvable, classify);
            EXPECT_EQ(failure.execution_state.get_call_stack().get_registers().get_concept_values()[0].value().get_name(), "bad");

            options.shuffle_choice_points = true;
            options.random_seed = 42;
            const auto first = ext::find_solution(context, program, options);
            const auto second = ext::find_solution(context, program, options);
            EXPECT_EQ(first.status, complete.status);
            EXPECT_EQ(second.status, first.status);
            ASSERT_TRUE(first.graph);
            ASSERT_TRUE(second.graph);
            EXPECT_EQ(first.graph->get_num_vertices(), complete.graph->get_num_vertices());
            EXPECT_EQ(first.graph->get_num_edges(), complete.graph->get_num_edges());
            ASSERT_EQ(first.graph->get_num_vertices(), second.graph->get_num_vertices());
            ASSERT_EQ(first.graph->get_num_edges(), second.graph->get_num_edges());
            EXPECT_EQ(first.open_states, second.open_states);
            EXPECT_EQ(first.deadend_states, second.deadend_states);
            EXPECT_EQ(first.cycle, second.cycle);
            for (const auto vertex : first.graph->get_vertex_indices())
                EXPECT_TRUE(first.graph->get_vertex(vertex).get_property() == second.graph->get_vertex(vertex).get_property());
            for (const auto edge : first.graph->get_edge_indices())
            {
                EXPECT_EQ(first.graph->get_source(edge), second.graph->get_source(edge));
                EXPECT_EQ(first.graph->get_target(edge), second.graph->get_target(edge));
                EXPECT_TRUE(first.graph->get_edge(edge).get_property() == second.graph->get_edge(edge).get_property());
            }
        }
    }
}

}  // namespace

TEST(RunirTests, ExtOrdinaryGroundExecutionKeepsCompleteGraph) { check_ordinary_execution_keeps_complete_graph<tyr::GroundTag>(); }
TEST(RunirTests, ExtOrdinaryLiftedExecutionKeepsCompleteGraph) { check_ordinary_execution_keeps_complete_graph<tyr::LiftedTag>(); }

}  // namespace runir::tests
