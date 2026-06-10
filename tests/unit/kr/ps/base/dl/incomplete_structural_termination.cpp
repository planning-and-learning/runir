#include <filesystem>
#include <gtest/gtest.h>
#include <runir/kr/ps/base/dl/incomplete_structural_termination.hpp>
#include <runir/kr/ps/base/dl/parser.hpp>
#include <runir/kr/ps/base/dl/sketch_factory.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <string>
#include <tyr/formalism/planning/parser.hpp>

namespace runir::tests
{

namespace
{

std::filesystem::path runir_root() { return std::filesystem::path(RUNIR_ROOT_DIR); }

std::filesystem::path gripper_domain()
{
    return runir_root() / "data" / "planning-benchmarks" / "tests" / "classical" / "gripper" / "domain.pddl";
}

}  // namespace

// Fixtures use dummy feature expressions over the gripper domain; the
// incomplete procedure depends only on the rule structure. See
// structural_termination.cpp for the paper-to-runir encoding convention.

TEST(RunirTests, IncompleteStructuralTerminationTppSketchIsTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(gripper_domain()).get_domain();
    auto dl_repository = kr::dl::base::ConstructorRepositoryFactory().create(planning_domain.get_repository());
    auto repository = kr::ps::base::RepositoryFactory().create(dl_repository);
    // incomplete_sieve.pdf Theorem 6 proof: eliminate r3 (mark n), then r2
    // (mark l), then r1 -- the paper proves this sketch syntactically.
    const auto sketch = kr::ps::base::dl::parse_sketch(R"((:sketch
      (:features
        (:numerical (:symbol fb) (:description "") (:expression (n_count (c_atomic_state "ball"))))
        (:numerical (:symbol fl) (:description "") (:expression (n_count (c_atomic_state "room"))))
        (:numerical (:symbol fn) (:description "") (:expression (n_count (c_atomic_state "gripper")))))
      (:rules
        (:rule (:symbol) (:description "")
          (:expression (:conditions (:greater_zero fb)) (:effects (:decreases fb) (:unchanged fl) (:unchanged fn))))
        (:rule (:symbol) (:description "")
          (:expression (:conditions (:greater_zero fl)) (:effects (:decreases fl) (:unchanged fn))))
        (:rule (:symbol) (:description "")
          (:expression (:conditions (:greater_zero fn)) (:effects (:decreases fn)))))))",
                                                       planning_domain.get_domain(),
                                                       *repository);

    const auto result = kr::ps::base::dl::incomplete_structural_termination(sketch);

    EXPECT_TRUE(result.is_terminating());
    EXPECT_TRUE(result.surviving_rules.empty());
}

TEST(RunirTests, IncompleteStructuralTerminationBarmanSketchIsTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(gripper_domain()).get_domain();
    auto dl_repository = kr::dl::base::ConstructorRepositoryFactory().create(planning_domain.get_repository());
    auto repository = kr::ps::base::RepositoryFactory().create(dl_repository);
    // incomplete_sieve.pdf Theorem 7 (Barman); provable via R1/R2 alone.
    const auto sketch = kr::ps::base::dl::parse_sketch(R"((:sketch
      (:features
        (:boolean (:symbol c1) (:description "") (:expression (b_nonempty (c_atomic_state "at-robby"))))
        (:boolean (:symbol c2) (:description "") (:expression (b_nonempty (c_atomic_state "free"))))
        (:numerical (:symbol g) (:description "") (:expression (n_count (c_atomic_state "ball"))))
        (:numerical (:symbol u) (:description "") (:expression (n_count (c_atomic_state "room")))))
      (:rules
        (:rule (:symbol) (:description "")
          (:expression (:conditions (:negative c1)) (:effects (:positive c1) (:unchanged c2) (:unchanged g))))
        (:rule (:symbol) (:description "")
          (:expression (:conditions (:positive c1) (:negative c2)) (:effects (:positive c2) (:unchanged c1) (:unchanged g))))
        (:rule (:symbol) (:description "")
          (:expression (:conditions (:greater_zero u)) (:effects (:decreases u) (:unchanged c1) (:unchanged c2) (:unchanged g))))
        (:rule (:symbol) (:description "")
          (:expression (:conditions (:greater_zero g)) (:effects (:decreases g) (:unchanged u)))))))",
                                                       planning_domain.get_domain(),
                                                       *repository);

    const auto result = kr::ps::base::dl::incomplete_structural_termination(sketch);

    EXPECT_TRUE(result.is_terminating());
}

TEST(RunirTests, IncompleteStructuralTerminationRequiresR3MarkedComplementaryConditions)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(gripper_domain()).get_domain();
    auto dl_repository = kr::dl::base::ConstructorRepositoryFactory().create(planning_domain.get_repository());
    auto repository = kr::ps::base::RepositoryFactory().create(dl_repository);
    // rC = {n > 0} -> {n dec} eliminates first and marks n. Then
    // rA = {q, n > 0} -> {not q} and rB = {not q, n = 0} -> {q} oppose each
    // other on q, but R3 discounts rB against rA: n is marked and n > 0 in
    // C(rA) is complementary to n = 0 in C(rB).
    const auto sketch = kr::ps::base::dl::parse_sketch(R"((:sketch
      (:features
        (:boolean (:symbol q) (:description "") (:expression (b_nonempty (c_atomic_state "at-robby"))))
        (:numerical (:symbol n) (:description "") (:expression (n_count (c_atomic_state "ball")))))
      (:rules
        (:rule (:symbol) (:description "")
          (:expression (:conditions (:positive q) (:greater_zero n)) (:effects (:negative q) (:unchanged n))))
        (:rule (:symbol) (:description "")
          (:expression (:conditions (:negative q) (:equal_zero n)) (:effects (:positive q) (:unchanged n))))
        (:rule (:symbol) (:description "")
          (:expression (:conditions (:greater_zero n)) (:effects (:decreases n) (:unchanged q)))))))",
                                                       planning_domain.get_domain(),
                                                       *repository);

    const auto result = kr::ps::base::dl::incomplete_structural_termination(sketch);

    EXPECT_TRUE(result.is_terminating());
}

TEST(RunirTests, IncompleteStructuralTerminationBooleanOscillatorIsUnknownWithBlockingReasons)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(gripper_domain()).get_domain();
    auto dl_repository = kr::dl::base::ConstructorRepositoryFactory().create(planning_domain.get_repository());
    auto repository = kr::ps::base::RepositoryFactory().create(dl_repository);
    const auto sketch = kr::ps::base::dl::parse_sketch(R"((:sketch
      (:features
        (:boolean (:symbol b1) (:description "") (:expression (b_nonempty (c_atomic_state "at-robby")))))
      (:rules
        (:rule (:symbol) (:description "") (:expression (:conditions (:negative b1)) (:effects (:positive b1))))
        (:rule (:symbol) (:description "") (:expression (:conditions (:positive b1)) (:effects (:negative b1)))))))",
                                                       planning_domain.get_domain(),
                                                       *repository);

    const auto result = kr::ps::base::dl::incomplete_structural_termination(sketch);

    ASSERT_FALSE(result.is_terminating());
    ASSERT_EQ(result.surviving_rules.size(), 2);
    for (const auto& surviving : result.surviving_rules)
    {
        ASSERT_EQ(surviving.blocking_reasons.size(), 1);
        const auto& reason = surviving.blocking_reasons.front();
        EXPECT_TRUE(reason.boolean_position.has_value());
        EXPECT_EQ(reason.boolean_position.value(), 0);
        // The opposing rule is exactly the other surviving rule.
        ASSERT_EQ(reason.opposing_rules.size(), 1);
        EXPECT_NE(reason.opposing_rules.front().get_index(), surviving.rule.get_index());
    }
}

TEST(RunirTests, IncompleteStructuralTerminationEmptySketchIsTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(gripper_domain()).get_domain();
    auto dl_repository = kr::dl::base::ConstructorRepositoryFactory().create(planning_domain.get_repository());
    auto repository = kr::ps::base::RepositoryFactory().create(dl_repository);
    const auto sketch = kr::ps::base::dl::SketchFactory::create_empty(*repository);

    const auto result = kr::ps::base::dl::incomplete_structural_termination(sketch);

    EXPECT_TRUE(result.is_terminating());
}

}  // namespace runir::tests
