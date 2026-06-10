#include <filesystem>
#include <gtest/gtest.h>
#include <runir/kr/ps/ext/dl/module_factory.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <runir/kr/ps/ext/dl/structural_termination.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <set>
#include <string>
#include <tyr/formalism/planning/parser.hpp>

namespace runir::tests
{

namespace
{

std::filesystem::path runir_root() { return std::filesystem::path(RUNIR_ROOT_DIR); }

std::filesystem::path benchmark_prefix() { return runir_root() / "data" / "planning-benchmarks"; }

}  // namespace

// Fixtures use dummy feature expressions over the gripper domain; structural
// termination depends only on the rule structure over feature symbols and
// the memory structure of the module.

TEST(RunirTests, ExtStructuralTerminationEmptyModuleIsTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ext::ConstructorRepositoryFactory().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    const auto module = kr::ps::ext::dl::ModuleFactory::create_empty(*repository);

    const auto result = kr::ps::ext::dl::structural_termination(module);

    EXPECT_TRUE(result.is_terminating());
    EXPECT_EQ(result.counterexample, nullptr);
}

TEST(RunirTests, ExtStructuralTerminationDecreaseWithUnchangedReturnIsTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ext::ConstructorRepositoryFactory().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    // m0 -> m1 decreases fn; m1 -> m0 keeps fn unchanged: every memory cycle
    // strictly decreases fn, so the module terminates.
    const auto module = kr::ps::ext::dl::parse_module(R"((:module
      (:symbol term)
      (:arguments)
      (:description "")
      (:registers)
      (:entry m0)
      (:memory m0 m1)
      (:features
        (:numerical (:symbol fn) (:description "") (:expression (n_count (c_atomic_state "ball")))))
      (:rules
        (:rule (:symbol) (:description "")
          (:expression (:source-memory m0) (:target-memory m1)
            (:sketch (:conditions (:greater_zero fn)) (:effects (:decreases fn)))))
        (:rule (:symbol) (:description "")
          (:expression (:source-memory m1) (:target-memory m0)
            (:sketch (:conditions) (:effects (:unchanged fn))))))))",
                                                      planning_task.get_domain().get_domain(),
                                                      *repository);

    const auto result = kr::ps::ext::dl::structural_termination(module);

    EXPECT_TRUE(result.is_terminating());
}

TEST(RunirTests, ExtStructuralTerminationUnconstrainedReturnIsNotTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ext::ConstructorRepositoryFactory().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    // As above, but the return rule m1 -> m0 leaves fn unconstrained: the
    // memory cycle can restore fn, so termination cannot be proven.
    const auto module = kr::ps::ext::dl::parse_module(R"((:module
      (:symbol nonterm)
      (:arguments)
      (:description "")
      (:registers)
      (:entry m0)
      (:memory m0 m1)
      (:features
        (:numerical (:symbol fn) (:description "") (:expression (n_count (c_atomic_state "ball")))))
      (:rules
        (:rule (:symbol) (:description "")
          (:expression (:source-memory m0) (:target-memory m1)
            (:sketch (:conditions (:greater_zero fn)) (:effects (:decreases fn)))))
        (:rule (:symbol) (:description "")
          (:expression (:source-memory m1) (:target-memory m0)
            (:sketch (:conditions) (:effects)))))))",
                                                      planning_task.get_domain().get_domain(),
                                                      *repository);

    const auto result = kr::ps::ext::dl::structural_termination(module);

    ASSERT_FALSE(result.is_terminating());
    ASSERT_NE(result.counterexample, nullptr);

    // The counterexample cycle uses both rules and spans both memory states.
    auto rule_indices = std::set<ygg::Index<kr::ps::ext::RuleVariant>> {};
    for (const auto& edge : result.counterexample->get_edges())
        rule_indices.insert(edge.get_property().rule.get_index());
    EXPECT_EQ(rule_indices.size(), 2);

    auto memory_state_indices = std::set<ygg::Index<kr::ps::ext::MemoryState>> {};
    for (const auto& vertex : result.counterexample->get_vertices())
        memory_state_indices.insert(vertex.get_property().memory_state.get_index());
    EXPECT_EQ(memory_state_indices.size(), 2);
}

}  // namespace runir::tests
