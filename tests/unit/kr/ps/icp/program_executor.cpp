#include "fixtures.hpp"
#include "planning_fixtures.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <gtest/gtest.h>
#include <runir/kr/ps/icp/dl/parser.hpp>
#include <runir/kr/ps/icp/program_executor.hpp>
#include <runir/kr/ps/icp/successor_expander.hpp>
#include <runir/kr/task_context.hpp>

namespace runir::tests
{

namespace
{

namespace icp = kr::ps::icp;
using Status = icp::ProgramProofStatus;
using Outcome = icp::detail::ProgramOutcome;

std::string rule(const std::string& name, const std::string& source, const std::string& target, const std::string& body)
{
    return "(:rule (:symbol " + name + ") (:expression (:source-memory " + source + ") (:target-memory " + target + ") " + body + "))";
}

std::string load(const std::string& feature, const std::string& reg = "r0", const std::string& effects = "")
{
    return "(:load (:conditions) (:concept " + feature + ") (:register (:concept " + reg + ")) (:effects " + effects + "))";
}

std::string move(const std::string& target)
{
    return R"((:crule (:action "move") (:arguments from to) (:conditions)
        (:xconditions (belongs (:argument to) (:concept )"
           + target + R"()))
        (:xeffects) (:effects (unchanged Count))))";
}

std::string module(const std::string& rules, const std::string& resets = "")
{
    return R"((:module (:symbol policy) (:arguments) (:registers (:concept r0) (:concept r1))
        (:entry m0) (:memory m0 m1 m2 m3 m4 m5 m6)
        (:features
            (:concept (:symbol Candidates) (:expression (c_atomic_state "candidate")))
            (:concept (:symbol Good) (:expression (c_and (c_atomic_state "candidate") (c_not (c_atomic_state "bad")))))
            (:concept (:symbol Empty) (:expression (c_bot)))
            (:concept (:symbol Here) (:expression (c_atomic_state "at")))
            (:concept (:symbol Goal) (:expression (c_atomic_goal "at" true)))
            (:concept (:symbol R) (:expression (c_register r0)))
            (:concept (:symbol R1) (:expression (c_register r1)))
            (:concept (:symbol Reset) (:expression (c_and (c_register r1) (c_atomic_state "at"))))
            (:concept (:symbol Together) (:expression (c_and (c_register r0) (c_register r1))))
            (:numerical (:symbol Count) (:expression (n_count (c_atomic_state "at"))))
            (:boolean (:symbol Bad) (:expression (b_nonempty (c_and (c_register r0) (c_atomic_state "bad"))))))
        (:rules )"
           + rules + ") (:reset-spo " + resets + "))";
}

template<tyr::TaskKind Kind>
auto make_context(const std::string& fixture)
{
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../../fixtures/kr/ps/ext" / fixture;
    auto search = [&]()
    {
        if constexpr (std::same_as<Kind, tyr::GroundTag>)
            return make_ground_context(directory / "domain.pddl", directory / "task.pddl");
        else
            return make_lifted_context(directory / "domain.pddl", directory / "task.pddl");
    }();
    return kr::TaskContext<Kind>::create(kr::DomainContext::create(search->task->get_domain()), search);
}

template<tyr::TaskKind Kind>
auto program(const kr::TaskContextPtr<Kind>& context, const std::string& text)
{
    return icp::dl::parse_program("(:program (:entry policy) " + text + ")",
                                  context->search_context->task->get_domain().get_domain(),
                                  *context->domain_context->icp_repository);
}

template<tyr::TaskKind Kind>
auto initial(icp::SuccessorExpander<Kind>& expander)
{
    auto& search = *expander.get_task_context()->search_context;
    return expander.initial_state(search.successor_generator->get_initial_node(*search.state_repository, *search.axiom_evaluator).get_state());
}

template<tyr::TaskKind Kind>
auto successors(icp::SuccessorExpander<Kind>& expander, icp::ProgramStateView<Kind> source)
{
    auto result = std::vector<typename icp::SuccessorExpander<Kind>::Step> {};
    auto stats = icp::ProgramSearchStatistics {};
    expander.for_each_successor(
        source,
        stats,
        [&](auto step)
        {
            result.push_back(step);
            return true;
        },
        [] { return false; });
    return result;
}

template<tyr::TaskKind Kind>
void check_executor()
{
    auto context = make_context<Kind>("choose");
    auto options = icp::ProgramSearchOptions<Kind> {};
    const auto tail = rule("move", "m1", "m2", move("R")) + rule("finish", "m2", "m3", move("Goal"));
    const auto filtered = program(context, module(rule("select", "m0", "m1", load("Candidates", "r0", "(negative Bad)")) + tail));
    for (const auto universal : { false, true })
    {
        options.universal = universal;
        const auto result = icp::find_solution(context, filtered, options);
        ASSERT_EQ(result.status, Status::SUCCESS);
        EXPECT_TRUE(result.cycle.empty());
        EXPECT_EQ(result.plan.has_value(), !universal);
        if (result.plan)
        {
            EXPECT_EQ(result.plan->get_length(), 2);
        }
    }

    // First load binding is the bad object; universal execution retains the other branch through the goal.
    const auto branching = program(context, module(rule("select", "m0", "m1", load("Candidates")) + tail));
    options.universal = false;
    const auto greedy = icp::find_solution(context, branching, options);
    EXPECT_EQ(greedy.status, Status::FAILURE);
    EXPECT_FALSE(greedy.plan);
    EXPECT_EQ(greedy.graph->get_out_degree(0), 1);
    options.universal = true;
    const auto complete = icp::find_solution(context, branching, options);
    EXPECT_EQ(complete.status, Status::FAILURE);
    EXPECT_EQ(complete.graph->get_out_degree(0), 2);
    auto goals = 0U;
    auto non_goals = 0U;
    for (const auto vertex : complete.graph->get_vertex_indices())
    {
        const auto goal = complete.graph->get_vertex(vertex).get_property().is_goal;
        goals += goal;
        non_goals += !goal;
    }

    EXPECT_EQ(goals, 1);
    EXPECT_EQ(complete.statistics.num_expanded, non_goals);

    const auto good_path = rule("move", "m0", "m1", move("Good")) + rule("finish", "m1", "m2", move("Goal"));
    const auto empty = program(context, module(rule("empty", "m0", "m1", load("Empty")) + good_path));
    options.universal = false;
    EXPECT_EQ(icp::find_solution(context, empty, options).status, Status::FAILURE);
    options.universal = true;
    const auto empty_result = icp::find_solution(context, empty, options);
    EXPECT_EQ(empty_result.status, Status::FAILURE);
    EXPECT_FALSE(empty_result.deadend_states.empty());
    EXPECT_EQ(empty_result.graph->get_num_edges(), 2);

    const auto cycle_program = program(context,
                                       module(rule("select", "m0", "m1", load("Here")) + rule("repeat", "m1", "m1", load("Here"))
                                              + rule("move", "m1", "m2", move("Good")) + rule("finish", "m2", "m3", move("Goal"))));
    const auto cycle = icp::find_solution(context, cycle_program, options);
    EXPECT_EQ(cycle.status, Status::FAILURE);
    EXPECT_FALSE(cycle.cycle.empty());
    goals = 0;
    for (const auto vertex : cycle.graph->get_vertex_indices())
        goals += cycle.graph->get_vertex(vertex).get_property().is_goal;
    EXPECT_EQ(goals, 1);

    // Parallel rules share their target configuration and its expansion, while preserving both labeled edges.
    const auto parallel =
        program(context,
                module(rule("first", "m0", "m1", move("Good")) + rule("parallel", "m0", "m1", move("Good")) + rule("finish", "m1", "m2", move("Goal"))));
    const auto joined = icp::find_solution(context, parallel, options);
    EXPECT_EQ(joined.status, Status::SUCCESS);
    EXPECT_EQ(joined.graph->get_num_vertices(), 3);
    EXPECT_EQ(joined.graph->get_num_edges(), 3);
    EXPECT_EQ(joined.statistics.num_expanded, 2);

    const auto unset = program(context, module(rule("unset", "m0", "m1", R"((:crule (:action "move") (:arguments from to)
        (:conditions) (:xconditions (not-belongs (:register (:concept r0)) (:concept Empty))) (:xeffects)))")));
    const auto unset_result = icp::find_solution(context, unset, options);
    EXPECT_EQ(unset_result.status, Status::FAILURE);
    EXPECT_EQ(unset_result.graph->get_num_edges(), 0);

    options.max_num_states = 0;
    EXPECT_EQ(icp::find_solution(context, filtered, options).status, Status::OUT_OF_STATES);
    options.max_num_states = 100;
    options.max_time = std::chrono::steady_clock::duration::zero();
    EXPECT_EQ(icp::find_solution(context, filtered, options).status, Status::OUT_OF_TIME);
}

template<tyr::TaskKind Kind>
void check_histories()
{
    auto context = make_context<Kind>("choose");
    const auto role_policy = program(context, R"((:module (:symbol policy) (:arguments) (:registers (:role edge))
        (:entry m0) (:memory m0 m1)
        (:features (:role (:symbol Edges) (:expression (r_atomic_state "edge")))
                   (:numerical (:symbol Bound) (:expression (n_count (r_register edge)))))
        (:rules (:rule (:symbol load-edge) (:expression (:source-memory m0) (:target-memory m1)
            (:load (:conditions) (:role Edges) (:register (:role edge)) (:effects (increases Bound))))))
        (:reset-spo)))");
    auto role_expander = icp::SuccessorExpander<Kind>(context, role_policy);
    const auto role_initial = initial(role_expander);
    const auto role_steps = successors(role_expander, role_initial);
    ASSERT_EQ(role_steps.size(), 3);
    for (const auto& step : role_steps)
    {
        EXPECT_EQ(step.status, Outcome::APPLIED);
        EXPECT_EQ(step.target.get_state(), role_initial.get_state());
        EXPECT_NE(step.target.get_registers(), role_initial.get_registers());
    }

    const auto prefix = rule("load-good", "m0", "m1", load("Good")) + rule("load-start", "m1", "m2", load("Here"));
    const auto policy = program(context, module(prefix + rule("reload-good", "m2", "m3", load("Good"))));
    auto expander = icp::SuccessorExpander<Kind>(context, policy);
    const auto source = initial(expander);
    for (const auto history : source.get_histories().get_concepts())
        EXPECT_EQ(history.get().count(), 0);
    const auto step1 = successors(expander, source);
    ASSERT_EQ(step1.size(), 1);
    ASSERT_EQ(step1.front().status, Outcome::APPLIED);
    const auto step2 = successors(expander, step1.front().target);
    ASSERT_EQ(step2.size(), 1);
    ASSERT_EQ(step2.front().status, Outcome::APPLIED);
    const auto repeated = successors(expander, step2.front().target);
    ASSERT_EQ(repeated.size(), 1);
    EXPECT_EQ(repeated.front().status, Outcome::NO_APPLICABLE_ACTION);
    EXPECT_EQ(source.get_state(), step2.front().target.get_state());

    // Equal planning state, memory, and registers with different histories must have distinct identities.
    auto cleared = step2.front().target.get_data();
    cleared.histories = source.get_histories().get_index();
    const auto unvisited = icp::get_or_create(*context->icp_execution_repository, cleared).first;
    EXPECT_NE(unvisited, step2.front().target);
    const auto allowed = successors(expander, unvisited);
    ASSERT_EQ(allowed.size(), 1);
    EXPECT_EQ(allowed.front().status, Outcome::APPLIED);

    const auto reset_policy =
        program(context, module(prefix + rule("reset", "m2", "m3", load("Here", "r1")) + rule("reload-good", "m3", "m4", load("Good")), "(:prec R R1)"));
    auto reset_expander = icp::SuccessorExpander<Kind>(context, reset_policy);
    auto current = initial(reset_expander);
    for (int i = 0; i < 4; ++i)
    {
        const auto next = successors(reset_expander, current);
        ASSERT_EQ(next.size(), 1);
        ASSERT_EQ(next.front().status, Outcome::APPLIED) << i;
        current = next.front().target;
    }

    EXPECT_EQ(current.get_memory_state().get_name(), "m4");

    // A reset concept losing its last member also clears predecessor histories.
    const auto exit_policy = program(context,
                                     module(rule("reset-start", "m0", "m1", load("Here", "r1")) + rule("load-good", "m1", "m2", load("Good"))
                                                + rule("load-start", "m2", "m3", load("Here")) + rule("reset-exit", "m3", "m4", load("Good", "r1"))
                                                + rule("reload-good", "m4", "m5", load("Good")),
                                            "(:prec R Reset)"));
    auto exit_expander = icp::SuccessorExpander<Kind>(context, exit_policy);
    current = initial(exit_expander);
    for (int i = 0; i < 5; ++i)
    {
        const auto next = successors(exit_expander, current);
        ASSERT_EQ(next.size(), 1);
        ASSERT_EQ(next.front().status, Outcome::APPLIED) << i;
        current = next.front().target;
    }

    EXPECT_EQ(current.get_memory_state().get_name(), "m5");

    // Re-entry is checked against the source history even if this very transition would reset it.
    const auto same_step = program(
        context,
        module(prefix + rule("prepare-reset", "m2", "m3", load("Good", "r1")) + rule("reenter-and-reset", "m3", "m4", load("Good")), "(:prec R Together)"));
    auto same_expander = icp::SuccessorExpander<Kind>(context, same_step);
    current = initial(same_expander);
    for (int i = 0; i < 3; ++i)
    {
        const auto next = successors(same_expander, current);
        ASSERT_EQ(next.size(), 1);
        ASSERT_EQ(next.front().status, Outcome::APPLIED) << i;
        current = next.front().target;
    }
    const auto same_result = successors(same_expander, current);
    ASSERT_EQ(same_result.size(), 1);
    EXPECT_EQ(same_result.front().status, Outcome::NO_APPLICABLE_ACTION);
}

template<tyr::TaskKind Kind>
void check_normalized_arguments()
{
    auto context = make_context<Kind>("action_existential");
    const auto make = [&](const std::string& column)
    {
        return program(context,
                       R"((:module (:symbol policy) (:arguments) (:registers) (:entry m0) (:memory m0 m1)
            (:features (:concept (:symbol Progress) (:expression (c_project )"
                           + column + R"( (q_join
                (q_atomic_state "done" ()) (q_atomic_state "supports" (X W)))))))
            (:rules )" + rule("finish", "m0", "m1", R"((:crule (:action "finish") (:arguments x) (:conditions) (:xconditions) (:xeffects)))")
                           + ") (:reset-spo))");
    };
    for (const auto universal : { false, true })
    {
        auto options = icp::ProgramSearchOptions<Kind> {};
        options.universal = universal;
        EXPECT_EQ(icp::find_solution(context, make("W"), options).status, Status::FAILURE);
        const auto progress = icp::find_solution(context, make("X"), options);
        EXPECT_EQ(progress.status, Status::SUCCESS);
        if (!universal)
        {
            ASSERT_TRUE(progress.plan);
            EXPECT_EQ(progress.plan->get_length(), 1);
        }
    }
}

template<tyr::TaskKind Kind>
void check_rule_evaluator_scheduling()
{
    auto context = make_context<Kind>("choose");
    const auto policy = program(context, module(rule("first", "m0", "m1", move("Candidates")) + rule("second", "m0", "m2", move("Candidates"))));
    auto expander = icp::SuccessorExpander<Kind>(context, policy);
    const auto source = initial(expander);
    const auto natural = successors(expander, source);
    ASSERT_EQ(natural.size(), 4);
    EXPECT_EQ(natural[0].rule, natural[1].rule);
    EXPECT_EQ(natural[2].rule, natural[3].rule);
    EXPECT_NE(natural[0].rule, natural[2].rule);

    auto history_buffer = [&]
    {
        auto histories = icp::checkout<icp::Histories>(context->icp_execution_builder);
        return histories->concepts.data();
    };
    const auto* reused_history_buffer = history_buffer();
    ASSERT_NE(reused_history_buffer, nullptr);

    {
        auto data = ygg::Data<icp::Histories>();
        data.concepts = source.get_histories().get_data().concepts;
        ASSERT_FALSE(data.concepts.empty());
        data.concepts.push_back(data.concepts.front());
        const auto* buffer = data.concepts.data();
        const auto expected_size = data.concepts.size();
        const auto [stored, created] = ygg::formalism::get_or_create(*context->icp_execution_repository, data);
        EXPECT_TRUE(created);
        EXPECT_EQ(stored.get_data().concepts, data.concepts);
        EXPECT_EQ(data.concepts.size(), expected_size);
        EXPECT_EQ(data.concepts.data(), buffer);
        EXPECT_FALSE(icp::get_or_create(*context->icp_execution_repository, data).second);
    }

    // Repeated grouped expansions retain action-group and history scratch, while the
    // public order remains binding-major and each rule still emits its own edge.
    for (int repetition = 0; repetition < 3; ++repetition)
    {
        auto grouped = std::vector<typename icp::SuccessorExpander<Kind>::Step> {};
        auto statistics = icp::ProgramSearchStatistics {};
        EXPECT_TRUE(expander.for_each_successor(
            source,
            statistics,
            [&](auto step)
            {
                grouped.push_back(std::move(step));
                return true;
            },
            [] { return false; },
            true));
        ASSERT_EQ(grouped.size(), 4);
        EXPECT_EQ(statistics.num_generated, 4);
        EXPECT_EQ(grouped[0].rule, natural[0].rule);
        EXPECT_EQ(grouped[1].rule, natural[2].rule);
        EXPECT_EQ(grouped[2].rule, natural[0].rule);
        EXPECT_EQ(grouped[3].rule, natural[2].rule);
        EXPECT_EQ(grouped[0].target.get_state(), grouped[1].target.get_state());
        EXPECT_EQ(grouped[0].target.get_histories(), grouped[1].target.get_histories());
        EXPECT_EQ(grouped[2].target.get_state(), grouped[3].target.get_state());
        EXPECT_EQ(grouped[2].target.get_histories(), grouped[3].target.get_histories());
        EXPECT_EQ(history_buffer(), reused_history_buffer);

        auto accepted = 0;
        statistics = {};
        EXPECT_FALSE(expander.for_each_successor(
            source,
            statistics,
            [&](auto)
            {
                ++accepted;
                return false;
            },
            [] { return false; },
            false));
        EXPECT_EQ(accepted, 1);
        EXPECT_EQ(statistics.num_generated, 1);
    }
}

}  // namespace

TEST(RunirTests, IcpGroundExecution)
{
    check_executor<tyr::GroundTag>();
    check_histories<tyr::GroundTag>();
    check_normalized_arguments<tyr::GroundTag>();
    check_rule_evaluator_scheduling<tyr::GroundTag>();
}

TEST(RunirTests, IcpLiftedExecution)
{
    check_executor<tyr::LiftedTag>();
    check_histories<tyr::LiftedTag>();
    check_normalized_arguments<tyr::LiftedTag>();
    check_rule_evaluator_scheduling<tyr::LiftedTag>();
}

}  // namespace runir::tests
