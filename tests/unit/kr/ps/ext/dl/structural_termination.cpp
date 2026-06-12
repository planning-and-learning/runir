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
    (:arguments
    )
    (:description "")
    (:registers
    )
    (:entry m0)
    (:memory
        m0
        m1
    )
    (:features
        (:numerical
            (:symbol fn)
            (:description "")
            (:expression
                (n_count
                    (c_atomic_state
                        "ball"))
            )
        )
    )
    (:rules
        (:rule
            (:symbol auto1)
            (:description "")
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:sketch
                    (:symbol auto2)
                    (:description "")
                    (:expression
                        (:conditions
                            (:greater_zero fn)
                        )
                        (:effects
                            (:decreases fn)
                        )
                    )
                )
            )
        )
        (:rule
            (:symbol auto3)
            (:description "")
            (:expression
                (:source-memory m1)
                (:target-memory m0)
                (:sketch
                    (:symbol auto4)
                    (:description "")
                    (:expression
                        (:conditions)
                        (:effects
                            (:unchanged fn)
                        )
                    )
                )
            )
        )
    )
)
)",
                                                      planning_task.get_domain().get_domain(),
                                                      *repository);

    const auto result = kr::ps::ext::dl::structural_termination(module);

    EXPECT_TRUE(result.is_terminating());
}

TEST(RunirTests, ExtStructuralTerminationUsesDoRuleEffects)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ext::ConstructorRepositoryFactory().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    // The do edge declares fn unchanged, and the return edge decreases fn.
    // This is structurally terminating even though a do edge is in the memory cycle.
    const auto module = kr::ps::ext::dl::parse_module(R"((:module
    (:symbol do_effects)
    (:arguments
    )
    (:description "")
    (:registers
    )
    (:entry m0)
    (:memory
        m0
        m1
    )
    (:features
        (:numerical
            (:symbol fn)
            (:description "")
            (:expression
                (n_count
                    (c_atomic_state
                        "ball"))
            )
        )
    )
    (:rules
        (:rule
            (:symbol auto5)
            (:description "")
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:do
                    (:symbol auto6)
                    (:description "")
                    (:expression
                        (:conditions
                            (:greater_zero fn)
                        )
                        (:action "pick")
                        (:arguments
                            (c_top)
                            (c_top)
                            (c_top)
                        )
                        (:effects
                            (:unchanged fn)
                        )
                    )
                )
            )
        )
        (:rule
            (:symbol auto7)
            (:description "")
            (:expression
                (:source-memory m1)
                (:target-memory m0)
                (:sketch
                    (:symbol auto8)
                    (:description "")
                    (:expression
                        (:conditions)
                        (:effects
                            (:decreases fn)
                        )
                    )
                )
            )
        )
    )
)
)",
                                                      planning_task.get_domain().get_domain(),
                                                      *repository);

    const auto result = kr::ps::ext::dl::structural_termination(module);

    EXPECT_TRUE(result.is_terminating());
}

TEST(RunirTests, ExtStructuralTerminationLoadPreservesRegisterIndependentFeature)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ext::ConstructorRepositoryFactory().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    // Loading r0 does not change the planning state and fn does not mention r0.
    // The load edge should therefore preserve fn, allowing the following
    // decreasing sketch edge to prove the memory cycle terminating.
    const auto module = kr::ps::ext::dl::parse_module(R"((:module
    (:symbol load_independent)
    (:arguments
    )
    (:description "")
    (:registers
        (:concept (:symbol 0))
    )
    (:entry m0)
    (:memory
        m0
        m1
    )
    (:features
        (:numerical
            (:symbol fn)
            (:description "")
            (:expression
                (n_count
                    (c_atomic_state
                        "ball"))
            )
        )
    )
    (:rules
        (:rule
            (:symbol auto9)
            (:description "")
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:load
                    (:symbol auto10)
                    (:description "")
                    (:expression
                        (:conditions
                            (:greater_zero fn)
                        )
                        (:concept
                            (c_atomic_state
                                "ball")
                        )
                        (:register 0)
                    )
                )
            )
        )
        (:rule
            (:symbol auto11)
            (:description "")
            (:expression
                (:source-memory m1)
                (:target-memory m0)
                (:sketch
                    (:symbol auto12)
                    (:description "")
                    (:expression
                        (:conditions)
                        (:effects
                            (:decreases fn)
                        )
                    )
                )
            )
        )
    )
)
)",
                                                      planning_task.get_domain().get_domain(),
                                                      *repository);

    const auto result = kr::ps::ext::dl::structural_termination(module);

    EXPECT_TRUE(result.is_terminating());
}

TEST(RunirTests, ExtStructuralTerminationLoadUnconstrainsRegisterDependentFeature)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ext::ConstructorRepositoryFactory().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    // Here fn is the cardinality of the concept stored in r0. The load edge
    // writes r0, so fn is unconstrained and can restore the ranking after the
    // decreasing edge; SIEVE must leave a counterexample cycle.
    const auto module = kr::ps::ext::dl::parse_module(R"((:module
    (:symbol load_dependent)
    (:arguments
    )
    (:description "")
    (:registers
        (:concept (:symbol 0))
    )
    (:entry m0)
    (:memory
        m0
        m1
    )
    (:features
        (:numerical
            (:symbol fn)
            (:description "")
            (:expression
                (n_count
                    (c_register
                        0))
            )
        )
    )
    (:rules
        (:rule
            (:symbol auto13)
            (:description "")
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:load
                    (:symbol auto14)
                    (:description "")
                    (:expression
                        (:conditions)
                        (:concept
                            (c_atomic_state
                                "ball")
                        )
                        (:register 0)
                    )
                )
            )
        )
        (:rule
            (:symbol auto15)
            (:description "")
            (:expression
                (:source-memory m1)
                (:target-memory m0)
                (:sketch
                    (:symbol auto16)
                    (:description "")
                    (:expression
                        (:conditions
                            (:greater_zero fn)
                        )
                        (:effects
                            (:decreases fn)
                        )
                    )
                )
            )
        )
    )
)
)",
                                                      planning_task.get_domain().get_domain(),
                                                      *repository);

    const auto result = kr::ps::ext::dl::structural_termination(module);

    ASSERT_FALSE(result.is_terminating());
    ASSERT_NE(result.counterexample, nullptr);
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
    (:arguments
    )
    (:description "")
    (:registers
    )
    (:entry m0)
    (:memory
        m0
        m1
    )
    (:features
        (:numerical
            (:symbol fn)
            (:description "")
            (:expression
                (n_count
                    (c_atomic_state
                        "ball"))
            )
        )
    )
    (:rules
        (:rule
            (:symbol auto17)
            (:description "")
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:sketch
                    (:symbol auto18)
                    (:description "")
                    (:expression
                        (:conditions
                            (:greater_zero fn)
                        )
                        (:effects
                            (:decreases fn)
                        )
                    )
                )
            )
        )
        (:rule
            (:symbol auto19)
            (:description "")
            (:expression
                (:source-memory m1)
                (:target-memory m0)
                (:sketch
                    (:symbol auto20)
                    (:description "")
                    (:expression
                        (:conditions)
                        (:effects)
                    )
                )
            )
        )
    )
)
)",
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

TEST(RunirTests, ExtStructuralTerminationAcyclicModuleProgramCallsAreTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ext::ConstructorRepositoryFactory().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    const auto program = kr::ps::ext::dl::parse_module_program(R"((:program
    (:entry "root")
    (:module
        (:symbol root)
        (:arguments
        )
        (:description "")
        (:registers
        )
        (:entry m0)
        (:memory
            m0
            m1
        )
        (:features
        )
        (:rules
            (:rule
                (:symbol auto21)
                (:description "")
                (:expression
                    (:source-memory m0)
                    (:target-memory m1)
                    (:call
                        (:conditions)
                        (:callee "leaf")
                        (:arguments)
                    )
                )
            )
        )
    )
    (:module
        (:symbol leaf)
        (:arguments
        )
        (:description "")
        (:registers
        )
        (:entry m0)
        (:memory
            m0
        )
        (:features
        )
        (:rules
        )
    )
)
)",
                                                               planning_task.get_domain().get_domain(),
                                                               *repository);

    const auto result = kr::ps::ext::dl::structural_termination(program);

    EXPECT_TRUE(result.is_terminating());
    EXPECT_EQ(result.module_results.size(), 2);
    EXPECT_TRUE(result.recursive_call_rules.empty());
}

TEST(RunirTests, ExtStructuralTerminationRecursiveModuleProgramCallsAreNotTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ext::ConstructorRepositoryFactory().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    // Each module is locally acyclic, so per-module structural termination is
    // insufficient. The program-level call graph root -> leaf -> root is a
    // recursive module cycle and is conservatively rejected.
    const auto program = kr::ps::ext::dl::parse_module_program(R"((:program
    (:entry "root")
    (:module
        (:symbol root)
        (:arguments
        )
        (:description "")
        (:registers
        )
        (:entry m0)
        (:memory
            m0
            m1
        )
        (:features
        )
        (:rules
            (:rule
                (:symbol auto22)
                (:description "")
                (:expression
                    (:source-memory m0)
                    (:target-memory m1)
                    (:call
                        (:conditions)
                        (:callee "leaf")
                        (:arguments)
                    )
                )
            )
        )
    )
    (:module
        (:symbol leaf)
        (:arguments
        )
        (:description "")
        (:registers
        )
        (:entry m0)
        (:memory
            m0
            m1
        )
        (:features
        )
        (:rules
            (:rule
                (:symbol auto23)
                (:description "")
                (:expression
                    (:source-memory m0)
                    (:target-memory m1)
                    (:call
                        (:conditions)
                        (:callee "root")
                        (:arguments)
                    )
                )
            )
        )
    )
)
)",
                                                               planning_task.get_domain().get_domain(),
                                                               *repository);

    const auto result = kr::ps::ext::dl::structural_termination(program);

    ASSERT_FALSE(result.is_terminating());
    EXPECT_EQ(result.module_results.size(), 2);
    ASSERT_FALSE(result.recursive_call_rules.empty());
    EXPECT_TRUE(result.module_results[0].is_terminating());
    EXPECT_TRUE(result.module_results[1].is_terminating());
}

}  // namespace runir::tests
