#include <filesystem>
#include <gtest/gtest.h>
#include <runir/kr/ps/base/dl/parser.hpp>
#include <runir/kr/ps/base/dl/sketch_factory.hpp>
#include <runir/kr/ps/base/dl/structural_termination.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <set>
#include <string>
#include <tyr/formalism/planning/parser.hpp>

namespace runir::tests
{

namespace
{

std::filesystem::path runir_root() { return std::filesystem::path(RUNIR_ROOT_DIR); }

std::filesystem::path benchmark_prefix() { return runir_root() / "data" / "planning-benchmarks"; }

std::filesystem::path gripper_domain() { return benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl"; }

std::set<ygg::Index<kr::ps::base::Rule>> counterexample_rules(const kr::ps::base::dl::StructuralTerminationResult& result)
{
    auto rules = std::set<ygg::Index<kr::ps::base::Rule>> {};
    for (const auto& edge : result.counterexample->get_edges())
        rules.insert(edge.get_property().rule.get_index());
    return rules;
}

std::set<ygg::Index<kr::ps::base::Rule>> sketch_rules(kr::ps::base::SketchView sketch)
{
    auto rules = std::set<ygg::Index<kr::ps::base::Rule>> {};
    for (auto rule : sketch.get_rules())
        rules.insert(rule.get_index());
    return rules;
}

}  // namespace

// The fixtures use dummy feature expressions over the gripper domain;
// structural termination depends only on the rule structure over the feature
// symbols, not on the feature denotations. Paper rules from
// incomplete_sieve.pdf use the convention that unmentioned features stay
// unchanged and "f?" marks an unconstrained feature; the runir sketch
// language encodes "f?" by omitting f and requires explicit (:unchanged f).

TEST(RunirTests, StructuralTerminationEmptySketchIsTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(gripper_domain()).get_domain();
    auto dl_repository = kr::dl::base::ConstructorRepositoryFactory().create(planning_domain.get_repository());
    auto repository = kr::ps::base::RepositoryFactory().create(dl_repository);
    const auto sketch = kr::ps::base::dl::SketchFactory::create_empty(*repository);

    const auto result = kr::ps::base::dl::structural_termination(sketch);

    EXPECT_TRUE(result.is_terminating());
    EXPECT_EQ(result.counterexample, nullptr);
}

TEST(RunirTests, StructuralTerminationBooleanOscillatorIsNotTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(gripper_domain()).get_domain();
    auto dl_repository = kr::dl::base::ConstructorRepositoryFactory().create(planning_domain.get_repository());
    auto repository = kr::ps::base::RepositoryFactory().create(dl_repository);
    // {not b1} -> {b1} and {b1} -> {not b1}: an unbreakable two-cycle.
    const auto sketch = kr::ps::base::dl::parse_sketch(R"((:sketch
    (:features
        (:boolean
            (:symbol b1)
            (:description "")
            (:expression
                (b_nonempty
                    (c_atomic_state
                        "at-robby"))
            )
        )
    )
    (:rules
        (:rule
            (:symbol auto1)
            (:description "")
            (:expression
                (:conditions
                    (:negative b1)
                )
                (:effects
                    (:positive b1)
                )
            )
        )
        (:rule
            (:symbol auto2)
            (:description "")
            (:expression
                (:conditions
                    (:positive b1)
                )
                (:effects
                    (:negative b1)
                )
            )
        )
    )
)
)",
                                                       planning_domain.get_domain(),
                                                       *repository);

    const auto result = kr::ps::base::dl::structural_termination(sketch);

    ASSERT_FALSE(result.is_terminating());
    ASSERT_NE(result.counterexample, nullptr);
    EXPECT_EQ(result.counterexample->get_num_vertices(), 2);
    EXPECT_EQ(result.counterexample->get_num_edges(), 2);
    EXPECT_EQ(counterexample_rules(result), sketch_rules(sketch));
}

TEST(RunirTests, StructuralTerminationNumericalIncreaseDecreasePairIsNotTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(gripper_domain()).get_domain();
    auto dl_repository = kr::dl::base::ConstructorRepositoryFactory().create(planning_domain.get_repository());
    auto repository = kr::ps::base::RepositoryFactory().create(dl_repository);
    // {n1 > 0} -> {n1 dec} and {} -> {n1 inc}: the increase regenerates n1.
    const auto sketch = kr::ps::base::dl::parse_sketch(R"((:sketch
    (:features
        (:numerical
            (:symbol n1)
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
            (:symbol auto3)
            (:description "")
            (:expression
                (:conditions
                    (:greater_zero n1)
                )
                (:effects
                    (:decreases n1)
                )
            )
        )
        (:rule
            (:symbol auto4)
            (:description "")
            (:expression
                (:conditions)
                (:effects
                    (:increases n1)
                )
            )
        )
    )
)
)",
                                                       planning_domain.get_domain(),
                                                       *repository);

    const auto result = kr::ps::base::dl::structural_termination(sketch);

    ASSERT_FALSE(result.is_terminating());
    ASSERT_NE(result.counterexample, nullptr);
    EXPECT_GE(result.counterexample->get_num_edges(), 2);
    EXPECT_EQ(counterexample_rules(result), sketch_rules(sketch));
}

TEST(RunirTests, StructuralTerminationFloortileSketchIsTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(gripper_domain()).get_domain();
    auto dl_repository = kr::dl::base::ConstructorRepositoryFactory().create(planning_domain.get_repository());
    auto repository = kr::ps::base::RepositoryFactory().create(dl_repository);
    // incomplete_sieve.pdf Section 5.2, Theorem 5: r = {v, g > 0} -> {g dec}.
    const auto sketch = kr::ps::base::dl::parse_sketch(R"((:sketch
    (:features
        (:boolean
            (:symbol v)
            (:description "")
            (:expression
                (b_nonempty
                    (c_atomic_state
                        "at-robby"))
            )
        )
        (:numerical
            (:symbol g)
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
                (:conditions
                    (:positive v)
                    (:greater_zero g)
                )
                (:effects
                    (:decreases g)
                    (:unchanged v)
                )
            )
        )
    )
)
)",
                                                       planning_domain.get_domain(),
                                                       *repository);

    const auto result = kr::ps::base::dl::structural_termination(sketch);

    EXPECT_TRUE(result.is_terminating());
}

TEST(RunirTests, StructuralTerminationTppSketchIsTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(gripper_domain()).get_domain();
    auto dl_repository = kr::dl::base::ConstructorRepositoryFactory().create(planning_domain.get_repository());
    auto repository = kr::ps::base::RepositoryFactory().create(dl_repository);
    // incomplete_sieve.pdf Section 5.3, Theorem 6:
    //   r1 = {b > 0} -> {b dec}, r2 = {l > 0} -> {b?, l dec},
    //   r3 = {n > 0} -> {b?, l?, n dec}.
    const auto sketch = kr::ps::base::dl::parse_sketch(R"((:sketch
    (:features
        (:numerical
            (:symbol fb)
            (:description "")
            (:expression
                (n_count
                    (c_atomic_state
                        "ball"))
            )
        )
        (:numerical
            (:symbol fl)
            (:description "")
            (:expression
                (n_count
                    (c_atomic_state
                        "room"))
            )
        )
        (:numerical
            (:symbol fn)
            (:description "")
            (:expression
                (n_count
                    (c_atomic_state
                        "gripper"))
            )
        )
    )
    (:rules
        (:rule
            (:symbol auto6)
            (:description "")
            (:expression
                (:conditions
                    (:greater_zero fb)
                )
                (:effects
                    (:decreases fb)
                    (:unchanged fl)
                    (:unchanged fn)
                )
            )
        )
        (:rule
            (:symbol auto7)
            (:description "")
            (:expression
                (:conditions
                    (:greater_zero fl)
                )
                (:effects
                    (:unchanged fn)
                    (:decreases fl)
                )
            )
        )
        (:rule
            (:symbol auto8)
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
)",
                                                       planning_domain.get_domain(),
                                                       *repository);

    const auto result = kr::ps::base::dl::structural_termination(sketch);

    EXPECT_TRUE(result.is_terminating());
}

TEST(RunirTests, StructuralTerminationBarmanSketchIsTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(gripper_domain()).get_domain();
    auto dl_repository = kr::dl::base::ConstructorRepositoryFactory().create(planning_domain.get_repository());
    auto repository = kr::ps::base::RepositoryFactory().create(dl_repository);
    // incomplete_sieve.pdf Section 5.4, Theorem 7:
    //   r1 = {not c1} -> {u?, c1}, r2 = {c1, not c2} -> {u?, c2},
    //   r3 = {u > 0} -> {u dec}, r4 = {g > 0} -> {g dec, c1?, c2?}.
    const auto sketch = kr::ps::base::dl::parse_sketch(R"((:sketch
    (:features
        (:boolean
            (:symbol c1)
            (:description "")
            (:expression
                (b_nonempty
                    (c_atomic_state
                        "at-robby"))
            )
        )
        (:boolean
            (:symbol c2)
            (:description "")
            (:expression
                (b_nonempty
                    (c_atomic_state
                        "free"))
            )
        )
        (:numerical
            (:symbol g)
            (:description "")
            (:expression
                (n_count
                    (c_atomic_state
                        "ball"))
            )
        )
        (:numerical
            (:symbol u)
            (:description "")
            (:expression
                (n_count
                    (c_atomic_state
                        "room"))
            )
        )
    )
    (:rules
        (:rule
            (:symbol auto9)
            (:description "")
            (:expression
                (:conditions
                    (:negative c1)
                )
                (:effects
                    (:positive c1)
                    (:unchanged c2)
                    (:unchanged g)
                )
            )
        )
        (:rule
            (:symbol auto10)
            (:description "")
            (:expression
                (:conditions
                    (:positive c1)
                    (:negative c2)
                )
                (:effects
                    (:unchanged g)
                    (:positive c2)
                    (:unchanged c1)
                )
            )
        )
        (:rule
            (:symbol auto11)
            (:description "")
            (:expression
                (:conditions
                    (:greater_zero u)
                )
                (:effects
                    (:unchanged c2)
                    (:unchanged g)
                    (:unchanged c1)
                    (:decreases u)
                )
            )
        )
        (:rule
            (:symbol auto12)
            (:description "")
            (:expression
                (:conditions
                    (:greater_zero g)
                )
                (:effects
                    (:decreases g)
                    (:unchanged u)
                )
            )
        )
    )
)
)",
                                                       planning_domain.get_domain(),
                                                       *repository);

    const auto result = kr::ps::base::dl::structural_termination(sketch);

    EXPECT_TRUE(result.is_terminating());
}

TEST(RunirTests, StructuralTerminationGripperFactorySketchIsTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(gripper_domain()).get_domain();
    auto dl_repository = kr::dl::base::ConstructorRepositoryFactory().create(planning_domain.get_repository());
    auto repository = kr::ps::base::RepositoryFactory().create(dl_repository);
    const auto sketch = kr::ps::base::dl::SketchFactory::create(kr::ps::base::dl::SketchSpecification::GRIPPER_FRANCE_ET_AL_AAAI2021,
                                                                planning_domain.get_domain(),
                                                                *repository);

    const auto result = kr::ps::base::dl::structural_termination(sketch);

    EXPECT_TRUE(result.is_terminating());
}

}  // namespace runir::tests
