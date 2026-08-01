#include "fixtures.hpp"
#include "module_fixtures.hpp"

#include <boost/variant/get.hpp>
#include <concepts>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <runir/kr/errors.hpp>
#include <runir/kr/ps/ext/dl/module_factory.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <runir/kr/ps/ext/dl/parser/parser.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/ext/syntactic_complexity.hpp>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <tyr/formalism/planning/parser.hpp>
#include <yggdrasil/serialization/json.hpp>

namespace runir::tests
{

namespace
{

template<typename FeatureTag, kr::dl::CategoryTag Category>
auto create_feature(kr::ps::ext::Repository& repository, ygg::Index<kr::dl::FamilyConstructor<kr::ExtFamilyTag, Category>> expression, const std::string& name)
{
    auto concrete_data = ygg::Data<kr::ps::ConcreteFeature<kr::ExtFamilyTag, kr::DlTag, FeatureTag>>(expression, name);
    const auto concrete = repository.get_or_create(concrete_data).first;
    auto feature_data = ygg::Data<kr::ps::Feature<kr::ExtFamilyTag, FeatureTag>>(concrete.get_index());
    return repository.get_or_create(feature_data).first;
}

}  // namespace

TEST(RunirTests, ExtSyntacticComplexityAggregatesDeclaredFeatures)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_path("classical/tests/gripper/domain.pddl");
    const auto planning_domain = fp::Parser(domain).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);

    const auto concept_expression = create_top_concept(*dl_repository);
    const auto role = kr::ps::ext::dl::parse_role("(r_universal)", planning_domain.get_domain(), *dl_repository);
    const auto boolean = kr::ps::ext::dl::parse_boolean("(b_nonempty (c_top))", planning_domain.get_domain(), *dl_repository);
    const auto numerical = kr::ps::ext::dl::parse_numerical("(n_count (c_top))", planning_domain.get_domain(), *dl_repository);
    const auto concept_feature = create_feature<kr::dl::ConceptTag>(*repository, concept_expression.get_index(), "concept");
    const auto role_feature = create_feature<kr::dl::RoleTag>(*repository, role.get_index(), "role");
    const auto boolean_feature = create_feature<kr::ps::dl::BooleanFeature>(*repository, boolean.get_index(), "boolean");
    const auto numerical_feature = create_feature<kr::ps::dl::NumericalFeature>(*repository, numerical.get_index(), "numerical");

    const auto expect_feature_complexity = [](auto feature, std::size_t expected)
    {
        const auto concrete = ygg::visit([](auto view) { return kr::ps::ext::dl::syntactic_complexity(view); }, feature.get_variant());
        EXPECT_EQ(kr::ps::ext::syntactic_complexity(feature), concrete);
        EXPECT_EQ(concrete, kr::dl::semantics::syntactic_complexity(feature.get_expression()));
        EXPECT_EQ(concrete, expected);
    };
    expect_feature_complexity(concept_feature, 1);
    expect_feature_complexity(role_feature, 1);
    expect_feature_complexity(boolean_feature, 2);
    expect_feature_complexity(numerical_feature, 2);

    const auto all_entry = create_memory_state(*repository, "all_entry");
    auto all_data = make_module_data(*repository, "all");
    all_data.entry_memory_state = all_entry.get_index();
    all_data.memory_states.push_back(all_entry.get_index());
    all_data.concept_features.push_back(concept_feature.get_index());
    all_data.role_features.push_back(role_feature.get_index());
    all_data.boolean_features.push_back(boolean_feature.get_index());
    all_data.numerical_features.push_back(numerical_feature.get_index());
    kr::ps::ext::canonicalize(all_data);
    const auto all = repository->get_or_create(all_data).first;

    const auto shared_feature = create_feature<kr::ps::dl::BooleanFeature>(*repository, boolean.get_index(), "shared");
    const auto shared_entry = create_memory_state(*repository, "shared_entry");
    auto shared_data = make_module_data(*repository, "shared");
    shared_data.entry_memory_state = shared_entry.get_index();
    shared_data.memory_states.push_back(shared_entry.get_index());
    shared_data.boolean_features.push_back(shared_feature.get_index());
    kr::ps::ext::canonicalize(shared_data);
    const auto shared = repository->get_or_create(shared_data).first;

    EXPECT_EQ(kr::ps::ext::syntactic_complexity(all), 6);
    EXPECT_EQ(kr::ps::ext::syntactic_complexity(shared), 2);
    EXPECT_EQ(kr::ps::ext::syntactic_complexity(create_module_program(*repository, all, { all, shared })), 8);

    const auto empty_entry = create_memory_state(*repository, "empty_entry");
    const auto empty = create_module(*repository, "empty", empty_entry, { empty_entry });
    EXPECT_EQ(kr::ps::ext::syntactic_complexity(empty), 0);
    EXPECT_EQ(kr::ps::ext::syntactic_complexity(create_module_program(*repository, empty, { empty })), 0);
}

TEST(RunirTests, ExtModuleParserLowersArgumentRegisterMemorySections)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_path("classical/tests/gripper/domain.pddl");
    const auto task_file = benchmark_path("classical/tests/gripper/test-1.pddl");
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto description = read_fixture("kr/ps/ext/executor/ext_module_parser_lowers_argument_register_memory_sections/description.module");

    const auto module = kr::ps::ext::dl::parse_module(description, planning_task.get_domain().get_domain(), *repository);
    EXPECT_EQ(module.get_name(), "entry");
    EXPECT_EQ(module.get_arguments<kr::dl::ConceptTag>().size(), 1);
    EXPECT_EQ(module.get_arguments<kr::dl::RoleTag>().size(), 1);
    EXPECT_EQ(module.get_arguments<kr::dl::BooleanTag>().size(), 1);
    EXPECT_EQ(module.get_arguments<kr::dl::NumericalTag>().size(), 1);
    EXPECT_EQ(module.get_registers<kr::dl::ConceptTag>().size(), 1);
    EXPECT_EQ(module.get_registers<kr::dl::RoleTag>().size(), 1);
    EXPECT_EQ(module.get_features<kr::dl::ConceptTag>().size(), 1);
    EXPECT_EQ(module.get_entry_memory_state().get_name(), "m0");
    EXPECT_EQ(module.get_memory_states().size(), 2);
}

TEST(RunirTests, ExtModuleParserLowersNamedCalleesWithoutPreexistingModules)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_path("classical/tests/gripper/domain.pddl");
    const auto task_file = benchmark_path("classical/tests/gripper/test-1.pddl");
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto caller_description =
        read_fixture("kr/ps/ext/executor/ext_module_parser_lowers_named_callees_without_preexisting_modules/caller_description.module");
    const auto callee_description =
        read_fixture("kr/ps/ext/executor/ext_module_parser_lowers_named_callees_without_preexisting_modules/callee_description.module");

    const auto modules = kr::ps::ext::dl::parse_modules({ caller_description, callee_description }, planning_task.get_domain().get_domain(), *repository);
    ASSERT_EQ(modules.size(), 2);
    EXPECT_EQ(modules[0].get_name(), "caller");
    EXPECT_EQ(modules[1].get_name(), "callee");

    ASSERT_EQ(modules[0].get_data().memory_transitions.size(), 1);
    const auto rules = modules[0].get_memory_transitions()[0];
    ASSERT_FALSE(rules.empty());
    auto found_call_rule = false;
    ygg::visit(
        [&](auto rule)
        {
            using RuleView = std::decay_t<decltype(rule)>;
            if constexpr (std::same_as<RuleView, kr::ps::ext::RuleView<kr::ps::ext::CallTag>>)
            {
                found_call_rule = true;
                EXPECT_EQ(rule.get_callee().get_name(), "callee");
            }
        },
        rules.front().get_variant());
    EXPECT_TRUE(found_call_rule) << "Expected call rule.";
}

TEST(RunirTests, ExtModuleParserRejectsInvalidModuleSets)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_path("classical/tests/gripper/domain.pddl");
    const auto task_file = benchmark_path("classical/tests/gripper/test-1.pddl");
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto caller_with_argument = read_fixture("kr/ps/ext/executor/ext_module_parser_rejects_invalid_module_sets/caller_with_argument.module");
    const auto callee_without_arguments =
        read_fixture("kr/ps/ext/executor/ext_module_parser_lowers_named_callees_without_preexisting_modules/callee_description.module");
    EXPECT_THROW(kr::ps::ext::dl::parse_modules({ caller_with_argument, callee_without_arguments }, planning_task.get_domain().get_domain(), *repository),
                 std::runtime_error);

    const auto duplicate_callee =
        read_fixture("kr/ps/ext/executor/ext_module_parser_lowers_named_callees_without_preexisting_modules/callee_description.module");
    EXPECT_THROW(kr::ps::ext::dl::parse_modules({ callee_without_arguments, duplicate_callee }, planning_task.get_domain().get_domain(), *repository),
                 std::runtime_error);
}

TEST(RunirTests, ExtModuleParserRejectsInvalidDoActions)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_path("classical/tests/gripper/domain.pddl");
    const auto task_file = benchmark_path("classical/tests/gripper/test-1.pddl");
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto unknown_action = read_fixture("kr/ps/ext/executor/ext_module_parser_rejects_invalid_do_actions/unknown_action.module");
    EXPECT_THROW(kr::ps::ext::dl::parse_module(unknown_action, planning_task.get_domain().get_domain(), *repository), std::runtime_error);

    const auto wrong_arity = std::string(read_fixture("kr/ps/ext/executor/ext_module_parser_rejects_invalid_do_actions/wrong_arity.module"));
    try
    {
        [[maybe_unused]] const auto module = kr::ps::ext::dl::parse_module(wrong_arity, planning_task.get_domain().get_domain(), *repository);
        FAIL() << "Expected wrong do-action arity to be rejected.";
    }
    catch (const std::runtime_error& err)
    {
        EXPECT_NE(std::string(err.what()).find("Arity mismatch for action move: expected 2, got 0"), std::string::npos) << err.what();
    }

    const auto undeclared_argument_feature = read_fixture("kr/ps/ext/executor/ext_module_parser_rejects_invalid_do_actions/undeclared_argument_feature.module");
    try
    {
        [[maybe_unused]] const auto module = kr::ps::ext::dl::parse_module(undeclared_argument_feature, planning_task.get_domain().get_domain(), *repository);
        FAIL() << "Expected undeclared do-argument concept feature to be rejected.";
    }
    catch (const std::runtime_error& err)
    {
        EXPECT_NE(std::string(err.what()).find("Undefined feature: Missing"), std::string::npos) << err.what();
    }

    const auto inline_argument_expression = read_fixture("kr/ps/ext/executor/ext_module_parser_rejects_invalid_do_actions/inline_argument_expression.module");
    EXPECT_THROW(kr::ps::ext::dl::parse_module(inline_argument_expression, planning_task.get_domain().get_domain(), *repository), kr::ParseError);
}

TEST(RunirTests, ExtModuleParserRejectsInvalidSections)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_path("classical/tests/gripper/domain.pddl");
    const auto task_file = benchmark_path("classical/tests/gripper/test-1.pddl");
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto expect_parse_error_containing = [&](const auto& module_text, const std::string& expected_fragment)
    {
        try
        {
            [[maybe_unused]] const auto module = kr::ps::ext::dl::parse_module(module_text, planning_task.get_domain().get_domain(), *repository);
            FAIL() << "Expected module parser error containing: " << expected_fragment;
        }
        catch (const std::runtime_error& err)
        {
            EXPECT_NE(std::string(err.what()).find(expected_fragment), std::string::npos) << err.what();
        }
    };

    const auto missing_transitions = read_fixture("kr/ps/ext/executor/ext_module_parser_rejects_invalid_sections/missing_transitions.module");
    EXPECT_THROW(kr::ps::ext::dl::parse_module(missing_transitions, planning_task.get_domain().get_domain(), *repository), std::runtime_error);

    const auto duplicate_memory = read_fixture("kr/ps/ext/executor/ext_module_parser_rejects_invalid_sections/duplicate_memory.module");
    EXPECT_THROW(kr::ps::ext::dl::parse_module(duplicate_memory, planning_task.get_domain().get_domain(), *repository), std::runtime_error);

    const auto empty_memory = read_fixture("kr/ps/ext/executor/ext_module_parser_rejects_invalid_sections/empty_memory.module");
    EXPECT_THROW(kr::ps::ext::dl::parse_module(empty_memory, planning_task.get_domain().get_domain(), *repository), std::runtime_error);

    const auto duplicate_register_identifier =
        read_fixture("kr/ps/ext/executor/ext_module_parser_rejects_invalid_sections/duplicate_register_identifier.module");
    expect_parse_error_containing(duplicate_register_identifier, "Duplicate concept register definition: r0");

    const auto duplicate_argument_identifier =
        read_fixture("kr/ps/ext/executor/ext_module_parser_rejects_invalid_sections/duplicate_argument_identifier.module");
    expect_parse_error_containing(duplicate_argument_identifier, "Duplicate concept argument definition: x");

    const auto out_of_range_argument = read_fixture("kr/ps/ext/executor/ext_module_parser_rejects_invalid_sections/out_of_range_argument.module");
    expect_parse_error_containing(out_of_range_argument, "Undefined concept argument: missing");

    const auto out_of_range_expression_argument =
        std::string(read_fixture("kr/ps/ext/executor/ext_module_parser_rejects_invalid_sections/out_of_range_expression_argument.module"));
    try
    {
        [[maybe_unused]] const auto module =
            kr::ps::ext::dl::parse_module(out_of_range_expression_argument, planning_task.get_domain().get_domain(), *repository);
        FAIL() << "Expected unknown expression argument to be rejected.";
    }
    catch (const std::runtime_error& err)
    {
        EXPECT_NE(std::string(err.what()).find("Undefined concept argument: missing"), std::string::npos) << err.what();
    }

    const auto undeclared_expression_register =
        read_fixture("kr/ps/ext/executor/ext_module_parser_rejects_invalid_sections/undeclared_expression_register.module");
    EXPECT_THROW(kr::ps::ext::dl::parse_module(undeclared_expression_register, planning_task.get_domain().get_domain(), *repository), std::runtime_error);

    const auto duplicate_rule_section = read_fixture("kr/ps/ext/executor/ext_module_parser_rejects_invalid_sections/duplicate_rule_section.module");
    EXPECT_THROW(kr::ps::ext::dl::parse_module(duplicate_rule_section, planning_task.get_domain().get_domain(), *repository), std::runtime_error);

    const auto invalid_rule_section = read_fixture("kr/ps/ext/executor/ext_module_parser_rejects_invalid_sections/invalid_rule_section.module");
    EXPECT_THROW(kr::ps::ext::dl::parse_module(invalid_rule_section, planning_task.get_domain().get_domain(), *repository), std::runtime_error);

    const auto flat_load_register = read_fixture("kr/ps/ext/executor/ext_module_parser_rejects_invalid_sections/flat_load_register.module");
    EXPECT_THROW(kr::ps::ext::dl::parse_module(flat_load_register, planning_task.get_domain().get_domain(), *repository), std::runtime_error);

    const auto mismatched_load_register = read_fixture("kr/ps/ext/executor/ext_module_parser_rejects_invalid_sections/mismatched_load_register.module");
    EXPECT_THROW(kr::ps::ext::dl::parse_module(mismatched_load_register, planning_task.get_domain().get_domain(), *repository), std::runtime_error);

    EXPECT_THROW(kr::ps::ext::dl::parse_module_program(read_fixture("kr/ps/ext/executor/ext_module_parser_rejects_invalid_sections/case_14.program"),
                                                       planning_task.get_domain().get_domain(),
                                                       *repository),
                 std::runtime_error);
}

TEST(RunirTests, ExtModuleProgramParserRejectsInvalidProgramWiring)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_path("classical/tests/gripper/domain.pddl");
    const auto task_file = benchmark_path("classical/tests/gripper/test-1.pddl");
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    EXPECT_THROW(
        kr::ps::ext::dl::parse_module_program(read_fixture("kr/ps/ext/executor/ext_module_program_parser_rejects_invalid_program_wiring/case_2.program"),
                                              planning_task.get_domain().get_domain(),
                                              *repository),
        std::runtime_error);
    EXPECT_THROW(
        kr::ps::ext::dl::parse_module_program(read_fixture("kr/ps/ext/executor/ext_module_program_parser_rejects_invalid_program_wiring/case_3.program"),
                                              planning_task.get_domain().get_domain(),
                                              *repository),
        std::runtime_error);
    EXPECT_THROW(
        kr::ps::ext::dl::parse_module_program(read_fixture("kr/ps/ext/executor/ext_module_program_parser_rejects_invalid_program_wiring/auto11.program"),
                                              planning_task.get_domain().get_domain(),
                                              *repository),
        std::runtime_error);
}

TEST(RunirTests, ExtModuleParserReadsPaperFactoryDescriptions)
{
    const auto on = kr::ps::ext::dl::parser::parse_module_ast(kr::ps::ext::dl::ModuleFactory::create_on_bonet_et_al_icaps2024_description());
    EXPECT_EQ(on.name.text, "on");
    EXPECT_EQ(on.arguments.size(), 2);
    auto concept_register_count = size_t(0);
    auto role_register_count = size_t(0);
    for (const auto& reg : on.registers)
    {
        boost::apply_visitor(
            [&](const auto& concrete)
            {
                using Register = std::remove_cvref_t<decltype(concrete)>;
                if constexpr (std::same_as<Register, kr::ps::ext::dl::ast::Register<kr::dl::ConceptTag>>)
                    ++concept_register_count;
                else if constexpr (std::same_as<Register, kr::ps::ext::dl::ast::Register<kr::dl::RoleTag>>)
                    ++role_register_count;
            },
            reg.get());
    }
    EXPECT_EQ(concept_register_count, 2);
    EXPECT_EQ(role_register_count, 0);
    EXPECT_EQ(on.memory_states.size(), 9);
    EXPECT_EQ(on.rule_entries.size(), 14);
    const auto* stack_rule = boost::get<kr::ps::ext::dl::ast::DoRule>(&on.rule_entries.back().rules.front().get());
    ASSERT_NE(stack_rule, nullptr);
    EXPECT_EQ(stack_rule->action.text, "stack");
    ASSERT_EQ(stack_rule->arguments.size(), 2);
    EXPECT_EQ(stack_rule->arguments[0].symbol.text, "DO_on_8");

    const auto on_table = kr::ps::ext::dl::parser::parse_module_ast(kr::ps::ext::dl::ModuleFactory::create_on_table_bonet_et_al_icaps2024_description());
    EXPECT_EQ(on_table.name.text, "on-table");
    ASSERT_EQ(on_table.arguments.size(), 1);
    const auto* x_argument = boost::get<kr::ps::ext::dl::ast::Argument<kr::dl::ConceptTag>>(&on_table.arguments.front().get());
    ASSERT_NE(x_argument, nullptr);
    EXPECT_EQ(x_argument->symbol.text, "X");
    const auto* putdown_rule = boost::get<kr::ps::ext::dl::ast::DoRule>(&on_table.rule_entries.back().rules.front().get());
    ASSERT_NE(putdown_rule, nullptr);
    EXPECT_EQ(putdown_rule->action.text, "putdown");

    const auto tower = kr::ps::ext::dl::parser::parse_module_ast(kr::ps::ext::dl::ModuleFactory::create_tower_bonet_et_al_icaps2024_description());
    EXPECT_EQ(tower.name.text, "tower");
    EXPECT_EQ(tower.arguments.size(), 2);
    EXPECT_EQ(tower.rule_entries.size(), 4);
    const auto* tower_call = boost::get<kr::ps::ext::dl::ast::CallRule>(&tower.rule_entries[2].rules.front().get());
    ASSERT_NE(tower_call, nullptr);
    EXPECT_EQ(tower_call->callee.text, "on");
    ASSERT_EQ(tower_call->arguments.size(), 2);
    EXPECT_EQ(tower_call->arguments[1].symbol.text, "W");

    const auto blocks = kr::ps::ext::dl::parser::parse_module_ast(kr::ps::ext::dl::ModuleFactory::create_blocks_bonet_et_al_icaps2024_description());
    EXPECT_EQ(blocks.name.text, "blocks");
    EXPECT_EQ(blocks.arguments.size(), 1);
    EXPECT_EQ(blocks.rule_entries.size(), 2);
    const auto* blocks_call = boost::get<kr::ps::ext::dl::ast::CallRule>(&blocks.rule_entries[1].rules.front().get());
    ASSERT_NE(blocks_call, nullptr);
    EXPECT_EQ(blocks_call->callee.text, "tower");

    const auto program = kr::ps::ext::dl::parser::parse_module_program_ast(kr::ps::ext::dl::ModuleFactory::create_bonet_et_al_icaps2024_program_description());
    EXPECT_EQ(program.entry.text, "root");
    ASSERT_EQ(program.modules.size(), 5);
    EXPECT_EQ(program.modules[0].name.text, "root");
    const auto* root_call = boost::get<kr::ps::ext::dl::ast::CallRule>(&program.modules[0].rule_entries.front().rules.front().get());
    ASSERT_NE(root_call, nullptr);
    EXPECT_EQ(root_call->callee.text, "blocks");
}

TEST(RunirTests, ExtModuleParserLowersPaperFactoryDescriptionsAgainstBlocksworld)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_path("classical/profiling/blocksworld-large-simple/domain.pddl");
    const auto task_file = benchmark_path("classical/profiling/blocksworld-large-simple/p-300-4.pddl");
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto modules = kr::ps::ext::dl::parse_modules(kr::ps::ext::dl::ModuleFactory::create_bonet_et_al_icaps2024_descriptions(),
                                                        planning_task.get_domain().get_domain(),
                                                        *repository);
    const auto suite = load_fixture_json("kr/ps/ext/dl/module_factory.json");
    const auto& suite_object = ygg::common::as_object(suite, "suite");
    const auto& cases = ygg::common::as_array(ygg::common::require_member(suite_object, "modules", "suite"), "suite.modules");
    ASSERT_EQ(modules.size(), cases.size());
    for (std::size_t i = 0; i < modules.size(); ++i)
    {
        const auto& expected = ygg::common::as_object(cases[i], "module");
        const auto module = modules[i];
        EXPECT_EQ(module.get_name(), ygg::common::as_string(expected, "name", "module"));
        EXPECT_EQ(module.get_features<kr::dl::ConceptTag>().size(), ygg::common::as_size(expected, "concept_features", "module"));
        EXPECT_EQ(module.get_features<kr::dl::RoleTag>().size(), ygg::common::as_size(expected, "role_features", "module"));
        EXPECT_EQ(module.get_features<kr::ps::dl::BooleanFeature>().size(), ygg::common::as_size(expected, "boolean_features", "module"));
        EXPECT_EQ(module.get_features<kr::ps::dl::NumericalFeature>().size(), ygg::common::as_size(expected, "numerical_features", "module"));
        EXPECT_EQ(module.get_memory_transitions().size(), ygg::common::as_size(expected, "transitions", "module"));
        EXPECT_EQ(kr::ps::ext::syntactic_complexity(module), ygg::common::as_size(expected, "syntactic_complexity", "module"));
    }

    const auto program = kr::ps::ext::dl::ModuleFactory::create_bonet_et_al_icaps2024_program(planning_task.get_domain().get_domain(), *repository);
    const auto& expected_program = ygg::common::as_object(ygg::common::require_member(suite_object, "program", "suite"), "suite.program");
    EXPECT_EQ(program.get_entry_module().get_name(), ygg::common::as_string(expected_program, "entry", "program"));
    const auto& expected_modules = ygg::common::as_array(ygg::common::require_member(expected_program, "modules", "program"), "program.modules");
    ASSERT_EQ(program.get_modules().size(), expected_modules.size());
    for (std::size_t i = 0; i < expected_modules.size(); ++i)
        EXPECT_EQ(program.get_modules()[i].get_name(), ygg::common::as_string(expected_modules[i], "program.module"));
    EXPECT_EQ(kr::ps::ext::syntactic_complexity(program), ygg::common::as_size(expected_program, "syntactic_complexity", "program"));
}

TEST(RunirTests, ExtModuleFormatterRoundTripsPaperFactoryDescriptions)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_path("classical/profiling/blocksworld-large-simple/domain.pddl");
    const auto task_file = benchmark_path("classical/profiling/blocksworld-large-simple/p-300-4.pddl");
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto modules = kr::ps::ext::dl::parse_modules(kr::ps::ext::dl::ModuleFactory::create_bonet_et_al_icaps2024_descriptions(),
                                                        planning_task.get_domain().get_domain(),
                                                        *repository);
    ASSERT_EQ(modules.size(), 4);

    for (const auto module : modules)
    {
        const auto formatted = fmt::format("{}", module);
        try
        {
            const auto reparsed = kr::ps::ext::dl::parse_module(formatted, planning_task.get_domain().get_domain(), *repository);
            EXPECT_EQ(reparsed.get_name(), module.get_name());
            EXPECT_EQ(reparsed.get_memory_states().size(), module.get_memory_states().size());
            EXPECT_EQ(reparsed.get_memory_transitions().size(), module.get_memory_transitions().size());
            EXPECT_EQ(reparsed.get_arguments<kr::dl::ConceptTag>().size(), module.get_arguments<kr::dl::ConceptTag>().size());
            EXPECT_EQ(reparsed.get_arguments<kr::dl::RoleTag>().size(), module.get_arguments<kr::dl::RoleTag>().size());
            EXPECT_EQ(reparsed.get_arguments<kr::dl::BooleanTag>().size(), module.get_arguments<kr::dl::BooleanTag>().size());
            EXPECT_EQ(reparsed.get_arguments<kr::dl::NumericalTag>().size(), module.get_arguments<kr::dl::NumericalTag>().size());
        }
        catch (const std::exception& err)
        {
            FAIL() << "Failed to reparse formatted module " << module.get_name() << ": " << err.what() << "\n" << formatted;
        }
    }

    const auto program = kr::ps::ext::dl::ModuleFactory::create_bonet_et_al_icaps2024_program(planning_task.get_domain().get_domain(), *repository);
    const auto formatted_program = fmt::format("{}", program);
    EXPECT_NE(formatted_program.find(formatter_fragment("module_factory_sketch")), std::string::npos) << formatted_program;
    const auto reparsed_program = kr::ps::ext::dl::parse_module_program(formatted_program, planning_task.get_domain().get_domain(), *repository);
    EXPECT_EQ(reparsed_program.get_entry_module().get_name(), program.get_entry_module().get_name());
    EXPECT_EQ(reparsed_program.get_modules().size(), program.get_modules().size());
}

TEST(RunirTests, ExtModuleFormatterPreservesOrderedDeclarations)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_path("classical/tests/gripper/domain.pddl");
    const auto task_file = benchmark_path("classical/tests/gripper/test-1.pddl");
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto seed = read_fixture("kr/ps/ext/executor/ext_module_formatter_preserves_ordered_declarations/seed.module");
    const auto target = read_fixture("kr/ps/ext/executor/ext_module_formatter_preserves_ordered_declarations/target.module");

    kr::ps::ext::dl::parse_module(seed, planning_task.get_domain().get_domain(), *repository);
    const auto module = kr::ps::ext::dl::parse_module(target, planning_task.get_domain().get_domain(), *repository);

    const auto arguments = module.get_arguments<kr::dl::ConceptTag>();
    ASSERT_EQ(arguments.size(), 2);
    EXPECT_EQ(arguments[0].get_name(), "Y");
    EXPECT_EQ(arguments[1].get_name(), "X");

    const auto registers = module.get_registers<kr::dl::ConceptTag>();
    ASSERT_EQ(registers.size(), 2);
    EXPECT_EQ(registers[0].get_name(), "RY");
    EXPECT_EQ(registers[1].get_name(), "RX");

    const auto formatted = fmt::format("{}", module);
    auto reparsed_dl_repository = dl_repository_factory.create(planning_task.get_repository());
    auto reparsed_repository = repository_factory.create(reparsed_dl_repository);
    const auto reparsed = kr::ps::ext::dl::parse_module(formatted, planning_task.get_domain().get_domain(), *reparsed_repository);
    EXPECT_EQ(fmt::format("{}", reparsed), formatted);
    EXPECT_EQ(reparsed.get_arguments<kr::dl::ConceptTag>()[0].get_name(), "Y");
    EXPECT_EQ(reparsed.get_registers<kr::dl::ConceptTag>()[0].get_name(), "RY");
}

TEST(RunirTests, ExtModuleFormatterEscapesQuotedStringContents)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_path("classical/tests/gripper/domain.pddl");
    const auto task_file = benchmark_path("classical/tests/gripper/test-1.pddl");
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto description = read_fixture("kr/ps/ext/executor/ext_module_formatter_escapes_quoted_string_contents/description.module");

    const auto module = kr::ps::ext::dl::parse_module(description, planning_task.get_domain().get_domain(), *repository);
    const auto formatted = fmt::format("{}", module);
    EXPECT_NE(formatted.find("(:symbol entry)"), std::string::npos) << formatted;
    EXPECT_EQ(formatted.find(std::string(":") + "description"), std::string::npos) << formatted;

    const auto reparsed = kr::ps::ext::dl::parse_module(formatted, planning_task.get_domain().get_domain(), *repository);
    EXPECT_EQ(reparsed.get_name(), module.get_name());
    EXPECT_EQ(fmt::format("{}", reparsed), formatted);
}

TEST(RunirTests, ExtModuleFormatterOmitsEmptyNestedRuleMetadata)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_path("classical/tests/gripper/domain.pddl");
    const auto task_file = benchmark_path("classical/tests/gripper/test-1.pddl");
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto description = read_fixture("kr/ps/ext/executor/ext_module_formatter_omits_empty_nested_rule_metadata/description.module");

    const auto module = kr::ps::ext::dl::parse_module(description, planning_task.get_domain().get_domain(), *repository);
    const auto formatted = fmt::format("{}", module);

    EXPECT_EQ(formatted.find("(:symbol )"), std::string::npos) << formatted;
    EXPECT_EQ(formatted.find(formatter_fragment("empty_nested_do_symbol")), std::string::npos) << formatted;
    EXPECT_EQ(formatted.find(std::string(":") + "description"), std::string::npos) << formatted;
    EXPECT_NE(formatted.find(formatter_fragment("concept_declaration")), std::string::npos) << formatted;
    EXPECT_NE(formatted.find(formatter_fragment("rule_declaration")), std::string::npos) << formatted;
    EXPECT_NE(formatted.find("(:arguments Any Any)"), std::string::npos) << formatted;

    const auto reparsed = kr::ps::ext::dl::parse_module(formatted, planning_task.get_domain().get_domain(), *repository);
    EXPECT_EQ(fmt::format("{}", reparsed), formatted);
}

TEST(RunirTests, ExtModuleParserLowersSupportedTransitions)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_path("classical/tests/gripper/domain.pddl");
    const auto task_file = benchmark_path("classical/tests/gripper/test-1.pddl");
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto description = read_fixture("kr/ps/ext/executor/ext_module_parser_lowers_supported_transitions/description.module");

    const auto module = kr::ps::ext::dl::parse_module(description, planning_task.get_domain().get_domain(), *repository);
    ASSERT_EQ(module.get_memory_transitions().size(), 3);

    const auto load_rules = module.get_memory_transitions()[0];
    ASSERT_EQ(load_rules.size(), 1);
    EXPECT_EQ(load_rules[0].get_symbol(), "load-edge");
    EXPECT_TRUE(ygg::visit(
        [](auto rule)
        {
            using View = std::decay_t<decltype(rule)>;
            using Expected = ygg::View<ygg::Index<kr::ps::ext::Rule<kr::ps::ext::LoadTag<kr::dl::ConceptTag>>>, kr::ps::ext::Repository>;
            if constexpr (std::same_as<View, Expected>)
                return ygg::uint_t(rule.get_register().get_identifier()) == 0 && rule.get_conditions().size() == 1;
            else
                return false;
        },
        load_rules[0].get_variant()));

    const auto sketch_rules = module.get_memory_transitions()[1];
    ASSERT_EQ(sketch_rules.size(), 1);

    const auto do_rules = module.get_memory_transitions()[2];
    ASSERT_EQ(do_rules.size(), 1);
    EXPECT_TRUE(ygg::visit(
        [](auto rule)
        {
            using View = std::decay_t<decltype(rule)>;
            using Expected = ygg::View<ygg::Index<kr::ps::ext::Rule<kr::ps::ext::DoTag>>, kr::ps::ext::Repository>;
            if constexpr (std::same_as<View, Expected>)
                return rule.get_action_name() == "pick" && rule.get_action_arguments().size() == 3 && rule.get_effects().size() == 1;
            else
                return false;
        },
        do_rules[0].get_variant()));
}

TEST(RunirTests, ExtModuleParserLowersExtDlConceptAndRoleExpressions)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_path("classical/tests/gripper/domain.pddl");
    const auto task_file = benchmark_path("classical/tests/gripper/test-1.pddl");
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>();
    auto dl_repository = dl_repository_factory.create(planning_task.get_repository());

    const auto concept_argument =
        kr::ps::ext::dl::parse_concept(read_fixture("kr/ps/ext/executor/ext_module_parser_lowers_ext_dl_concept_and_role_expressions/case_1.dsl"),
                                       planning_task.get_domain().get_domain(),
                                       *dl_repository);
    EXPECT_TRUE(ygg::visit(
        [](auto child)
        {
            using View = std::decay_t<decltype(child)>;
            using Expected = ygg::View<ygg::Index<kr::dl::Concept<kr::ExtFamilyTag, kr::dl::ArgumentTag<kr::dl::ConceptTag>>>,
                                       kr::dl::ConstructorRepositoryFor<kr::ExtFamilyTag>>;
            if constexpr (std::same_as<View, Expected>)
                return ygg::uint_t(child.get_argument().get_identifier()) == 0;
            else
                return false;
        },
        concept_argument.get_variant()));

    const auto concept_register =
        kr::ps::ext::dl::parse_concept(read_fixture("kr/ps/ext/executor/ext_module_parser_lowers_ext_dl_concept_and_role_expressions/case_2.dsl"),
                                       planning_task.get_domain().get_domain(),
                                       *dl_repository);
    EXPECT_TRUE(ygg::visit(
        [](auto child)
        {
            using View = std::decay_t<decltype(child)>;
            using Expected = ygg::View<ygg::Index<kr::dl::Concept<kr::ExtFamilyTag, kr::dl::RegisterTag>>, kr::dl::ConstructorRepositoryFor<kr::ExtFamilyTag>>;
            if constexpr (std::same_as<View, Expected>)
                return ygg::uint_t(child.get_register().get_identifier()) == 1;
            else
                return false;
        },
        concept_register.get_variant()));

    const auto role_argument =
        kr::ps::ext::dl::parse_role(read_fixture("kr/ps/ext/executor/ext_module_parser_lowers_ext_dl_concept_and_role_expressions/case_3.dsl"),
                                    planning_task.get_domain().get_domain(),
                                    *dl_repository);
    EXPECT_TRUE(ygg::visit(
        [](auto child)
        {
            using View = std::decay_t<decltype(child)>;
            using Expected =
                ygg::View<ygg::Index<kr::dl::Role<kr::ExtFamilyTag, kr::dl::ArgumentTag<kr::dl::RoleTag>>>, kr::dl::ConstructorRepositoryFor<kr::ExtFamilyTag>>;
            if constexpr (std::same_as<View, Expected>)
                return ygg::uint_t(child.get_argument().get_identifier()) == 0;
            else
                return false;
        },
        role_argument.get_variant()));

    EXPECT_NO_THROW(kr::ps::ext::dl::parse_concept(read_fixture("kr/ps/ext/executor/ext_module_parser_lowers_ext_dl_concept_and_role_expressions/case_4.dsl"),
                                                   planning_task.get_domain().get_domain(),
                                                   *dl_repository));
    EXPECT_NO_THROW(kr::ps::ext::dl::parse_role(read_fixture("kr/ps/ext/executor/ext_module_parser_lowers_ext_dl_concept_and_role_expressions/case_5.dsl"),
                                                planning_task.get_domain().get_domain(),
                                                *dl_repository));
    EXPECT_NO_THROW(kr::ps::ext::dl::parse_boolean(read_fixture("kr/ps/ext/executor/ext_module_parser_lowers_ext_dl_concept_and_role_expressions/case_6.dsl"),
                                                   planning_task.get_domain().get_domain(),
                                                   *dl_repository));
    EXPECT_NO_THROW(kr::ps::ext::dl::parse_numerical(read_fixture("kr/ps/ext/executor/ext_module_parser_lowers_ext_dl_concept_and_role_expressions/case_7.dsl"),
                                                     planning_task.get_domain().get_domain(),
                                                     *dl_repository));
}

TEST(RunirTests, RejectsUnknownModuleFactorySpecification)
{
    EXPECT_THROW(
        try {
            static_cast<void>(kr::ps::ext::dl::ModuleFactory::create_description(static_cast<kr::ps::ext::dl::ModuleSpecification>(999)));
        } catch (const std::runtime_error& error) {
            EXPECT_STREQ(error.what(), "Unknown module specification: 999.");
            throw;
        },
        std::runtime_error);
}

TEST(RunirTests, ExtModuleFactoryCreatesEmptyModule)
{
    namespace fp = tyr::formalism::planning;

    const auto planning_domain = fp::Parser(benchmark_path("classical/profiling/blocksworld-large-simple/domain.pddl")).get_domain();

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(planning_domain.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto empty = kr::ps::ext::dl::ModuleFactory::create_empty(*repository);
    EXPECT_EQ(empty.get_name(), "empty");
    EXPECT_EQ(empty.get_entry_memory_state().get_name(), "m0");
    EXPECT_EQ(empty.get_memory_states().size(), 1);
    EXPECT_EQ(kr::ps::ext::syntactic_complexity(empty), 0);

    const auto description = kr::ps::ext::dl::ModuleFactory::create_empty_description();
    const auto reparsed = kr::ps::ext::dl::parse_module(description, planning_domain.get_domain(), *repository);
    EXPECT_EQ(reparsed.get_name(), empty.get_name());
    EXPECT_EQ(reparsed.get_entry_memory_state().get_name(), empty.get_entry_memory_state().get_name());
    EXPECT_EQ(reparsed.get_memory_states().size(), empty.get_memory_states().size());
    EXPECT_EQ(kr::ps::ext::syntactic_complexity(reparsed), 0);
}

}  // namespace runir::tests
