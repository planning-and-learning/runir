#include "fixtures.hpp"

#include <gtest/gtest.h>
#include <runir/graphs/cycle.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/ps/base/dl/parser.hpp>
#include <runir/kr/ps/base/dl/sketch_factory.hpp>
#include <runir/kr/ps/base/dl/structural_termination.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <set>
#include <tyr/formalism/planning/parser.hpp>
#include <yggdrasil/semantics/comparison.hpp>
#include <yggdrasil/serialization/json.hpp>

namespace runir::tests
{

namespace
{

auto counterexample_rules(const kr::ps::base::dl::SieveStructuralTerminationResult& sieve) -> std::set<ygg::Index<kr::ps::Rule<kr::BaseFamilyTag>>>
{
    auto rules = std::set<ygg::Index<kr::ps::Rule<kr::BaseFamilyTag>>> {};
    for (const auto& edge : sieve.counterexample->get_edges())
        rules.insert(edge.get_property().get_index());
    auto surviving = std::set<ygg::Index<kr::ps::Rule<kr::BaseFamilyTag>>> {};
    for (const auto rule : sieve.surviving_rules)
        surviving.insert(rule.get_index());
    EXPECT_EQ(surviving.size(), sieve.surviving_rules.size());
    EXPECT_EQ(surviving, rules);
    return rules;
}

auto sketch_rules(kr::ps::base::SketchView sketch) -> std::set<ygg::Index<kr::ps::Rule<kr::BaseFamilyTag>>>
{
    auto rules = std::set<ygg::Index<kr::ps::Rule<kr::BaseFamilyTag>>> {};
    for (auto rule : sketch.get_rules())
        rules.insert(rule.get_index());
    return rules;
}

}  // namespace

TEST(RunirTests, BaseStructuralTerminationFixturesMatch)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(benchmark_path("classical/tests/gripper/domain.pddl")).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::base::RepositoryFactory().create(dl_repository);
    const auto suite = load_fixture_json("kr/ps/structural_termination.json");
    const auto& cases = ygg::common::as_array(ygg::common::require_member(ygg::common::as_object(suite, "suite"), "base", "suite"), "suite.base");

    for (const auto& value : cases)
    {
        const auto& test_case = ygg::common::as_object(value, "case");
        const auto sketch =
            kr::ps::base::dl::parse_sketch(read_fixture(ygg::common::as_string(test_case, "file", "case")), planning_domain.get_domain(), *repository);
        EXPECT_EQ(kr::ps::base::dl::structural_termination(sketch).is_terminating(), ygg::common::as_bool(test_case, "terminating", "case"))
            << ygg::common::as_string(test_case, "name", "case");
    }
}

TEST(RunirTests, StructuralTerminationEmptySketchIsTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(benchmark_path("classical/tests/gripper/domain.pddl")).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::base::RepositoryFactory().create(dl_repository);
    const auto sketch = kr::ps::base::dl::SketchFactory::create_empty(*repository);

    const auto result = kr::ps::base::dl::structural_termination(sketch);
    const auto without_incomplete = kr::ps::base::dl::structural_termination(sketch, kr::ps::dl::default_max_features, false);
    EXPECT_TRUE(result.is_terminating());
    ASSERT_TRUE(result.incomplete_result.has_value());
    EXPECT_EQ(result.incomplete_result->status, kr::ps::base::dl::IncompleteStructuralTerminationStatus::TERMINATING);
    EXPECT_FALSE(result.sieve_result.has_value());
    EXPECT_TRUE(without_incomplete.is_terminating());
    EXPECT_FALSE(without_incomplete.incomplete_result.has_value());
    ASSERT_TRUE(without_incomplete.sieve_result.has_value());
    EXPECT_EQ(without_incomplete.sieve_result->counterexample, nullptr);
    EXPECT_TRUE(without_incomplete.sieve_result->scc_results.empty());
    EXPECT_TRUE(without_incomplete.sieve_result->surviving_rules.empty());
}

TEST(RunirTests, StructuralTerminationBooleanOscillatorCounterexample)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(benchmark_path("classical/tests/gripper/domain.pddl")).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::base::RepositoryFactory().create(dl_repository);
    const auto sketch = kr::ps::base::dl::parse_sketch(read_fixture("kr/ps/base/dl/oscillator.sketch"), planning_domain.get_domain(), *repository);

    const auto result = kr::ps::base::dl::structural_termination(sketch);
    const auto booleans = sketch.get_features<kr::ps::dl::BooleanFeature>();
    ASSERT_FALSE(result.is_terminating());
    ASSERT_TRUE(result.incomplete_result.has_value());
    EXPECT_EQ(result.incomplete_result->status, kr::ps::base::dl::IncompleteStructuralTerminationStatus::UNKNOWN);
    ASSERT_TRUE(result.sieve_result.has_value());
    ASSERT_NE(result.sieve_result->counterexample, nullptr);
    EXPECT_EQ(result.sieve_result->counterexample->get_num_vertices(), 2);
    EXPECT_EQ(result.sieve_result->counterexample->get_num_edges(), 2);
    EXPECT_EQ(counterexample_rules(*result.sieve_result), sketch_rules(sketch));
    ASSERT_EQ(result.sieve_result->scc_results.size(), 1);
    ASSERT_EQ(result.sieve_result->scc_results.front().booleans.size(), 1);
    EXPECT_EQ(result.sieve_result->scc_results.front().booleans.front(), booleans.front());
    EXPECT_TRUE(result.sieve_result->scc_results.front().numericals.empty());
}

TEST(RunirTests, StructuralTerminationNumericalCycleCounterexample)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(benchmark_path("classical/tests/gripper/domain.pddl")).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::base::RepositoryFactory().create(dl_repository);
    const auto sketch = kr::ps::base::dl::parse_sketch(read_fixture("kr/ps/base/dl/numerical_cycle.sketch"), planning_domain.get_domain(), *repository);

    const auto result = kr::ps::base::dl::structural_termination(sketch);
    const auto numericals = sketch.get_features<kr::ps::dl::NumericalFeature>();
    ASSERT_FALSE(result.is_terminating());
    ASSERT_TRUE(result.sieve_result.has_value());
    ASSERT_NE(result.sieve_result->counterexample, nullptr);
    EXPECT_GE(result.sieve_result->counterexample->get_num_edges(), 2);
    EXPECT_EQ(counterexample_rules(*result.sieve_result), sketch_rules(sketch));
    ASSERT_EQ(result.sieve_result->scc_results.size(), 1);
    EXPECT_TRUE(result.sieve_result->scc_results.front().booleans.empty());
    ASSERT_EQ(result.sieve_result->scc_results.front().numericals.size(), 1);
    EXPECT_EQ(result.sieve_result->scc_results.front().numericals.front(), numericals.front());
}

TEST(RunirTests, StructuralTerminationSurvivingRulesIncludeDisjointCycles)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(benchmark_path("classical/tests/gripper/domain.pddl")).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::base::RepositoryFactory().create(dl_repository);
    const auto sketch = kr::ps::base::dl::parse_sketch(R"((:sketch
        (:features (:boolean (:symbol b) (:expression (b_nonempty (c_atomic_state "at-robby")))))
        (:rules
            (:rule (:symbol shared) (:expression (:conditions) (:effects (unchanged b))))
            (:rule (:symbol false_loop) (:expression (:conditions (negative b)) (:effects (unchanged b))))
            (:rule (:symbol true_loop) (:expression (:conditions (positive b)) (:effects (unchanged b))))
        )))",
                                                    planning_domain.get_domain(),
                                                    *repository);

    for (const auto preprocessing : { false, true })
    {
        SCOPED_TRACE(preprocessing);
        const auto result = kr::ps::base::dl::structural_termination(sketch, kr::ps::dl::default_max_features, preprocessing);
        ASSERT_FALSE(result.is_terminating());
        ASSERT_TRUE(result.sieve_result.has_value());
        const auto& sieve = *result.sieve_result;
        ASSERT_NE(sieve.counterexample, nullptr);
        EXPECT_EQ(sieve.counterexample->get_num_vertices(), 2);
        EXPECT_EQ(sieve.counterexample->get_num_edges(), 4);
        EXPECT_EQ(counterexample_rules(sieve), sketch_rules(sketch));
        EXPECT_EQ(sieve.surviving_rules.size(), 3);
        EXPECT_EQ(graphs::find_edge_cycle(*sieve.counterexample).size(), 1);
    }
}

TEST(RunirTests, StructuralTerminationTppDetails)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(benchmark_path("classical/tests/gripper/domain.pddl")).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::base::RepositoryFactory().create(dl_repository);
    const auto sketch = kr::ps::base::dl::parse_sketch(read_fixture("kr/ps/base/dl/tpp.sketch"), planning_domain.get_domain(), *repository);

    const auto result = kr::ps::base::dl::structural_termination(sketch);
    const auto without_incomplete = kr::ps::base::dl::structural_termination(sketch, 3, false);
    EXPECT_TRUE(result.is_terminating());
    ASSERT_TRUE(result.incomplete_result.has_value());
    EXPECT_EQ(result.incomplete_result->status, kr::ps::base::dl::IncompleteStructuralTerminationStatus::TERMINATING);
    EXPECT_FALSE(result.sieve_result.has_value());
    ASSERT_TRUE(without_incomplete.sieve_result.has_value());
    EXPECT_EQ(without_incomplete.sieve_result->counterexample, nullptr);
    EXPECT_TRUE(without_incomplete.sieve_result->surviving_rules.empty());
    ASSERT_EQ(without_incomplete.sieve_result->scc_results.size(), 1);
    EXPECT_TRUE(without_incomplete.sieve_result->scc_results.front().booleans.empty());
    EXPECT_EQ(without_incomplete.sieve_result->scc_results.front().numericals.size(), 3);
}

TEST(RunirTests, StructuralTerminationGripperFactorySketchIsTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(benchmark_path("classical/tests/gripper/domain.pddl")).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::base::RepositoryFactory().create(dl_repository);
    const auto sketch = kr::ps::base::dl::SketchFactory::create(kr::ps::base::dl::SketchSpecification::GRIPPER_FRANCE_ET_AL_AAAI2021,
                                                                planning_domain.get_domain(),
                                                                *repository);
    EXPECT_TRUE(kr::ps::base::dl::structural_termination(sketch).is_terminating());
}

}  // namespace runir::tests
