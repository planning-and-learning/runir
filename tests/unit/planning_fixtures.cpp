#include "planning_fixtures.hpp"

#include "fixtures.hpp"

#include <tyr/formalism/planning/parser.hpp>
#include <tyr/planning/ground/task.hpp>
#include <tyr/planning/lifted/task.hpp>
#include <utility>
#include <yggdrasil/execution/onetbb.hpp>

namespace runir::tests
{

auto make_ground_context(const std::filesystem::path& domain_file,
                         const std::filesystem::path& task_file) -> datasets::TaskSearchContextPtr<tyr::GroundTag>
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<tyr::LiftedTag>::create(fp::Parser(domain_file).parse_task(task_file));
    auto task = lifted_task->instantiate_ground_task(*execution_context).task;
    return datasets::TaskSearchContext<tyr::GroundTag>::create(std::move(task), std::move(execution_context));
}

auto make_lifted_context(const std::filesystem::path& domain_file,
                         const std::filesystem::path& task_file) -> datasets::TaskSearchContextPtr<tyr::LiftedTag>
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    auto execution_context = ygg::ExecutionContext::create(1);
    auto task = p::Task<tyr::LiftedTag>::create(fp::Parser(domain_file).parse_task(task_file));
    return datasets::TaskSearchContext<tyr::LiftedTag>::create(std::move(task), std::move(execution_context));
}

auto make_ground_contexts(const std::filesystem::path& domain_file,
                          const std::vector<std::filesystem::path>& task_files) -> datasets::TaskSearchContextList<tyr::GroundTag>
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    auto parser = fp::Parser(domain_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto contexts = datasets::TaskSearchContextList<tyr::GroundTag> {};
    contexts.reserve(task_files.size());

    for (const auto& task_file : task_files)
    {
        const auto planning_task = parser.parse_task(task_file);
        auto lifted_task = p::Task<tyr::LiftedTag>(planning_task);
        auto task = lifted_task.instantiate_ground_task(*execution_context).task;
        contexts.push_back(datasets::TaskSearchContext<tyr::GroundTag>::create(task, execution_context));
    }

    return contexts;
}

auto make_gripper_ground_context() -> datasets::TaskSearchContextPtr<tyr::GroundTag>
{
    const auto root = benchmark_path("classical/tests/gripper");
    return make_ground_context(root / "domain.pddl", root / "test-1.pddl");
}

}  // namespace runir::tests
