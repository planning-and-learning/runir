#include <filesystem>
#include <gtest/gtest.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/ps/ext/dl/incomplete_structural_termination.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <string>
#include <tyr/formalism/planning/parser.hpp>
#include <variant>
#include <yggdrasil/semantics/equal_to.hpp>

namespace runir::tests
{

namespace
{

std::filesystem::path gripper_domain() { return std::filesystem::path(BENCHMARKS_DIR) / "classical" / "tests" / "gripper" / "domain.pddl"; }

}  // namespace

TEST(RunirTests, ExtIncompleteStructuralTerminationMapsNumericalOpponent)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(gripper_domain()).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    const auto module = kr::ps::ext::dl::parse_module(R"((:module
        (:symbol numeric)
        (:arguments)
        (:registers)
        (:entry m0)
        (:memory m0)
        (:features
            (:numerical
                (:symbol n)
                (:expression (n_count (c_atomic_state "ball")))
            )
        )
        (:rules
            (:rule
                (:symbol r1)
                (:expression
                    (:source-memory m0)
                    (:target-memory m0)
                    (:sketch
                        (:conditions (greater_zero n))
                        (:effects (decreases n))
                    )
                )
            )
            (:rule
                (:symbol r2)
                (:expression
                    (:source-memory m0)
                    (:target-memory m0)
                    (:sketch
                        (:conditions (equal_zero n))
                        (:effects (increases n))
                    )
                )
            )
        )
    ))",
                                                      planning_domain.get_domain(),
                                                      *repository);

    const auto result = kr::ps::ext::dl::incomplete_structural_termination(module);

    ASSERT_FALSE(result.is_terminating());
    ASSERT_EQ(result.numericals.size(), 1);
    ASSERT_EQ(result.surviving_rules.size(), 2);
    const auto& decreasing = result.surviving_rules.front();
    ASSERT_EQ(decreasing.blocking_reasons.size(), 1);
    const auto& reason = decreasing.blocking_reasons.front();
    ASSERT_TRUE(std::holds_alternative<kr::ps::ext::dl::NumericalFeatureView>(reason.feature));
    EXPECT_TRUE(
        ygg::EqualTo<kr::ps::ext::dl::NumericalFeatureView> {}(std::get<kr::ps::ext::dl::NumericalFeatureView>(reason.feature), result.numericals.front()));
    ASSERT_EQ(reason.opposing_rules.size(), 1);
    EXPECT_TRUE(ygg::EqualTo<kr::ps::ext::RuleVariantView> {}(reason.opposing_rules.front(), result.surviving_rules.back().rule));
}

TEST(RunirTests, ExtIncompleteStructuralTerminationUsesMemorySeparation)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(gripper_domain()).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    const auto module = kr::ps::ext::dl::parse_module(R"((:module
        (:symbol memory_independent)
        (:arguments)
        (:registers)
        (:entry m0)
        (:memory m0 m1)
        (:features
            (:boolean
                (:symbol b)
                (:expression (b_nonempty (c_atomic_state "ball")))
            )
        )
        (:rules
            (:rule
                (:symbol r1)
                (:expression
                    (:source-memory m0)
                    (:target-memory m0)
                    (:sketch
                        (:conditions (negative b))
                        (:effects (positive b))
                    )
                )
            )
            (:rule
                (:symbol r2)
                (:expression
                    (:source-memory m1)
                    (:target-memory m1)
                    (:sketch
                        (:conditions (positive b))
                        (:effects (negative b))
                    )
                )
            )
        )
    ))",
                                                      planning_domain.get_domain(),
                                                      *repository);

    const auto result = kr::ps::ext::dl::incomplete_structural_termination(module);

    EXPECT_TRUE(result.is_terminating());
    EXPECT_TRUE(result.surviving_rules.empty());
}

TEST(RunirTests, ExtIncompleteStructuralTerminationSeparatedRegisterLoadDoesNotBlockDecrease)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(gripper_domain()).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    const auto module = kr::ps::ext::dl::parse_module(R"((:module
        (:symbol load)
        (:arguments)
        (:registers (:concept r0))
        (:entry m0)
        (:memory m0 m1)
        (:features
            (:concept
                (:symbol B)
                (:expression (c_atomic_state "ball"))
            )
            (:numerical
                (:symbol n)
                (:expression (n_count (c_register r0)))
            )
        )
        (:rules
            (:rule
                (:symbol r1)
                (:expression
                    (:source-memory m0)
                    (:target-memory m0)
                    (:sketch
                        (:conditions (greater_zero n))
                        (:effects (decreases n))
                    )
                )
            )
            (:rule
                (:symbol r2)
                (:expression
                    (:source-memory m1)
                    (:target-memory m1)
                    (:load
                        (:conditions)
                        (:concept B)
                        (:register (:concept r0))
                    )
                )
            )
        )
    ))",
                                                      planning_domain.get_domain(),
                                                      *repository);

    const auto result = kr::ps::ext::dl::incomplete_structural_termination(module);

    ASSERT_FALSE(result.is_terminating());
    ASSERT_EQ(result.surviving_rules.size(), 1);
    EXPECT_EQ(result.surviving_rules.front().rule.get_symbol(), "r2");
    EXPECT_TRUE(result.surviving_rules.front().blocking_reasons.empty());
}

TEST(RunirTests, ExtIncompleteStructuralTerminationHasNoFeatureLimit)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(gripper_domain()).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    auto description = std::string { "(:module (:symbol many) (:arguments) (:registers) (:entry m0) (:memory m0) (:features " };
    for (auto position = 0; position < 15; ++position)
        description += "(:numerical (:symbol n" + std::to_string(position) + ") (:expression (n_count (c_atomic_state \"ball\"))))";
    description += ") (:rules))";
    const auto module = kr::ps::ext::dl::parse_module(description, planning_domain.get_domain(), *repository);

    const auto result = kr::ps::ext::dl::incomplete_structural_termination(module);

    EXPECT_TRUE(result.is_terminating());
    EXPECT_EQ(result.numericals.size(), 15);
}

TEST(RunirTests, ExtIncompleteStructuralTerminationAcyclicCallIsLocallyTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(gripper_domain()).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    const auto program = kr::ps::ext::dl::parse_module_program(R"((:program
        (:entry root)
        (:module
            (:symbol root)
            (:arguments)
            (:registers)
            (:entry m0)
            (:memory m0 m1)
            (:features)
            (:rules
                (:rule
                    (:symbol r1)
                    (:expression
                        (:source-memory m0)
                        (:target-memory m1)
                        (:call
                            (:conditions)
                            (:callee leaf)
                            (:arguments)
                        )
                    )
                )
            )
        )
        (:module
            (:symbol leaf)
            (:arguments)
            (:registers)
            (:entry m0)
            (:memory m0)
            (:features)
            (:rules)
        )
    ))",
                                                               planning_domain.get_domain(),
                                                               *repository);

    const auto result = kr::ps::ext::dl::incomplete_structural_termination(program);

    ASSERT_TRUE(result.is_terminating());
    ASSERT_EQ(result.module_results.size(), 2);
    for (const auto& module_result : result.module_results)
        EXPECT_TRUE(module_result.is_terminating());
    EXPECT_TRUE(result.recursive_call_rules.empty());
}

TEST(RunirTests, ExtIncompleteStructuralTerminationReportsRecursiveCalls)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(gripper_domain()).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    const auto program = kr::ps::ext::dl::parse_module_program(R"((:program
        (:entry root)
        (:module
            (:symbol root)
            (:arguments)
            (:registers)
            (:entry m0)
            (:memory m0 m1)
            (:features)
            (:rules
                (:rule
                    (:symbol r1)
                    (:expression
                        (:source-memory m0)
                        (:target-memory m1)
                        (:call (:conditions) (:callee leaf) (:arguments))
                    )
                )
            )
        )
        (:module
            (:symbol leaf)
            (:arguments)
            (:registers)
            (:entry m0)
            (:memory m0 m1)
            (:features)
            (:rules
                (:rule
                    (:symbol r2)
                    (:expression
                        (:source-memory m0)
                        (:target-memory m1)
                        (:call (:conditions) (:callee root) (:arguments))
                    )
                )
            )
        )
    ))",
                                                               planning_domain.get_domain(),
                                                               *repository);

    const auto result = kr::ps::ext::dl::incomplete_structural_termination(program);

    EXPECT_FALSE(result.is_terminating());
    EXPECT_EQ(result.module_results.size(), 2);
    for (const auto& module_result : result.module_results)
        EXPECT_TRUE(module_result.is_terminating());
    EXPECT_EQ(result.recursive_call_rules.size(), 2);
}
TEST(RunirTests, ExtIncompleteStructuralTerminationAcyclicEmptyProgramIsTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(gripper_domain()).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    const auto program = kr::ps::ext::dl::parse_module_program(R"((:program
        (:entry root)
        (:module
            (:symbol root)
            (:arguments)
            (:registers)
            (:entry m0)
            (:memory m0)
            (:features)
            (:rules)
        )
        (:module
            (:symbol leaf)
            (:arguments)
            (:registers)
            (:entry m0)
            (:memory m0)
            (:features)
            (:rules)
        )
    ))",
                                                               planning_domain.get_domain(),
                                                               *repository);

    const auto result = kr::ps::ext::dl::incomplete_structural_termination(program);

    EXPECT_TRUE(result.is_terminating());
    EXPECT_EQ(result.module_results.size(), 2);
    EXPECT_TRUE(result.recursive_call_rules.empty());
}

}  // namespace runir::tests
