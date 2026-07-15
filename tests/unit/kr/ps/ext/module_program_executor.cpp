#include <boost/variant/get.hpp>
#include <cista/serialization.h>
#include <filesystem>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <random>
#include <runir/datasets/task_class.hpp>
#include <runir/kr/dl/semantics/builder.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/errors.hpp>
#include <runir/kr/ps/ext/dl/module_factory.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <runir/kr/ps/ext/execution_builder.hpp>
#include <runir/kr/ps/ext/execution_repository.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/module_program_executor.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/ext/successor_expander.hpp>
#include <runir/kr/task_context.hpp>
#include <runir/kr/uns/dl/parser.hpp>
#include <runir/kr/uns/repository.hpp>
#include <set>
#include <type_traits>
#include <tyr/formalism/planning/parser.hpp>
#include <tyr/planning/algorithms/portable_shuffle.hpp>
#include <tyr/planning/planning.hpp>
#include <variant>
#include <vector>
#include <yggdrasil/execution/onetbb.hpp>

namespace runir::tests
{
namespace
{

std::filesystem::path benchmark_prefix() { return std::filesystem::path(BENCHMARKS_DIR); }

auto create_memory_state(kr::ps::ext::Repository& repository, const std::string& name)
{
    auto data = ygg::Data<kr::ps::ext::MemoryState>(name);
    return repository.get_or_create(data).first;
}

auto make_module_data(kr::ps::ext::Repository& repository, const std::string& name)
{
    auto symbol_data = ygg::Data<kr::ps::ext::ModuleSymbol>(name);
    const auto symbol = repository.get_or_create(symbol_data).first;
    return ygg::Data<kr::ps::ext::Module>(symbol.get_index());
}

auto create_module(kr::ps::ext::Repository& repository,
                   const std::string& name,
                   kr::ps::ext::MemoryStateView entry,
                   std::initializer_list<kr::ps::ext::MemoryStateView> memory_states)
{
    auto data = make_module_data(repository, name);
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
    auto data = ygg::Data<kr::dl::Register<kr::dl::ConceptTag>>(name, kr::dl::RegisterIdentifier<kr::dl::ConceptTag>(identifier));
    return repository.get_dl_repository().get_or_create(data).first;
}

auto create_top_concept(kr::dl::ConstructorRepositoryFor<kr::ExtFamilyTag>& repository)
{
    auto top_data = ygg::Data<kr::dl::Concept<kr::ExtFamilyTag, kr::dl::TopTag>>();
    const auto top = repository.get_or_create(top_data).first;
    auto constructor_data = ygg::Data<kr::dl::Constructor<kr::ExtFamilyTag, kr::dl::ConceptTag>>(top.get_index());
    return repository.get_or_create(constructor_data).first;
}

template<typename FeatureTag, kr::dl::CategoryTag Category>
auto create_feature(kr::ps::ext::Repository& repository, ygg::Index<kr::dl::FamilyConstructor<kr::ExtFamilyTag, Category>> expression, const std::string& name)
{
    auto concrete_data = ygg::Data<kr::ps::ConcreteFeature<kr::ExtFamilyTag, kr::DlTag, FeatureTag>>(expression, name);
    const auto concrete = repository.get_or_create(concrete_data).first;
    auto feature_data = ygg::Data<kr::ps::Feature<kr::ExtFamilyTag, FeatureTag>>(concrete.get_index());
    return repository.get_or_create(feature_data).first;
}

auto create_concept_feature(kr::ps::ext::Repository& repository,
                            ygg::Index<kr::dl::FamilyConstructor<kr::ExtFamilyTag, kr::dl::ConceptTag>> expression,
                            const std::string& name)
{
    return create_feature<kr::dl::ConceptTag>(repository, expression, name);
}

template<tyr::planning::TaskKind Kind>
auto create_task_context(const std::filesystem::path& domain, const std::filesystem::path& task_file)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>::create(fp::Parser(domain).parse_task(task_file));
    auto task = p::TaskPtr<Kind> {};
    if constexpr (std::same_as<Kind, p::LiftedTag>)
        task = std::move(lifted_task);
    else
        task = lifted_task->instantiate_ground_task(*execution_context).task;
    return kr::TaskContext<Kind>::create(runir::datasets::TaskSearchContext<Kind>::create(std::move(task), std::move(execution_context)));
}

template<typename T>
void expect_cista_round_trip(const T& value)
{
    auto bytes = cista::serialize(value);
    const auto* decoded = cista::deserialize<T>(bytes);
    EXPECT_TRUE(ygg::EqualTo<T> {}(value, *decoded));
}

}  // namespace

TEST(RunirTests, ExtDistanceFeatureEvaluationReusesTaskContextCache)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);
    auto task_context = kr::TaskContext<p::GroundTag>::create(search_context);

    auto dl_repository = task_context->ext_dl_repository;
    auto repository = task_context->ext_repository;
    const auto module = kr::ps::ext::dl::parse_module(R"((:module
        (:symbol distance)
        (:arguments)
        (:registers)
        (:entry source)
        (:memory source target)
        (:features
            (:numerical
                (:symbol D)
                (:expression
                    (n_distance
                        (c_atomic_state "ball")
                        (r_atomic_state "at")
                        (c_atomic_state "room")
                    )
                )
            )
        )
        (:rules
            (:rule
                (:symbol advance)
                (:expression
                    (:source-memory source)
                    (:target-memory target)
                    (:sketch
                        (:conditions (greater_zero D))
                        (:effects)
                    )
                )
            )
        )
    ))",
                                                      planning_task.get_domain().get_domain(),
                                                      *repository);
    const auto program = create_module_program(*repository, module, { module });
    auto expander = kr::ps::ext::SuccessorExpander<p::GroundTag>(task_context, program);
    const auto initial_state = expander.initial_state();
    const auto first = expander.control_steps(initial_state);
    ASSERT_EQ(first.size(), 1);
    EXPECT_EQ(first.front().status, kr::ps::ext::detail::ModuleProgramOutcome::APPLIED);
    EXPECT_GT(task_context->dl_denotation_repository->size<kr::dl::NumericalTag>(), 0);
    const auto cached_denotations = task_context->dl_denotation_repository->size<kr::dl::NumericalTag>();

    const auto second = expander.control_steps(initial_state);
    ASSERT_EQ(second.size(), 1);
    EXPECT_EQ(second.front().get_target().get_index(), first.front().get_target().get_index());
    EXPECT_EQ(task_context->dl_denotation_repository->size<kr::dl::NumericalTag>(), cached_denotations);
    EXPECT_EQ(initial_state.get_call_stack().get_memory_state().get_name(), "source");
}

TEST(RunirTests, ExtFindSolutionTreatsClassifierMatchesAsTerminalFailures)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);
    auto task_context = kr::TaskContext<p::GroundTag>::create(search_context);

    auto dl_repository = task_context->ext_dl_repository;
    auto repository = task_context->ext_repository;
    const auto module = kr::ps::ext::dl::parse_module(R"((:module
        (:symbol module)
        (:arguments)
        (:registers)
        (:entry source)
        (:memory source target)
        (:features)
        (:rules
            (:rule
                (:symbol advance)
                (:expression
                    (:source-memory source)
                    (:target-memory target)
                    (:sketch (:conditions) (:effects))
                )
            )
        )
    ))",
                                                      task->get_domain().get_domain(),
                                                      *repository);
    const auto program = create_module_program(*repository, module, { module });

    auto classifier_dl_repository = task_context->uns_dl_repository;
    auto classifier_repository = task_context->uns_repository;
    const auto classifier = kr::uns::dl::parse_classifier(R"((:classifier
        (:symbol all)
        (:features
            (:boolean
                (:symbol yes)
                (:expression (b_const true))
            )
        )
        (:expression (or (and yes)))
    ))",
                                                          task->get_domain().get_domain(),
                                                          *classifier_repository);

    auto options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag> {};
    options.classifier = classifier;
    const auto result = kr::ps::ext::find_solution(task_context, program, options);

    EXPECT_EQ(result.status, kr::ps::ext::ModuleProgramProofStatus::FAILURE);
    ASSERT_TRUE(result.graph);
    ASSERT_EQ(result.graph->get_num_vertices(), 1);
    EXPECT_EQ(result.graph->get_num_edges(), 0);
    ASSERT_EQ(result.deadend_states.size(), 1);
    EXPECT_TRUE(result.open_states.empty());
    const auto& label = result.graph->get_vertex(result.deadend_states.front()).get_property();
    EXPECT_FALSE(label.is_goal);
    EXPECT_FALSE(label.is_alive);
    EXPECT_TRUE(label.is_unsolvable);
    EXPECT_EQ(label.execution_state.get_call_stack().get_memory_state().get_name(), "source");
    EXPECT_GT(task_context->dl_denotation_repository->size<kr::dl::BooleanTag>(), 0);
}

TEST(RunirTests, ExtModuleParserLowersArgumentRegisterMemorySections)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>();
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

    const auto blocksworld_domain = benchmark_prefix() / "classical" / "profiling" / "blocksworld-large-simple" / "domain.pddl";
    const auto blocksworld_task_file = benchmark_prefix() / "classical" / "profiling" / "blocksworld-large-simple" / "p-300-4.pddl";
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
                EXPECT_EQ(rule.get_callee().get_name(), "tower");
            }
        },
        call_rule.get_variant());
    EXPECT_TRUE(found_call_rule) << "Expected call rule.";
}

TEST(RunirTests, ExtModuleParserLowersNamedCalleesWithoutPreexistingModules)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>();
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
                EXPECT_EQ(rule.get_callee().get_name(), "callee");
            }
        },
        rules.front().get_variant());
    EXPECT_TRUE(found_call_rule) << "Expected call rule.";
}

TEST(RunirTests, ExtModuleParserRejectsInvalidModuleSets)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>();
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

    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>();
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
        EXPECT_NE(std::string(err.what()).find("Arity mismatch for action move: expected 2, got 0"), std::string::npos) << err.what();
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
        EXPECT_NE(std::string(err.what()).find("Undefined feature: Missing"), std::string::npos) << err.what();
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
    EXPECT_THROW(kr::ps::ext::dl::parse_module(inline_argument_expression, planning_task.get_domain().get_domain(), *repository), kr::ParseError);
}

TEST(RunirTests, ExtModuleParserRejectsInvalidSections)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
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
    expect_parse_error_containing(duplicate_register_identifier, "Duplicate concept register definition: r0");

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
    expect_parse_error_containing(duplicate_argument_identifier, "Duplicate concept argument definition: x");

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
    expect_parse_error_containing(out_of_range_argument, "Undefined concept argument: missing");

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
        EXPECT_NE(std::string(err.what()).find("Undefined concept argument: missing"), std::string::npos) << err.what();
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
    (:features
        (:concept
            (:symbol B)
            (:expression (c_top))
        )
    )
    (:rules
        (:rule
            (:symbol auto7)
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:load
                    (:conditions)
                    (:conditions)
                    (:concept B)
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
    (:features
        (:concept
            (:symbol B)
            (:expression (c_top))
        )
    )
    (:rules
        (:rule
            (:symbol auto9)
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:load
                    (:conditions)
                    (:concept B)
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
    (:features
        (:concept
            (:symbol B)
            (:expression (c_top))
        )
    )
    (:rules
        (:rule
            (:symbol auto11)
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:load
                    (:conditions)
                    (:concept B)
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
    (:features
        (:concept
            (:symbol B)
            (:expression (c_top))
        )
    )
    (:rules
        (:rule
            (:symbol auto12)
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:load
                    (:conditions)
                    (:concept B)
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

    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>();
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

    const auto domain = benchmark_prefix() / "classical" / "profiling" / "blocksworld-large-simple" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "profiling" / "blocksworld-large-simple" / "p-300-4.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>();
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

    const auto domain = benchmark_prefix() / "classical" / "profiling" / "blocksworld-large-simple" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "profiling" / "blocksworld-large-simple" / "p-300-4.pddl";
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
    const auto reparsed_program = kr::ps::ext::dl::parse_module_program(formatted_program, planning_task.get_domain().get_domain(), *repository);
    EXPECT_EQ(reparsed_program.get_entry_module().get_name(), program.get_entry_module().get_name());
    EXPECT_EQ(reparsed_program.get_modules().size(), program.get_modules().size());
}

TEST(RunirTests, ExtModuleFormatterPreservesOrderedDeclarations)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>();
    auto repository_factory = kr::ps::ext::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(planning_task.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto seed = R"RUNIR((:module
    (:symbol seed)
    (:arguments (:concept Z) (:concept X))
    (:registers (:concept RZ) (:concept RX))
    (:entry m0)
    (:memory m0)
    (:features)
    (:rules)
))RUNIR";
    const auto target = R"RUNIR((:module
    (:symbol target)
    (:arguments (:concept Y) (:concept X))
    (:registers (:concept RY) (:concept RX))
    (:entry m0)
    (:memory m0)
    (:features)
    (:rules)
))RUNIR";

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

    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>();
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

    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>();
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

    const auto domain = benchmark_prefix() / "classical" / "profiling" / "blocksworld-large-simple" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "profiling" / "blocksworld-large-simple" / "p-100-2.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);
    auto task_context = kr::TaskContext<p::GroundTag>::create(search_context);

    auto dl_repository = task_context->ext_dl_repository;
    auto repository = task_context->ext_repository;

    const auto program = kr::ps::ext::dl::ModuleFactory::create_bonet_et_al_icaps2024_program(planning_task.get_domain().get_domain(), *repository);
    ASSERT_EQ(program.get_modules().size(), 5);

    auto search_options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag>();

    const auto search_result = kr::ps::ext::find_solution(task_context, program, search_options);
    EXPECT_TRUE(search_result.is_successful());
    ASSERT_TRUE(search_result.plan.has_value());
    EXPECT_EQ(search_result.plan->get_length(), 4);

    auto proof_options = search_options;
    proof_options.universal = true;
    const auto proof = kr::ps::ext::find_solution(task_context, program, proof_options);
    EXPECT_EQ(proof.status, kr::ps::ext::ModuleProgramProofStatus::FAILURE) << fmt::format("{}", proof);
    ASSERT_TRUE(proof.graph);
    ASSERT_TRUE(search_result.graph);
    EXPECT_GT(proof.graph->get_num_vertices(), search_result.graph->get_num_vertices());

    auto has_internal_memory_state = false;
    auto has_external_memory_state = false;
    for (const auto vertex : proof.graph->get_vertex_indices())
    {
        const auto phase = proof.graph->get_vertex(vertex).get_property().execution_state.get_phase();
        has_internal_memory_state |= phase == kr::ps::ext::ExecutionPhase::INTERNAL;
        has_external_memory_state |= phase == kr::ps::ext::ExecutionPhase::EXTERNAL;
    }
    EXPECT_TRUE(has_internal_memory_state);
    EXPECT_TRUE(has_external_memory_state);

    EXPECT_FALSE(proof.cycle.empty());
}

TEST(RunirTests, ExtModuleParserLowersSupportedTransitions)
{
    namespace fp = tyr::formalism::planning;

    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>();
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
                        (greater_zero N)
                    )
                    (:concept B)
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
                        (unchanged N)
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
                        (unchanged N)
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

    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>();
    auto dl_repository = dl_repository_factory.create(planning_task.get_repository());

    const auto concept_argument = kr::ps::ext::dl::parse_concept(R"((c_argument
    0))",
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

    const auto concept_register = kr::ps::ext::dl::parse_concept(R"((c_register
    1))",
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

    const auto role_argument = kr::ps::ext::dl::parse_role(R"((r_argument
    0))",
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

    const auto domain = benchmark_prefix() / "classical" / "profiling" / "blocksworld-large-simple" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "profiling" / "blocksworld-large-simple" / "p-300-4.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>();
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
    EXPECT_EQ(kr::ps::ext::dl::parser::parse_module_ast(descriptions[0]).name.text, "on");
    EXPECT_EQ(kr::ps::ext::dl::parser::parse_module_ast(descriptions[1]).name.text, "on-table");
    EXPECT_EQ(kr::ps::ext::dl::parser::parse_module_ast(descriptions[2]).name.text, "tower");
    EXPECT_EQ(kr::ps::ext::dl::parser::parse_module_ast(descriptions[3]).name.text, "blocks");

    const auto modules = kr::ps::ext::dl::ModuleFactory::create_bonet_et_al_icaps2024_modules(planning_task.get_domain().get_domain(), *repository);
    ASSERT_EQ(modules.size(), 4);
    EXPECT_EQ(modules[0].get_name(), "on");
    EXPECT_EQ(modules[1].get_name(), "on-table");
    EXPECT_EQ(modules[2].get_name(), "tower");
    EXPECT_EQ(modules[3].get_name(), "blocks");

    const auto program_description =
        kr::ps::ext::dl::parser::parse_module_program_ast(kr::ps::ext::dl::ModuleFactory::create_bonet_et_al_icaps2024_program_description());
    EXPECT_EQ(program_description.entry.text, "root");
    ASSERT_EQ(program_description.modules.size(), 5);
    EXPECT_EQ(program_description.modules.front().name.text, "root");

    const auto program = kr::ps::ext::dl::ModuleFactory::create_bonet_et_al_icaps2024_program(planning_task.get_domain().get_domain(), *repository);
    EXPECT_EQ(program.get_entry_module().get_name(), "root");
    ASSERT_EQ(program.get_modules().size(), 5);
    EXPECT_EQ(program.get_modules()[0].get_name(), "root");
}

TEST(RunirTests, ExtExecutionRepositoryPersistsRecordsAndSharesCallers)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);
    auto task_context = kr::TaskContext<p::GroundTag>::create(search_context);

    auto dl_repository = task_context->ext_dl_repository;
    auto repository = task_context->ext_repository;

    const auto caller_entry = create_memory_state(*repository, "caller_entry");
    const auto caller_return = create_memory_state(*repository, "caller_return");
    const auto callee_entry = create_memory_state(*repository, "callee_entry");
    const auto caller = create_module(*repository, "caller", caller_entry, { caller_entry, caller_return });
    const auto callee = create_module(*repository, "callee", callee_entry, { callee_entry });

    const auto program = create_module_program(*repository, caller, { caller, callee });
    auto& execution_builder = task_context->execution_builder;
    auto execution_repository = task_context->execution_repository;

    const void* scratch_address = nullptr;
    {
        auto scratch = execution_builder.get_builder<kr::ps::ext::RegisterValues>();
        scratch_address = scratch.get();
        EXPECT_EQ(scratch->concept_values.size(), kr::dl::num_registers);
        EXPECT_EQ(scratch->role_values.size(), kr::dl::num_registers);
        EXPECT_TRUE(kr::ps::ext::is_canonical(*scratch));
        scratch->concept_values[0] = ygg::Index<tyr::formalism::Object>(0);
    }
    {
        auto scratch = execution_builder.get_builder<kr::ps::ext::RegisterValues>();
        EXPECT_EQ(scratch.get(), scratch_address);
        EXPECT_FALSE(scratch->concept_values[0]);
        EXPECT_FALSE(scratch->role_values[0]);
    }

    const auto registers = [&]()
    {
        auto data = execution_builder.get_builder<kr::ps::ext::RegisterValues>();
        data->concept_values[0] = ygg::Index<tyr::formalism::Object>(0);
        auto& role = data->role_values[0].emplace();
        role.first = ygg::Index<tyr::formalism::Object>(0);
        role.second = ygg::Index<tyr::formalism::Object>(1);
        kr::ps::ext::canonicalize(*data);
        return execution_repository->get_or_create(*data).first;
    }();

    {
        auto data = execution_builder.get_builder<kr::ps::ext::RegisterValues>();
        data->concept_values[0] = ygg::Index<tyr::formalism::Object>(0);
        auto& role = data->role_values[0].emplace();
        role.first = ygg::Index<tyr::formalism::Object>(0);
        role.second = ygg::Index<tyr::formalism::Object>(1);
        kr::ps::ext::canonicalize(*data);
        const auto found = execution_repository->find(*data);
        ASSERT_TRUE(found);
        EXPECT_EQ(found->get_index(), registers.get_index());
        const auto [duplicate, created] = execution_repository->get_or_create(*data);
        EXPECT_FALSE(created);
        EXPECT_EQ(duplicate.get_index(), registers.get_index());
    }

    const auto arguments = [&]()
    {
        auto data = execution_builder.get_builder<kr::ps::ext::CallArguments>();
        kr::ps::ext::canonicalize(*data);
        return execution_repository->get_or_create(*data).first;
    }();

    const auto caller_frame = [&]()
    {
        auto data = execution_builder.get_builder<kr::ps::ext::CallStack>();
        ygg::set(caller, data->module);
        ygg::set(caller_return, data->memory_state);
        ygg::set(registers, data->registers);
        ygg::set(arguments, data->arguments);
        kr::ps::ext::canonicalize(*data);
        return execution_repository->get_or_create(*data).first;
    }();

    const auto callee_frame = [&]()
    {
        auto data = execution_builder.get_builder<kr::ps::ext::CallStack>();
        ygg::set(callee, data->module);
        ygg::set(callee_entry, data->memory_state);
        ygg::set(registers, data->registers);
        ygg::set(arguments, data->arguments);
        ygg::set(std::optional { caller_frame }, data->caller);
        kr::ps::ext::canonicalize(*data);
        return execution_repository->get_or_create(*data).first;
    }();

    ASSERT_TRUE(callee_frame.get_caller());
    EXPECT_EQ(callee_frame.get_caller()->get_index(), caller_frame.get_index());
    EXPECT_EQ(callee_frame.get_caller()->get_registers().get_concept_values()[0].value().get_index(), ygg::Index<tyr::formalism::Object>(0));
    const auto role_value = callee_frame.get_caller()->get_registers().get_role_values()[0];
    ASSERT_TRUE(role_value);
    const auto pair = role_value.value();
    EXPECT_EQ(pair.get_first().get_index(), ygg::Index<tyr::formalism::Object>(0));
    EXPECT_EQ(pair.get_second().get_index(), ygg::Index<tyr::formalism::Object>(1));

    const auto state = search_context->successor_generator->get_initial_node().get_state();
    const auto returned_state = [&]()
    {
        auto data = execution_builder.get_builder<kr::ps::ext::ExecutionState<p::GroundTag>>();
        ygg::set(state, data->state);
        ygg::set(program, data->program);
        ygg::set(caller_frame, data->call_stack);
        data->phase = kr::ps::ext::ExecutionPhase::EXTERNAL;
        kr::ps::ext::canonicalize(*data);
        return execution_repository->get_or_create(*data).first;
    }();
    EXPECT_EQ(returned_state.get_call_stack().get_module().get_name(), "caller");
    EXPECT_EQ(returned_state.get_program().get_index(), program.get_index());
}

template<tyr::planning::TaskKind Kind>
void expect_initial_execution_state_uses_expander_repository()
{
    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    auto task_context = create_task_context<Kind>(domain, task_file);
    auto dl_repository = task_context->ext_dl_repository;
    auto repository = task_context->ext_repository;
    const auto entry = create_memory_state(*repository, "entry");
    const auto module = create_module(*repository, "module", entry, { entry });
    const auto program = create_module_program(*repository, module, { module });
    auto expander = kr::ps::ext::SuccessorExpander<Kind>(task_context, program);
    const auto state = expander.initial_state();

    task_context.reset();
    repository.reset();
    dl_repository.reset();
    EXPECT_EQ(state.get_call_stack().get_module().get_name(), "module");
    EXPECT_EQ(state.get_call_stack().get_memory_state().get_name(), "entry");
    EXPECT_EQ(state.get_program().get_entry_module().get_name(), "module");
}

TEST(RunirTests, ExtGroundAndLiftedInitialStatesUseExpanderRepository)
{
    expect_initial_execution_state_uses_expander_repository<tyr::planning::GroundTag>();
    expect_initial_execution_state_uses_expander_repository<tyr::planning::LiftedTag>();
}

TEST(RunirTests, ExtProofLabelsOutliveTheGraphAndExpander)
{
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    auto task_context = create_task_context<p::GroundTag>(domain, task_file);
    auto dl_repository = task_context->ext_dl_repository;
    auto repository = task_context->ext_repository;
    const auto module = kr::ps::ext::dl::parse_module(R"((:module
        (:symbol module)
        (:arguments)
        (:registers)
        (:entry source)
        (:memory source target)
        (:features)
        (:rules
            (:rule
                (:symbol advance)
                (:expression
                    (:source-memory source)
                    (:target-memory target)
                    (:sketch (:conditions) (:effects))
                )
            )
        )
    ))",
                                                      task_context->search_context->task->get_domain().get_domain(),
                                                      *repository);
    const auto program = create_module_program(*repository, module, { module });
    const auto expected_rule = module.get_memory_transitions().front().front().get_index();
    auto options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag> {};
    options.universal = true;
    auto result = kr::ps::ext::find_solution(task_context, program, options);
    EXPECT_TRUE(result.graph);
    EXPECT_EQ(result.graph->get_num_vertices(), 2);
    EXPECT_EQ(result.graph->get_num_edges(), 1);
    auto graph = result.graph;
    const auto task_context_weak = std::weak_ptr(task_context);

    result.graph.reset();
    result.task_context_owner.reset();
    task_context.reset();
    repository.reset();
    dl_repository.reset();
    ASSERT_FALSE(task_context_weak.expired());
    const auto labels = std::pair(graph->get_vertex(0).get_property(), graph->get_edge(0).get_property());

    result.task_context_owner = task_context_weak.lock();
    graph.reset();
    EXPECT_EQ(labels.first.execution_state.get_call_stack().get_module().get_name(), "module");
    ASSERT_TRUE(labels.second.rule);
    EXPECT_EQ(labels.second.rule->get_index(), expected_rule);
    EXPECT_EQ(labels.second.rule->get_symbol(), "advance");
}

TEST(RunirTests, ExtExecutionRecordsAreCistaCompatible)
{
    namespace p = tyr::planning;

    auto builder = kr::ps::ext::ExecutionBuilder<p::GroundTag>();
    {
        auto data = builder.get_builder<kr::ps::ext::RegisterValues>();
        EXPECT_EQ(data->concept_values.size(), kr::dl::num_registers);
        EXPECT_EQ(data->role_values.size(), kr::dl::num_registers);
        data->concept_values[0] = ygg::Index<tyr::formalism::Object>(3);
        auto& role = data->role_values[0].emplace();
        role.first = ygg::Index<tyr::formalism::Object>(4);
        role.second = ygg::Index<tyr::formalism::Object>(5);
        expect_cista_round_trip(*data);
    }
    {
        auto data = builder.get_builder<kr::ps::ext::CallArguments>();
        expect_cista_round_trip(*data);
    }
    {
        auto data = builder.get_builder<kr::ps::ext::CallStack>();
        data->module = ygg::Index<kr::ps::ext::Module>(1);
        data->memory_state = ygg::Index<kr::ps::ext::MemoryState>(2);
        data->registers = ygg::Index<kr::ps::ext::RegisterValues>(3);
        data->arguments = ygg::Index<kr::ps::ext::CallArguments>(4);
        expect_cista_round_trip(*data);
    }
    {
        auto data = builder.get_builder<kr::ps::ext::ExecutionState<p::GroundTag>>();
        data->state = ygg::Index<p::State<p::GroundTag>>(5);
        data->program = ygg::Index<kr::ps::ext::ModuleProgram>(6);
        data->call_stack = ygg::Index<kr::ps::ext::CallStack>(7);
        data->phase = kr::ps::ext::ExecutionPhase::INTERNAL;
        expect_cista_round_trip(*data);
    }
}

TEST(RunirTests, ExtLoadRuleEnumeratesAllObjectsAndAdvancesMemory)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);
    auto task_context = kr::TaskContext<p::GroundTag>::create(search_context);

    auto dl_repository = task_context->ext_dl_repository;
    auto repository = task_context->ext_repository;

    const auto source = create_memory_state(*repository, "source");
    const auto target = create_memory_state(*repository, "target");
    const auto reg = create_register(*repository, "r0", 0);
    const auto top_concept = create_top_concept(*dl_repository);
    const auto top_feature = create_concept_feature(*repository, top_concept.get_index(), "top");

    auto load_data = ygg::Data<kr::ps::ext::Rule<kr::ps::ext::LoadTag<kr::dl::ConceptTag>>>();
    load_data.source = source.get_index();
    load_data.target = target.get_index();
    load_data.feature = top_feature.get_index();
    load_data.reg = reg.get_index();
    kr::ps::ext::canonicalize(load_data);
    const auto load = repository->get_or_create(load_data).first;

    auto variant_data = ygg::Data<kr::ps::ext::RuleVariant>(load.get_index());
    const auto variant = repository->get_or_create(variant_data).first;

    auto module_data = make_module_data(*repository, "module");
    module_data.entry_memory_state = source.get_index();
    module_data.memory_states.push_back(source.get_index());
    module_data.memory_states.push_back(target.get_index());
    module_data.concept_registers.push_back(reg.get_index());
    module_data.concept_features.push_back(top_feature.get_index());
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
    EXPECT_NE(formatted.find("(:concept top)"), std::string::npos) << formatted;
    EXPECT_NE(formatted.find("(:register\n                        (:concept r0)\n                    )"), std::string::npos) << formatted;

    const auto program = create_module_program(*repository, module, { module });
    auto expander = kr::ps::ext::SuccessorExpander<p::GroundTag>(task_context, program);
    const auto initial_state = expander.initial_state();
    const auto steps = expander.load_steps(initial_state);
    ASSERT_GT(steps.size(), 1);

    auto loaded_objects = std::set<ygg::uint_t> {};
    for (const auto& step : steps)
    {
        EXPECT_EQ(step.status, kr::ps::ext::detail::ModuleProgramOutcome::APPLIED);
        const auto target_state = step.get_target();
        EXPECT_EQ(target_state.get_state().get_index(), initial_state.get_state().get_index());
        EXPECT_EQ(target_state.get_call_stack().get_memory_state().get_index(), target.get_index());
        const auto loaded = target_state.get_call_stack().get_registers().get_concept_values()[0];
        ASSERT_TRUE(loaded);
        loaded_objects.insert(ygg::uint_t(loaded.value().get_index()));
    }
    EXPECT_EQ(loaded_objects.size(), steps.size());

    auto greedy_options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag> {};
    const auto greedy = kr::ps::ext::find_solution(task_context, program, greedy_options);
    auto universal_options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag> {};
    universal_options.universal = true;
    const auto universal = kr::ps::ext::find_solution(task_context, program, universal_options);
    ASSERT_TRUE(greedy.graph);
    ASSERT_TRUE(universal.graph);
    EXPECT_EQ(greedy.graph->get_out_degree(0), 1);
    EXPECT_EQ(universal.graph->get_out_degree(0), steps.size());

    auto expected_steps = steps;
    auto random = std::mt19937_64(1);
    p::portable_shuffle(expected_steps.begin(), expected_steps.end(), random);
    ASSERT_NE(expected_steps.front().get_target().get_index(), steps.front().get_target().get_index());

    auto shuffled_options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag> {};
    shuffled_options.random_seed = 1;
    shuffled_options.shuffle_choice_points = true;
    const auto shuffled = kr::ps::ext::find_solution(task_context, program, shuffled_options);
    ASSERT_TRUE(shuffled.graph);
    ASSERT_EQ(shuffled.graph->get_out_degree(0), 1);
    const auto shuffled_edge = shuffled.graph->get_out_edge_indices(0).front();
    const auto shuffled_target = shuffled.graph->get_vertex(shuffled.graph->get_target(shuffled_edge)).get_property().execution_state;
    const auto actual_loaded = shuffled_target.get_call_stack().get_registers().get_concept_values()[0];
    const auto expected_loaded = expected_steps.front().get_target().get_call_stack().get_registers().get_concept_values()[0];
    ASSERT_TRUE(actual_loaded);
    ASSERT_TRUE(expected_loaded);
    EXPECT_EQ(actual_loaded.value().get_index(), expected_loaded.value().get_index());

    auto options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag> {};
    options.max_num_states = 1;
    options.universal = true;
    const auto bounded = kr::ps::ext::find_solution(task_context, program, options);
    EXPECT_EQ(bounded.status, kr::ps::ext::ModuleProgramProofStatus::OUT_OF_STATES);
    ASSERT_TRUE(bounded.graph);
    EXPECT_EQ(bounded.graph->get_num_vertices(), 1);
}

TEST(RunirTests, ExtRoleLoadRuleEnumeratesAllPairsAndAdvancesMemory)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);
    auto task_context = kr::TaskContext<p::GroundTag>::create(search_context);

    auto dl_repository = task_context->ext_dl_repository;
    auto repository = task_context->ext_repository;

    const auto module = kr::ps::ext::dl::parse_module(R"((:module
    (:symbol module)
    (:arguments)
    (:registers
        (:role r0)
    )
    (:entry source)
    (:memory source target)
    (:features
        (:role
            (:symbol At)
            (:expression (r_atomic_state "at"))
        )
    )
    (:rules
        (:rule
            (:symbol load-edge)
            (:expression
                (:source-memory source)
                (:target-memory target)
                (:load
                    (:conditions)
                    (:role At)
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

    const auto formatted = fmt::format("{}", module);
    EXPECT_NE(formatted.find("(:role At)"), std::string::npos) << formatted;
    EXPECT_NE(formatted.find("(:register\n                        (:role r0)\n                    )"), std::string::npos) << formatted;

    const auto program = create_module_program(*repository, module, { module });
    auto expander = kr::ps::ext::SuccessorExpander<p::GroundTag>(task_context, program);
    const auto initial_state = expander.initial_state();
    const auto steps = expander.load_steps(initial_state);
    ASSERT_GT(steps.size(), 1);

    auto loaded_pairs = std::set<std::pair<ygg::uint_t, ygg::uint_t>> {};
    for (const auto& step : steps)
    {
        EXPECT_EQ(step.status, kr::ps::ext::detail::ModuleProgramOutcome::APPLIED);
        const auto target_state = step.get_target();
        EXPECT_EQ(target_state.get_state().get_index(), initial_state.get_state().get_index());
        EXPECT_EQ(target_state.get_call_stack().get_memory_state().get_name(), "target");
        const auto loaded = target_state.get_call_stack().get_registers().get_role_values()[0];
        ASSERT_TRUE(loaded);
        const auto pair = loaded.value();
        loaded_pairs.emplace(ygg::uint_t(pair.get_first().get_index()), ygg::uint_t(pair.get_second().get_index()));
    }
    EXPECT_EQ(loaded_pairs.size(), steps.size());
}

TEST(RunirTests, ExtCallRulePassesArgumentDenotationsToCallee)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);
    auto task_context = kr::TaskContext<p::GroundTag>::create(search_context);

    auto dl_repository = task_context->ext_dl_repository;
    auto repository = task_context->ext_repository;

    const auto caller_entry = create_memory_state(*repository, "caller_entry");
    const auto caller_return = create_memory_state(*repository, "caller_return");
    const auto callee_entry = create_memory_state(*repository, "callee_entry");
    auto concept_arg_data = ygg::Data<kr::dl::Argument<kr::dl::ConceptTag>>(std::string("x"), kr::dl::ArgumentIdentifier<kr::dl::ConceptTag>(0));
    const auto concept_arg = dl_repository->get_or_create(concept_arg_data).first;
    auto role_arg_data = ygg::Data<kr::dl::Argument<kr::dl::RoleTag>>(std::string("r"), kr::dl::ArgumentIdentifier<kr::dl::RoleTag>(0));
    const auto role_arg = dl_repository->get_or_create(role_arg_data).first;
    auto boolean_arg_data = ygg::Data<kr::dl::Argument<kr::dl::BooleanTag>>(std::string("b"), kr::dl::ArgumentIdentifier<kr::dl::BooleanTag>(0));
    const auto boolean_arg = dl_repository->get_or_create(boolean_arg_data).first;
    auto numerical_arg_data = ygg::Data<kr::dl::Argument<kr::dl::NumericalTag>>(std::string("n"), kr::dl::ArgumentIdentifier<kr::dl::NumericalTag>(0));
    const auto numerical_arg = dl_repository->get_or_create(numerical_arg_data).first;

    auto callee_data = make_module_data(*repository, "callee");
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
    auto call_data = ygg::Data<kr::ps::ext::Rule<kr::ps::ext::CallTag>>();
    call_data.source = caller_entry.get_index();
    call_data.target = caller_return.get_index();
    const auto top_feature = create_feature<kr::dl::ConceptTag>(*repository, top_concept.get_index(), "top");
    const auto universal_feature = create_feature<kr::dl::RoleTag>(*repository, universal_role.get_index(), "universal");
    const auto true_feature = create_feature<kr::ps::dl::BooleanFeature>(*repository, true_boolean.get_index(), "true");
    const auto count_feature = create_feature<kr::ps::dl::NumericalFeature>(*repository, object_count.get_index(), "count");
    call_data.callee = callee.get_symbol().get_index();
    call_data.arguments.push_back(top_feature.get_index());
    call_data.arguments.push_back(universal_feature.get_index());
    call_data.arguments.push_back(true_feature.get_index());
    call_data.arguments.push_back(count_feature.get_index());
    kr::ps::ext::canonicalize(call_data);
    const auto call = repository->get_or_create(call_data).first;

    auto variant_data = ygg::Data<kr::ps::ext::RuleVariant>(call.get_index());
    const auto variant = repository->get_or_create(variant_data).first;
    auto caller_data = make_module_data(*repository, "caller");
    caller_data.entry_memory_state = caller_entry.get_index();
    caller_data.memory_states.push_back(caller_entry.get_index());
    caller_data.memory_states.push_back(caller_return.get_index());
    auto transition = ygg::IndexList<kr::ps::ext::RuleVariant> {};
    transition.push_back(variant.get_index());
    ygg::canonicalize(transition);
    caller_data.memory_transitions.push_back(std::move(transition));
    kr::ps::ext::canonicalize(caller_data);
    const auto caller = repository->get_or_create(caller_data).first;

    const auto program = create_module_program(*repository, caller, { caller, callee });
    auto expander = kr::ps::ext::SuccessorExpander<p::GroundTag>(task_context, program);
    const auto initial_state = expander.initial_state();
    const auto call_steps = expander.control_steps(initial_state);
    ASSERT_EQ(call_steps.size(), 1);
    EXPECT_EQ(call_steps.front().status, kr::ps::ext::detail::ModuleProgramOutcome::APPLIED);
    const auto call_target = call_steps.front().get_target();
    const auto call_stack = call_target.get_call_stack();
    EXPECT_EQ(call_stack.get_module().get_index(), callee.get_index());
    EXPECT_EQ(call_stack.get_memory_state().get_index(), callee_entry.get_index());
    const auto arguments = call_stack.get_arguments();
    const auto concept_arguments = arguments.get<kr::dl::ConceptTag>();
    const auto role_arguments = arguments.get<kr::dl::RoleTag>();
    const auto boolean_arguments = arguments.get<kr::dl::BooleanTag>();
    const auto numerical_arguments = arguments.get<kr::dl::NumericalTag>();
    ASSERT_EQ(concept_arguments.size(), 1);
    ASSERT_EQ(role_arguments.size(), 1);
    ASSERT_EQ(boolean_arguments.size(), 1);
    ASSERT_EQ(numerical_arguments.size(), 1);

    const auto concept_denotation = concept_arguments.front();
    const auto concept_first = concept_denotation.begin();
    ASSERT_NE(concept_first, concept_denotation.end());
    EXPECT_EQ((*concept_first).get_index(), ygg::Index<tyr::formalism::Object>(0));

    const auto role_denotation = role_arguments.front();
    const auto role_first = role_denotation.begin();
    ASSERT_NE(role_first, role_denotation.end());
    const auto role_pair = *role_first;
    EXPECT_EQ(role_pair.first.get_index(), ygg::Index<tyr::formalism::Object>(0));
    EXPECT_EQ(role_pair.second.get_index(), ygg::Index<tyr::formalism::Object>(0));

    EXPECT_TRUE(boolean_arguments.front().get());
    EXPECT_GT(numerical_arguments.front().get(), 0);

    const auto caller_frame = call_stack.get_caller();
    ASSERT_TRUE(caller_frame);
    const auto return_steps = expander.control_steps(call_target);
    ASSERT_EQ(return_steps.size(), 1);
    EXPECT_EQ(return_steps.front().status, kr::ps::ext::detail::ModuleProgramOutcome::RESTORED_CALLER);
    EXPECT_EQ(return_steps.front().get_target().get_call_stack().get_index(), caller_frame->get_index());
    EXPECT_EQ(return_steps.front().get_target().get_call_stack().get_module().get_index(), caller.get_index());
    EXPECT_EQ(return_steps.front().get_target().get_call_stack().get_memory_state().get_index(), caller_return.get_index());

    const auto repeated_call_steps = expander.control_steps(initial_state);
    ASSERT_EQ(repeated_call_steps.size(), 1);
    EXPECT_EQ(repeated_call_steps.front().get_target().get_index(), call_target.get_index());
}

TEST(RunirTests, ExtCallRuleResolvesNamedCalleeFromModuleRegistry)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);
    auto task_context = kr::TaskContext<p::GroundTag>::create(search_context);

    auto dl_repository = task_context->ext_dl_repository;
    auto repository = task_context->ext_repository;

    const auto caller_entry = create_memory_state(*repository, "caller_entry");
    const auto caller_return = create_memory_state(*repository, "caller_return");
    const auto callee_entry = create_memory_state(*repository, "callee_entry");
    const auto callee = create_module(*repository, "callee", callee_entry, { callee_entry });

    auto call_data = ygg::Data<kr::ps::ext::Rule<kr::ps::ext::CallTag>>();
    call_data.source = caller_entry.get_index();
    call_data.target = caller_return.get_index();
    call_data.callee = callee.get_symbol().get_index();
    kr::ps::ext::canonicalize(call_data);
    const auto call = repository->get_or_create(call_data).first;

    auto variant_data = ygg::Data<kr::ps::ext::RuleVariant>(call.get_index());
    const auto variant = repository->get_or_create(variant_data).first;
    auto caller_data = make_module_data(*repository, "caller");
    caller_data.entry_memory_state = caller_entry.get_index();
    caller_data.memory_states.push_back(caller_entry.get_index());
    caller_data.memory_states.push_back(caller_return.get_index());
    auto transition = ygg::IndexList<kr::ps::ext::RuleVariant> {};
    transition.push_back(variant.get_index());
    ygg::canonicalize(transition);
    caller_data.memory_transitions.push_back(std::move(transition));
    kr::ps::ext::canonicalize(caller_data);
    const auto caller = repository->get_or_create(caller_data).first;

    const auto program = create_module_program(*repository, caller, { caller, callee });
    auto expander = kr::ps::ext::SuccessorExpander<p::GroundTag>(task_context, program);
    const auto steps = expander.control_steps(expander.initial_state());
    ASSERT_EQ(steps.size(), 1);
    EXPECT_EQ(steps.front().status, kr::ps::ext::detail::ModuleProgramOutcome::APPLIED);
    EXPECT_EQ(steps.front().get_target().get_call_stack().get_module().get_index(), callee.get_index());
    EXPECT_EQ(steps.front().get_target().get_call_stack().get_memory_state().get_index(), callee_entry.get_index());
}

TEST(RunirTests, ExtDoRuleAppliesMatchingActionAndAdvancesMemory)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);
    auto task_context = kr::TaskContext<p::GroundTag>::create(search_context);

    auto dl_repository = task_context->ext_dl_repository;
    auto repository = task_context->ext_repository;

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
    auto variant_data = ygg::Data<kr::ps::ext::RuleVariant>(rule.get_index());
    const auto variant = repository->get_or_create(variant_data).first;

    auto module_data = make_module_data(*repository, "module");
    module_data.entry_memory_state = source.get_index();
    module_data.memory_states.push_back(source.get_index());
    module_data.memory_states.push_back(target.get_index());
    auto transition = ygg::IndexList<kr::ps::ext::RuleVariant> {};
    transition.push_back(variant.get_index());
    ygg::canonicalize(transition);
    module_data.memory_transitions.push_back(std::move(transition));
    kr::ps::ext::canonicalize(module_data);
    const auto module = repository->get_or_create(module_data).first;
    const auto program = create_module_program(*repository, module, { module });
    auto expander = kr::ps::ext::SuccessorExpander<p::GroundTag>(task_context, program);
    const auto initial_state = expander.initial_state();
    const auto steps = expander.control_steps(initial_state);
    ASSERT_GT(steps.size(), 1);

    for (const auto& step : steps)
    {
        EXPECT_EQ(step.status, kr::ps::ext::detail::ModuleProgramOutcome::APPLIED);
        ASSERT_EQ(step.plan_suffix.size(), 1);
        EXPECT_EQ(step.plan_suffix.front().label.get_action().get_name(), "pick");
        EXPECT_EQ(step.get_target().get_call_stack().get_memory_state().get_index(), target.get_index());
        EXPECT_NE(step.get_target().get_state().get_index(), initial_state.get_state().get_index());
    }

    auto greedy_options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag> {};
    const auto greedy = kr::ps::ext::find_solution(task_context, program, greedy_options);
    auto universal_options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag> {};
    universal_options.universal = true;
    const auto universal = kr::ps::ext::find_solution(task_context, program, universal_options);
    ASSERT_TRUE(greedy.graph);
    ASSERT_TRUE(universal.graph);
    EXPECT_EQ(greedy.graph->get_out_degree(0), 1);
    EXPECT_EQ(universal.graph->get_out_degree(0), steps.size());

    auto expected_steps = steps;
    auto random = std::mt19937_64(1);
    p::portable_shuffle(expected_steps.begin(), expected_steps.end(), random);
    ASSERT_NE(expected_steps.front().get_target().get_index(), steps.front().get_target().get_index());

    auto shuffled_options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag> {};
    shuffled_options.random_seed = 1;
    shuffled_options.shuffle_choice_points = true;
    const auto shuffled = kr::ps::ext::find_solution(task_context, program, shuffled_options);
    ASSERT_TRUE(shuffled.graph);
    ASSERT_EQ(shuffled.graph->get_out_degree(0), 1);
    const auto shuffled_edge = shuffled.graph->get_out_edge_indices(0).front();
    const auto shuffled_target = shuffled.graph->get_vertex(shuffled.graph->get_target(shuffled_edge)).get_property().execution_state;
    EXPECT_EQ(shuffled_target.get_state().get_index(), expected_steps.front().get_target().get_state().get_index());
    EXPECT_EQ(shuffled_target.get_call_stack().get_memory_state().get_index(),
              expected_steps.front().get_target().get_call_stack().get_memory_state().get_index());
}

TEST(RunirTests, ExtDoRuleRejectsActionWithIncompatibleDeclaredEffects)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);
    auto task_context = kr::TaskContext<p::GroundTag>::create(search_context);

    auto dl_repository = task_context->ext_dl_repository;
    auto repository = task_context->ext_repository;

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
                (n_distance
                    (c_atomic_state "ball")
                    (r_atomic_state "at")
                    (c_atomic_state "room")
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
                        (decreases C)
                    )
                )
            )
        )
    )
))",
                                                      planning_task.get_domain().get_domain(),
                                                      *repository);
    const auto program = create_module_program(*repository, module, { module });
    auto expander = kr::ps::ext::SuccessorExpander<p::GroundTag>(task_context, program);
    const auto initial_state = expander.initial_state();
    const auto steps = expander.control_steps(initial_state);
    ASSERT_EQ(steps.size(), 1);
    EXPECT_EQ(steps.front().status, kr::ps::ext::detail::ModuleProgramOutcome::NO_APPLICABLE_ACTION);
    EXPECT_EQ(steps.front().get_target().get_state().get_index(), initial_state.get_state().get_index());

    auto options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag> {};
    options.universal = true;
    const auto result = kr::ps::ext::find_solution(task_context, program, options);
    EXPECT_EQ(result.status, kr::ps::ext::ModuleProgramProofStatus::FAILURE);
    ASSERT_TRUE(result.graph);
    EXPECT_EQ(result.graph->get_num_vertices(), 1);
    EXPECT_EQ(result.graph->get_num_edges(), 0);
    EXPECT_TRUE(result.deadend_states.empty());
    EXPECT_FALSE(result.open_states.empty());
}

TEST(RunirTests, ExtImmediateExternalRulesUseCanonicalFirstApplicableRule)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);
    auto task_context = kr::TaskContext<p::GroundTag>::create(search_context);

    auto dl_repository = task_context->ext_dl_repository;
    auto repository = task_context->ext_repository;

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

    auto module_data = make_module_data(*repository, "module");
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

    const auto program = create_module_program(*repository, module, { module });
    auto expander = kr::ps::ext::SuccessorExpander<p::GroundTag>(task_context, program);
    const auto steps = expander.control_steps(expander.initial_state());
    ASSERT_GT(steps.size(), 2);
    EXPECT_EQ(steps.front().status, kr::ps::ext::detail::ModuleProgramOutcome::APPLIED);
    EXPECT_EQ(steps.front().get_target().get_call_stack().get_memory_state().get_index(), move_target.get_index());

    auto reached_move_target = false;
    auto reached_pick_target = false;
    for (const auto& step : steps)
    {
        ASSERT_EQ(step.plan_suffix.size(), 1);
        reached_move_target |= step.get_target().get_call_stack().get_memory_state().get_index() == move_target.get_index();
        reached_pick_target |= step.get_target().get_call_stack().get_memory_state().get_index() == pick_target.get_index();
    }
    EXPECT_TRUE(reached_move_target);
    EXPECT_TRUE(reached_pick_target);

    auto greedy_options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag> {};
    const auto greedy = kr::ps::ext::find_solution(task_context, program, greedy_options);
    auto universal_options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag> {};
    universal_options.universal = true;
    const auto universal = kr::ps::ext::find_solution(task_context, program, universal_options);
    ASSERT_TRUE(greedy.graph);
    ASSERT_TRUE(universal.graph);
    EXPECT_EQ(greedy.graph->get_out_degree(0), 1);
    EXPECT_EQ(universal.graph->get_out_degree(0), steps.size());
}

TEST(RunirTests, ExtSketchUsesOnlyImmediateOutcomesAndUniversalPreservesParallelEdges)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);
    auto task_context = kr::TaskContext<p::GroundTag>::create(search_context);

    auto dl_repository = task_context->ext_dl_repository;
    auto repository = task_context->ext_repository;
    const auto module = kr::ps::ext::dl::parse_module(R"((:module
        (:symbol module)
        (:arguments)
        (:registers)
        (:entry source)
        (:memory source target)
        (:features
            (:numerical
                (:symbol N)
                (:expression (n_count (c_top)))
            )
        )
        (:rules
            (:rule
                (:symbol first)
                (:expression
                    (:source-memory source)
                    (:target-memory target)
                    (:sketch (:conditions) (:effects (unchanged N)))
                )
            )
            (:rule
                (:symbol second)
                (:expression
                    (:source-memory source)
                    (:target-memory target)
                    (:sketch (:conditions) (:effects (unchanged N)))
                )
            )
        )
    ))",
                                                      task->get_domain().get_domain(),
                                                      *repository);
    const auto program = create_module_program(*repository, module, { module });
    auto expander = kr::ps::ext::SuccessorExpander<p::GroundTag>(task_context, program);
    const auto initial_state = expander.initial_state();
    const auto immediate = expander.labeled_successors(initial_state);
    const auto steps = expander.control_steps(initial_state, immediate);
    ASSERT_GT(immediate.size(), 1);
    ASSERT_EQ(steps.size(), immediate.size() * 2);
    for (const auto& step : steps)
        EXPECT_EQ(step.plan_suffix.size(), 1);

    auto greedy_options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag> {};
    const auto greedy = kr::ps::ext::find_solution(task_context, program, greedy_options);
    auto universal_options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag> {};
    universal_options.universal = true;
    const auto universal = kr::ps::ext::find_solution(task_context, program, universal_options);
    ASSERT_TRUE(greedy.graph);
    ASSERT_TRUE(universal.graph);
    EXPECT_EQ(greedy.graph->get_out_degree(0), 1);
    EXPECT_EQ(universal.graph->get_out_degree(0), steps.size());

    auto targets = std::set<runir::graphs::VertexIndex> {};
    for (const auto edge : universal.graph->get_out_edge_indices(0))
        targets.insert(universal.graph->get_target(edge));
    EXPECT_LT(targets.size(), universal.graph->get_out_degree(0));

    const auto two_step_module = kr::ps::ext::dl::parse_module(R"((:module
        (:symbol two-step-module)
        (:arguments)
        (:registers)
        (:entry two-step-source)
        (:memory two-step-source two-step-target)
        (:features
            (:boolean
                (:symbol C)
                (:expression (b_nonempty (c_some (r_atomic_state "carry") (c_top))))
            )
            (:boolean
                (:symbol R)
                (:expression
                    (b_nonempty
                        (c_some
                            (r_atomic_goal "at" true)
                            (c_atomic_state "at-robby")
                        )
                    )
                )
            )
        )
        (:rules
            (:rule
                (:symbol two-step-only)
                (:expression
                    (:source-memory two-step-source)
                    (:target-memory two-step-target)
                    (:sketch
                        (:conditions (negative C) (negative R))
                        (:effects (positive C) (positive R))
                    )
                )
            )
        )
    ))",
                                                               task->get_domain().get_domain(),
                                                               *repository);
    const auto two_step_program = create_module_program(*repository, two_step_module, { two_step_module });
    auto two_step_expander = kr::ps::ext::SuccessorExpander<p::GroundTag>(task_context, two_step_program);
    const auto two_step_state = two_step_expander.initial_state();
    const auto two_step_outcomes = two_step_expander.control_steps(two_step_state);
    ASSERT_EQ(two_step_outcomes.size(), 1);
    EXPECT_EQ(two_step_outcomes.front().status, kr::ps::ext::detail::ModuleProgramOutcome::NO_APPLICABLE_ACTION);

    auto rejected_options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag> {};
    const auto rejected = kr::ps::ext::find_solution(task_context, two_step_program, rejected_options);
    EXPECT_EQ(rejected.status, kr::ps::ext::ModuleProgramProofStatus::FAILURE);
    ASSERT_TRUE(rejected.graph);
    EXPECT_EQ(rejected.graph->get_num_vertices(), 1);
    EXPECT_EQ(rejected.graph->get_num_edges(), 0);
}

TEST(RunirTests, ExtFindSolutionReportsTheCompleteThreeStateCycle)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);
    auto task_context = kr::TaskContext<p::GroundTag>::create(search_context);

    auto dl_repository = task_context->ext_dl_repository;
    auto repository = task_context->ext_repository;
    const auto module = kr::ps::ext::dl::parse_module(R"((:module
        (:symbol module)
        (:arguments)
        (:registers)
        (:entry m0)
        (:memory m0 m1 m2)
        (:features)
        (:rules
            (:rule
                (:symbol step-01)
                (:expression (:source-memory m0) (:target-memory m1) (:sketch (:conditions) (:effects)))
            )
            (:rule
                (:symbol step-12)
                (:expression (:source-memory m1) (:target-memory m2) (:sketch (:conditions) (:effects)))
            )
            (:rule
                (:symbol step-20)
                (:expression (:source-memory m2) (:target-memory m0) (:sketch (:conditions) (:effects)))
            )
        )
    ))",
                                                      task->get_domain().get_domain(),
                                                      *repository);
    const auto program = create_module_program(*repository, module, { module });

    auto options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag> {};
    const auto result = kr::ps::ext::find_solution(task_context, program, options);
    EXPECT_EQ(result.status, kr::ps::ext::ModuleProgramProofStatus::FAILURE);
    ASSERT_TRUE(result.graph);
    EXPECT_EQ(result.graph->get_num_vertices(), 3);
    EXPECT_EQ(result.graph->get_num_edges(), 3);
    ASSERT_EQ(result.cycle.size(), 4);
    EXPECT_EQ(result.cycle.front(), result.cycle.back());
    EXPECT_EQ(std::set(result.cycle.begin(), result.cycle.end()).size(), 3);
}

TEST(RunirTests, ExtExecutorReportsStructuredFailureStatuses)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = runir::datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);
    auto task_context = kr::TaskContext<p::GroundTag>::create(search_context);

    auto dl_repository = task_context->ext_dl_repository;
    auto repository = task_context->ext_repository;

    const auto source = create_memory_state(*repository, "source");
    const auto target = create_memory_state(*repository, "target");
    const auto top_concept = create_top_concept(*dl_repository);
    const auto top_feature = create_concept_feature(*repository, top_concept.get_index(), "top");

    const auto empty_module = create_module(*repository, "empty", source, { source });

    auto proof_options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag>();
    proof_options.universal = true;
    const auto empty_program = create_module_program(*repository, empty_module, { empty_module });
    const auto empty_proof = kr::ps::ext::find_solution(task_context, empty_program, proof_options);
    EXPECT_EQ(empty_proof.status, kr::ps::ext::ModuleProgramProofStatus::FAILURE);
    ASSERT_TRUE(empty_proof.graph);
    EXPECT_EQ(empty_proof.graph->get_num_vertices(), 1);
    EXPECT_EQ(empty_proof.graph->get_num_edges(), 0);
    EXPECT_TRUE(empty_proof.deadend_states.empty());
    EXPECT_FALSE(empty_proof.open_states.empty());
    EXPECT_TRUE(empty_proof.cycle.empty());

    auto load_data = ygg::Data<kr::ps::ext::Rule<kr::ps::ext::LoadTag<kr::dl::ConceptTag>>>();
    load_data.source = source.get_index();
    load_data.target = source.get_index();
    load_data.feature = top_feature.get_index();
    load_data.reg = create_register(*repository, "r0", 0).get_index();
    kr::ps::ext::canonicalize(load_data);
    const auto load = repository->get_or_create(load_data).first;
    auto load_variant_data = ygg::Data<kr::ps::ext::RuleVariant>(load.get_index());
    const auto load_variant = repository->get_or_create(load_variant_data).first;
    auto load_module_data = make_module_data(*repository, "load-loop");
    load_module_data.entry_memory_state = source.get_index();
    load_module_data.memory_states.push_back(source.get_index());
    load_module_data.concept_registers.push_back(load_data.reg);
    load_module_data.concept_features.push_back(top_feature.get_index());
    auto load_transition = ygg::IndexList<kr::ps::ext::RuleVariant>();
    load_transition.push_back(load_variant.get_index());
    ygg::canonicalize(load_transition);
    load_module_data.memory_transitions.push_back(std::move(load_transition));
    kr::ps::ext::canonicalize(load_module_data);
    const auto load_module = repository->get_or_create(load_module_data).first;
    auto load_proof_options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag>();
    load_proof_options.universal = true;
    const auto load_program = create_module_program(*repository, load_module, { load_module });
    const auto load_proof = kr::ps::ext::find_solution(task_context, load_program, load_proof_options);
    EXPECT_EQ(load_proof.status, kr::ps::ext::ModuleProgramProofStatus::FAILURE);
    ASSERT_TRUE(load_proof.graph);
    EXPECT_TRUE(load_proof.deadend_states.empty());
    EXPECT_FALSE(load_proof.cycle.empty());

    auto concept_arg_data = ygg::Data<kr::dl::Argument<kr::dl::ConceptTag>>(std::string("x"), kr::dl::ArgumentIdentifier<kr::dl::ConceptTag>(0));
    const auto concept_arg = dl_repository->get_or_create(concept_arg_data).first;
    auto callee_data = make_module_data(*repository, "callee");
    callee_data.entry_memory_state = target.get_index();
    callee_data.memory_states.push_back(target.get_index());
    callee_data.concept_arguments.push_back(concept_arg.get_index());
    kr::ps::ext::canonicalize(callee_data);
    const auto callee = repository->get_or_create(callee_data).first;
    auto call_data = ygg::Data<kr::ps::ext::Rule<kr::ps::ext::CallTag>>();
    call_data.source = source.get_index();
    call_data.target = target.get_index();
    call_data.callee = callee.get_symbol().get_index();
    kr::ps::ext::canonicalize(call_data);
    const auto call = repository->get_or_create(call_data).first;
    auto call_variant_data = ygg::Data<kr::ps::ext::RuleVariant>(call.get_index());
    const auto call_variant = repository->get_or_create(call_variant_data).first;
    auto caller_data = make_module_data(*repository, "caller");
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
    auto caller_proof_options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag> {};
    caller_proof_options.universal = true;
    const auto caller_proof = kr::ps::ext::find_solution(task_context, caller_program, caller_proof_options);
    EXPECT_EQ(caller_proof.status, kr::ps::ext::ModuleProgramProofStatus::FAILURE);
    ASSERT_TRUE(caller_proof.graph);
    EXPECT_TRUE(caller_proof.deadend_states.empty());
    EXPECT_FALSE(caller_proof.open_states.empty());

    auto do_data = ygg::Data<kr::ps::ext::Rule<kr::ps::ext::DoTag>>(std::string("missing-action"));
    do_data.source = source.get_index();
    do_data.target = target.get_index();
    kr::ps::ext::canonicalize(do_data);
    const auto do_rule = repository->get_or_create(do_data).first;
    auto do_variant_data = ygg::Data<kr::ps::ext::RuleVariant>(do_rule.get_index());
    const auto do_variant = repository->get_or_create(do_variant_data).first;
    auto do_module_data = make_module_data(*repository, "no-action");
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
    auto do_proof_options = kr::ps::ext::ModuleProgramSearchOptions<p::GroundTag> {};
    do_proof_options.universal = true;
    const auto do_proof = kr::ps::ext::find_solution(task_context, do_program, do_proof_options);
    EXPECT_EQ(do_proof.status, kr::ps::ext::ModuleProgramProofStatus::FAILURE);
    ASSERT_TRUE(do_proof.graph);
    EXPECT_EQ(do_proof.graph->get_num_vertices(), 1);
    EXPECT_EQ(do_proof.graph->get_num_edges(), 0);
    EXPECT_TRUE(do_proof.deadend_states.empty());
    EXPECT_FALSE(do_proof.open_states.empty());
    EXPECT_TRUE(do_proof.cycle.empty());
}

}  // namespace runir::tests
