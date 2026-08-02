#include "fixtures.hpp"
#include "module_fixtures.hpp"
#include "planning_fixtures.hpp"

#include <concepts>
#include <filesystem>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <random>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <runir/kr/ps/ext/execution_repository.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/module_program_executor.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/ext/successor_expander.hpp>
#include <runir/kr/task_context.hpp>
#include <set>
#include <string>
#include <tyr/planning/algorithms/portable_shuffle.hpp>
#include <utility>

namespace runir::tests
{

namespace
{

auto create_register(kr::ps::ext::Repository& repository, const std::string& name, ygg::uint_t identifier)
{
    auto data = ygg::Data<kr::dl::Register<kr::dl::ConceptTag>>(name, kr::dl::RegisterIdentifier<kr::dl::ConceptTag>(identifier));
    return repository.get_dl_repository().get_or_create(data).first;
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
    if constexpr (std::same_as<Kind, tyr::planning::GroundTag>)
        return kr::TaskContext<Kind>::create(make_ground_context(domain, task_file));
    else
        return kr::TaskContext<Kind>::create(make_lifted_context(domain, task_file));
}

template<tyr::planning::TaskKind Kind>
void expect_initial_execution_state_uses_expander_repository()
{
    const auto domain = benchmark_path("classical/tests/gripper/domain.pddl");
    const auto task_file = benchmark_path("classical/tests/gripper/test-1.pddl");
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

}  // namespace

TEST(RunirTests, ExtDistanceFeatureEvaluationReusesTaskContextCache)
{
    namespace p = tyr::planning;

    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
    auto task_context = kr::TaskContext<p::GroundTag>::create(search_context);

    auto dl_repository = task_context->ext_dl_repository;
    auto repository = task_context->ext_repository;
    const auto module =
        kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/executor/ext_distance_feature_evaluation_reuses_task_context_cache/distance.module"),
                                      task->get_domain().get_domain(),
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

TEST(RunirTests, ExtGroundAndLiftedInitialStatesUseExpanderRepository)
{
    expect_initial_execution_state_uses_expander_repository<tyr::planning::GroundTag>();
    expect_initial_execution_state_uses_expander_repository<tyr::planning::LiftedTag>();
}

TEST(RunirTests, ExtLoadRuleEnumeratesAllObjectsAndAdvancesMemory)
{
    namespace p = tyr::planning;

    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
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
    EXPECT_NE(formatted.find(formatter_fragment("concept_load_register")), std::string::npos) << formatted;

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

    auto random = std::mt19937_64(1);
    auto expected_successors = expander.labeled_successors(initial_state);
    p::portable_shuffle(expected_successors.begin(), expected_successors.end(), random);
    auto expected_steps = expander.steps(initial_state, expected_successors);
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
    namespace p = tyr::planning;

    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
    auto task_context = kr::TaskContext<p::GroundTag>::create(search_context);

    auto dl_repository = task_context->ext_dl_repository;
    auto repository = task_context->ext_repository;

    const auto module =
        kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/executor/ext_role_load_rule_enumerates_all_pairs_and_advances_memory/module.module"),
                                      task->get_domain().get_domain(),
                                      *repository);

    ASSERT_EQ(module.get_registers<kr::dl::RoleTag>().size(), 1);
    const auto transitions = module.get_memory_transitions();
    ASSERT_EQ(transitions.size(), 1);
    ASSERT_EQ(transitions[0].size(), 1);

    const auto formatted = fmt::format("{}", module);
    EXPECT_NE(formatted.find("(:role At)"), std::string::npos) << formatted;
    EXPECT_NE(formatted.find(formatter_fragment("role_load_register")), std::string::npos) << formatted;

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

TEST(RunirTests, ExtSuccessorEnumerationCombinesAllApplicableRuleKinds)
{
    namespace p = tyr::planning;

    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
    auto task_context = kr::TaskContext<p::GroundTag>::create(search_context);

    auto repository = task_context->ext_repository;
    const auto module =
        kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/executor/ext_successor_enumeration_combines_all_applicable_rule_kinds/module.module"),
                                      task->get_domain().get_domain(),
                                      *repository);
    const auto program = create_module_program(*repository, module, { module });
    auto expander = kr::ps::ext::SuccessorExpander<p::GroundTag>(task_context, program);
    const auto initial_state = expander.initial_state();
    const auto successors = expander.labeled_successors(initial_state);
    const auto steps = expander.steps(initial_state, successors);

    auto load_steps = std::size_t(0);
    auto do_steps = std::size_t(0);
    auto sketch_steps = std::size_t(0);
    for (const auto& step : steps)
    {
        EXPECT_EQ(step.status, kr::ps::ext::detail::ModuleProgramOutcome::APPLIED);
        const auto& target = step.get_target();
        const auto memory = target.get_call_stack().get_memory_state().get_name();
        if (memory == "load-target")
        {
            EXPECT_EQ(target.get_phase(), kr::ps::ext::ExecutionPhase::INTERNAL);
            ++load_steps;
        }
        else if (memory == "do-target")
        {
            EXPECT_EQ(target.get_phase(), kr::ps::ext::ExecutionPhase::EXTERNAL);
            ++do_steps;
        }
        else if (memory == "sketch-target")
        {
            EXPECT_EQ(target.get_phase(), kr::ps::ext::ExecutionPhase::EXTERNAL);
            ++sketch_steps;
        }
        else
        {
            ADD_FAILURE() << "unexpected target memory state: " << memory;
        }
    }
    EXPECT_GT(load_steps, 0);
    EXPECT_GT(do_steps, 0);
    EXPECT_EQ(sketch_steps, 1);
    EXPECT_EQ(steps.size(), load_steps + do_steps + sketch_steps);

    const auto control_steps = expander.control_steps(initial_state, successors);
    EXPECT_EQ(control_steps.size(), do_steps + sketch_steps);

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

TEST(RunirTests, ExtCallRulePassesArgumentDenotationsToCallee)
{
    namespace p = tyr::planning;

    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
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
    const auto universal_role = kr::ps::ext::dl::parse_role("(r_universal)", task->get_domain().get_domain(), *dl_repository);
    const auto true_boolean =
        kr::ps::ext::dl::parse_boolean(read_fixture("kr/ps/ext/executor/ext_module_parser_lowers_ext_dl_concept_and_role_expressions/case_6.dsl"),
                                       task->get_domain().get_domain(),
                                       *dl_repository);
    const auto object_count =
        kr::ps::ext::dl::parse_numerical(read_fixture("kr/ps/ext/executor/ext_module_parser_lowers_ext_dl_concept_and_role_expressions/case_7.dsl"),
                                         task->get_domain().get_domain(),
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
    namespace p = tyr::planning;

    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
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
    namespace p = tyr::planning;

    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
    auto task_context = kr::TaskContext<p::GroundTag>::create(search_context);

    auto dl_repository = task_context->ext_dl_repository;
    auto repository = task_context->ext_repository;

    const auto source = create_memory_state(*repository, "source");
    const auto target = create_memory_state(*repository, "target");
    const auto ball_feature =
        create_concept_feature(*repository,
                               kr::ps::ext::dl::parse_concept("(c_atomic_state \"ball\")", task->get_domain().get_domain(), *dl_repository).get_index(),
                               "ball");
    const auto room_feature =
        create_concept_feature(*repository,
                               kr::ps::ext::dl::parse_concept("(c_atomic_state \"room\")", task->get_domain().get_domain(), *dl_repository).get_index(),
                               "room");
    const auto gripper_feature =
        create_concept_feature(*repository,
                               kr::ps::ext::dl::parse_concept("(c_atomic_state \"gripper\")", task->get_domain().get_domain(), *dl_repository).get_index(),
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
        EXPECT_EQ(step.plan_suffix.front().label.get_relation().get_name(), "pick");
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

    auto random = std::mt19937_64(1);
    auto expected_successors = expander.labeled_successors(initial_state);
    p::portable_shuffle(expected_successors.begin(), expected_successors.end(), random);
    auto expected_steps = expander.steps(initial_state, expected_successors);
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
    namespace p = tyr::planning;

    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
    auto task_context = kr::TaskContext<p::GroundTag>::create(search_context);

    auto dl_repository = task_context->ext_dl_repository;
    auto repository = task_context->ext_repository;

    const auto module =
        kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/executor/ext_do_rule_rejects_action_with_incompatible_declared_effects/module.module"),
                                      task->get_domain().get_domain(),
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
    namespace p = tyr::planning;

    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
    auto task_context = kr::TaskContext<p::GroundTag>::create(search_context);

    auto dl_repository = task_context->ext_dl_repository;
    auto repository = task_context->ext_repository;

    const auto source = create_memory_state(*repository, "source");
    const auto move_target = create_memory_state(*repository, "move_target");
    const auto pick_target = create_memory_state(*repository, "pick_target");
    const auto ball_feature =
        create_concept_feature(*repository,
                               kr::ps::ext::dl::parse_concept("(c_atomic_state \"ball\")", task->get_domain().get_domain(), *dl_repository).get_index(),
                               "ball");
    const auto room_feature =
        create_concept_feature(*repository,
                               kr::ps::ext::dl::parse_concept("(c_atomic_state \"room\")", task->get_domain().get_domain(), *dl_repository).get_index(),
                               "room");
    const auto gripper_feature =
        create_concept_feature(*repository,
                               kr::ps::ext::dl::parse_concept("(c_atomic_state \"gripper\")", task->get_domain().get_domain(), *dl_repository).get_index(),
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

}  // namespace runir::tests
