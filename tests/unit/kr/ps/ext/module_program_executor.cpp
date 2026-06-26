#include <boost/variant/get.hpp>
#include <filesystem>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <runir/datasets/task_class.hpp>
#include <runir/kr/dl/semantics/builder.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/ps/ext/detail/execution.hpp>
#include <runir/kr/ps/ext/dl/module_factory.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <runir/kr/ps/ext/evaluation.hpp>
#include <runir/kr/ps/ext/evaluation_context.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/module_program_executor.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/ext/successor_expander.hpp>
#include <type_traits>
#include <tyr/formalism/planning/parser.hpp>
#include <tyr/planning/planning.hpp>
#include <variant>
#include <vector>
#include <yggdrasil/execution/onetbb.hpp>

namespace runir::tests
{
namespace
{

std::filesystem::path benchmark_prefix() { return std::filesystem::path(RUNIR_ROOT_DIR) / "data" / "planning-benchmarks"; }

auto create_memory_state(kr::ps::ext::Repository& repository, const std::string& name)
{
    auto data = ygg::Data<kr::ps::ext::MemoryState>(name);
    return repository.get_or_create(data).first;
}

auto create_module(kr::ps::ext::Repository& repository,
                   const std::string& name,
                   kr::ps::ext::MemoryStateView entry,
                   std::initializer_list<kr::ps::ext::MemoryStateView> memory_states)
{
    auto data = ygg::Data<kr::ps::ext::Module>(name);
    data.entry_memory_state = entry.get_index();
    for (auto state : memory_states)
        data.memory_states.push_back(state.get_index());
    kr::ps::ext::canonicalize(data);
    return repository.get_or_create(data).first;
}

auto create_module_program(kr::ps::ext::Repository& repository, kr::ps::ext::ModuleView entry, std::initializer_list<kr::ps::ext::ModuleView> modules)
{
    auto data = ygg::Data<kr::ps::ext::ModuleProgram>();
    data.entry_module = entry.get_index();
    for (auto module : modules)
        data.modules.push_back(module.get_index());
    kr::ps::ext::canonicalize(data);
    return repository.get_or_create(data).first;
}

auto create_register(kr::ps::ext::Repository& repository, const std::string& name, ygg::uint_t identifier)
{
    auto data = ygg::Data<kr::ps::ext::Register<kr::dl::ConceptTag>>(name, kr::dl::RegisterIdentifier<kr::dl::ConceptTag>(identifier));
    return repository.get_or_create(data).first;
}

auto create_top_concept(kr::dl::ext::ConstructorRepository& repository)
{
    auto top_data = ygg::Data<kr::dl::Concept<kr::ExtFamilyTag, kr::dl::TopTag>>();
    const auto top = repository.get_or_create(top_data).first;
    auto constructor_data = ygg::Data<kr::dl::Constructor<kr::ExtFamilyTag, kr::dl::ConceptTag>>(top.get_index());
    return repository.get_or_create(constructor_data).first;
}

auto create_concept_feature(kr::ps::ext::Repository& repository, kr::ps::ext::ConceptArgument concept_index, const std::string& name)
{
    auto concrete_data = ygg::Data<kr::ps::ext::ConcreteFeature<kr::DlTag, kr::dl::ConceptTag>>(concept_index, name);
    const auto concrete = repository.get_or_create(concrete_data).first;
    auto feature_data = ygg::Data<kr::ps::ext::Feature<kr::dl::ConceptTag>>(concrete.get_index());
    return repository.get_or_create(feature_data).first;
}

auto create_concept_argument(kr::dl::ext::ConstructorRepository& repository, ygg::uint_t identifier)
{
    auto argument_data =
        ygg::Data<kr::dl::Concept<kr::ExtFamilyTag, kr::dl::ArgumentTag<kr::dl::ConceptTag>>>(kr::dl::ArgumentIdentifier<kr::dl::ConceptTag>(identifier));
    const auto argument = repository.get_or_create(argument_data).first;
    auto constructor_data = ygg::Data<kr::dl::Constructor<kr::ExtFamilyTag, kr::dl::ConceptTag>>(argument.get_index());
    return repository.get_or_create(constructor_data).first;
}

}  // namespace

TEST(RunirTests, ExtModuleParserLowersArgumentRegisterMemorySections)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto description = R"((:module
    (:symbol entry)
    (:arguments
        (:concept X)
        (:role O)
        (:boolean B)
        (:numerical N)
    )
    (:registers
        (:concept r0)
        (:role r0)
    )
    (:entry m0)
    (:memory m0 m1)
    (:features
        (:concept
            (:symbol linked)
            (:expression
                (c_some
                    (r_register r0)
                    (c_top)
                )
            )
        )
    )
    (:rules)
))";

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

    const auto blocksworld_domain = benchmark_prefix() / "profiling" / "htg" / "blocksworld-large-simple" / "domain.pddl";
    const auto blocksworld_task_file = benchmark_prefix() / "profiling" / "htg" / "blocksworld-large-simple" / "p-300-4.pddl";
    const auto blocksworld_task = fp::Parser(blocksworld_domain).parse_task(blocksworld_task_file);
    auto blocksworld_dl_repository = dl_repository_factory.create(blocksworld_task.get_repository());
    auto blocksworld_repository = repository_factory.create(blocksworld_dl_repository);

    const auto blocks = kr::ps::ext::dl::parse_module(kr::ps::ext::dl::ModuleFactory::create_blocks_bonet_et_al_icaps2024_description(),
                                                      blocksworld_task.get_domain().get_domain(),
                                                      *blocksworld_repository);
    EXPECT_EQ(blocks.get_name(), "blocks");
    ASSERT_GT(blocks.get_memory_transitions().size(), 1);
    const auto block_rules = blocks.get_memory_transitions()[1];
    ASSERT_FALSE(block_rules.empty());
    auto call_rule = block_rules.front();
    auto found_call_rule = false;
    ygg::visit(
        [&](auto rule)
        {
            using RuleView = std::decay_t<decltype(rule)>;
            if constexpr (std::same_as<RuleView, kr::ps::ext::RuleView<kr::ps::ext::CallTag>>)
            {
                found_call_rule = true;
                EXPECT_EQ(rule.get_callee_name(), "tower");
            }
        },
        call_rule.get_variant());
    EXPECT_TRUE(found_call_rule) << "Expected call rule.";
}

TEST(RunirTests, ExtModuleParserLowersNamedCalleesWithoutPreexistingModules)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto caller_description = R"((:module
    (:symbol caller)
    (:arguments)
    (:registers)
    (:entry m0)
    (:memory m0 m1)
    (:features)
    (:rules
        (:rule
            (:symbol auto1)
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:call
                    (:conditions)
                    (:callee callee)
                    (:arguments)
                )
            )
        )
    )
))";
    const auto callee_description = R"((:module
    (:symbol callee)
    (:arguments)
    (:registers)
    (:entry m0)
    (:memory m0)
    (:features)
    (:rules)
))";

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
                EXPECT_EQ(rule.get_callee_name(), "callee");
            }
        },
        rules.front().get_variant());
    EXPECT_TRUE(found_call_rule) << "Expected call rule.";
}

TEST(RunirTests, ExtModuleParserRejectsInvalidModuleSets)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto caller_with_argument = R"((:module
    (:symbol caller)
    (:arguments)
    (:registers)
    (:entry m0)
    (:memory m0 m1)
    (:features
        (:concept
            (:symbol Any)
            (:expression
                (c_top)
            )
        )
    )
    (:rules
        (:rule
            (:symbol auto2)
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:call
                    (:conditions)
                    (:callee callee)
                    (:arguments Any)
                )
            )
        )
    )
))";
    const auto callee_without_arguments = R"((:module
    (:symbol callee)
    (:arguments)
    (:registers)
    (:entry m0)
    (:memory m0)
    (:features)
    (:rules)
))";
    EXPECT_THROW(kr::ps::ext::dl::parse_modules({ caller_with_argument, callee_without_arguments }, planning_task.get_domain().get_domain(), *repository),
                 std::runtime_error);

    const auto duplicate_callee = R"((:module
    (:symbol callee)
    (:arguments)
    (:registers)
    (:entry m0)
    (:memory m0)
    (:features)
    (:rules)
))";
    EXPECT_THROW(kr::ps::ext::dl::parse_modules({ callee_without_arguments, duplicate_callee }, planning_task.get_domain().get_domain(), *repository),
                 std::runtime_error);
}

TEST(RunirTests, ExtModuleParserRejectsInvalidDoActions)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto unknown_action = R"((:module
    (:symbol entry)
    (:arguments)
    (:registers)
    (:entry m0)
    (:memory m0 m1)
    (:features)
    (:rules
        (:rule
            (:symbol auto3)
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:do
                    (:conditions)
                    (:action "missing-action")
                    (:arguments)
                )
            )
        )
    )
))";
    EXPECT_THROW(kr::ps::ext::dl::parse_module(unknown_action, planning_task.get_domain().get_domain(), *repository), std::runtime_error);

    const auto wrong_arity = std::string(R"((:module
    (:symbol entry)
    (:arguments)
    (:registers)
    (:entry m0)
    (:memory m0 m1)
    (:features)
    (:rules
        (:rule
            (:symbol auto5)
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:do
                    (:conditions)
                    (:action "move")
                    (:arguments)
                    (:effects)
                )
            )
        )
    )
))");
    try
    {
        [[maybe_unused]] const auto module = kr::ps::ext::dl::parse_module(wrong_arity, planning_task.get_domain().get_domain(), *repository);
        FAIL() << "Expected wrong do-action arity to be rejected.";
    }
    catch (const std::runtime_error& err)
    {
        EXPECT_NE(std::string(err.what()).find("Do rule for action \"move\" has 0 arguments; expected 2"), std::string::npos) << err.what();
    }

    const auto undeclared_argument_feature = R"((:module
    (:symbol entry)
    (:arguments)
    (:registers)
    (:entry m0)
    (:memory m0 m1)
    (:features
        (:concept
            (:symbol B)
            (:expression
                (c_top)
            )
        )
    )
    (:rules
        (:rule
            (:symbol auto7)
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:do
                    (:conditions)
                    (:action "pick")
                    (:arguments B Missing B)
                    (:effects)
                )
            )
        )
    )
))";
    try
    {
        [[maybe_unused]] const auto module = kr::ps::ext::dl::parse_module(undeclared_argument_feature, planning_task.get_domain().get_domain(), *repository);
        FAIL() << "Expected undeclared do-argument concept feature to be rejected.";
    }
    catch (const std::runtime_error& err)
    {
        EXPECT_NE(std::string(err.what()).find("Unknown feature \"Missing\""), std::string::npos) << err.what();
    }

    const auto inline_argument_expression = R"((:module
    (:symbol entry)
    (:arguments)
    (:registers)
    (:entry m0)
    (:memory m0 m1)
    (:features
        (:concept
            (:symbol B)
            (:expression
                (c_top)
            )
        )
    )
    (:rules
        (:rule
            (:symbol auto9)
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:do
                    (:conditions)
                    (:action "pick")
                    (:arguments
                        (c_top)
                        B
                        B
                    )
                    (:effects)
                )
            )
        )
    )
))";
    try
    {
        [[maybe_unused]] const auto module = kr::ps::ext::dl::parse_module(inline_argument_expression, planning_task.get_domain().get_domain(), *repository);
        FAIL() << "Expected inline do-argument expression to be rejected.";
    }
    catch (const std::runtime_error& err)
    {
        EXPECT_NE(std::string(err.what()).find("Do-rule action arguments must reference declared concept features by symbol"), std::string::npos) << err.what();
    }
}

TEST(RunirTests, ExtModuleParserRejectsInvalidSections)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
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

    const auto missing_transitions = R"((:module
    (:symbol entry)
    (:arguments)
    (:registers)
    (:entry m0)
    (:memory m0)
    (:features)
))";
    EXPECT_THROW(kr::ps::ext::dl::parse_module(missing_transitions, planning_task.get_domain().get_domain(), *repository), std::runtime_error);

    const auto duplicate_memory = R"((:module
    (:symbol entry)
    (:arguments)
    (:registers)
    (:entry m0)
    (:memory m0)
    (:memory m1)
    (:features)
    (:rules)
))";
    EXPECT_THROW(kr::ps::ext::dl::parse_module(duplicate_memory, planning_task.get_domain().get_domain(), *repository), std::runtime_error);

    const auto empty_memory = R"((:module
    (:symbol entry)
    (:arguments)
    (:registers)
    (:entry m0)
    (:memory)
    (:features)
    (:rules)
))";
    EXPECT_THROW(kr::ps::ext::dl::parse_module(empty_memory, planning_task.get_domain().get_domain(), *repository), std::runtime_error);

    const auto duplicate_register_identifier = R"((:module
    (:symbol entry)
    (:arguments)
    (:registers
        (:concept r0)
        (:concept r0)
    )
    (:entry m0)
    (:memory m0)
    (:features)
    (:rules)
))";
    expect_parse_error_containing(duplicate_register_identifier, "Duplicate concept register name \"r0\".");

    const auto duplicate_argument_identifier = R"((:module
    (:symbol entry)
    (:arguments
        (:concept x)
        (:concept x)
    )
    (:registers)
    (:entry m0)
    (:memory m0)
    (:features)
    (:rules)
))";
    expect_parse_error_containing(duplicate_argument_identifier, "Duplicate concept argument name \"x\".");

    const auto out_of_range_argument = R"((:module
    (:symbol entry)
    (:arguments
        (:concept x)
    )
    (:registers)
    (:entry m0)
    (:memory m0)
    (:features
        (:concept
            (:symbol Bad)
            (:expression
                (c_argument missing)
            )
        )
    )
    (:rules)
))";
    expect_parse_error_containing(out_of_range_argument, "Unknown concept argument \"missing\".");

    const auto out_of_range_expression_argument = std::string(R"((:module
    (:symbol entry)
    (:arguments)
    (:registers)
    (:entry m0)
    (:memory m0)
    (:features
        (:concept
            (:symbol X)
            (:expression
                (c_argument missing)
            )
        )
    )
    (:rules)
))");
    try
    {
        [[maybe_unused]] const auto module =
            kr::ps::ext::dl::parse_module(out_of_range_expression_argument, planning_task.get_domain().get_domain(), *repository);
        FAIL() << "Expected unknown expression argument to be rejected.";
    }
    catch (const std::runtime_error& err)
    {
        EXPECT_NE(std::string(err.what()).find("Unknown concept argument \"missing\""), std::string::npos) << err.what();
    }

    const auto undeclared_expression_register = R"((:module
    (:symbol entry)
    (:arguments)
    (:registers
        (:concept r0)
    )
    (:entry m0)
    (:memory m0)
    (:features
        (:concept
            (:symbol X)
            (:expression
                (c_register missing)
            )
        )
    )
    (:rules)
))";
    EXPECT_THROW(kr::ps::ext::dl::parse_module(undeclared_expression_register, planning_task.get_domain().get_domain(), *repository), std::runtime_error);

    const auto duplicate_rule_section = R"((:module
    (:symbol entry)
    (:arguments)
    (:registers
        (:concept r0)
    )
    (:entry m0)
    (:memory m0 m1)
    (:features)
    (:rules
        (:rule
            (:symbol auto7)
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:load
                    (:conditions)
                    (:conditions)
                    (:concept
                        (c_top)
                    )
                    (:register
                        (:concept r0)
                    )
                )
            )
        )
    )
))";
    EXPECT_THROW(kr::ps::ext::dl::parse_module(duplicate_rule_section, planning_task.get_domain().get_domain(), *repository), std::runtime_error);

    const auto invalid_rule_section = R"((:module
    (:symbol entry)
    (:arguments)
    (:registers
        (:concept r0)
    )
    (:entry m0)
    (:memory m0 m1)
    (:features)
    (:rules
        (:rule
            (:symbol auto9)
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:load
                    (:conditions)
                    (:concept
                        (c_top)
                    )
                    (:register
                        (:concept r0)
                    )
                    (:action "move")
                )
            )
        )
    )
))";
    EXPECT_THROW(kr::ps::ext::dl::parse_module(invalid_rule_section, planning_task.get_domain().get_domain(), *repository), std::runtime_error);

    const auto flat_load_register = R"((:module
    (:symbol entry)
    (:arguments)
    (:registers
        (:concept r0)
    )
    (:entry m0)
    (:memory m0 m1)
    (:features)
    (:rules
        (:rule
            (:symbol auto11)
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:load
                    (:conditions)
                    (:concept
                        (c_top)
                    )
                    (:register r0)
                )
            )
        )
    )
))";
    EXPECT_THROW(kr::ps::ext::dl::parse_module(flat_load_register, planning_task.get_domain().get_domain(), *repository), std::runtime_error);

    const auto mismatched_load_register = R"((:module
    (:symbol entry)
    (:arguments)
    (:registers
        (:concept r0)
    )
    (:entry m0)
    (:memory m0 m1)
    (:features)
    (:rules
        (:rule
            (:symbol auto12)
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:load
                    (:conditions)
                    (:concept
                        (c_top)
                    )
                    (:register
                        (:role r0)
                    )
                )
            )
        )
    )
))";
    EXPECT_THROW(kr::ps::ext::dl::parse_module(mismatched_load_register, planning_task.get_domain().get_domain(), *repository), std::runtime_error);

    const auto root = R"((:module
    (:symbol root)
    (:arguments)
    (:registers)
    (:entry m0)
    (:memory m0)
    (:features)
    (:rules)
))";
    EXPECT_THROW(kr::ps::ext::dl::parse_module_program(R"(
(:program
  (:entry root)
  (:entry root)
)" + std::string(root) + ")",
                                                       planning_task.get_domain().get_domain(),
                                                       *repository),
                 std::runtime_error);
}

TEST(RunirTests, ExtModuleProgramParserRejectsInvalidProgramWiring)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto root = R"((:module
    (:symbol root)
    (:arguments)
    (:registers)
    (:entry m0)
    (:memory m0)
    (:features)
    (:rules)
))";

    EXPECT_THROW(kr::ps::ext::dl::parse_module_program(R"(
(:program
  (:entry missing)
)" + std::string(root) + ")",
                                                       planning_task.get_domain().get_domain(),
                                                       *repository),
                 std::runtime_error);
    EXPECT_THROW(kr::ps::ext::dl::parse_module_program(R"(
(:program
  (:entry root)
)" + std::string(root) + std::string(root) + ")",
                                                       planning_task.get_domain().get_domain(),
                                                       *repository),
                 std::runtime_error);
    EXPECT_THROW(kr::ps::ext::dl::parse_module_program(R"((:program
    (:entry root)
    (:module
        "root"
        (:arguments)
        (:registers)
        (:entry m0)
        (:memory m0 m1)
        (:features)
        (:rules
            (:rule
                (:symbol auto11)
                (:expression
                    (:source-memory m0)
                    (:target-memory m1)
                    (:call
                        (:expression
                            (:conditions)
                            (:callee missing)
                            (:arguments)
                        )
                    )
                )
            )
        )
    )
))",
                                                       planning_task.get_domain().get_domain(),
                                                       *repository),
                 std::runtime_error);
}

TEST(RunirTests, ExtModuleParserReadsPaperFactoryDescriptions)
{
    const auto on = kr::ps::ext::dl::parser::parse_module_ast(kr::ps::ext::dl::ModuleFactory::create_on_bonet_et_al_icaps2024_description());
    EXPECT_EQ(on.name, "on");
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
    EXPECT_EQ(stack_rule->action, "stack");
    ASSERT_EQ(stack_rule->arguments.size(), 2);
    const auto* stack_arg = boost::get<kr::ps::ext::dl::ast::SymbolExpression>(&stack_rule->arguments[0].get());
    ASSERT_NE(stack_arg, nullptr);
    EXPECT_EQ(stack_arg->symbol, "DO_on_8");

    const auto tower = kr::ps::ext::dl::parser::parse_module_ast(kr::ps::ext::dl::ModuleFactory::create_tower_bonet_et_al_icaps2024_description());
    EXPECT_EQ(tower.name, "tower");
    EXPECT_EQ(tower.arguments.size(), 2);
    EXPECT_EQ(tower.rule_entries.size(), 4);
    const auto* tower_call = boost::get<kr::ps::ext::dl::ast::CallRule>(&tower.rule_entries[2].rules.front().get());
    ASSERT_NE(tower_call, nullptr);
    EXPECT_EQ(tower_call->callee, "on");
    ASSERT_EQ(tower_call->arguments.size(), 2);
    const auto* tower_arg = boost::get<kr::ps::ext::dl::ast::SymbolExpression>(&tower_call->arguments[1].get());
    ASSERT_NE(tower_arg, nullptr);
    EXPECT_EQ(tower_arg->symbol, "W");

    const auto blocks = kr::ps::ext::dl::parser::parse_module_ast(kr::ps::ext::dl::ModuleFactory::create_blocks_bonet_et_al_icaps2024_description());
    EXPECT_EQ(blocks.name, "blocks");
    EXPECT_EQ(blocks.arguments.size(), 1);
    EXPECT_EQ(blocks.rule_entries.size(), 2);
    const auto* blocks_call = boost::get<kr::ps::ext::dl::ast::CallRule>(&blocks.rule_entries[1].rules.front().get());
    ASSERT_NE(blocks_call, nullptr);
    EXPECT_EQ(blocks_call->callee, "tower");

    const auto program = kr::ps::ext::dl::parser::parse_module_program_ast(kr::ps::ext::dl::ModuleFactory::create_bonet_et_al_icaps2024_program_description());
    EXPECT_EQ(program.entry, "root");
    ASSERT_EQ(program.modules.size(), 5);
    EXPECT_EQ(program.modules[0].name, "root");
    const auto* root_call = boost::get<kr::ps::ext::dl::ast::CallRule>(&program.modules[0].rule_entries.front().rules.front().get());
    ASSERT_NE(root_call, nullptr);
    EXPECT_EQ(root_call->callee, "blocks");
}

TEST(RunirTests, ExtModuleParserLowersPaperFactoryDescriptionsAgainstBlocksworld)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_prefix() / "profiling" / "htg" / "blocksworld-large-simple" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "profiling" / "htg" / "blocksworld-large-simple" / "p-300-4.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto modules = kr::ps::ext::dl::parse_modules(kr::ps::ext::dl::ModuleFactory::create_bonet_et_al_icaps2024_descriptions(),
                                                        planning_task.get_domain().get_domain(),
                                                        *repository);
    ASSERT_EQ(modules.size(), 4);
    EXPECT_EQ(modules[0].get_name(), "on");
    EXPECT_EQ(modules[1].get_name(), "on-table");
    EXPECT_EQ(modules[2].get_name(), "tower");
    EXPECT_EQ(modules[3].get_name(), "blocks");

    const auto program = kr::ps::ext::dl::ModuleFactory::create_bonet_et_al_icaps2024_program(planning_task.get_domain().get_domain(), *repository);
    EXPECT_EQ(program.get_entry_module().get_name(), "root");
    EXPECT_EQ(program.get_modules().size(), 5);
}

TEST(RunirTests, ExtModuleFormatterRoundTripsPaperFactoryDescriptions)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_prefix() / "profiling" / "htg" / "blocksworld-large-simple" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "profiling" / "htg" / "blocksworld-large-simple" / "p-300-4.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
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
    const auto reparsed_program = kr::ps::ext::dl::parse_module_program(formatted_program, planning_task.get_domain().get_domain(), *repository);
    EXPECT_EQ(reparsed_program.get_entry_module().get_name(), program.get_entry_module().get_name());
    EXPECT_EQ(reparsed_program.get_modules().size(), program.get_modules().size());
}

TEST(RunirTests, ExtModuleFormatterEscapesQuotedStringContents)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto description = R"RUNIR((:module
    (:symbol entry)
    (:arguments)
    (:registers)
    (:entry m0)
    (:memory m0)
    (:features)
    (:rules)
))RUNIR";

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

    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto description = R"RUNIR((:module
    (:symbol entry)
    (:arguments)
    (:registers)
    (:entry m0)
    (:memory m0 m1)
    (:features
        (:concept
            (:symbol Any)
            (:expression
                (c_top)
            )
        )
    )
    (:rules
        (:rule
            (:symbol move-once)
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:do
                    (:conditions)
                    (:action "move")
                    (:arguments Any Any)
                    (:effects)
                )
            )
        )
    )
))RUNIR";

    const auto module = kr::ps::ext::dl::parse_module(description, planning_task.get_domain().get_domain(), *repository);
    const auto formatted = fmt::format("{}", module);

    EXPECT_EQ(formatted.find("(:symbol )"), std::string::npos) << formatted;
    EXPECT_EQ(formatted.find("(:do\n                        (:symbol"), std::string::npos) << formatted;
    EXPECT_EQ(formatted.find(std::string(":") + "description"), std::string::npos) << formatted;
    EXPECT_NE(formatted.find("(:concept\n            (:symbol Any)\n            (:expression (c_top))"), std::string::npos) << formatted;
    EXPECT_NE(formatted.find("(:rule\n            (:symbol move-once)\n            (:expression"), std::string::npos) << formatted;
    EXPECT_NE(formatted.find("(:arguments Any Any)"), std::string::npos) << formatted;

    const auto reparsed = kr::ps::ext::dl::parse_module(formatted, planning_task.get_domain().get_domain(), *repository);
    EXPECT_EQ(fmt::format("{}", reparsed), formatted);
}

TEST(RunirTests, ExtPaperModulesExecuteOnSmallBlocksworldInstance)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "profiling" / "htg" / "blocksworld-large-simple" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "profiling" / "htg" / "blocksworld-large-simple" / "p-100-2.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(task->get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto program = kr::ps::ext::dl::ModuleFactory::create_bonet_et_al_icaps2024_program(planning_task.get_domain().get_domain(), *repository);
    ASSERT_EQ(program.get_modules().size(), 5);

    auto search_options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag>();
    search_options.max_arity = 1;

    const auto search_result = kr::ps::ext::find_solution(search_context, program, search_options);
    EXPECT_TRUE(search_result.is_successful());
    ASSERT_TRUE(search_result.plan.has_value());
    EXPECT_EQ(search_result.plan->get_length(), 4);

    const auto proof = kr::ps::ext::prove_solution(search_context, program, search_options);
    EXPECT_EQ(proof.status, kr::ps::ext::ModuleProgramProofStatus::SUCCESS) << fmt::format("{}", proof);
    ASSERT_TRUE(proof.graph);
    EXPECT_EQ(proof.graph->get_num_vertices(), 16);
    EXPECT_EQ(proof.graph->get_num_edges(), 15);

    auto has_internal_memory_state = false;
    auto has_external_memory_state = false;
    for (const auto vertex : proof.graph->get_vertex_indices())
    {
        const auto& memory_state = proof.graph->get_vertex(vertex).get_property().extended_state.memory_state;
        has_internal_memory_state |= std::holds_alternative<kr::ps::ext::InternalMemoryState>(memory_state);
        has_external_memory_state |= std::holds_alternative<kr::ps::ext::ExternalMemoryState>(memory_state);
    }
    EXPECT_TRUE(has_internal_memory_state);
    EXPECT_TRUE(has_external_memory_state);

    EXPECT_TRUE(proof.deadend_transitions.empty());
    EXPECT_TRUE(proof.open_states.empty());
    EXPECT_TRUE(proof.cycle.empty());
}

TEST(RunirTests, ExtModuleParserLowersSupportedTransitions)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto description = R"((:module
    (:symbol entry)
    (:arguments
        (:concept X)
    )
    (:registers
        (:concept r0)
    )
    (:entry m0)
    (:memory m0 m1 m2)
    (:features
        (:concept
            (:symbol B)
            (:expression
                (c_top)
            )
        )
        (:concept
            (:symbol C0)
            (:expression
                (c_register r0)
            )
        )
        (:boolean
            (:symbol H)
            (:expression
                (b_nonempty
                    (c_top)
                )
            )
        )
        (:numerical
            (:symbol N)
            (:expression
                (n_count
                    (c_top)
                )
            )
        )
    )
    (:rules
        (:rule
            (:symbol load-edge)
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:load
                    (:conditions
                        (greater_zero B)
                    )
                    (:concept
                        (c_top)
                    )
                    (:register
                        (:concept r0)
                    )
                )
            )
        )
        (:rule
            (:symbol auto12)
            (:expression
                (:source-memory m1)
                (:target-memory m1)
                (:sketch
                    (:conditions
                        (positive H)
                        (greater_zero N)
                    )
                    (:effects
                        (unchanged B)
                    )
                )
            )
        )
        (:rule
            (:symbol auto14)
            (:expression
                (:source-memory m1)
                (:target-memory m2)
                (:do
                    (:conditions)
                    (:action "pick")
                    (:arguments C0 B B)
                    (:effects
                        (unchanged B)
                    )
                )
            )
        )
    )
))";

    const auto module = kr::ps::ext::dl::parse_module(description, planning_task.get_domain().get_domain(), *repository);
    ASSERT_EQ(module.get_memory_transitions().size(), 3);

    const auto load_rules = module.get_memory_transitions()[0];
    ASSERT_EQ(load_rules.size(), 1);
    EXPECT_EQ(load_rules[0].get_symbol(), "load-edge");
    EXPECT_TRUE(ygg::visit(
        [](auto rule)
        {
            using View = std::decay_t<decltype(rule)>;
            using Expected = ygg::View<ygg::Index<kr::ps::ext::Rule<kr::ps::ext::LoadTag, kr::dl::ConceptTag>>, kr::ps::ext::Repository>;
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

    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto dl_repository = dl_repository_factory.create(planning_task.get_repository());

    const auto concept_argument = kr::ps::ext::dl::parse_concept(R"((c_argument
    0))",
                                                                 planning_task.get_domain().get_domain(),
                                                                 *dl_repository);
    EXPECT_TRUE(ygg::visit(
        [](auto child)
        {
            using View = std::decay_t<decltype(child)>;
            using Expected =
                ygg::View<ygg::Index<kr::dl::Concept<kr::ExtFamilyTag, kr::dl::ArgumentTag<kr::dl::ConceptTag>>>, kr::dl::ext::ConstructorRepository>;
            if constexpr (std::same_as<View, Expected>)
                return ygg::uint_t(child.get_data().identifier) == 0;
            else
                return false;
        },
        concept_argument.get_variant()));

    const auto concept_register = kr::ps::ext::dl::parse_concept(R"((c_register
    1))",
                                                                 planning_task.get_domain().get_domain(),
                                                                 *dl_repository);
    EXPECT_TRUE(ygg::visit(
        [](auto child)
        {
            using View = std::decay_t<decltype(child)>;
            using Expected = ygg::View<ygg::Index<kr::dl::Concept<kr::ExtFamilyTag, kr::dl::RegisterTag>>, kr::dl::ext::ConstructorRepository>;
            if constexpr (std::same_as<View, Expected>)
                return ygg::uint_t(child.get_data().identifier) == 1;
            else
                return false;
        },
        concept_register.get_variant()));

    const auto role_argument = kr::ps::ext::dl::parse_role(R"((r_argument
    0))",
                                                           planning_task.get_domain().get_domain(),
                                                           *dl_repository);
    EXPECT_TRUE(ygg::visit(
        [](auto child)
        {
            using View = std::decay_t<decltype(child)>;
            using Expected = ygg::View<ygg::Index<kr::dl::Role<kr::ExtFamilyTag, kr::dl::ArgumentTag<kr::dl::RoleTag>>>, kr::dl::ext::ConstructorRepository>;
            if constexpr (std::same_as<View, Expected>)
                return ygg::uint_t(child.get_data().identifier) == 0;
            else
                return false;
        },
        role_argument.get_variant()));

    EXPECT_NO_THROW(kr::ps::ext::dl::parse_concept(R"((c_some
    (r_inverse
        (r_argument
            0))
    (c_register
        1)))",
                                                   planning_task.get_domain().get_domain(),
                                                   *dl_repository));
    EXPECT_NO_THROW(kr::ps::ext::dl::parse_role(R"((r_atomic_state
    "at"))",
                                                planning_task.get_domain().get_domain(),
                                                *dl_repository));
    EXPECT_NO_THROW(kr::ps::ext::dl::parse_boolean(R"((b_nonempty
    (c_top)))",
                                                   planning_task.get_domain().get_domain(),
                                                   *dl_repository));
    EXPECT_NO_THROW(kr::ps::ext::dl::parse_numerical(R"((n_count
    (c_top)))",
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

TEST(RunirTests, ExtModuleFactoryExposesPaperDescriptionsAndEmptyModule)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_prefix() / "profiling" / "htg" / "blocksworld-large-simple" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "profiling" / "htg" / "blocksworld-large-simple" / "p-300-4.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto empty = kr::ps::ext::dl::ModuleFactory::create_empty(*repository);
    EXPECT_EQ(empty.get_name(), "empty");
    EXPECT_EQ(empty.get_entry_memory_state().get_name(), "m0");
    EXPECT_EQ(empty.get_memory_states().size(), 1);

    const auto empty_description = kr::ps::ext::dl::ModuleFactory::create_empty_description();
    EXPECT_NE(empty_description.find("(:module\n    (:symbol empty)\n    (:arguments)"), std::string::npos);

    const auto on_description = kr::ps::ext::dl::ModuleFactory::create_description(kr::ps::ext::dl::ModuleSpecification::ON_BONET_ET_AL_ICAPS2024);
    EXPECT_NE(on_description.find("(:module\n    (:symbol on)\n    (:arguments\n        (:concept X)\n        (:concept Y)\n    )"), std::string::npos);
    EXPECT_NE(on_description.find("(:action \"stack\")"), std::string::npos);
    EXPECT_NE(on_description.find("(c_argument X)"), std::string::npos);

    const auto on_table_description = kr::ps::ext::dl::ModuleFactory::create_on_table_bonet_et_al_icaps2024_description();
    EXPECT_NE(on_table_description.find("(:module\n    (:symbol on-table)\n    (:arguments\n        (:concept X)\n    )"), std::string::npos);
    EXPECT_NE(on_table_description.find("(:action \"putdown\")"), std::string::npos);

    const auto tower_description = kr::ps::ext::dl::ModuleFactory::create_tower_bonet_et_al_icaps2024_description();
    EXPECT_NE(tower_description.find("(:module\n    (:symbol tower)\n    (:arguments\n        (:concept X)\n        (:role O)\n    )"), std::string::npos);
    EXPECT_NE(tower_description.find("(:callee on)"), std::string::npos);
    EXPECT_NE(tower_description.find("(r_argument O)"), std::string::npos);

    const auto blocks_description = kr::ps::ext::dl::ModuleFactory::create_blocks_bonet_et_al_icaps2024_description();
    EXPECT_NE(blocks_description.find("(:module\n    (:symbol blocks)\n    (:arguments\n        (:role O)\n    )"), std::string::npos);
    EXPECT_NE(blocks_description.find("(:callee tower)"), std::string::npos);

    const auto on = kr::ps::ext::dl::ModuleFactory::create_on_bonet_et_al_icaps2024(planning_task.get_domain().get_domain(), *repository);
    const auto on_table = kr::ps::ext::dl::ModuleFactory::create_on_table_bonet_et_al_icaps2024(planning_task.get_domain().get_domain(), *repository);
    const auto tower = kr::ps::ext::dl::ModuleFactory::create_tower_bonet_et_al_icaps2024(planning_task.get_domain().get_domain(), *repository);
    const auto blocks = kr::ps::ext::dl::ModuleFactory::create_blocks_bonet_et_al_icaps2024(planning_task.get_domain().get_domain(), *repository);
    EXPECT_EQ(on.get_name(), "on");
    EXPECT_EQ(on_table.get_name(), "on-table");
    EXPECT_EQ(tower.get_name(), "tower");
    EXPECT_EQ(blocks.get_name(), "blocks");

    const auto selected_on = kr::ps::ext::dl::ModuleFactory::create(kr::ps::ext::dl::ModuleSpecification::ON_BONET_ET_AL_ICAPS2024,
                                                                    planning_task.get_domain().get_domain(),
                                                                    *repository);
    EXPECT_EQ(selected_on.get_name(), "on");

    const auto descriptions = kr::ps::ext::dl::ModuleFactory::create_bonet_et_al_icaps2024_descriptions();
    ASSERT_EQ(descriptions.size(), 4);
    EXPECT_EQ(kr::ps::ext::dl::parser::parse_module_ast(descriptions[0]).name, "on");
    EXPECT_EQ(kr::ps::ext::dl::parser::parse_module_ast(descriptions[1]).name, "on-table");
    EXPECT_EQ(kr::ps::ext::dl::parser::parse_module_ast(descriptions[2]).name, "tower");
    EXPECT_EQ(kr::ps::ext::dl::parser::parse_module_ast(descriptions[3]).name, "blocks");

    const auto modules = kr::ps::ext::dl::ModuleFactory::create_bonet_et_al_icaps2024_modules(planning_task.get_domain().get_domain(), *repository);
    ASSERT_EQ(modules.size(), 4);
    EXPECT_EQ(modules[0].get_name(), "on");
    EXPECT_EQ(modules[1].get_name(), "on-table");
    EXPECT_EQ(modules[2].get_name(), "tower");
    EXPECT_EQ(modules[3].get_name(), "blocks");

    const auto program_description =
        kr::ps::ext::dl::parser::parse_module_program_ast(kr::ps::ext::dl::ModuleFactory::create_bonet_et_al_icaps2024_program_description());
    EXPECT_EQ(program_description.entry, "root");
    ASSERT_EQ(program_description.modules.size(), 5);
    EXPECT_EQ(program_description.modules.front().name, "root");

    const auto program = kr::ps::ext::dl::ModuleFactory::create_bonet_et_al_icaps2024_program(planning_task.get_domain().get_domain(), *repository);
    EXPECT_EQ(program.get_entry_module().get_name(), "root");
    ASSERT_EQ(program.get_modules().size(), 5);
    EXPECT_EQ(program.get_modules()[0].get_name(), "root");
}

TEST(RunirTests, ExtModuleEvaluationContextIsolatesAndRestoresCallFrames)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(task->get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto caller_entry = create_memory_state(*repository, "caller_entry");
    const auto caller_return = create_memory_state(*repository, "caller_return");
    const auto callee_entry = create_memory_state(*repository, "callee_entry");
    const auto caller = create_module(*repository, "caller", caller_entry, { caller_entry, caller_return });
    const auto callee = create_module(*repository, "callee", callee_entry, { callee_entry });

    auto builder = kr::dl::semantics::Builder();
    auto denotation_repository_factory = kr::dl::semantics::DenotationRepositoryFactory();
    auto denotation_repository = denotation_repository_factory.create();
    auto context = kr::ps::ext::EvaluationContext<p::GroundTag>(search_context->successor_generator->get_initial_node().get_state(),
                                                                caller,
                                                                builder,
                                                                denotation_repository);

    const auto saved_object = ygg::Index<tyr::formalism::Object>(0);
    context.set(kr::dl::RegisterIdentifier<kr::dl::ConceptTag>(0), saved_object);
    context.enter_module(callee, caller_return);

    EXPECT_EQ(context.get_module().get_index(), callee.get_index());
    EXPECT_EQ(context.get_memory_state().get_index(), callee_entry.get_index());
    EXPECT_FALSE(context.concept_registers()[0]);
    ASSERT_EQ(context.get_call_stack().size(), 1);

    ASSERT_TRUE(context.restore_caller());
    EXPECT_EQ(context.get_module().get_index(), caller.get_index());
    EXPECT_EQ(context.get_memory_state().get_index(), caller_return.get_index());
    ASSERT_TRUE(context.concept_registers()[0]);
    EXPECT_EQ(*context.concept_registers()[0], saved_object);
    EXPECT_FALSE(context.restore_caller());
}

TEST(RunirTests, ExtLoadRuleStoresFirstObjectAndAdvancesMemory)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(task->get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto source = create_memory_state(*repository, "source");
    const auto target = create_memory_state(*repository, "target");
    const auto reg = create_register(*repository, "r0", 0);
    const auto top_concept = create_top_concept(*dl_repository);

    auto load_data = ygg::Data<kr::ps::ext::Rule<kr::ps::ext::LoadTag, kr::dl::ConceptTag>>();
    load_data.source = source.get_index();
    load_data.target = target.get_index();
    load_data.load_expression = top_concept.get_index();
    load_data.reg = reg.get_index();
    kr::ps::ext::canonicalize(load_data);
    const auto load = repository->get_or_create(load_data).first;

    auto variant_data = ygg::Data<kr::ps::ext::RuleVariant>(load.get_index());
    const auto variant = repository->get_or_create(variant_data).first;

    auto module_data = ygg::Data<kr::ps::ext::Module>(std::string("module"));
    module_data.entry_memory_state = source.get_index();
    module_data.memory_states.push_back(source.get_index());
    module_data.memory_states.push_back(target.get_index());
    module_data.concept_registers.push_back(reg.get_index());
    auto transition = ygg::IndexList<kr::ps::ext::RuleVariant>();
    transition.push_back(variant.get_index());
    ygg::canonicalize(transition);
    module_data.memory_transitions.push_back(std::move(transition));
    kr::ps::ext::canonicalize(module_data);
    const auto module = repository->get_or_create(module_data).first;

    const auto formatted = fmt::format("{}", module);
    EXPECT_NE(formatted.find("(:symbol module)"), std::string::npos) << formatted;
    EXPECT_NE(formatted.find("(:load"), std::string::npos);
    EXPECT_NE(formatted.find("(:expression"), std::string::npos);
    EXPECT_NE(formatted.find("(:register\n                        (:concept r0)\n                    )"), std::string::npos) << formatted;

    auto builder = kr::dl::semantics::Builder();
    auto denotation_repository_factory = kr::dl::semantics::DenotationRepositoryFactory();
    auto denotation_repository = denotation_repository_factory.create();
    auto context = kr::ps::ext::EvaluationContext<p::GroundTag>(search_context->successor_generator->get_initial_node().get_state(),
                                                                module,
                                                                builder,
                                                                denotation_repository);
    const auto initial_state = context.get_state().get_index();

    EXPECT_EQ(kr::ps::ext::detail::execute_load(load, context), kr::ps::ext::detail::RuleExecutionStatus::APPLIED);
    EXPECT_EQ(kr::ps::ext::detail::execute_load(load, context), kr::ps::ext::detail::RuleExecutionStatus::NOT_APPLICABLE);
    EXPECT_EQ(context.get_state().get_index(), initial_state);
    EXPECT_EQ(context.get_memory_state().get_index(), target.get_index());
    ASSERT_TRUE(context.concept_registers()[0]);
    EXPECT_EQ(*context.concept_registers()[0], ygg::Index<tyr::formalism::Object>(0));
}

TEST(RunirTests, ExtRoleLoadRuleStoresFirstPairAndAdvancesMemory)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(task->get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto module = kr::ps::ext::dl::parse_module(R"((:module
    (:symbol module)
    (:arguments)
    (:registers
        (:role r0)
    )
    (:entry source)
    (:memory source target)
    (:features)
    (:rules
        (:rule
            (:symbol load-edge)
            (:expression
                (:source-memory source)
                (:target-memory target)
                (:load
                    (:conditions)
                    (:role
                        (r_atomic_state "at")
                    )
                    (:register
                        (:role r0)
                    )
                )
            )
        )
    )
))",
                                                      task->get_domain().get_domain(),
                                                      *repository);

    ASSERT_EQ(module.get_registers<kr::dl::RoleTag>().size(), 1);
    const auto transitions = module.get_memory_transitions();
    ASSERT_EQ(transitions.size(), 1);
    ASSERT_EQ(transitions[0].size(), 1);

    auto maybe_load = std::optional<kr::ps::ext::RuleView<kr::ps::ext::LoadTag, kr::dl::RoleTag>>();
    ygg::visit(
        [&](auto rule)
        {
            using RuleView = std::decay_t<decltype(rule)>;
            if constexpr (std::same_as<RuleView, kr::ps::ext::RuleView<kr::ps::ext::LoadTag, kr::dl::RoleTag>>)
                maybe_load = rule;
        },
        transitions[0][0].get_variant());
    ASSERT_TRUE(maybe_load);

    const auto formatted = fmt::format("{}", module);
    EXPECT_NE(formatted.find("(:role (r_atomic_state \"at\"))"), std::string::npos) << formatted;
    EXPECT_NE(formatted.find("(:register\n                        (:role r0)\n                    )"), std::string::npos) << formatted;

    auto builder = kr::dl::semantics::Builder();
    auto denotation_repository_factory = kr::dl::semantics::DenotationRepositoryFactory();
    auto denotation_repository = denotation_repository_factory.create();
    auto context = kr::ps::ext::EvaluationContext<p::GroundTag>(search_context->successor_generator->get_initial_node().get_state(),
                                                                module,
                                                                builder,
                                                                denotation_repository);
    const auto initial_state = context.get_state().get_index();

    EXPECT_EQ(kr::ps::ext::detail::execute_load(*maybe_load, context), kr::ps::ext::detail::RuleExecutionStatus::APPLIED);
    EXPECT_EQ(context.get_state().get_index(), initial_state);
    EXPECT_EQ(context.get_memory_state().get_name(), "target");
    ASSERT_TRUE(context.role_registers()[0]);
    EXPECT_NE(context.role_registers()[0]->first, context.role_registers()[0]->second);
}

TEST(RunirTests, ExtCallRulePassesArgumentDenotationsToCallee)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(task->get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto caller_entry = create_memory_state(*repository, "caller_entry");
    const auto caller_return = create_memory_state(*repository, "caller_return");
    const auto callee_entry = create_memory_state(*repository, "callee_entry");
    const auto caller = create_module(*repository, "caller", caller_entry, { caller_entry, caller_return });

    auto concept_arg_data = ygg::Data<kr::ps::ext::Argument<kr::dl::ConceptTag>>(std::string("x"), kr::dl::ArgumentIdentifier<kr::dl::ConceptTag>(0));
    const auto concept_arg = repository->get_or_create(concept_arg_data).first;
    auto role_arg_data = ygg::Data<kr::ps::ext::Argument<kr::dl::RoleTag>>(std::string("r"), kr::dl::ArgumentIdentifier<kr::dl::RoleTag>(0));
    const auto role_arg = repository->get_or_create(role_arg_data).first;
    auto boolean_arg_data = ygg::Data<kr::ps::ext::Argument<kr::dl::BooleanTag>>(std::string("b"), kr::dl::ArgumentIdentifier<kr::dl::BooleanTag>(0));
    const auto boolean_arg = repository->get_or_create(boolean_arg_data).first;
    auto numerical_arg_data = ygg::Data<kr::ps::ext::Argument<kr::dl::NumericalTag>>(std::string("n"), kr::dl::ArgumentIdentifier<kr::dl::NumericalTag>(0));
    const auto numerical_arg = repository->get_or_create(numerical_arg_data).first;

    auto callee_data = ygg::Data<kr::ps::ext::Module>(std::string("callee"));
    callee_data.entry_memory_state = callee_entry.get_index();
    callee_data.memory_states.push_back(callee_entry.get_index());
    callee_data.concept_arguments.push_back(concept_arg.get_index());
    callee_data.role_arguments.push_back(role_arg.get_index());
    callee_data.boolean_arguments.push_back(boolean_arg.get_index());
    callee_data.numerical_arguments.push_back(numerical_arg.get_index());
    kr::ps::ext::canonicalize(callee_data);
    const auto callee = repository->get_or_create(callee_data).first;

    const auto top_concept = create_top_concept(*dl_repository);
    const auto universal_role = kr::ps::ext::dl::parse_role("(r_universal)", planning_task.get_domain().get_domain(), *dl_repository);
    const auto true_boolean = kr::ps::ext::dl::parse_boolean(R"((b_nonempty
    (c_top)))",
                                                             planning_task.get_domain().get_domain(),
                                                             *dl_repository);
    const auto object_count = kr::ps::ext::dl::parse_numerical(R"((n_count
    (c_top)))",
                                                               planning_task.get_domain().get_domain(),
                                                               *dl_repository);
    const auto concept_argument = create_concept_argument(*dl_repository, 0);
    const auto role_argument = kr::ps::ext::dl::parse_role(R"((r_argument
    0))",
                                                           planning_task.get_domain().get_domain(),
                                                           *dl_repository);
    const auto boolean_argument = kr::ps::ext::dl::parse_boolean(R"((b_argument
    0))",
                                                                 planning_task.get_domain().get_domain(),
                                                                 *dl_repository);
    const auto numerical_argument = kr::ps::ext::dl::parse_numerical(R"((n_argument
    0))",
                                                                     planning_task.get_domain().get_domain(),
                                                                     *dl_repository);

    auto call_data = ygg::Data<kr::ps::ext::Rule<kr::ps::ext::CallTag>>();
    call_data.source = caller_entry.get_index();
    call_data.target = caller_return.get_index();
    call_data.callee = callee.get_index();
    call_data.arguments.push_back(top_concept.get_index());
    call_data.arguments.push_back(universal_role.get_index());
    call_data.arguments.push_back(true_boolean.get_index());
    call_data.arguments.push_back(object_count.get_index());
    kr::ps::ext::canonicalize(call_data);
    const auto call = repository->get_or_create(call_data).first;

    auto builder = kr::dl::semantics::Builder();
    auto denotation_repository_factory = kr::dl::semantics::DenotationRepositoryFactory();
    auto denotation_repository = denotation_repository_factory.create();
    auto context = kr::ps::ext::EvaluationContext<p::GroundTag>(search_context->successor_generator->get_initial_node().get_state(),
                                                                caller,
                                                                builder,
                                                                denotation_repository);

    EXPECT_EQ(kr::ps::ext::detail::execute_call(call, context), kr::ps::ext::detail::RuleExecutionStatus::APPLIED);
    EXPECT_EQ(context.get_module().get_index(), callee.get_index());
    EXPECT_EQ(context.get_memory_state().get_index(), callee_entry.get_index());
    ASSERT_EQ(context.arguments<kr::dl::ConceptTag>().size(), 1);
    ASSERT_EQ(context.arguments<kr::dl::RoleTag>().size(), 1);
    ASSERT_EQ(context.arguments<kr::dl::BooleanTag>().size(), 1);
    ASSERT_EQ(context.arguments<kr::dl::NumericalTag>().size(), 1);

    const auto concept_denotation = kr::ps::ext::evaluate_argument(concept_argument.get_index(), context);
    const auto concept_first = concept_denotation.begin();
    ASSERT_NE(concept_first, concept_denotation.end());
    EXPECT_EQ((*concept_first).get_index(), ygg::Index<tyr::formalism::Object>(0));

    const auto role_denotation = kr::ps::ext::evaluate_argument(role_argument.get_index(), context);
    const auto role_first = role_denotation.begin();
    ASSERT_NE(role_first, role_denotation.end());
    const auto role_pair = *role_first;
    EXPECT_EQ(role_pair.first.get_index(), ygg::Index<tyr::formalism::Object>(0));
    EXPECT_EQ(role_pair.second.get_index(), ygg::Index<tyr::formalism::Object>(0));

    EXPECT_TRUE(kr::ps::ext::evaluate_argument(boolean_argument.get_index(), context).get());
    EXPECT_GT(kr::ps::ext::evaluate_argument(numerical_argument.get_index(), context).get(), 0);

    ASSERT_TRUE(context.restore_caller());
    EXPECT_EQ(context.get_module().get_index(), caller.get_index());
    EXPECT_EQ(context.get_memory_state().get_index(), caller_return.get_index());
}

TEST(RunirTests, ExtCallRuleResolvesNamedCalleeFromModuleRegistry)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(task->get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto caller_entry = create_memory_state(*repository, "caller_entry");
    const auto caller_return = create_memory_state(*repository, "caller_return");
    const auto callee_entry = create_memory_state(*repository, "callee_entry");
    const auto caller = create_module(*repository, "caller", caller_entry, { caller_entry, caller_return });
    const auto callee = create_module(*repository, "callee", callee_entry, { callee_entry });

    auto call_data = ygg::Data<kr::ps::ext::Rule<kr::ps::ext::CallTag>>();
    call_data.source = caller_entry.get_index();
    call_data.target = caller_return.get_index();
    call_data.callee_name = "callee";
    kr::ps::ext::canonicalize(call_data);
    const auto call = repository->get_or_create(call_data).first;

    auto builder = kr::dl::semantics::Builder();
    auto denotation_repository_factory = kr::dl::semantics::DenotationRepositoryFactory();
    auto denotation_repository = denotation_repository_factory.create();
    auto context = kr::ps::ext::EvaluationContext<p::GroundTag>(search_context->successor_generator->get_initial_node().get_state(),
                                                                caller,
                                                                builder,
                                                                denotation_repository,
                                                                {},
                                                                {},
                                                                {},
                                                                {},
                                                                std::vector<kr::ps::ext::ModuleView> { caller, callee });

    EXPECT_EQ(kr::ps::ext::detail::execute_call(call, context), kr::ps::ext::detail::RuleExecutionStatus::APPLIED);
    EXPECT_EQ(context.get_module().get_index(), callee.get_index());
    EXPECT_EQ(context.get_memory_state().get_index(), callee_entry.get_index());
}

TEST(RunirTests, ExtDoRuleAppliesMatchingActionAndAdvancesMemory)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(task->get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto source = create_memory_state(*repository, "source");
    const auto target = create_memory_state(*repository, "target");
    const auto ball_feature =
        create_concept_feature(*repository,
                               kr::ps::ext::dl::parse_concept("(c_atomic_state \"ball\")", planning_task.get_domain().get_domain(), *dl_repository).get_index(),
                               "ball");
    const auto room_feature =
        create_concept_feature(*repository,
                               kr::ps::ext::dl::parse_concept("(c_atomic_state \"room\")", planning_task.get_domain().get_domain(), *dl_repository).get_index(),
                               "room");
    const auto gripper_feature = create_concept_feature(
        *repository,
        kr::ps::ext::dl::parse_concept("(c_atomic_state \"gripper\")", planning_task.get_domain().get_domain(), *dl_repository).get_index(),
        "gripper");

    auto do_data = ygg::Data<kr::ps::ext::Rule<kr::ps::ext::DoTag>>(std::string("pick"));
    do_data.source = source.get_index();
    do_data.target = target.get_index();
    do_data.arguments.push_back(ball_feature.get_index());
    do_data.arguments.push_back(room_feature.get_index());
    do_data.arguments.push_back(gripper_feature.get_index());
    kr::ps::ext::canonicalize(do_data);
    const auto rule = repository->get_or_create(do_data).first;

    const auto module = create_module(*repository, "module", source, { source, target });
    auto builder = kr::dl::semantics::Builder();
    auto denotation_repository_factory = kr::dl::semantics::DenotationRepositoryFactory();
    auto denotation_repository = denotation_repository_factory.create();
    auto context = kr::ps::ext::EvaluationContext<p::GroundTag>(search_context->successor_generator->get_initial_node().get_state(),
                                                                module,
                                                                builder,
                                                                denotation_repository);

    const auto initial_node = search_context->successor_generator->get_initial_node();
    const auto initial_state = context.get_state().get_index();
    const auto successors = search_context->successor_generator->get_labeled_successor_nodes(initial_node);
    const auto selected = kr::ps::ext::detail::select_do_successor(rule, context, successors);
    ASSERT_TRUE(selected);
    EXPECT_EQ(selected->label.get_action().get_name(), "pick");

    EXPECT_EQ(kr::ps::ext::detail::execute_do(rule, context, successors), kr::ps::ext::detail::RuleExecutionStatus::APPLIED);
    EXPECT_EQ(context.get_memory_state().get_index(), target.get_index());
    EXPECT_NE(context.get_state().get_index(), initial_state);
    EXPECT_EQ(context.get_state().get_index(), selected->node.get_state().get_index());
}

TEST(RunirTests, ExtDoRuleRejectsActionWithIncompatibleDeclaredEffects)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(task->get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto module = kr::ps::ext::dl::parse_module(R"((:module
    (:symbol module)
    (:arguments)
    (:registers)
    (:entry source)
    (:memory source target)
    (:features
        (:concept
            (:symbol T)
            (:expression
                (c_top)
            )
        )
        (:numerical
            (:symbol C)
            (:expression
                (n_count
                    (c_atomic_state "free")
                )
            )
        )
    )
    (:rules
        (:rule
            (:symbol auto16)
            (:expression
                (:source-memory source)
                (:target-memory target)
                (:do
                    (:conditions)
                    (:action "pick")
                    (:arguments T T T)
                    (:effects
                        (unchanged C)
                    )
                )
            )
        )
    )
))",
                                                      planning_task.get_domain().get_domain(),
                                                      *repository);
    const auto rule_variant = module.get_memory_transitions()[0][0];

    auto builder = kr::dl::semantics::Builder();
    auto denotation_repository_factory = kr::dl::semantics::DenotationRepositoryFactory();
    auto denotation_repository = denotation_repository_factory.create();
    auto context = kr::ps::ext::EvaluationContext<p::GroundTag>(search_context->successor_generator->get_initial_node().get_state(),
                                                                module,
                                                                builder,
                                                                denotation_repository);

    const auto initial_node = search_context->successor_generator->get_initial_node();
    const auto successors = search_context->successor_generator->get_labeled_successor_nodes(initial_node);
    ygg::visit(
        [&](auto rule)
        {
            using RuleView = std::decay_t<decltype(rule)>;
            if constexpr (std::same_as<RuleView, ygg::View<ygg::Index<kr::ps::ext::Rule<kr::ps::ext::DoTag>>, kr::ps::ext::Repository>>)
            {
                EXPECT_FALSE(kr::ps::ext::detail::select_do_successor(rule, context, successors));
                EXPECT_EQ(kr::ps::ext::detail::execute_do(rule, context, successors), kr::ps::ext::detail::RuleExecutionStatus::NO_APPLICABLE_ACTION);
            }
            else
            {
                FAIL() << "expected do rule";
            }
        },
        rule_variant.get_variant());
}

TEST(RunirTests, ExtImmediateExternalRulesUseCanonicalFirstApplicableRule)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(task->get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto source = create_memory_state(*repository, "source");
    const auto move_target = create_memory_state(*repository, "move_target");
    const auto pick_target = create_memory_state(*repository, "pick_target");
    const auto ball_feature =
        create_concept_feature(*repository,
                               kr::ps::ext::dl::parse_concept("(c_atomic_state \"ball\")", planning_task.get_domain().get_domain(), *dl_repository).get_index(),
                               "ball");
    const auto room_feature =
        create_concept_feature(*repository,
                               kr::ps::ext::dl::parse_concept("(c_atomic_state \"room\")", planning_task.get_domain().get_domain(), *dl_repository).get_index(),
                               "room");
    const auto gripper_feature = create_concept_feature(
        *repository,
        kr::ps::ext::dl::parse_concept("(c_atomic_state \"gripper\")", planning_task.get_domain().get_domain(), *dl_repository).get_index(),
        "gripper");

    auto move_data = ygg::Data<kr::ps::ext::Rule<kr::ps::ext::DoTag>>(std::string("pick"));
    move_data.source = source.get_index();
    move_data.target = move_target.get_index();
    move_data.arguments.push_back(ball_feature.get_index());
    move_data.arguments.push_back(room_feature.get_index());
    move_data.arguments.push_back(gripper_feature.get_index());
    kr::ps::ext::canonicalize(move_data);
    const auto move_rule = repository->get_or_create(move_data).first;
    auto move_variant_data = ygg::Data<kr::ps::ext::RuleVariant>(move_rule.get_index());
    const auto move_variant = repository->get_or_create(move_variant_data).first;

    auto pick_data = ygg::Data<kr::ps::ext::Rule<kr::ps::ext::DoTag>>(std::string("pick"));
    pick_data.source = source.get_index();
    pick_data.target = pick_target.get_index();
    pick_data.arguments.push_back(ball_feature.get_index());
    pick_data.arguments.push_back(room_feature.get_index());
    pick_data.arguments.push_back(gripper_feature.get_index());
    kr::ps::ext::canonicalize(pick_data);
    const auto pick_rule = repository->get_or_create(pick_data).first;
    auto pick_variant_data = ygg::Data<kr::ps::ext::RuleVariant>(pick_rule.get_index());
    const auto pick_variant = repository->get_or_create(pick_variant_data).first;

    auto module_data = ygg::Data<kr::ps::ext::Module>(std::string("module"));
    module_data.entry_memory_state = source.get_index();
    module_data.memory_states.push_back(source.get_index());
    module_data.memory_states.push_back(move_target.get_index());
    module_data.memory_states.push_back(pick_target.get_index());

    auto move_transition = ygg::IndexList<kr::ps::ext::RuleVariant>();
    move_transition.push_back(move_variant.get_index());
    ygg::canonicalize(move_transition);
    module_data.memory_transitions.push_back(std::move(move_transition));

    auto pick_transition = ygg::IndexList<kr::ps::ext::RuleVariant>();
    pick_transition.push_back(pick_variant.get_index());
    ygg::canonicalize(pick_transition);
    module_data.memory_transitions.push_back(std::move(pick_transition));

    kr::ps::ext::canonicalize(module_data);
    const auto module = repository->get_or_create(module_data).first;

    auto builder = kr::dl::semantics::Builder();
    auto denotation_repository_factory = kr::dl::semantics::DenotationRepositoryFactory();
    auto denotation_repository = denotation_repository_factory.create();
    auto context = kr::ps::ext::EvaluationContext<p::GroundTag>(search_context->successor_generator->get_initial_node().get_state(),
                                                                module,
                                                                builder,
                                                                denotation_repository);

    const auto initial_node = search_context->successor_generator->get_initial_node();
    const auto successors = search_context->successor_generator->get_labeled_successor_nodes(initial_node);

    auto expander = kr::ps::ext::SuccessorExpander<p::GroundTag>(*search_context, initial_node);
    const auto options = kr::ps::ext::detail::ModuleExecutionOptions<p::GroundTag> {};
    const auto steps = expander.control_steps(*search_context, context, options);
    ASSERT_FALSE(steps.empty());
    EXPECT_EQ(steps.front().status, kr::ps::ext::detail::ModuleProgramOutcome::APPLIED);
    EXPECT_EQ(steps.front().context.get_memory_state().get_index(), move_target.get_index());
}

TEST(RunirTests, ExtExecutorReportsStructuredFailureStatuses)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(task->get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto source = create_memory_state(*repository, "source");
    const auto target = create_memory_state(*repository, "target");
    const auto top_concept = create_top_concept(*dl_repository);

    const auto empty_module = create_module(*repository, "empty", source, { source });

    auto proof_options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag>();
    const auto empty_program = create_module_program(*repository, empty_module, { empty_module });
    const auto empty_proof = kr::ps::ext::prove_solution(search_context, empty_program, proof_options);
    EXPECT_EQ(empty_proof.status, kr::ps::ext::ModuleProgramProofStatus::FAILURE);
    ASSERT_TRUE(empty_proof.graph);
    EXPECT_EQ(empty_proof.graph->get_num_vertices(), 1);
    EXPECT_EQ(empty_proof.graph->get_num_edges(), 0);
    EXPECT_TRUE(empty_proof.deadend_transitions.empty());
    EXPECT_FALSE(empty_proof.open_states.empty());
    EXPECT_TRUE(empty_proof.cycle.empty());

    auto load_data = ygg::Data<kr::ps::ext::Rule<kr::ps::ext::LoadTag, kr::dl::ConceptTag>>();
    load_data.source = source.get_index();
    load_data.target = source.get_index();
    load_data.load_expression = top_concept.get_index();
    load_data.reg = create_register(*repository, "r0", 0).get_index();
    kr::ps::ext::canonicalize(load_data);
    const auto load = repository->get_or_create(load_data).first;
    auto load_variant_data = ygg::Data<kr::ps::ext::RuleVariant>(load.get_index());
    const auto load_variant = repository->get_or_create(load_variant_data).first;
    auto load_module_data = ygg::Data<kr::ps::ext::Module>(std::string("load-loop"));
    load_module_data.entry_memory_state = source.get_index();
    load_module_data.memory_states.push_back(source.get_index());
    load_module_data.concept_registers.push_back(load_data.reg);
    auto load_transition = ygg::IndexList<kr::ps::ext::RuleVariant>();
    load_transition.push_back(load_variant.get_index());
    ygg::canonicalize(load_transition);
    load_module_data.memory_transitions.push_back(std::move(load_transition));
    kr::ps::ext::canonicalize(load_module_data);
    const auto load_module = repository->get_or_create(load_module_data).first;
    auto load_proof_options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag>();
    const auto load_program = create_module_program(*repository, load_module, { load_module });
    const auto load_proof = kr::ps::ext::prove_solution(search_context, load_program, load_proof_options);
    EXPECT_EQ(load_proof.status, kr::ps::ext::ModuleProgramProofStatus::FAILURE);
    ASSERT_TRUE(load_proof.graph);
    EXPECT_TRUE(load_proof.deadend_transitions.empty());
    EXPECT_FALSE(load_proof.cycle.empty());

    auto concept_arg_data = ygg::Data<kr::ps::ext::Argument<kr::dl::ConceptTag>>(std::string("x"), kr::dl::ArgumentIdentifier<kr::dl::ConceptTag>(0));
    const auto concept_arg = repository->get_or_create(concept_arg_data).first;
    auto callee_data = ygg::Data<kr::ps::ext::Module>(std::string("callee"));
    callee_data.entry_memory_state = target.get_index();
    callee_data.memory_states.push_back(target.get_index());
    callee_data.concept_arguments.push_back(concept_arg.get_index());
    kr::ps::ext::canonicalize(callee_data);
    const auto callee = repository->get_or_create(callee_data).first;
    auto call_data = ygg::Data<kr::ps::ext::Rule<kr::ps::ext::CallTag>>();
    call_data.source = source.get_index();
    call_data.target = target.get_index();
    call_data.callee = callee.get_index();
    kr::ps::ext::canonicalize(call_data);
    const auto call = repository->get_or_create(call_data).first;
    auto call_variant_data = ygg::Data<kr::ps::ext::RuleVariant>(call.get_index());
    const auto call_variant = repository->get_or_create(call_variant_data).first;
    auto caller_data = ygg::Data<kr::ps::ext::Module>(std::string("caller"));
    caller_data.entry_memory_state = source.get_index();
    caller_data.memory_states.push_back(source.get_index());
    caller_data.memory_states.push_back(target.get_index());
    auto call_transition = ygg::IndexList<kr::ps::ext::RuleVariant>();
    call_transition.push_back(call_variant.get_index());
    ygg::canonicalize(call_transition);
    caller_data.memory_transitions.push_back(std::move(call_transition));
    kr::ps::ext::canonicalize(caller_data);
    const auto caller = repository->get_or_create(caller_data).first;
    const auto caller_program = create_module_program(*repository, caller, { caller, callee });
    const auto caller_proof = kr::ps::ext::prove_solution(search_context, caller_program);
    EXPECT_EQ(caller_proof.status, kr::ps::ext::ModuleProgramProofStatus::FAILURE);
    ASSERT_TRUE(caller_proof.graph);
    EXPECT_TRUE(caller_proof.deadend_transitions.empty());
    EXPECT_FALSE(caller_proof.open_states.empty());

    auto do_data = ygg::Data<kr::ps::ext::Rule<kr::ps::ext::DoTag>>(std::string("missing-action"));
    do_data.source = source.get_index();
    do_data.target = target.get_index();
    kr::ps::ext::canonicalize(do_data);
    const auto do_rule = repository->get_or_create(do_data).first;
    auto do_variant_data = ygg::Data<kr::ps::ext::RuleVariant>(do_rule.get_index());
    const auto do_variant = repository->get_or_create(do_variant_data).first;
    auto do_module_data = ygg::Data<kr::ps::ext::Module>(std::string("no-action"));
    do_module_data.entry_memory_state = source.get_index();
    do_module_data.memory_states.push_back(source.get_index());
    do_module_data.memory_states.push_back(target.get_index());
    auto do_transition = ygg::IndexList<kr::ps::ext::RuleVariant>();
    do_transition.push_back(do_variant.get_index());
    ygg::canonicalize(do_transition);
    do_module_data.memory_transitions.push_back(std::move(do_transition));
    kr::ps::ext::canonicalize(do_module_data);
    const auto do_module = repository->get_or_create(do_module_data).first;

    const auto do_program = create_module_program(*repository, do_module, { do_module });
    const auto do_proof = kr::ps::ext::prove_solution(search_context, do_program);
    EXPECT_EQ(do_proof.status, kr::ps::ext::ModuleProgramProofStatus::FAILURE);
    ASSERT_TRUE(do_proof.graph);
    EXPECT_EQ(do_proof.graph->get_num_vertices(), 1);
    EXPECT_EQ(do_proof.graph->get_num_edges(), 0);
    EXPECT_TRUE(do_proof.deadend_transitions.empty());
    EXPECT_FALSE(do_proof.open_states.empty());
    EXPECT_TRUE(do_proof.cycle.empty());
}

}  // namespace runir::tests
