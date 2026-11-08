#ifndef RUNIR_TESTS_UNIT_PLANNING_FIXTURES_HPP_
#define RUNIR_TESTS_UNIT_PLANNING_FIXTURES_HPP_

#include <filesystem>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <runir/datasets/task_class.hpp>
#include <vector>

namespace runir::tests
{

auto make_ground_context(const std::filesystem::path& domain_file, const std::filesystem::path& task_file) -> datasets::TaskSearchContextPtr<tyr::GroundTag>;

auto make_lifted_context(const std::filesystem::path& domain_file, const std::filesystem::path& task_file) -> datasets::TaskSearchContextPtr<tyr::LiftedTag>;

auto make_ground_contexts(const std::filesystem::path& domain_file, const urd::Store<urd::sequence<std::filesystem::path>>& task_files)
    -> datasets::TaskSearchContextList<tyr::GroundTag>;

auto make_gripper_ground_context() -> datasets::TaskSearchContextPtr<tyr::GroundTag>;

/// A row's objects (an action label's), read as object columns and resolved through the row's context.
template<typename R>
auto objects_of(const R& row)
{ return urd::resolve(row.template get<urd::index<tyr::formalism::Object>>(), row.context()); }

}  // namespace runir::tests

#endif
