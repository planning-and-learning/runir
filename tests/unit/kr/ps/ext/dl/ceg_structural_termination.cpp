#include <filesystem>
#include <gtest/gtest.h>
#include <runir/kr/ps/ext/dl/ceg_structural_termination.hpp>
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

std::filesystem::path benchmark_prefix() { return std::filesystem::path(BENCHMARKS_DIR); }

std::set<ygg::Index<kr::ps::ext::RuleVariant>> counterexample_rules(const kr::ps::ext::dl::ModuleStructuralTerminationResult& result)
{
    auto rules = std::set<ygg::Index<kr::ps::ext::RuleVariant>> {};
    for (const auto& edge : result.counterexample->get_edges())
        rules.insert(edge.get_property().rule.get_index());
    return rules;
}

std::set<ygg::Index<kr::ps::ext::MemoryState>> counterexample_memory_states(const kr::ps::ext::dl::ModuleStructuralTerminationResult& result)
{
    auto memory_states = std::set<ygg::Index<kr::ps::ext::MemoryState>> {};
    for (const auto& vertex : result.counterexample->get_vertices())
        memory_states.insert(vertex.get_property().memory_state.get_index());
    return memory_states;
}

}  // namespace

TEST(RunirTests, CegStructuralTerminationAgreesWithCompleteSieve)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);

    const auto terminating_description = std::string(R"((:module
    (:symbol term)
    (:arguments)
    (:registers)
    (:entry m0)
    (:memory m0 m1)
    (:features
        (:numerical
            (:symbol fn)
            (:expression
                (n_count
                    (c_atomic_state "ball")
                )
            )
        )
    )
    (:rules
        (:rule
            (:symbol auto1)
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:sketch
                    (:conditions
                        (greater_zero fn)
                    )
                    (:effects
                        (decreases fn)
                    )
                )
            )
        )
        (:rule
            (:symbol auto3)
            (:expression
                (:source-memory m1)
                (:target-memory m0)
                (:sketch
                    (:conditions)
                    (:effects
                        (unchanged fn)
                    )
                )
            )
        )
    )
))");
    const auto non_terminating_description = std::string(R"((:module
    (:symbol nonterm)
    (:arguments)
    (:registers)
    (:entry m0)
    (:memory m0 m1)
    (:features
        (:numerical
            (:symbol fn)
            (:expression
                (n_count
                    (c_atomic_state "ball")
                )
            )
        )
    )
    (:rules
        (:rule
            (:symbol auto5)
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:sketch
                    (:conditions
                        (greater_zero fn)
                    )
                    (:effects
                        (decreases fn)
                    )
                )
            )
        )
        (:rule
            (:symbol auto7)
            (:expression
                (:source-memory m1)
                (:target-memory m0)
                (:sketch
                    (:conditions)
                    (:effects)
                )
            )
        )
    )
))");

    {
        const auto module = kr::ps::ext::dl::ModuleFactory::create_empty(*repository);
        EXPECT_EQ(kr::ps::ext::dl::ceg_structural_termination(module).is_terminating(), kr::ps::ext::dl::structural_termination(module).is_terminating());
    }
    {
        const auto module = kr::ps::ext::dl::parse_module(terminating_description, planning_task.get_domain().get_domain(), *repository);
        EXPECT_TRUE(kr::ps::ext::dl::ceg_structural_termination(module).is_terminating());
        EXPECT_TRUE(kr::ps::ext::dl::structural_termination(module).is_terminating());
    }
    {
        const auto module = kr::ps::ext::dl::parse_module(non_terminating_description, planning_task.get_domain().get_domain(), *repository);
        const auto ceg = kr::ps::ext::dl::ceg_structural_termination(module);
        const auto complete = kr::ps::ext::dl::structural_termination(module);
        ASSERT_FALSE(ceg.is_terminating());
        ASSERT_FALSE(complete.is_terminating());
        // The single memory component uses every feature, so the
        // counterexamples coincide exactly.
        EXPECT_EQ(counterexample_rules(ceg), counterexample_rules(complete));
        EXPECT_EQ(counterexample_memory_states(ceg), counterexample_memory_states(complete));
        EXPECT_EQ(ceg.counterexample->get_num_vertices(), complete.counterexample->get_num_vertices());
        EXPECT_EQ(ceg.counterexample->get_num_edges(), complete.counterexample->get_num_edges());
    }
}

TEST(RunirTests, CegStructuralTerminationDecomposesMemoryComponents)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    // Two memory cycles bridged one-way: m0 <-> m1 terminates on fa; m2 <->
    // m3 does not terminate on fb. The bridge rule m1 -> m2 crosses memory
    // components and never participates in a cycle.
    const auto module = kr::ps::ext::dl::parse_module(R"((:module
    (:symbol multi)
    (:arguments)
    (:registers)
    (:entry m0)
    (:memory m0 m1 m2 m3)
    (:features
        (:numerical
            (:symbol fa)
            (:expression
                (n_count
                    (c_atomic_state "ball")
                )
            )
        )
        (:numerical
            (:symbol fb)
            (:expression
                (n_count
                    (c_atomic_state "room")
                )
            )
        )
    )
    (:rules
        (:rule
            (:symbol auto9)
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:sketch
                    (:conditions
                        (greater_zero fa)
                    )
                    (:effects
                        (decreases fa)
                    )
                )
            )
        )
        (:rule
            (:symbol auto11)
            (:expression
                (:source-memory m1)
                (:target-memory m0)
                (:sketch
                    (:conditions)
                    (:effects
                        (unchanged fa)
                    )
                )
            )
        )
        (:rule
            (:symbol auto13)
            (:expression
                (:source-memory m1)
                (:target-memory m2)
                (:sketch
                    (:conditions)
                    (:effects)
                )
            )
        )
        (:rule
            (:symbol auto15)
            (:expression
                (:source-memory m2)
                (:target-memory m3)
                (:sketch
                    (:conditions
                        (greater_zero fb)
                    )
                    (:effects
                        (decreases fb)
                    )
                )
            )
        )
        (:rule
            (:symbol auto17)
            (:expression
                (:source-memory m3)
                (:target-memory m2)
                (:sketch
                    (:conditions)
                    (:effects)
                )
            )
        )
    )
))",
                                                      planning_task.get_domain().get_domain(),
                                                      *repository);

    const auto ceg = kr::ps::ext::dl::ceg_structural_termination(module);
    const auto complete = kr::ps::ext::dl::structural_termination(module);

    ASSERT_FALSE(ceg.is_terminating());
    ASSERT_FALSE(complete.is_terminating());
    // Verdicts and the surviving rules/memory states agree; the ceg
    // counterexample is the canonical projection, hence at most as large.
    EXPECT_EQ(counterexample_rules(ceg), counterexample_rules(complete));
    EXPECT_EQ(counterexample_memory_states(ceg), counterexample_memory_states(complete));
    EXPECT_LE(ceg.counterexample->get_num_vertices(), complete.counterexample->get_num_vertices());
    // Only the m2 <-> m3 cycle survives.
    EXPECT_EQ(counterexample_rules(ceg).size(), 2);
    EXPECT_EQ(counterexample_memory_states(ceg).size(), 2);
}

}  // namespace runir::tests
