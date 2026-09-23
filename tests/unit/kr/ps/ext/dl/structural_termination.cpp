#include "fixtures.hpp"

#include <cstdint>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <runir/graphs/cycle.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/ps/ext/dl/incomplete_structural_termination.hpp>
#include <runir/kr/ps/ext/dl/module_factory.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <runir/kr/ps/ext/dl/structural_termination.hpp>
#include <runir/kr/ps/ext/memory_state_data.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/ext/rule_view.hpp>
#include <set>
#include <string>
#include <tyr/formalism/planning/parser.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/semantics/comparison.hpp>

namespace runir::tests
{

TEST(RunirTests, ExtStructuralTerminationEmptyModuleIsTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_path("classical/tests/gripper/domain.pddl");
    const auto task_file = benchmark_path("classical/tests/gripper/test-1.pddl");
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    const auto module_ = kr::ps::ext::dl::ModuleFactory::create_empty(*repository);

    const auto result = kr::ps::ext::dl::structural_termination(module_);
    const auto without_incomplete = kr::ps::ext::dl::structural_termination(module_, kr::ps::dl::default_max_features, false);

    EXPECT_TRUE(result.is_terminating());
    EXPECT_FALSE(result.sieve_result.has_value());
    ASSERT_TRUE(without_incomplete.sieve_result.has_value());
    EXPECT_EQ(without_incomplete.sieve_result->counterexample, nullptr);
    EXPECT_TRUE(without_incomplete.sieve_result->scc_results.empty());
    EXPECT_TRUE(without_incomplete.sieve_result->surviving_rules.empty());
}

TEST(RunirTests, ExtStructuralTerminationDecreaseWithUnchangedReturnIsTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_path("classical/tests/gripper/domain.pddl");
    const auto task_file = benchmark_path("classical/tests/gripper/test-1.pddl");
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    // m0 -> m1 decreases fn; m1 -> m0 keeps fn unchanged: every memory cycle
    // strictly decreases fn, so the module terminates.
    const auto module_ = kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/dl/terminating.module"), planning_task.get_domain().get_domain(), *repository);

    const auto incomplete_result = kr::ps::ext::dl::incomplete_structural_termination(module_);
    const auto result = kr::ps::ext::dl::structural_termination(module_);
    const auto without_incomplete = kr::ps::ext::dl::structural_termination(module_, kr::ps::dl::default_max_features, false);
    const auto numericals = module_.get_features<kr::ps::dl::NumericalFeature>();

    EXPECT_TRUE(result.is_terminating());
    ASSERT_TRUE(result.incomplete_result.has_value());
    EXPECT_EQ(result.incomplete_result->status, incomplete_result.status);
    EXPECT_TRUE(without_incomplete.is_terminating());
    EXPECT_FALSE(without_incomplete.incomplete_result.has_value());
    EXPECT_FALSE(result.sieve_result.has_value());
    ASSERT_TRUE(without_incomplete.sieve_result.has_value());
    EXPECT_EQ(without_incomplete.sieve_result->counterexample, nullptr);
    EXPECT_TRUE(without_incomplete.sieve_result->surviving_rules.empty());
    ASSERT_EQ(without_incomplete.sieve_result->scc_results.size(), 1);
    EXPECT_TRUE(without_incomplete.sieve_result->scc_results.front().booleans.empty());
    ASSERT_EQ(without_incomplete.sieve_result->scc_results.front().numericals.size(), 1);
    EXPECT_EQ(without_incomplete.sieve_result->scc_results.front().numericals.front(), numericals.front());
}

TEST(RunirTests, ExtStructuralTerminationUsesDoRuleEffects)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_path("classical/tests/gripper/domain.pddl");
    const auto task_file = benchmark_path("classical/tests/gripper/test-1.pddl");
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    // The do edge declares fn unchanged, and the return edge decreases fn.
    // This is structurally terminating even though a do edge is in the memory cycle.
    const auto module_ =
        kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/dl/do_rule_effects.module"), planning_task.get_domain().get_domain(), *repository);

    const auto result = kr::ps::ext::dl::structural_termination(module_);

    EXPECT_TRUE(result.is_terminating());
}

TEST(RunirTests, ExtStructuralTerminationActionRulesUseEffectOverapproximation)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = fp::Parser(benchmark_path("classical/tests/gripper/domain.pddl")).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(domain.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);

    for (const auto& [effect, terminating] : std::vector<std::pair<std::string, bool>> {
             { "(unchanged n)", true }, { "(decreases n)", true }, { "(increases n)", false }, { "", false } })
    {
        SCOPED_TRACE(effect);
        const auto module_source = [&](bool action_rule)
        {
            return fmt::format(R"(
(:module (:symbol worker) (:arguments) (:registers (:role selected))
  (:entry m0) (:memory m0 m1)
  (:features
    (:concept (:symbol All) (:expression (c_top)))
    (:query (:symbol Moves) (:expression (q_role (from to) (r_register selected))))
    (:numerical (:symbol n) (:expression (n_count (c_atomic_state "ball"))))
  )
  (:rules
    (:rule (:symbol outward)
      (:expression (:source-memory m0) (:target-memory m1)
        ({0} (:conditions (greater_zero n)) (:action "move") {1} (:effects {2}))
      )
    )
    (:rule (:symbol backward)
      (:expression (:source-memory m1) (:target-memory m0)
        (:sketch (:conditions (greater_zero n)) (:effects (decreases n)))
      )
    )
  )
))", action_rule ? ":action" : ":do", action_rule ? "(:query Moves)" : "(:arguments All All)", effect);
        };
        const auto action = kr::ps::ext::dl::parse_module(module_source(true), domain.get_domain(), *repository);
        const auto do_rule = kr::ps::ext::dl::parse_module(module_source(false), domain.get_domain(), *repository);
        for (const auto preprocessing : { false, true })
        {
            const auto action_result = kr::ps::ext::dl::structural_termination(action, kr::ps::dl::default_max_features, preprocessing);
            const auto do_result = kr::ps::ext::dl::structural_termination(do_rule, kr::ps::dl::default_max_features, preprocessing);
            EXPECT_EQ(action_result.is_terminating(), terminating);
            EXPECT_EQ(action_result.status, do_result.status);
        }
        const auto action_incomplete = kr::ps::ext::dl::incomplete_structural_termination(action);
        const auto do_incomplete = kr::ps::ext::dl::incomplete_structural_termination(do_rule);
        EXPECT_EQ(action_incomplete.is_terminating(), terminating);
        EXPECT_EQ(action_incomplete.status, do_incomplete.status);
        EXPECT_EQ(action_incomplete.surviving_rules.size(), do_incomplete.surviving_rules.size());

        const auto program_source = fmt::format(R"(
(:program (:entry root)
  (:module (:symbol root) (:arguments) (:registers) (:entry start) (:memory start done) (:features)
    (:rules
      (:rule (:symbol invoke)
        (:expression (:source-memory start) (:target-memory done)
          (:call (:conditions) (:callee worker) (:arguments))
        )
      )
    )
  )
  {0}
))", module_source(true));
        const auto program = kr::ps::ext::dl::parse_program(program_source, domain.get_domain(), *repository);
        EXPECT_EQ(kr::ps::ext::dl::structural_termination(program).is_terminating(), terminating);
        EXPECT_EQ(kr::ps::ext::dl::incomplete_structural_termination(program).is_terminating(), terminating);
    }
}

TEST(RunirTests, ExtStructuralTerminationLoadPreservesRegisterIndependentFeature)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_path("classical/tests/gripper/domain.pddl");
    const auto task_file = benchmark_path("classical/tests/gripper/test-1.pddl");
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    // Loading r0 does not change the planning state and fn does not mention r0.
    // The load edge should therefore preserve fn, allowing the following
    // decreasing sketch edge to prove the memory cycle terminating.
    const auto module_ =
        kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/dl/load_independent.module"), planning_task.get_domain().get_domain(), *repository);

    const auto result = kr::ps::ext::dl::structural_termination(module_);

    EXPECT_TRUE(result.is_terminating());
}

TEST(RunirTests, ExtStructuralTerminationLoadUnconstrainsRegisterDependentFeature)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_path("classical/tests/gripper/domain.pddl");
    const auto task_file = benchmark_path("classical/tests/gripper/test-1.pddl");
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    // Here fn is the cardinality of the concept stored in r0. The load edge
    // writes r0, so fn is unconstrained and can restore the ranking after the
    // decreasing edge; SIEVE must leave a counterexample cycle.
    const auto module_ = kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/dl/load_dependent.module"), planning_task.get_domain().get_domain(), *repository);

    const auto result = kr::ps::ext::dl::structural_termination(module_);

    ASSERT_FALSE(result.is_terminating());
    ASSERT_TRUE(result.sieve_result.has_value());
    ASSERT_NE(result.sieve_result->counterexample, nullptr);
}

TEST(RunirTests, ExtStructuralTerminationFindsRegistersInsideQueries)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = fp::Parser(benchmark_path("classical/tests/gripper/domain.pddl")).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(domain.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    for (const auto* expression : { "c_top", "c_register r0" })
    {
        auto source = read_fixture("kr/ps/ext/dl/load_dependent.module");
        const auto position = source.find("(c_register r0)");
        ASSERT_NE(position, std::string::npos);
        source.replace(position,
                       std::string("(c_register r0)").size(),
                       std::string("(c_project x (q_project (x) (q_join (q_concept x (c_top)) (q_concept x (") + expression + ")))))");
        const auto module_ = kr::ps::ext::dl::parse_module(source, domain.get_domain(), *repository);
        for (const auto preprocessing : { false, true })
            EXPECT_EQ(kr::ps::ext::dl::structural_termination(module_, kr::ps::dl::default_max_features, preprocessing).is_terminating(),
                      std::string(expression) == "c_top");
    }
}

TEST(RunirTests, ExtStructuralTerminationLoadUnconstrainsRoleRegisterDependentFeature)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_path("classical/tests/gripper/domain.pddl");
    const auto task_file = benchmark_path("classical/tests/gripper/test-1.pddl");
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    // Here fn counts objects reached through the role stored in r0. The load edge
    // writes r0, so fn is unconstrained and can restore the ranking after the
    // decreasing edge; SIEVE must leave a counterexample cycle.
    const auto module_ =
        kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/dl/role_load_dependent.module"), planning_task.get_domain().get_domain(), *repository);

    const auto result = kr::ps::ext::dl::structural_termination(module_);

    ASSERT_FALSE(result.is_terminating());
    ASSERT_TRUE(result.sieve_result.has_value());
    ASSERT_NE(result.sieve_result->counterexample, nullptr);
}

TEST(RunirTests, ExtStructuralTerminationChoosePreservesBindingSemantics)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(benchmark_path("classical/tests/gripper/domain.pddl")).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    for (const auto& fixture : { "load_independent", "load_dependent", "role_load_dependent" })
    {
        auto source = read_fixture(std::string("kr/ps/ext/dl/") + fixture + ".module");
        source.replace(source.find("(:load"), 6, "(:choose");
        const auto module_ = kr::ps::ext::dl::parse_module(source, planning_domain.get_domain(), *repository);
        for (const auto preprocessing : { false, true })
            EXPECT_EQ(kr::ps::ext::dl::structural_termination(module_, kr::ps::dl::default_max_features, preprocessing).is_terminating(),
                      std::string(fixture) == "load_independent");
    }
}

TEST(RunirTests, ExtStructuralTerminationBindingEffectsOverrideImplicitPreservation)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(benchmark_path("classical/tests/gripper/domain.pddl")).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    for (const auto* kind : { "load", "choose" })
        for (const auto* expression : { "c_top", "c_register selected" })
            for (const auto* effect : { "increases", "decreases", "unchanged" })
            {
                SCOPED_TRACE(::testing::Message() << kind << ": " << expression << ", " << effect);
                const auto source = std::string("(:module (:symbol binding) (:arguments) (:registers (:concept selected)) ")
                                    + "(:entry m0) (:memory m0) (:features (:concept (:symbol all) (:expression (c_top))) "
                                    + "(:boolean (:symbol b) (:expression (b_nonempty (c_top)))) " + "(:numerical (:symbol n) (:expression (n_count ("
                                    + expression + "))))) " + "(:rules (:rule (:symbol bind) (:expression (:source-memory m0) (:target-memory m0) (:" + kind
                                    + " (:conditions (greater_zero n)) (:concept all) (:register (:concept selected)) " + "(:effects (positive b) (" + effect
                                    + " n)))))))";
                const auto module_ = kr::ps::ext::dl::parse_module(source, planning_domain.get_domain(), *repository);
                // Counts are bounded on each finite instance, so either unopposed
                // strict direction terminates; unchanged admits a self-loop.
                for (const auto preprocessing : { false, true })
                    EXPECT_EQ(kr::ps::ext::dl::structural_termination(module_, kr::ps::dl::default_max_features, preprocessing).is_terminating(),
                              std::string(effect) != "unchanged");
            }
}

TEST(RunirTests, ExtStructuralTerminationUnconstrainedReturnIsNotTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_path("classical/tests/gripper/domain.pddl");
    const auto task_file = benchmark_path("classical/tests/gripper/test-1.pddl");
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    // As above, but the return rule m1 -> m0 leaves fn unconstrained: the
    // memory cycle can restore fn, so termination cannot be proven.
    const auto module_ =
        kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/dl/non_terminating.module"), planning_task.get_domain().get_domain(), *repository);

    const auto result = kr::ps::ext::dl::structural_termination(module_);

    ASSERT_FALSE(result.is_terminating());
    ASSERT_TRUE(result.sieve_result.has_value());
    ASSERT_NE(result.sieve_result->counterexample, nullptr);

    // The counterexample cycle uses both rules and spans both memory states.
    auto rule_indices = std::set<ygg::Index<kr::ps::Rule<kr::ExtFamilyTag>>> {};
    for (const auto& edge : result.sieve_result->counterexample->get_edges())
        rule_indices.insert(edge.get_property().get_index());
    EXPECT_EQ(rule_indices.size(), 2);

    auto memory_state_indices = std::set<ygg::Index<kr::ps::ext::MemoryState>> {};
    for (const auto& vertex : result.sieve_result->counterexample->get_vertices())
        memory_state_indices.insert(vertex.get_property().memory_state.get_index());
    EXPECT_EQ(memory_state_indices.size(), 2);
}

TEST(RunirTests, ExtStructuralTerminationPreservesSparseMemoryStateIdentities)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_path("classical/tests/gripper/domain.pddl");
    const auto task_file = benchmark_path("classical/tests/gripper/test-1.pddl");
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    for (const auto* name : { "unused_before", "m1", "unused_between", "m0" })
    {
        auto data = ygg::Data<kr::ps::ext::MemoryState>(std::string(name));
        repository->get_or_create(data);
    }
    const auto module_ =
        kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/dl/non_terminating.module"), planning_task.get_domain().get_domain(), *repository);
    const auto memory_states = module_.get_memory_states();
    ASSERT_EQ(memory_states.size(), 2);
    EXPECT_EQ(memory_states[0].get_index(), ygg::Index<kr::ps::ext::MemoryState>(1));
    EXPECT_EQ(memory_states[1].get_index(), ygg::Index<kr::ps::ext::MemoryState>(3));
    EXPECT_EQ(memory_states[0].get_name(), "m1");
    EXPECT_EQ(memory_states[1].get_name(), "m0");

    for (const auto preprocessing : { false, true })
    {
        SCOPED_TRACE(preprocessing);
        const auto result = kr::ps::ext::dl::structural_termination(module_, kr::ps::dl::default_max_features, preprocessing);
        ASSERT_FALSE(result.is_terminating());
        ASSERT_TRUE(result.sieve_result.has_value());
        ASSERT_NE(result.sieve_result->counterexample, nullptr);
        const auto& graph = *result.sieve_result->counterexample;
        ASSERT_GT(graph.get_num_edges(), 0);
        for (const auto& edge : graph.get_edges())
            ygg::visit(
                [&](auto rule)
                {
                    EXPECT_EQ(graph.get_vertex(edge.get_source()).get_property().memory_state.get_index(), rule.get_source().get_index());
                    EXPECT_EQ(graph.get_vertex(edge.get_target()).get_property().memory_state.get_index(), rule.get_target().get_index());
                },
                edge.get_property().get_variant());
    }
}

TEST(RunirTests, ExtStructuralTerminationIgnoresOneWayBridgeBetweenMemoryCycles)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_path("classical/tests/gripper/domain.pddl");
    const auto task_file = benchmark_path("classical/tests/gripper/test-1.pddl");
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    // m0 <-> m1 terminates on fa, m2 <-> m3 does not terminate on fb, and
    // the one-way bridge m1 -> m2 cannot participate in a cycle.
    const auto module_ = kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/dl/one_way_bridge.module"), planning_task.get_domain().get_domain(), *repository);

    const auto result = kr::ps::ext::dl::structural_termination(module_);

    ASSERT_FALSE(result.is_terminating());
    ASSERT_TRUE(result.sieve_result.has_value());
    ASSERT_NE(result.sieve_result->counterexample, nullptr);

    auto rules = std::set<ygg::Index<kr::ps::Rule<kr::ExtFamilyTag>>> {};
    for (const auto& edge : result.sieve_result->counterexample->get_edges())
        rules.insert(edge.get_property().get_index());
    EXPECT_EQ(rules.size(), 2);

    auto memory_states = std::set<std::string> {};
    for (const auto& vertex : result.sieve_result->counterexample->get_vertices())
        memory_states.emplace(vertex.get_property().memory_state.get_name());
    EXPECT_EQ(memory_states, (std::set<std::string> { "m2", "m3" }));
}

TEST(RunirTests, ExtSurvivingRulesRetainOnlyResidualLabelsAcrossRuleVariants)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_path("classical/tests/gripper/domain.pddl");
    const auto task_file = benchmark_path("classical/tests/gripper/test-1.pddl");
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    const std::pair<const char*, std::set<std::string>> cases[] = {
        { "kr/ps/ext/dl/one_way_bridge.module", { "auto31", "auto33" } },
        { "kr/ps/ext/dl/load_dependent.module", { "auto13", "auto15" } },
        { "kr/ps/ext/dl/projected_components.module", { "keep_n0", "keep_n1", "to_false", "to_true" } },
    };

    for (const auto& [fixture, expected] : cases)
    {
        SCOPED_TRACE(fixture);
        const auto module_ = kr::ps::ext::dl::parse_module(read_fixture(fixture), planning_task.get_domain().get_domain(), *repository);
        for (const auto preprocessing : { false, true })
        {
            SCOPED_TRACE(preprocessing);
            const auto result = kr::ps::ext::dl::structural_termination(module_, kr::ps::dl::default_max_features, preprocessing);
            ASSERT_TRUE(result.sieve_result.has_value());
            const auto& rules = result.sieve_result->surviving_rules;
            auto symbols = std::set<std::string> {};
            for (const auto rule : rules)
                symbols.emplace(rule.get_symbol());
            EXPECT_EQ(rules.size(), expected.size());
            EXPECT_EQ(symbols, expected);
            ASSERT_NE(result.sieve_result->counterexample, nullptr);
            auto edge_symbols = std::set<std::string> {};
            for (const auto& edge : result.sieve_result->counterexample->get_edges())
                edge_symbols.emplace(edge.get_property().get_symbol());
            EXPECT_EQ(symbols, edge_symbols);
        }
    }
}

TEST(RunirTests, ExtStructuralTerminationLiftsProjectedComponentsToGlobalAxes)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_path("classical/tests/gripper/domain.pddl");
    const auto task_file = benchmark_path("classical/tests/gripper/test-1.pddl");
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    const auto module_ =
        kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/dl/projected_components.module"), planning_task.get_domain().get_domain(), *repository);

    const auto result = kr::ps::ext::dl::structural_termination(module_);
    const auto booleans = module_.get_features<kr::ps::dl::BooleanFeature>();
    const auto numericals = module_.get_features<kr::ps::dl::NumericalFeature>();

    ASSERT_FALSE(result.is_terminating());
    ASSERT_TRUE(result.sieve_result.has_value());
    ASSERT_NE(result.sieve_result->counterexample, nullptr);
    ASSERT_EQ(booleans.size(), 1);
    ASSERT_EQ(numericals.size(), 2);
    ASSERT_EQ(result.sieve_result->scc_results.size(), 3);
    auto saw_boolean_component = false;
    auto numerical_components = std::set<std::size_t> {};
    for (const auto& scc_result : result.sieve_result->scc_results)
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
    for (const auto& vertex : result.sieve_result->counterexample->get_vertices())
    {
        const auto& label = vertex.get_property();
        EXPECT_LE(label.boolean_values, 1);
        EXPECT_LE(label.numerical_values, 3);
        memory_states.emplace(label.memory_state.get_name());
        if (label.memory_state.get_name() == "m0")
            EXPECT_EQ(label.numerical_values, 0);
        else if (label.memory_state.get_name() == "m1")
        {
            EXPECT_EQ(label.boolean_values, 0);
            EXPECT_EQ(label.numerical_values & 1, 0);
            saw_positive_n1 |= (label.numerical_values & 2) != 0;
        }
        else
        {
            EXPECT_EQ(label.boolean_values, 0);
            EXPECT_EQ(label.numerical_values & 2, 0);
            saw_positive_n0 |= (label.numerical_values & 1) != 0;
        }
    }
    EXPECT_EQ(memory_states, (std::set<std::string> { "m0", "m1", "m2" }));
    EXPECT_TRUE(saw_positive_n0);
    EXPECT_TRUE(saw_positive_n1);

    auto rule_symbols = std::set<std::string> {};
    for (const auto& edge : result.sieve_result->counterexample->get_edges())
        rule_symbols.emplace(edge.get_property().get_symbol());
    EXPECT_EQ(rule_symbols, (std::set<std::string> { "keep_n0", "keep_n1", "to_false", "to_true" }));
    EXPECT_EQ(result.sieve_result->surviving_rules.size(), 4);
    EXPECT_LT(graphs::find_edge_cycle(*result.sieve_result->counterexample).size(), result.sieve_result->surviving_rules.size());
}

TEST(RunirTests, ExtStructuralTerminationAppliesFeatureLimitPerResidualComponent)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_path("classical/tests/gripper/domain.pddl");
    const auto task_file = benchmark_path("classical/tests/gripper/test-1.pddl");
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);

    const auto module_ = kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/dl/split_features.module"), planning_task.get_domain().get_domain(), *repository);

    const auto result = kr::ps::ext::dl::structural_termination(module_);
    const auto numericals = module_.get_features<kr::ps::dl::NumericalFeature>();

    EXPECT_FALSE(result.is_terminating());
    EXPECT_EQ(numericals.size(), 15);
    ASSERT_TRUE(result.sieve_result.has_value());
    ASSERT_NE(result.sieve_result->counterexample, nullptr);
    for (const auto& vertex : result.sieve_result->counterexample->get_vertices())
        EXPECT_LT(vertex.get_property().numerical_values, std::uint64_t { 1 } << 15);
}

TEST(RunirTests, ExtStructuralTerminationAcyclicProgramCallsAreTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_path("classical/tests/gripper/domain.pddl");
    const auto task_file = benchmark_path("classical/tests/gripper/test-1.pddl");
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    const auto program =
        kr::ps::ext::dl::parse_program(read_fixture("kr/ps/ext/dl/acyclic_calls.program"), planning_task.get_domain().get_domain(), *repository);

    const auto result = kr::ps::ext::dl::structural_termination(program);

    EXPECT_TRUE(result.is_terminating());
    EXPECT_EQ(result.module_results.size(), 2);
    EXPECT_TRUE(result.recursive_call_rules.empty());
}

TEST(RunirTests, ExtStructuralTerminationRecursiveProgramCallsAreNotTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = benchmark_path("classical/tests/gripper/domain.pddl");
    const auto task_file = benchmark_path("classical/tests/gripper/test-1.pddl");
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    // Each module is locally acyclic, so per-module structural termination is
    // insufficient. The program-level call graph root -> leaf -> root is a
    // recursive module cycle and is conservatively rejected.
    const auto program =
        kr::ps::ext::dl::parse_program(read_fixture("kr/ps/ext/dl/recursive_calls.program"), planning_task.get_domain().get_domain(), *repository);

    const auto result = kr::ps::ext::dl::structural_termination(program);

    ASSERT_FALSE(result.is_terminating());
    EXPECT_EQ(result.module_results.size(), 2);
    ASSERT_FALSE(result.recursive_call_rules.empty());
    EXPECT_TRUE(result.module_results[0].is_terminating());
    EXPECT_TRUE(result.module_results[1].is_terminating());
}

TEST(RunirTests, ExtStructuralTerminationPreservesCallLabelsAndOrder)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(benchmark_path("classical/tests/gripper/domain.pddl")).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    const auto original = kr::ps::ext::dl::parse_program(read_fixture("kr/ps/ext/dl/ordered_calls.program"), planning_domain.get_domain(), *repository);

    for (const auto omit_target : { false, true })
    {
        auto data = original.get_data();
        if (omit_target)
            data.modules.pop_back();
        const auto program = repository->get_or_create(data).first;
        auto expected = std::vector<std::string> { "self", "first", "second", "back", "unreachable" };
        if (omit_target)
            expected.insert(expected.begin(), "missing");
        const auto check = [&](const auto& result)
        {
            EXPECT_FALSE(result.is_terminating());
            EXPECT_EQ(result.module_results.size(), omit_target ? 3 : 4);
            for (const auto& module_result : result.module_results)
                EXPECT_TRUE(module_result.is_terminating());
            auto actual = std::vector<std::string> {};
            for (const auto& rule : result.recursive_call_rules)
                actual.emplace_back(rule.get_symbol());
            EXPECT_EQ(actual, expected);
        };
        check(kr::ps::ext::dl::incomplete_structural_termination(program));
        check(kr::ps::ext::dl::structural_termination(program));
        check(kr::ps::ext::dl::structural_termination(program, kr::ps::dl::default_max_features, false));
    }
}

}  // namespace runir::tests
