#include "fixtures.hpp"

#include <filesystem>
#include <gtest/gtest.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/ps/ext/dl/incomplete_structural_termination.hpp>
#include <runir/kr/ps/ext/dl/module_factory.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <runir/kr/ps/ext/dl/structural_termination.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <set>
#include <string>
#include <tyr/formalism/planning/parser.hpp>
#include <yggdrasil/semantics/comparison.hpp>
#include <yggdrasil/serialization/json.hpp>

namespace runir::tests
{

namespace
{

std::filesystem::path benchmark_prefix() { return std::filesystem::path(BENCHMARKS_DIR); }

}  // namespace

// Fixtures use dummy feature expressions over the gripper domain; structural
// termination depends only on the rule structure over feature symbols and
// the memory structure of the module.

TEST(RunirTests, ExtStructuralTerminationFixturesMatch)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl").get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    const auto suite = load_fixture_json("kr/ps/structural_termination.json");
    const auto& cases = ygg::common::as_array(ygg::common::require_member(ygg::common::as_object(suite, "suite"), "ext", "suite"), "suite.ext");

    for (const auto& value : cases)
    {
        const auto& test_case = ygg::common::as_object(value, "case");
        const auto module =
            kr::ps::ext::dl::parse_module(read_fixture(ygg::common::as_string(test_case, "file", "case")), planning_domain.get_domain(), *repository);
        EXPECT_EQ(kr::ps::ext::dl::structural_termination(module).is_terminating(), ygg::common::as_bool(test_case, "terminating", "case"))
            << ygg::common::as_string(test_case, "name", "case");
    }
}

TEST(RunirTests, ExtStructuralTerminationEmptyModuleIsTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    const auto module = kr::ps::ext::dl::ModuleFactory::create_empty(*repository);

    const auto result = kr::ps::ext::dl::structural_termination(module);
    const auto without_incomplete = kr::ps::ext::dl::structural_termination(module, kr::ps::dl::default_max_features, false);

    EXPECT_TRUE(result.is_terminating());
    EXPECT_EQ(result.counterexample, nullptr);
    EXPECT_FALSE(result.scc_results.has_value());
    ASSERT_TRUE(without_incomplete.scc_results.has_value());
    EXPECT_TRUE(without_incomplete.scc_results->empty());
}

TEST(RunirTests, ExtStructuralTerminationDecreaseWithUnchangedReturnIsTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    // m0 -> m1 decreases fn; m1 -> m0 keeps fn unchanged: every memory cycle
    // strictly decreases fn, so the module terminates.
    const auto module = kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/dl/terminating.module"), planning_task.get_domain().get_domain(), *repository);

    const auto incomplete_result = kr::ps::ext::dl::incomplete_structural_termination(module);
    const auto result = kr::ps::ext::dl::structural_termination(module);
    const auto without_incomplete = kr::ps::ext::dl::structural_termination(module, kr::ps::dl::default_max_features, false);
    const auto numericals = module.get_features<kr::ps::dl::NumericalFeature>();

    EXPECT_TRUE(result.is_terminating());
    ASSERT_TRUE(result.incomplete_result.has_value());
    EXPECT_EQ(result.incomplete_result->status, incomplete_result.status);
    EXPECT_TRUE(without_incomplete.is_terminating());
    EXPECT_FALSE(without_incomplete.incomplete_result.has_value());
    EXPECT_FALSE(result.scc_results.has_value());
    ASSERT_TRUE(without_incomplete.scc_results.has_value());
    ASSERT_EQ(without_incomplete.scc_results->size(), 1);
    EXPECT_TRUE(without_incomplete.scc_results->front().booleans.empty());
    ASSERT_EQ(without_incomplete.scc_results->front().numericals.size(), 1);
    EXPECT_EQ(without_incomplete.scc_results->front().numericals.front(), numericals.front());
}

TEST(RunirTests, ExtStructuralTerminationUsesDoRuleEffects)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    // The do edge declares fn unchanged, and the return edge decreases fn.
    // This is structurally terminating even though a do edge is in the memory cycle.
    const auto module =
        kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/dl/do_rule_effects.module"), planning_task.get_domain().get_domain(), *repository);

    const auto result = kr::ps::ext::dl::structural_termination(module);

    EXPECT_TRUE(result.is_terminating());
}

TEST(RunirTests, ExtStructuralTerminationLoadPreservesRegisterIndependentFeature)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    // Loading r0 does not change the planning state and fn does not mention r0.
    // The load edge should therefore preserve fn, allowing the following
    // decreasing sketch edge to prove the memory cycle terminating.
    const auto module =
        kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/dl/load_independent.module"), planning_task.get_domain().get_domain(), *repository);

    const auto result = kr::ps::ext::dl::structural_termination(module);

    EXPECT_TRUE(result.is_terminating());
}

TEST(RunirTests, ExtStructuralTerminationLoadUnconstrainsRegisterDependentFeature)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    // Here fn is the cardinality of the concept stored in r0. The load edge
    // writes r0, so fn is unconstrained and can restore the ranking after the
    // decreasing edge; SIEVE must leave a counterexample cycle.
    const auto module = kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/dl/load_dependent.module"), planning_task.get_domain().get_domain(), *repository);

    const auto result = kr::ps::ext::dl::structural_termination(module);

    ASSERT_FALSE(result.is_terminating());
    ASSERT_NE(result.counterexample, nullptr);
}

TEST(RunirTests, ExtStructuralTerminationLoadUnconstrainsRoleRegisterDependentFeature)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    // Here fn counts objects reached through the role stored in r0. The load edge
    // writes r0, so fn is unconstrained and can restore the ranking after the
    // decreasing edge; SIEVE must leave a counterexample cycle.
    const auto module =
        kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/dl/role_load_dependent.module"), planning_task.get_domain().get_domain(), *repository);

    const auto result = kr::ps::ext::dl::structural_termination(module);

    ASSERT_FALSE(result.is_terminating());
    ASSERT_NE(result.counterexample, nullptr);
}

TEST(RunirTests, ExtStructuralTerminationUnconstrainedReturnIsNotTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    // As above, but the return rule m1 -> m0 leaves fn unconstrained: the
    // memory cycle can restore fn, so termination cannot be proven.
    const auto module =
        kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/dl/non_terminating.module"), planning_task.get_domain().get_domain(), *repository);

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

TEST(RunirTests, ExtStructuralTerminationIgnoresOneWayBridgeBetweenMemoryCycles)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    // m0 <-> m1 terminates on fa, m2 <-> m3 does not terminate on fb, and
    // the one-way bridge m1 -> m2 cannot participate in a cycle.
    const auto module = kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/dl/one_way_bridge.module"), planning_task.get_domain().get_domain(), *repository);

    const auto result = kr::ps::ext::dl::structural_termination(module);

    ASSERT_FALSE(result.is_terminating());
    ASSERT_NE(result.counterexample, nullptr);

    auto rules = std::set<ygg::Index<kr::ps::ext::RuleVariant>> {};
    for (const auto& edge : result.counterexample->get_edges())
        rules.insert(edge.get_property().rule.get_index());
    EXPECT_EQ(rules.size(), 2);

    auto memory_states = std::set<std::string> {};
    for (const auto& vertex : result.counterexample->get_vertices())
        memory_states.emplace(vertex.get_property().memory_state.get_name());
    EXPECT_EQ(memory_states, (std::set<std::string> { "m2", "m3" }));
}

TEST(RunirTests, ExtStructuralTerminationLiftsProjectedComponentsToGlobalAxes)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    const auto module =
        kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/dl/projected_components.module"), planning_task.get_domain().get_domain(), *repository);

    const auto result = kr::ps::ext::dl::structural_termination(module);
    const auto booleans = module.get_features<kr::ps::dl::BooleanFeature>();
    const auto numericals = module.get_features<kr::ps::dl::NumericalFeature>();

    ASSERT_FALSE(result.is_terminating());
    ASSERT_NE(result.counterexample, nullptr);
    ASSERT_EQ(booleans.size(), 1);
    ASSERT_EQ(numericals.size(), 2);
    ASSERT_TRUE(result.scc_results.has_value());
    ASSERT_EQ(result.scc_results->size(), 3);
    auto saw_boolean_component = false;
    auto numerical_components = std::set<std::size_t> {};
    for (const auto& scc_result : *result.scc_results)
    {
        ASSERT_EQ(scc_result.booleans.size() + scc_result.numericals.size(), 1);
        if (!scc_result.booleans.empty())
        {
            EXPECT_EQ(scc_result.booleans.front(), booleans.front());
            saw_boolean_component = true;
        }
        else if (scc_result.numericals.front() == numericals[0])
            numerical_components.insert(0);
        else
        {
            EXPECT_EQ(scc_result.numericals.front(), numericals[1]);
            numerical_components.insert(1);
        }
    }
    EXPECT_TRUE(saw_boolean_component);
    EXPECT_EQ(numerical_components, (std::set<std::size_t> { 0, 1 }));

    auto memory_states = std::set<std::string> {};
    auto saw_positive_n0 = false;
    auto saw_positive_n1 = false;
    for (const auto& vertex : result.counterexample->get_vertices())
    {
        const auto& label = vertex.get_property();
        ASSERT_EQ(label.boolean_values.size(), booleans.size());
        ASSERT_EQ(label.numerical_values.size(), numericals.size());
        memory_states.emplace(label.memory_state.get_name());
        if (label.memory_state.get_name() == "m0")
            EXPECT_FALSE(label.numerical_values.any());
        else if (label.memory_state.get_name() == "m1")
        {
            EXPECT_FALSE(label.boolean_values.any());
            EXPECT_FALSE(label.numerical_values.test(0));
            saw_positive_n1 |= label.numerical_values.test(1);
        }
        else
        {
            EXPECT_FALSE(label.boolean_values.any());
            EXPECT_FALSE(label.numerical_values.test(1));
            saw_positive_n0 |= label.numerical_values.test(0);
        }
    }
    EXPECT_EQ(memory_states, (std::set<std::string> { "m0", "m1", "m2" }));
    EXPECT_TRUE(saw_positive_n0);
    EXPECT_TRUE(saw_positive_n1);

    auto rule_symbols = std::set<std::string> {};
    for (const auto& edge : result.counterexample->get_edges())
    {
        const auto& label = edge.get_property();
        const auto symbol = std::string(label.rule.get_symbol());
        rule_symbols.emplace(symbol);
        EXPECT_EQ(label.numerical_changes.size(), numericals.size());
        if (symbol == "keep_n0")
        {
            EXPECT_EQ(label.numerical_changes[0], kr::ps::dl::NumericalChange::UNCHANGED);
            EXPECT_EQ(label.numerical_changes[1], kr::ps::dl::NumericalChange::UNCONSTRAINED);
        }
        else if (symbol == "keep_n1")
        {
            EXPECT_EQ(label.numerical_changes[0], kr::ps::dl::NumericalChange::UNCONSTRAINED);
            EXPECT_EQ(label.numerical_changes[1], kr::ps::dl::NumericalChange::UNCHANGED);
        }
        else
        {
            EXPECT_EQ(label.numerical_changes[0], kr::ps::dl::NumericalChange::UNCONSTRAINED);
            EXPECT_EQ(label.numerical_changes[1], kr::ps::dl::NumericalChange::UNCONSTRAINED);
        }
    }
    EXPECT_EQ(rule_symbols, (std::set<std::string> { "keep_n0", "keep_n1", "to_false", "to_true" }));
}

TEST(RunirTests, ExtStructuralTerminationAppliesFeatureLimitPerResidualComponent)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);

    const auto module = kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/dl/split_features.module"), planning_task.get_domain().get_domain(), *repository);

    const auto result = kr::ps::ext::dl::structural_termination(module);
    const auto numericals = module.get_features<kr::ps::dl::NumericalFeature>();

    EXPECT_FALSE(result.is_terminating());
    EXPECT_EQ(numericals.size(), 15);
    ASSERT_NE(result.counterexample, nullptr);
    for (const auto& vertex : result.counterexample->get_vertices())
        EXPECT_EQ(vertex.get_property().numerical_values.size(), numericals.size());
}

TEST(RunirTests, ExtStructuralTerminationAcyclicModuleProgramCallsAreTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    const auto program =
        kr::ps::ext::dl::parse_module_program(read_fixture("kr/ps/ext/dl/acyclic_calls.program"), planning_task.get_domain().get_domain(), *repository);

    const auto result = kr::ps::ext::dl::structural_termination(program);

    EXPECT_TRUE(result.is_terminating());
    EXPECT_EQ(result.module_results.size(), 2);
    EXPECT_TRUE(result.recursive_call_rules.empty());
}

TEST(RunirTests, ExtStructuralTerminationRecursiveModuleProgramCallsAreNotTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    // Each module is locally acyclic, so per-module structural termination is
    // insufficient. The program-level call graph root -> leaf -> root is a
    // recursive module cycle and is conservatively rejected.
    const auto program =
        kr::ps::ext::dl::parse_module_program(read_fixture("kr/ps/ext/dl/recursive_calls.program"), planning_task.get_domain().get_domain(), *repository);

    const auto result = kr::ps::ext::dl::structural_termination(program);

    ASSERT_FALSE(result.is_terminating());
    EXPECT_EQ(result.module_results.size(), 2);
    ASSERT_FALSE(result.recursive_call_rules.empty());
    EXPECT_TRUE(result.module_results[0].is_terminating());
    EXPECT_TRUE(result.module_results[1].is_terminating());
}

}  // namespace runir::tests
