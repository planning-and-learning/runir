#include "fixtures.hpp"
#include "planning_fixtures.hpp"

#include <gtest/gtest.h>
#include <runir/kr/domain_context.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <runir/kr/ps/ext/program_executor.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/task_context.hpp>
#include <string>
#include <yggdrasil/serialization/json.hpp>

namespace runir::tests
{

/// The policies whose features are pure Datalog (kr/ps/ext/datalog/*.program: Sokoban, Visitall and Transport) solve
/// small tasks with the expansions of the lifted SIWM planner (planner-images/siwm-lifted, without state memorization),
/// recorded in kr/ps/ext/datalog/cases.json: the goal searches of their weighted heads (tyr's Datalog evaluator) find
/// the same distances along the policy's steps.
TEST(RunirTests, DatalogPoliciesSolveTasksWithTheRecordedExpansions)
{
    const auto suite = load_fixture_json("kr/ps/ext/datalog/cases.json");
    const auto& cases = ygg::common::as_array(ygg::common::require_member(ygg::common::as_object(suite, "suite"), "cases", "suite"), "suite.cases");
    for (const auto& value : cases)
    {
        const auto& test_case = ygg::common::as_object(value, "case");
        const auto program_file = ygg::common::as_string(test_case, "program_file", "case");
        const auto directory = ygg::common::as_string(test_case, "directory", "case");
        for (const auto& task_value : ygg::common::as_array(ygg::common::require_member(test_case, "tasks", "case"), "case.tasks"))
        {
            const auto& task = ygg::common::as_object(task_value, "task");
            const auto name = ygg::common::as_string(task, "name", "task");
            auto trace = program_file;
            trace.append(" on ").append(name);
            SCOPED_TRACE(trace);
            auto domain_file = directory;
            domain_file.append("/domain.pddl");
            auto task_file = directory;
            task_file.append("/").append(name).append(".pddl");
            auto search_context = make_lifted_context(benchmark_path(domain_file), benchmark_path(task_file));
            auto task_context = kr::TaskContext<tyr::LiftedTag>::create(kr::DomainContext::create(search_context->task->get_domain()), search_context);
            const auto program = kr::ps::ext::dl::parse_program(read_fixture(program_file),
                                                                search_context->task->get_domain().get_domain(),
                                                                *task_context->domain_context->ext_repository);
            auto options = kr::ps::ext::ProgramSearchOptions<tyr::LiftedTag>();
            options.universal = false;
            options.state_memorization = kr::ps::ext::StateMemorization::NONE;
            const auto result = kr::ps::ext::find_solution(task_context, program, options);
            EXPECT_EQ(result.status, kr::ps::ProofStatus::SUCCESS);
            EXPECT_EQ(result.statistics.num_expanded, ygg::common::as_size(task, "num_expanded", "task"));
        }
    }
}

}  // namespace runir::tests
