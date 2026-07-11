#ifndef RUNIR_TESTS_UNIT_DATASETS_FIXTURES_HPP_
#define RUNIR_TESTS_UNIT_DATASETS_FIXTURES_HPP_

#include <filesystem>
#include <runir/datasets/task_class.hpp>
#include <tyr/formalism/planning/parser.hpp>
#include <tyr/planning/planning.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/execution/onetbb.hpp>
#include <yggdrasil/serialization/json_loader.hpp>

namespace runir::tests
{

inline auto gripper_benchmark_root() -> std::filesystem::path { return ygg::common::root_path() / "data/benchmarks/classical/tests/gripper"; }

inline auto make_ground_contexts(const std::filesystem::path& domain_file, const std::vector<std::filesystem::path>& task_files)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    auto parser = fp::Parser(domain_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto contexts = datasets::TaskSearchContextList<p::GroundTag> {};
    contexts.reserve(task_files.size());

    for (const auto& task_file : task_files)
    {
        const auto planning_task = parser.parse_task(task_file);
        auto lifted_task = p::Task<p::LiftedTag>(planning_task);
        auto task = lifted_task.instantiate_ground_task(*execution_context).task;
        contexts.push_back(datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context));
    }

    return contexts;
}

inline auto make_gripper_ground_context()
{
    const auto root = gripper_benchmark_root();
    auto contexts = make_ground_contexts(root / "domain.pddl", { root / "test-1.pddl" });
    return std::move(contexts.front());
}

}  // namespace runir::tests

#endif
