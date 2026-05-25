#include <filesystem>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <runir/datasets/task_class.hpp>
#include <runir/kr/dl/semantics/builder.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/ps/ext/dl/module_factory.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <runir/kr/ps/ext/evaluation.hpp>
#include <runir/kr/ps/ext/evaluation_context.hpp>
#include <runir/kr/ps/ext/execution.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/module_program_executor.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <type_traits>
#include <tyr/formalism/planning/parser.hpp>
#include <tyr/planning/planning.hpp>
#include <vector>

namespace runir::tests
{
namespace
{

std::filesystem::path benchmark_prefix() { return std::filesystem::path(RUNIR_ROOT_DIR) / "data" / "planning-benchmarks"; }

auto create_memory_state(kr::ps::ext::Repository& repository, const std::string& name)
{
    auto data = tyr::Data<kr::ps::ext::MemoryState>(name);
    return repository.get_or_create(data).first;
}

auto create_module(kr::ps::ext::Repository& repository,
                   const std::string& name,
                   kr::ps::ext::MemoryStateView entry,
                   std::initializer_list<kr::ps::ext::MemoryStateView> memory_states)
{
    auto data = tyr::Data<kr::ps::ext::Module>(name);
    data.entry_memory_state = entry.get_index();
    for (auto state : memory_states)
        data.memory_states.push_back(state.get_index());
    kr::ps::ext::canonicalize(data);
    return repository.get_or_create(data).first;
}

auto create_register(kr::ps::ext::Repository& repository, const std::string& name, tyr::uint_t identifier)
{
    auto data = tyr::Data<kr::ps::ext::Register>(name, kr::dl::RegisterIdentifier<kr::dl::ConceptTag>(identifier));
    return repository.get_or_create(data).first;
}

auto create_top_concept(kr::dl::ext::ConstructorRepository& repository)
{
    auto top_data = tyr::Data<kr::dl::Concept<kr::ExtFamilyTag, kr::dl::TopTag>>();
    const auto top = repository.get_or_create(top_data).first;
    auto constructor_data = tyr::Data<kr::dl::Constructor<kr::ExtFamilyTag, kr::dl::ConceptTag>>(top.get_index());
    return repository.get_or_create(constructor_data).first;
}

auto create_concept_argument(kr::dl::ext::ConstructorRepository& repository, tyr::uint_t identifier)
{
    auto argument_data =
        tyr::Data<kr::dl::Concept<kr::ExtFamilyTag, kr::dl::ArgumentTag<kr::dl::ConceptTag>>>(kr::dl::ArgumentIdentifier<kr::dl::ConceptTag>(identifier));
    const auto argument = repository.get_or_create(argument_data).first;
    auto constructor_data = tyr::Data<kr::dl::Constructor<kr::ExtFamilyTag, kr::dl::ConceptTag>>(argument.get_index());
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
    auto dl_repository = dl_repository_factory.create_shared(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto description = R"((module "entry"
      (:arguments
        (concept "X" 0)
        (role "O" 0)
        (boolean "B" 0)
        (numerical "N" 0)
      )
      (:registers
        (concept "r0" 0)
      )
      (:entry "m0")
      (:memory "m0" "m1")
      (:features)
      (:transitions)
    ))";

    const auto module = kr::ps::ext::dl::parse_module(description, planning_task.get_domain().get_domain(), repository);
    EXPECT_EQ(module.get_name(), "entry");
    EXPECT_EQ(module.template get_arguments<kr::dl::ConceptTag>().size(), 1);
    EXPECT_EQ(module.template get_arguments<kr::dl::RoleTag>().size(), 1);
    EXPECT_EQ(module.template get_arguments<kr::dl::BooleanTag>().size(), 1);
    EXPECT_EQ(module.template get_arguments<kr::dl::NumericalTag>().size(), 1);
    EXPECT_EQ(module.get_registers().size(), 1);
    EXPECT_EQ(module.get_entry_memory_state().get_name(), "m0");
    EXPECT_EQ(module.get_memory_states().size(), 2);

    const auto blocksworld_domain = benchmark_prefix() / "profiling" / "htg" / "blocksworld-large-simple" / "domain.pddl";
    const auto blocksworld_task_file = benchmark_prefix() / "profiling" / "htg" / "blocksworld-large-simple" / "p-300-4.pddl";
    const auto blocksworld_task = fp::Parser(blocksworld_domain).parse_task(blocksworld_task_file);
    auto blocksworld_dl_repository = dl_repository_factory.create_shared(blocksworld_task.get_repository());
    auto blocksworld_repository = repository_factory.create(blocksworld_dl_repository);

    const auto blocks = kr::ps::ext::dl::parse_module(kr::ps::ext::dl::ModuleFactory::create_blocks_bonet_et_al_icaps2024_description(),
                                                      blocksworld_task.get_domain().get_domain(),
                                                      blocksworld_repository);
    EXPECT_EQ(blocks.get_name(), "blocks");
    ASSERT_TRUE(blocks.get_rules(blocks.get_data().memory_transitions[1].source, blocks.get_data().memory_transitions[1].target).has_value());
    auto call_rule = (*blocks.get_rules(blocks.get_data().memory_transitions[1].source, blocks.get_data().memory_transitions[1].target)).front();
    auto found_call_rule = false;
    tyr::visit(
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
    auto dl_repository = dl_repository_factory.create_shared(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto caller_description = R"((module "caller"
      (:arguments)
      (:registers)
      (:entry "m0")
      (:memory "m0" "m1")
      (:features)
      (:transitions
        ("m0" "m1" (call (:conditions) (:callee "callee") (:arguments)))
      )
    ))";
    const auto callee_description = R"((module "callee"
      (:arguments)
      (:registers)
      (:entry "m0")
      (:memory "m0")
      (:features)
      (:transitions)
    ))";

    const auto modules = kr::ps::ext::dl::parse_modules({ caller_description, callee_description }, planning_task.get_domain().get_domain(), repository);
    ASSERT_EQ(modules.size(), 2);
    EXPECT_EQ(modules[0].get_name(), "caller");
    EXPECT_EQ(modules[1].get_name(), "callee");

    ASSERT_EQ(modules[0].get_data().memory_transitions.size(), 1);
    const auto rules = modules[0].get_rules(modules[0].get_data().memory_transitions[0].source, modules[0].get_data().memory_transitions[0].target);
    ASSERT_TRUE(rules.has_value());
    auto found_call_rule = false;
    tyr::visit(
        [&](auto rule)
        {
            using RuleView = std::decay_t<decltype(rule)>;
            if constexpr (std::same_as<RuleView, kr::ps::ext::RuleView<kr::ps::ext::CallTag>>)
            {
                found_call_rule = true;
                EXPECT_EQ(rule.get_callee_name(), "callee");
            }
        },
        rules->front().get_variant());
    EXPECT_TRUE(found_call_rule) << "Expected call rule.";
}

TEST(RunirTests, ExtModuleProgramParserRejectsInvalidProgramWiring)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create_shared(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto root = R"((module "root"
      (:arguments)
      (:registers)
      (:entry "m0")
      (:memory "m0")
      (:features)
      (:transitions)
    ))";

    EXPECT_THROW(
        kr::ps::ext::dl::parse_module_program(R"((program (:entry "missing") )" + std::string(root) + ")", planning_task.get_domain().get_domain(), repository),
        std::runtime_error);
    EXPECT_THROW(kr::ps::ext::dl::parse_module_program(R"((program (:entry "root") )" + std::string(root) + std::string(root) + ")",
                                                       planning_task.get_domain().get_domain(),
                                                       repository),
                 std::runtime_error);
    EXPECT_THROW(kr::ps::ext::dl::parse_module_program(R"((program
      (:entry "root")
      (module "root"
        (:arguments)
        (:registers)
        (:entry "m0")
        (:memory "m0" "m1")
        (:features)
        (:transitions
          ("m0" "m1" (call (:conditions) (:callee "missing") (:arguments)))
        )
      )
    ))",
                                                       planning_task.get_domain().get_domain(),
                                                       repository),
                 std::runtime_error);
}

TEST(RunirTests, ExtModuleParserReadsPaperFactoryDescriptions)
{
    const auto on = kr::ps::ext::dl::parser::parse_module_ast(kr::ps::ext::dl::ModuleFactory::create_on_bonet_et_al_icaps2024_description());
    EXPECT_EQ(on.name, "on");
    EXPECT_EQ(on.arguments.size(), 2);
    EXPECT_EQ(on.registers.size(), 2);
    EXPECT_EQ(on.memory_states.size(), 9);
    EXPECT_EQ(on.transitions.size(), 14);
    EXPECT_EQ(on.transitions.back().rules.front().action, "stack");
    ASSERT_EQ(on.transitions.back().rules.front().arguments.size(), 2);
    EXPECT_EQ(on.transitions.back().rules.front().arguments[0], "(c_argument 0)");

    const auto tower = kr::ps::ext::dl::parser::parse_module_ast(kr::ps::ext::dl::ModuleFactory::create_tower_bonet_et_al_icaps2024_description());
    EXPECT_EQ(tower.name, "tower");
    EXPECT_EQ(tower.arguments.size(), 2);
    EXPECT_EQ(tower.transitions.size(), 4);
    EXPECT_EQ(tower.transitions[2].rules.front().callee, "on");
    ASSERT_EQ(tower.transitions[2].rules.front().arguments.size(), 2);
    EXPECT_EQ(tower.transitions[2].rules.front().arguments[1], "(c_some (r_inverse (r_argument 0)) (c_register 0))");

    const auto blocks = kr::ps::ext::dl::parser::parse_module_ast(kr::ps::ext::dl::ModuleFactory::create_blocks_bonet_et_al_icaps2024_description());
    EXPECT_EQ(blocks.name, "blocks");
    EXPECT_EQ(blocks.arguments.size(), 1);
    EXPECT_EQ(blocks.transitions.size(), 2);
    EXPECT_EQ(blocks.transitions[1].rules.front().callee, "tower");

    const auto program = kr::ps::ext::dl::parser::parse_module_program_ast(kr::ps::ext::dl::ModuleFactory::create_bonet_et_al_icaps2024_program_description());
    EXPECT_EQ(program.entry, "root");
    ASSERT_EQ(program.modules.size(), 5);
    EXPECT_EQ(program.modules[0].name, "root");
    EXPECT_EQ(program.modules[0].transitions.front().rules.front().callee, "blocks");
}

TEST(RunirTests, ExtModuleParserLowersPaperFactoryDescriptionsAgainstBlocksworld)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_prefix() / "profiling" / "htg" / "blocksworld-large-simple" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "profiling" / "htg" / "blocksworld-large-simple" / "p-300-4.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create_shared(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto modules = kr::ps::ext::dl::parse_modules(kr::ps::ext::dl::ModuleFactory::create_bonet_et_al_icaps2024_descriptions(),
                                                        planning_task.get_domain().get_domain(),
                                                        repository);
    ASSERT_EQ(modules.size(), 4);
    EXPECT_EQ(modules[0].get_name(), "on");
    EXPECT_EQ(modules[1].get_name(), "on-table");
    EXPECT_EQ(modules[2].get_name(), "tower");
    EXPECT_EQ(modules[3].get_name(), "blocks");

    const auto program = kr::ps::ext::dl::ModuleFactory::create_bonet_et_al_icaps2024_program(planning_task.get_domain().get_domain(), repository);
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
    auto dl_repository = dl_repository_factory.create_shared(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto modules = kr::ps::ext::dl::parse_modules(kr::ps::ext::dl::ModuleFactory::create_bonet_et_al_icaps2024_descriptions(),
                                                        planning_task.get_domain().get_domain(),
                                                        repository);
    ASSERT_EQ(modules.size(), 4);

    for (const auto module : modules)
    {
        const auto formatted = fmt::format("{}", module);
        try
        {
            const auto reparsed = kr::ps::ext::dl::parse_module(formatted, planning_task.get_domain().get_domain(), repository);
            EXPECT_EQ(reparsed.get_name(), module.get_name());
            EXPECT_EQ(reparsed.get_memory_states().size(), module.get_memory_states().size());
            EXPECT_EQ(reparsed.get_memory_transitions().size(), module.get_memory_transitions().size());
            EXPECT_EQ(reparsed.template get_arguments<kr::dl::ConceptTag>().size(), module.template get_arguments<kr::dl::ConceptTag>().size());
            EXPECT_EQ(reparsed.template get_arguments<kr::dl::RoleTag>().size(), module.template get_arguments<kr::dl::RoleTag>().size());
            EXPECT_EQ(reparsed.template get_arguments<kr::dl::BooleanTag>().size(), module.template get_arguments<kr::dl::BooleanTag>().size());
            EXPECT_EQ(reparsed.template get_arguments<kr::dl::NumericalTag>().size(), module.template get_arguments<kr::dl::NumericalTag>().size());
        }
        catch (const std::exception& err)
        {
            FAIL() << "Failed to reparse formatted module " << module.get_name() << ": " << err.what() << "\n" << formatted;
        }
    }

    const auto program = kr::ps::ext::dl::ModuleFactory::create_bonet_et_al_icaps2024_program(planning_task.get_domain().get_domain(), repository);
    const auto formatted_program = fmt::format("{}", program);
    const auto reparsed_program = kr::ps::ext::dl::parse_module_program(formatted_program, planning_task.get_domain().get_domain(), repository);
    EXPECT_EQ(reparsed_program.get_entry_module().get_name(), program.get_entry_module().get_name());
    EXPECT_EQ(reparsed_program.get_modules().size(), program.get_modules().size());
}

TEST(RunirTests, ExtPaperModulesExecuteOnSmallBlocksworldInstance)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "profiling" / "htg" / "blocksworld-large-simple" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "profiling" / "htg" / "blocksworld-large-simple" / "p-100-2.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = tyr::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>(task, execution_context);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create_shared(task->get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto program = kr::ps::ext::dl::ModuleFactory::create_bonet_et_al_icaps2024_program(planning_task.get_domain().get_domain(), repository);
    ASSERT_EQ(program.get_modules().size(), 5);

    auto options = kr::ps::ext::ModuleExecutionOptions<p::GroundTag>();
    options.max_arity = 1;
    options.max_steps = 1024;
    options.max_load_steps = 1024;

    const auto result = kr::ps::ext::execute_solution(search_context, program, options);
    EXPECT_EQ(result.status, kr::ps::ext::ModuleExecutionStatus::SUCCESS)
        << "module=" << result.module.get_name() << " memory=" << result.memory_state.get_name() << " steps=" << result.num_steps
        << " call_depth=" << result.call_depth;
}

TEST(RunirTests, ExtModuleParserLowersSupportedTransitions)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create_shared(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto description = R"((module "entry"
      (:arguments
        (concept "X" 0)
      )
      (:registers
        (concept "r0" 0)
      )
      (:entry "m0")
      (:memory "m0" "m1" "m2")
      (:features
        (concept B "B" "all blocks" (c_top))
        (boolean H "H" "has blocks" (b_nonempty (c_top)))
        (numerical N "N" "block count" (n_count (c_top)))
      )
      (:transitions
        ("m0" "m1" (load (:conditions (greater_zero B)) (:concept B) (:register "r0")))
        ("m1" "m1" (sketch (:conditions (positive H) (greater_zero N)) (:effects (unchanged B))))
        ("m1" "m2" (do (:conditions) (:action "pickup") (:arguments (c_register 0))))
      )
    ))";

    const auto module = kr::ps::ext::dl::parse_module(description, planning_task.get_domain().get_domain(), repository);
    ASSERT_EQ(module.get_memory_transitions().size(), 3);

    const auto load_rules = module.get_rules(module.get_memory_transitions()[0].source, module.get_memory_transitions()[0].target);
    ASSERT_TRUE(load_rules);
    ASSERT_EQ(load_rules->size(), 1);
    EXPECT_TRUE(tyr::visit(
        [](auto rule)
        {
            using View = std::decay_t<decltype(rule)>;
            using Expected = tyr::View<tyr::Index<kr::ps::ext::Rule<kr::ps::ext::LoadTag>>, kr::ps::ext::Repository>;
            if constexpr (std::same_as<View, Expected>)
                return rule.get_register().get_name() == "r0" && rule.get_conditions().size() == 1;
            else
                return false;
        },
        (*load_rules)[0].get_variant()));

    const auto sketch_rules = module.get_rules(module.get_memory_transitions()[1].source, module.get_memory_transitions()[1].target);
    ASSERT_TRUE(sketch_rules);
    ASSERT_EQ(sketch_rules->size(), 1);

    const auto do_rules = module.get_rules(module.get_memory_transitions()[2].source, module.get_memory_transitions()[2].target);
    ASSERT_TRUE(do_rules);
    ASSERT_EQ(do_rules->size(), 1);
    EXPECT_TRUE(tyr::visit(
        [](auto rule)
        {
            using View = std::decay_t<decltype(rule)>;
            using Expected = tyr::View<tyr::Index<kr::ps::ext::Rule<kr::ps::ext::DoTag>>, kr::ps::ext::Repository>;
            if constexpr (std::same_as<View, Expected>)
                return rule.get_action_name() == "pickup" && rule.get_action_arguments().size() == 1;
            else
                return false;
        },
        (*do_rules)[0].get_variant()));
}

TEST(RunirTests, ExtModuleParserLowersExtDlConceptAndRoleExpressions)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto dl_repository = dl_repository_factory.create_shared(planning_task.get_repository());

    const auto concept_argument = kr::ps::ext::dl::parse_concept("(c_argument 0)", planning_task.get_domain().get_domain(), *dl_repository);
    EXPECT_TRUE(tyr::visit(
        [](auto child)
        {
            using View = std::decay_t<decltype(child)>;
            using Expected =
                tyr::View<tyr::Index<kr::dl::Concept<kr::ExtFamilyTag, kr::dl::ArgumentTag<kr::dl::ConceptTag>>>, kr::dl::ext::ConstructorRepository>;
            if constexpr (std::same_as<View, Expected>)
                return tyr::uint_t(child.get_data().identifier) == 0;
            else
                return false;
        },
        concept_argument.get_variant()));

    const auto concept_register = kr::ps::ext::dl::parse_concept("(c_register 1)", planning_task.get_domain().get_domain(), *dl_repository);
    EXPECT_TRUE(tyr::visit(
        [](auto child)
        {
            using View = std::decay_t<decltype(child)>;
            using Expected = tyr::View<tyr::Index<kr::dl::Concept<kr::ExtFamilyTag, kr::dl::RegisterTag>>, kr::dl::ext::ConstructorRepository>;
            if constexpr (std::same_as<View, Expected>)
                return tyr::uint_t(child.get_data().identifier) == 1;
            else
                return false;
        },
        concept_register.get_variant()));

    const auto role_argument = kr::ps::ext::dl::parse_role("(r_argument 0)", planning_task.get_domain().get_domain(), *dl_repository);
    EXPECT_TRUE(tyr::visit(
        [](auto child)
        {
            using View = std::decay_t<decltype(child)>;
            using Expected = tyr::View<tyr::Index<kr::dl::Role<kr::ExtFamilyTag, kr::dl::ArgumentTag<kr::dl::RoleTag>>>, kr::dl::ext::ConstructorRepository>;
            if constexpr (std::same_as<View, Expected>)
                return tyr::uint_t(child.get_data().identifier) == 0;
            else
                return false;
        },
        role_argument.get_variant()));

    EXPECT_NO_THROW(
        kr::ps::ext::dl::parse_concept("(c_some (r_inverse (r_argument 0)) (c_register 1))", planning_task.get_domain().get_domain(), *dl_repository));
    EXPECT_NO_THROW(kr::ps::ext::dl::parse_role("(r_atomic_state \"at\")", planning_task.get_domain().get_domain(), *dl_repository));
    EXPECT_NO_THROW(kr::ps::ext::dl::parse_boolean("(b_nonempty (c_top))", planning_task.get_domain().get_domain(), *dl_repository));
    EXPECT_NO_THROW(kr::ps::ext::dl::parse_numerical("(n_count (c_top))", planning_task.get_domain().get_domain(), *dl_repository));
}

TEST(RunirTests, ExtModuleFactoryExposesPaperDescriptionsAndEmptyModule)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_prefix() / "profiling" / "htg" / "blocksworld-large-simple" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "profiling" / "htg" / "blocksworld-large-simple" / "p-300-4.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create_shared(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto empty = kr::ps::ext::dl::ModuleFactory::create_empty(repository);
    EXPECT_EQ(empty.get_name(), "empty");
    EXPECT_EQ(empty.get_entry_memory_state().get_name(), "m0");
    EXPECT_EQ(empty.get_memory_states().size(), 1);

    const auto empty_description = kr::ps::ext::dl::ModuleFactory::create_empty_description();
    EXPECT_NE(empty_description.find("(module \"empty\""), std::string::npos);

    const auto on_description = kr::ps::ext::dl::ModuleFactory::create_description(kr::ps::ext::dl::ModuleSpecification::ON_BONET_ET_AL_ICAPS2024);
    EXPECT_NE(on_description.find("(module \"on\""), std::string::npos);
    EXPECT_NE(on_description.find("(:action \"stack\")"), std::string::npos);
    EXPECT_NE(on_description.find("(c_argument 0)"), std::string::npos);

    const auto on_table_description = kr::ps::ext::dl::ModuleFactory::create_on_table_bonet_et_al_icaps2024_description();
    EXPECT_NE(on_table_description.find("(module \"on-table\""), std::string::npos);
    EXPECT_NE(on_table_description.find("(:action \"putdown\")"), std::string::npos);

    const auto tower_description = kr::ps::ext::dl::ModuleFactory::create_tower_bonet_et_al_icaps2024_description();
    EXPECT_NE(tower_description.find("(module \"tower\""), std::string::npos);
    EXPECT_NE(tower_description.find("(:callee \"on\")"), std::string::npos);
    EXPECT_NE(tower_description.find("(r_argument 0)"), std::string::npos);

    const auto blocks_description = kr::ps::ext::dl::ModuleFactory::create_blocks_bonet_et_al_icaps2024_description();
    EXPECT_NE(blocks_description.find("(module \"blocks\""), std::string::npos);
    EXPECT_NE(blocks_description.find("(:callee \"tower\")"), std::string::npos);

    const auto on = kr::ps::ext::dl::ModuleFactory::create_on_bonet_et_al_icaps2024(planning_task.get_domain().get_domain(), repository);
    const auto on_table = kr::ps::ext::dl::ModuleFactory::create_on_table_bonet_et_al_icaps2024(planning_task.get_domain().get_domain(), repository);
    const auto tower = kr::ps::ext::dl::ModuleFactory::create_tower_bonet_et_al_icaps2024(planning_task.get_domain().get_domain(), repository);
    const auto blocks = kr::ps::ext::dl::ModuleFactory::create_blocks_bonet_et_al_icaps2024(planning_task.get_domain().get_domain(), repository);
    EXPECT_EQ(on.get_name(), "on");
    EXPECT_EQ(on_table.get_name(), "on-table");
    EXPECT_EQ(tower.get_name(), "tower");
    EXPECT_EQ(blocks.get_name(), "blocks");

    const auto selected_on = kr::ps::ext::dl::ModuleFactory::create(kr::ps::ext::dl::ModuleSpecification::ON_BONET_ET_AL_ICAPS2024,
                                                                    planning_task.get_domain().get_domain(),
                                                                    repository);
    EXPECT_EQ(selected_on.get_name(), "on");

    const auto descriptions = kr::ps::ext::dl::ModuleFactory::create_bonet_et_al_icaps2024_descriptions();
    ASSERT_EQ(descriptions.size(), 4);
    EXPECT_EQ(kr::ps::ext::dl::parser::parse_module_ast(descriptions[0]).name, "on");
    EXPECT_EQ(kr::ps::ext::dl::parser::parse_module_ast(descriptions[1]).name, "on-table");
    EXPECT_EQ(kr::ps::ext::dl::parser::parse_module_ast(descriptions[2]).name, "tower");
    EXPECT_EQ(kr::ps::ext::dl::parser::parse_module_ast(descriptions[3]).name, "blocks");

    const auto modules = kr::ps::ext::dl::ModuleFactory::create_bonet_et_al_icaps2024_modules(planning_task.get_domain().get_domain(), repository);
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

    const auto program = kr::ps::ext::dl::ModuleFactory::create_bonet_et_al_icaps2024_program(planning_task.get_domain().get_domain(), repository);
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
    auto execution_context = tyr::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>(task, execution_context);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create_shared(task->get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto caller_entry = create_memory_state(repository, "caller_entry");
    const auto caller_return = create_memory_state(repository, "caller_return");
    const auto callee_entry = create_memory_state(repository, "callee_entry");
    const auto caller = create_module(repository, "caller", caller_entry, { caller_entry, caller_return });
    const auto callee = create_module(repository, "callee", callee_entry, { callee_entry });

    auto builder = kr::dl::semantics::Builder();
    auto denotation_repository_factory = kr::dl::semantics::DenotationRepositoryFactory();
    auto denotation_repository = denotation_repository_factory.create();
    auto context = kr::ps::ext::EvaluationContext<p::GroundTag>(search_context.successor_generator->get_initial_node().get_state(),
                                                                caller,
                                                                builder,
                                                                denotation_repository);

    const auto saved_object = tyr::Index<tyr::formalism::Object>(0);
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
    auto execution_context = tyr::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>(task, execution_context);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create_shared(task->get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto source = create_memory_state(repository, "source");
    const auto target = create_memory_state(repository, "target");
    const auto reg = create_register(repository, "x", 0);
    const auto top_concept = create_top_concept(*dl_repository);

    auto load_data = tyr::Data<kr::ps::ext::Rule<kr::ps::ext::LoadTag>>();
    load_data.source = source.get_index();
    load_data.target = target.get_index();
    load_data.load_concept = top_concept.get_index();
    load_data.reg = reg.get_index();
    kr::ps::ext::canonicalize(load_data);
    const auto load = repository.get_or_create(load_data).first;

    auto variant_data = tyr::Data<kr::ps::ext::RuleVariant>(load.get_index());
    const auto variant = repository.get_or_create(variant_data).first;

    auto module_data = tyr::Data<kr::ps::ext::Module>(std::string("module"));
    module_data.entry_memory_state = source.get_index();
    module_data.memory_states.push_back(source.get_index());
    module_data.memory_states.push_back(target.get_index());
    module_data.registers.push_back(reg.get_index());
    auto transition = kr::ps::ext::MemoryTransition();
    transition.source = source.get_index();
    transition.target = target.get_index();
    transition.rules.push_back(variant.get_index());
    module_data.memory_transitions.push_back(transition);
    kr::ps::ext::canonicalize(module_data);
    const auto module = repository.get_or_create(module_data).first;

    const auto formatted = fmt::format("{}", module);
    EXPECT_NE(formatted.find("(module \"module\""), std::string::npos);
    EXPECT_NE(formatted.find("(load"), std::string::npos);
    EXPECT_NE(formatted.find("(:register \"x\")"), std::string::npos);

    auto builder = kr::dl::semantics::Builder();
    auto denotation_repository_factory = kr::dl::semantics::DenotationRepositoryFactory();
    auto denotation_repository = denotation_repository_factory.create();
    auto context = kr::ps::ext::EvaluationContext<p::GroundTag>(search_context.successor_generator->get_initial_node().get_state(),
                                                                module,
                                                                builder,
                                                                denotation_repository);
    const auto initial_state = context.get_state().get_index();

    EXPECT_EQ(kr::ps::ext::execute_loads(context, 4), kr::ps::ext::LoadExecutionStatus::STABLE);
    EXPECT_EQ(context.get_state().get_index(), initial_state);
    EXPECT_EQ(context.get_memory_state().get_index(), target.get_index());
    ASSERT_TRUE(context.concept_registers()[0]);
    EXPECT_EQ(*context.concept_registers()[0], tyr::Index<tyr::formalism::Object>(0));
}

TEST(RunirTests, ExtCallRulePassesArgumentDenotationsToCallee)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = tyr::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>(task, execution_context);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create_shared(task->get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto caller_entry = create_memory_state(repository, "caller_entry");
    const auto caller_return = create_memory_state(repository, "caller_return");
    const auto callee_entry = create_memory_state(repository, "callee_entry");
    const auto caller = create_module(repository, "caller", caller_entry, { caller_entry, caller_return });

    auto concept_arg_data = tyr::Data<kr::ps::ext::Argument<kr::dl::ConceptTag>>(std::string("x"), kr::dl::ArgumentIdentifier<kr::dl::ConceptTag>(0));
    const auto concept_arg = repository.get_or_create(concept_arg_data).first;
    auto role_arg_data = tyr::Data<kr::ps::ext::Argument<kr::dl::RoleTag>>(std::string("r"), kr::dl::ArgumentIdentifier<kr::dl::RoleTag>(0));
    const auto role_arg = repository.get_or_create(role_arg_data).first;
    auto boolean_arg_data = tyr::Data<kr::ps::ext::Argument<kr::dl::BooleanTag>>(std::string("b"), kr::dl::ArgumentIdentifier<kr::dl::BooleanTag>(0));
    const auto boolean_arg = repository.get_or_create(boolean_arg_data).first;
    auto numerical_arg_data = tyr::Data<kr::ps::ext::Argument<kr::dl::NumericalTag>>(std::string("n"), kr::dl::ArgumentIdentifier<kr::dl::NumericalTag>(0));
    const auto numerical_arg = repository.get_or_create(numerical_arg_data).first;

    auto callee_data = tyr::Data<kr::ps::ext::Module>(std::string("callee"));
    callee_data.entry_memory_state = callee_entry.get_index();
    callee_data.memory_states.push_back(callee_entry.get_index());
    callee_data.concept_arguments.push_back(concept_arg.get_index());
    callee_data.role_arguments.push_back(role_arg.get_index());
    callee_data.boolean_arguments.push_back(boolean_arg.get_index());
    callee_data.numerical_arguments.push_back(numerical_arg.get_index());
    kr::ps::ext::canonicalize(callee_data);
    const auto callee = repository.get_or_create(callee_data).first;

    const auto top_concept = create_top_concept(*dl_repository);
    const auto universal_role = kr::ps::ext::dl::parse_role("(r_universal)", planning_task.get_domain().get_domain(), *dl_repository);
    const auto true_boolean = kr::ps::ext::dl::parse_boolean("(b_nonempty (c_top))", planning_task.get_domain().get_domain(), *dl_repository);
    const auto object_count = kr::ps::ext::dl::parse_numerical("(n_count (c_top))", planning_task.get_domain().get_domain(), *dl_repository);
    const auto concept_argument = create_concept_argument(*dl_repository, 0);
    const auto role_argument = kr::ps::ext::dl::parse_role("(r_argument 0)", planning_task.get_domain().get_domain(), *dl_repository);
    const auto boolean_argument = kr::ps::ext::dl::parse_boolean("(b_argument 0)", planning_task.get_domain().get_domain(), *dl_repository);
    const auto numerical_argument = kr::ps::ext::dl::parse_numerical("(n_argument 0)", planning_task.get_domain().get_domain(), *dl_repository);

    auto call_data = tyr::Data<kr::ps::ext::Rule<kr::ps::ext::CallTag>>();
    call_data.source = caller_entry.get_index();
    call_data.target = caller_return.get_index();
    call_data.callee = callee.get_index();
    call_data.arguments.push_back(top_concept.get_index());
    call_data.arguments.push_back(universal_role.get_index());
    call_data.arguments.push_back(true_boolean.get_index());
    call_data.arguments.push_back(object_count.get_index());
    kr::ps::ext::canonicalize(call_data);
    const auto call = repository.get_or_create(call_data).first;

    auto builder = kr::dl::semantics::Builder();
    auto denotation_repository_factory = kr::dl::semantics::DenotationRepositoryFactory();
    auto denotation_repository = denotation_repository_factory.create();
    auto context = kr::ps::ext::EvaluationContext<p::GroundTag>(search_context.successor_generator->get_initial_node().get_state(),
                                                                caller,
                                                                builder,
                                                                denotation_repository);

    EXPECT_EQ(kr::ps::ext::execute_call(call, context), kr::ps::ext::RuleExecutionStatus::APPLIED);
    EXPECT_EQ(context.get_module().get_index(), callee.get_index());
    EXPECT_EQ(context.get_memory_state().get_index(), callee_entry.get_index());
    ASSERT_EQ(context.arguments<kr::dl::ConceptTag>().size(), 1);
    ASSERT_EQ(context.arguments<kr::dl::RoleTag>().size(), 1);
    ASSERT_EQ(context.arguments<kr::dl::BooleanTag>().size(), 1);
    ASSERT_EQ(context.arguments<kr::dl::NumericalTag>().size(), 1);

    const auto concept_denotation = kr::ps::ext::evaluate_argument(concept_argument.get_index(), context);
    const auto concept_first = concept_denotation.begin();
    ASSERT_NE(concept_first, concept_denotation.end());
    EXPECT_EQ((*concept_first).get_index(), tyr::Index<tyr::formalism::Object>(0));

    const auto role_denotation = kr::ps::ext::evaluate_argument(role_argument.get_index(), context);
    const auto role_first = role_denotation.begin();
    ASSERT_NE(role_first, role_denotation.end());
    const auto role_pair = *role_first;
    EXPECT_EQ(role_pair.first.get_index(), tyr::Index<tyr::formalism::Object>(0));
    EXPECT_EQ(role_pair.second.get_index(), tyr::Index<tyr::formalism::Object>(0));

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
    auto execution_context = tyr::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>(task, execution_context);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create_shared(task->get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto caller_entry = create_memory_state(repository, "caller_entry");
    const auto caller_return = create_memory_state(repository, "caller_return");
    const auto callee_entry = create_memory_state(repository, "callee_entry");
    const auto caller = create_module(repository, "caller", caller_entry, { caller_entry, caller_return });
    const auto callee = create_module(repository, "callee", callee_entry, { callee_entry });

    auto call_data = tyr::Data<kr::ps::ext::Rule<kr::ps::ext::CallTag>>();
    call_data.source = caller_entry.get_index();
    call_data.target = caller_return.get_index();
    call_data.callee_name = "callee";
    kr::ps::ext::canonicalize(call_data);
    const auto call = repository.get_or_create(call_data).first;

    auto builder = kr::dl::semantics::Builder();
    auto denotation_repository_factory = kr::dl::semantics::DenotationRepositoryFactory();
    auto denotation_repository = denotation_repository_factory.create();
    auto context = kr::ps::ext::EvaluationContext<p::GroundTag>(search_context.successor_generator->get_initial_node().get_state(),
                                                                caller,
                                                                builder,
                                                                denotation_repository,
                                                                {},
                                                                {},
                                                                {},
                                                                {},
                                                                std::vector<kr::ps::ext::ModuleView> { caller, callee });

    EXPECT_EQ(kr::ps::ext::execute_call(call, context), kr::ps::ext::RuleExecutionStatus::APPLIED);
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
    auto execution_context = tyr::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>(task, execution_context);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create_shared(task->get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto source = create_memory_state(repository, "source");
    const auto target = create_memory_state(repository, "target");
    const auto top_concept = create_top_concept(*dl_repository);

    auto do_data = tyr::Data<kr::ps::ext::Rule<kr::ps::ext::DoTag>>(std::string("pick"));
    do_data.source = source.get_index();
    do_data.target = target.get_index();
    do_data.arguments.push_back(top_concept.get_index());
    do_data.arguments.push_back(top_concept.get_index());
    do_data.arguments.push_back(top_concept.get_index());
    kr::ps::ext::canonicalize(do_data);
    const auto rule = repository.get_or_create(do_data).first;

    const auto module = create_module(repository, "module", source, { source, target });
    auto builder = kr::dl::semantics::Builder();
    auto denotation_repository_factory = kr::dl::semantics::DenotationRepositoryFactory();
    auto denotation_repository = denotation_repository_factory.create();
    auto context = kr::ps::ext::EvaluationContext<p::GroundTag>(search_context.successor_generator->get_initial_node().get_state(),
                                                                module,
                                                                builder,
                                                                denotation_repository);

    const auto initial_node = search_context.successor_generator->get_initial_node();
    const auto initial_state = context.get_state().get_index();
    const auto successors = search_context.successor_generator->get_labeled_successor_nodes(initial_node);
    const auto selected = kr::ps::ext::select_do_successor(rule, context, successors);
    ASSERT_TRUE(selected);
    EXPECT_EQ(selected->label.get_action().get_name(), "pick");

    EXPECT_EQ(kr::ps::ext::execute_do(rule, context, successors), kr::ps::ext::RuleExecutionStatus::APPLIED);
    EXPECT_EQ(context.get_memory_state().get_index(), target.get_index());
    EXPECT_NE(context.get_state().get_index(), initial_state);
    EXPECT_EQ(context.get_state().get_index(), selected->node.get_state().get_index());
}

TEST(RunirTests, ExtImmediateExternalRulesUseCanonicalFirstApplicableRule)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = tyr::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>(task, execution_context);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create_shared(task->get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto source = create_memory_state(repository, "source");
    const auto move_target = create_memory_state(repository, "move_target");
    const auto pick_target = create_memory_state(repository, "pick_target");
    const auto top_concept = create_top_concept(*dl_repository);

    auto move_data = tyr::Data<kr::ps::ext::Rule<kr::ps::ext::DoTag>>(std::string("move"));
    move_data.source = source.get_index();
    move_data.target = move_target.get_index();
    move_data.arguments.push_back(top_concept.get_index());
    move_data.arguments.push_back(top_concept.get_index());
    kr::ps::ext::canonicalize(move_data);
    const auto move_rule = repository.get_or_create(move_data).first;
    auto move_variant_data = tyr::Data<kr::ps::ext::RuleVariant>(move_rule.get_index());
    const auto move_variant = repository.get_or_create(move_variant_data).first;

    auto pick_data = tyr::Data<kr::ps::ext::Rule<kr::ps::ext::DoTag>>(std::string("pick"));
    pick_data.source = source.get_index();
    pick_data.target = pick_target.get_index();
    pick_data.arguments.push_back(top_concept.get_index());
    pick_data.arguments.push_back(top_concept.get_index());
    pick_data.arguments.push_back(top_concept.get_index());
    kr::ps::ext::canonicalize(pick_data);
    const auto pick_rule = repository.get_or_create(pick_data).first;
    auto pick_variant_data = tyr::Data<kr::ps::ext::RuleVariant>(pick_rule.get_index());
    const auto pick_variant = repository.get_or_create(pick_variant_data).first;

    auto module_data = tyr::Data<kr::ps::ext::Module>(std::string("module"));
    module_data.entry_memory_state = source.get_index();
    module_data.memory_states.push_back(source.get_index());
    module_data.memory_states.push_back(move_target.get_index());
    module_data.memory_states.push_back(pick_target.get_index());

    auto pick_transition = kr::ps::ext::MemoryTransition();
    pick_transition.source = source.get_index();
    pick_transition.target = pick_target.get_index();
    pick_transition.rules.push_back(pick_variant.get_index());
    module_data.memory_transitions.push_back(pick_transition);

    auto move_transition = kr::ps::ext::MemoryTransition();
    move_transition.source = source.get_index();
    move_transition.target = move_target.get_index();
    move_transition.rules.push_back(move_variant.get_index());
    module_data.memory_transitions.push_back(move_transition);

    kr::ps::ext::canonicalize(module_data);
    const auto module = repository.get_or_create(module_data).first;

    auto builder = kr::dl::semantics::Builder();
    auto denotation_repository_factory = kr::dl::semantics::DenotationRepositoryFactory();
    auto denotation_repository = denotation_repository_factory.create();
    auto context = kr::ps::ext::EvaluationContext<p::GroundTag>(search_context.successor_generator->get_initial_node().get_state(),
                                                                module,
                                                                builder,
                                                                denotation_repository);

    const auto initial_node = search_context.successor_generator->get_initial_node();
    const auto successors = search_context.successor_generator->get_labeled_successor_nodes(initial_node);

    EXPECT_EQ(kr::ps::ext::execute_next_immediate_external_rule(context, successors), kr::ps::ext::RuleExecutionStatus::APPLIED);
    EXPECT_EQ(context.get_memory_state().get_index(), move_target.get_index());
}

TEST(RunirTests, ExtExecutorReportsStructuredFailureStatuses)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = tyr::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>(task, execution_context);

    auto dl_repository_factory = kr::dl::ext::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create_shared(task->get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto source = create_memory_state(repository, "source");
    const auto target = create_memory_state(repository, "target");
    const auto top_concept = create_top_concept(*dl_repository);

    auto options = kr::ps::ext::ModuleExecutionOptions<p::GroundTag>();
    options.max_steps = 0;
    const auto empty_module = create_module(repository, "empty", source, { source });
    EXPECT_EQ(kr::ps::ext::execute_solution(search_context, empty_module, options).status, kr::ps::ext::ModuleExecutionStatus::STEP_LIMIT_REACHED);

    auto load_data = tyr::Data<kr::ps::ext::Rule<kr::ps::ext::LoadTag>>();
    load_data.source = source.get_index();
    load_data.target = source.get_index();
    load_data.load_concept = top_concept.get_index();
    load_data.reg = create_register(repository, "x", 0).get_index();
    kr::ps::ext::canonicalize(load_data);
    const auto load = repository.get_or_create(load_data).first;
    auto load_variant_data = tyr::Data<kr::ps::ext::RuleVariant>(load.get_index());
    const auto load_variant = repository.get_or_create(load_variant_data).first;
    auto load_module_data = tyr::Data<kr::ps::ext::Module>(std::string("load-loop"));
    load_module_data.entry_memory_state = source.get_index();
    load_module_data.memory_states.push_back(source.get_index());
    load_module_data.registers.push_back(load_data.reg);
    auto load_transition = kr::ps::ext::MemoryTransition();
    load_transition.source = source.get_index();
    load_transition.target = source.get_index();
    load_transition.rules.push_back(load_variant.get_index());
    load_module_data.memory_transitions.push_back(load_transition);
    kr::ps::ext::canonicalize(load_module_data);
    const auto load_module = repository.get_or_create(load_module_data).first;
    options = kr::ps::ext::ModuleExecutionOptions<p::GroundTag>();
    options.max_load_steps = 1;
    EXPECT_EQ(kr::ps::ext::execute_solution(search_context, load_module, options).status, kr::ps::ext::ModuleExecutionStatus::LOAD_LIMIT_REACHED);

    auto concept_arg_data = tyr::Data<kr::ps::ext::Argument<kr::dl::ConceptTag>>(std::string("x"), kr::dl::ArgumentIdentifier<kr::dl::ConceptTag>(0));
    const auto concept_arg = repository.get_or_create(concept_arg_data).first;
    auto callee_data = tyr::Data<kr::ps::ext::Module>(std::string("callee"));
    callee_data.entry_memory_state = target.get_index();
    callee_data.memory_states.push_back(target.get_index());
    callee_data.concept_arguments.push_back(concept_arg.get_index());
    kr::ps::ext::canonicalize(callee_data);
    const auto callee = repository.get_or_create(callee_data).first;
    auto call_data = tyr::Data<kr::ps::ext::Rule<kr::ps::ext::CallTag>>();
    call_data.source = source.get_index();
    call_data.target = target.get_index();
    call_data.callee = callee.get_index();
    kr::ps::ext::canonicalize(call_data);
    const auto call = repository.get_or_create(call_data).first;
    auto call_variant_data = tyr::Data<kr::ps::ext::RuleVariant>(call.get_index());
    const auto call_variant = repository.get_or_create(call_variant_data).first;
    auto caller_data = tyr::Data<kr::ps::ext::Module>(std::string("caller"));
    caller_data.entry_memory_state = source.get_index();
    caller_data.memory_states.push_back(source.get_index());
    caller_data.memory_states.push_back(target.get_index());
    auto call_transition = kr::ps::ext::MemoryTransition();
    call_transition.source = source.get_index();
    call_transition.target = target.get_index();
    call_transition.rules.push_back(call_variant.get_index());
    caller_data.memory_transitions.push_back(call_transition);
    kr::ps::ext::canonicalize(caller_data);
    const auto caller = repository.get_or_create(caller_data).first;
    EXPECT_EQ(kr::ps::ext::execute_solution(search_context, caller, std::vector<kr::ps::ext::ModuleView> { caller, callee }).status,
              kr::ps::ext::ModuleExecutionStatus::MALFORMED_CALL);

    auto do_data = tyr::Data<kr::ps::ext::Rule<kr::ps::ext::DoTag>>(std::string("missing-action"));
    do_data.source = source.get_index();
    do_data.target = target.get_index();
    kr::ps::ext::canonicalize(do_data);
    const auto do_rule = repository.get_or_create(do_data).first;
    auto do_variant_data = tyr::Data<kr::ps::ext::RuleVariant>(do_rule.get_index());
    const auto do_variant = repository.get_or_create(do_variant_data).first;
    auto do_module_data = tyr::Data<kr::ps::ext::Module>(std::string("no-action"));
    do_module_data.entry_memory_state = source.get_index();
    do_module_data.memory_states.push_back(source.get_index());
    do_module_data.memory_states.push_back(target.get_index());
    auto do_transition = kr::ps::ext::MemoryTransition();
    do_transition.source = source.get_index();
    do_transition.target = target.get_index();
    do_transition.rules.push_back(do_variant.get_index());
    do_module_data.memory_transitions.push_back(do_transition);
    kr::ps::ext::canonicalize(do_module_data);
    const auto do_module = repository.get_or_create(do_module_data).first;
    EXPECT_EQ(kr::ps::ext::execute_solution(search_context, do_module).status, kr::ps::ext::ModuleExecutionStatus::NO_APPLICABLE_ACTION);
}

}  // namespace runir::tests
