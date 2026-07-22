#include "fixtures.hpp"

#include <gtest/gtest.h>
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

auto counterexample_rules(const kr::ps::base::dl::StructuralTerminationResult& result) -> std::set<ygg::Index<kr::ps::base::Rule>>
{
    auto rules = std::set<ygg::Index<kr::ps::base::Rule>> {};
    for (const auto& edge : result.counterexample->get_edges())
        rules.insert(edge.get_property().rule.get_index());
    return rules;
}

auto sketch_rules(kr::ps::base::SketchView sketch) -> std::set<ygg::Index<kr::ps::base::Rule>>
{
    auto rules = std::set<ygg::Index<kr::ps::base::Rule>> {};
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
    EXPECT_EQ(result.counterexample, nullptr);
    EXPECT_FALSE(result.scc_results.has_value());
    EXPECT_TRUE(without_incomplete.is_terminating());
    EXPECT_FALSE(without_incomplete.incomplete_result.has_value());
    ASSERT_TRUE(without_incomplete.scc_results.has_value());
    EXPECT_TRUE(without_incomplete.scc_results->empty());
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
    ASSERT_NE(result.counterexample, nullptr);
    EXPECT_EQ(result.counterexample->get_num_vertices(), 2);
    EXPECT_EQ(result.counterexample->get_num_edges(), 2);
    EXPECT_EQ(counterexample_rules(result), sketch_rules(sketch));
    ASSERT_TRUE(result.scc_results.has_value());
    ASSERT_EQ(result.scc_results->size(), 1);
    ASSERT_EQ(result.scc_results->front().booleans.size(), 1);
    EXPECT_EQ(result.scc_results->front().booleans.front(), booleans.front());
    EXPECT_TRUE(result.scc_results->front().numericals.empty());
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
    ASSERT_NE(result.counterexample, nullptr);
    EXPECT_GE(result.counterexample->get_num_edges(), 2);
    EXPECT_EQ(counterexample_rules(result), sketch_rules(sketch));
    ASSERT_TRUE(result.scc_results.has_value());
    ASSERT_EQ(result.scc_results->size(), 1);
    EXPECT_TRUE(result.scc_results->front().booleans.empty());
    ASSERT_EQ(result.scc_results->front().numericals.size(), 1);
    EXPECT_EQ(result.scc_results->front().numericals.front(), numericals.front());
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
    EXPECT_FALSE(result.scc_results.has_value());
    ASSERT_TRUE(without_incomplete.scc_results.has_value());
    ASSERT_EQ(without_incomplete.scc_results->size(), 1);
    EXPECT_TRUE(without_incomplete.scc_results->front().booleans.empty());
    EXPECT_EQ(without_incomplete.scc_results->front().numericals.size(), 3);
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
