#ifndef RUNIR_TESTS_UNIT_PLANNING_FIXTURES_HPP_
#define RUNIR_TESTS_UNIT_PLANNING_FIXTURES_HPP_

#include <filesystem>
#include <runir/datasets/task_class.hpp>
#include <vector>

namespace runir::tests
{

auto make_ground_context(const std::filesystem::path& domain_file,
                         const std::filesystem::path& task_file) -> datasets::TaskSearchContextPtr<tyr::GroundTag>;

auto make_lifted_context(const std::filesystem::path& domain_file,
                         const std::filesystem::path& task_file) -> datasets::TaskSearchContextPtr<tyr::LiftedTag>;

auto make_ground_contexts(const std::filesystem::path& domain_file,
                          const std::vector<std::filesystem::path>& task_files) -> datasets::TaskSearchContextList<tyr::GroundTag>;

auto make_gripper_ground_context() -> datasets::TaskSearchContextPtr<tyr::GroundTag>;

}  // namespace runir::tests

#endif
