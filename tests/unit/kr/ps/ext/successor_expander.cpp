#include "successor_expander_fixtures.hpp"

#include <gtest/gtest.h>
#include <ranges>
#include <urd/repr/values/unordered_flat_set.hpp>
#include <urd/store.hpp>

namespace runir::tests
{

namespace
{

struct UnrelatedExecutionStorage
{
};

struct MissingStoreStorage : kr::ps::ext::InternedExecutionStorage<tyr::GroundTag>
{
    using InternedExecutionStorage::InternedExecutionStorage;
    template<typename... Args>
    void store(Args&&...) = delete;
};

struct WrongRetainStorage : kr::ps::ext::InternedExecutionStorage<tyr::GroundTag>
{
    using InternedExecutionStorage::InternedExecutionStorage;
    [[nodiscard]] int retain(kr::ps::ext::ProgramStateView<tyr::GroundTag>) const noexcept;
};

template<typename Storage, tyr::TaskKind Kind, bool Expected>
constexpr bool execution_storage_constraints_match =
    (kr::ps::ext::ExecutionStorageConcept<Storage, Kind> == Expected)
    && ((requires {
            sizeof(
                kr::ps::RuleEvaluationContext<kr::ExtFamilyTag, Kind, Storage, tyr::planning::StateView<Kind>, kr::dl::semantics::FullEvaluationPolicy<Kind>>);
        }) == Expected) && ((requires { typename kr::ps::ext::SuccessorExpander<Kind, Storage>; }) == Expected);

static_assert(execution_storage_constraints_match<kr::ps::ext::InternedExecutionStorage<tyr::GroundTag>, tyr::GroundTag, true>);
static_assert(execution_storage_constraints_match<kr::ps::ext::InternedExecutionStorage<tyr::LiftedTag>, tyr::LiftedTag, true>);
static_assert(execution_storage_constraints_match<kr::ps::ext::TransientExecutionStorage<tyr::GroundTag>, tyr::GroundTag, true>);
static_assert(execution_storage_constraints_match<kr::ps::ext::TransientExecutionStorage<tyr::LiftedTag>, tyr::LiftedTag, true>);
static_assert(execution_storage_constraints_match<kr::ps::ext::InternedExecutionStorage<tyr::GroundTag>, tyr::LiftedTag, false>);
static_assert(execution_storage_constraints_match<kr::ps::ext::InternedExecutionStorage<tyr::LiftedTag>, tyr::GroundTag, false>);
static_assert(execution_storage_constraints_match<kr::ps::ext::TransientExecutionStorage<tyr::GroundTag>, tyr::LiftedTag, false>);
static_assert(execution_storage_constraints_match<kr::ps::ext::TransientExecutionStorage<tyr::LiftedTag>, tyr::GroundTag, false>);
static_assert(execution_storage_constraints_match<int, tyr::GroundTag, false>);
static_assert(execution_storage_constraints_match<UnrelatedExecutionStorage, tyr::GroundTag, false>);
static_assert(execution_storage_constraints_match<MissingStoreStorage, tyr::GroundTag, false>);
static_assert(execution_storage_constraints_match<WrongRetainStorage, tyr::GroundTag, false>);

static_assert(!kr::ps::ext::ExecutionStateViewConcept<kr::ps::ext::ProgramStateView<tyr::GroundTag>, int>);
static_assert(!kr::ps::ext::ExecutionStateViewConcept<kr::ps::ext::ProgramStateView<tyr::GroundTag>, kr::ps::ext::TransientExecutionStorage<tyr::GroundTag>>);
static_assert(
    !kr::ps::ext::ExecutionStateViewConcept<kr::ps::ext::BuilderProgramStateView<tyr::GroundTag>, kr::ps::ext::InternedExecutionStorage<tyr::GroundTag>>);

static_assert(kr::ps::ext::ExecutionStateViewConcept<kr::ps::ext::ProgramStateView<tyr::GroundTag>, kr::ps::ext::InternedExecutionStorage<tyr::GroundTag>>);
static_assert(kr::ps::ext::ExecutionStateViewConcept<kr::ps::ext::ProgramStateView<tyr::LiftedTag>, kr::ps::ext::InternedExecutionStorage<tyr::LiftedTag>>);
static_assert(
    kr::ps::ext::ExecutionStateViewConcept<kr::ps::ext::BuilderProgramStateView<tyr::GroundTag>, kr::ps::ext::TransientExecutionStorage<tyr::GroundTag>>);
static_assert(
    kr::ps::ext::ExecutionStateViewConcept<kr::ps::ext::BuilderProgramStateView<tyr::LiftedTag>, kr::ps::ext::TransientExecutionStorage<tyr::LiftedTag>>);
static_assert(!kr::ps::ext::ExecutionStateViewConcept<int, kr::ps::ext::InternedExecutionStorage<tyr::GroundTag>>);
static_assert(!kr::ps::ext::ExecutionStateViewConcept<int, kr::ps::ext::InternedExecutionStorage<tyr::LiftedTag>>);
static_assert(!kr::ps::ext::ExecutionStateViewConcept<int, kr::ps::ext::TransientExecutionStorage<tyr::GroundTag>>);
static_assert(!kr::ps::ext::ExecutionStateViewConcept<int, kr::ps::ext::TransientExecutionStorage<tyr::LiftedTag>>);

static_assert(kr::ps::EmitConcept<bool (*)(kr::ps::ProgramStep<kr::ExtFamilyTag, tyr::GroundTag>), kr::ps::ProgramStep<kr::ExtFamilyTag, tyr::GroundTag>>);
static_assert(!kr::ps::EmitConcept<void (*)(kr::ps::ProgramStep<kr::ExtFamilyTag, tyr::GroundTag>), kr::ps::ProgramStep<kr::ExtFamilyTag, tyr::GroundTag>>);
static_assert(kr::ps::StopConcept<bool (*)()>);
static_assert(!kr::ps::StopConcept<void (*)()>);

// Indexed sources can be inspected by transient execution, but emission needs its builder views.
static_assert(
    []<typename State>()
    {
        using Storage = kr::ps::ext::TransientExecutionStorage<tyr::GroundTag>;
        using Context = kr::ps::RuleEvaluationContext<kr::ExtFamilyTag,
                                                      tyr::GroundTag,
                                                      Storage,
                                                      tyr::planning::StateView<tyr::GroundTag>,
                                                      kr::dl::semantics::FullEvaluationPolicy<tyr::GroundTag>>;
        using Step = kr::ps::ProgramStep<kr::ExtFamilyTag, tyr::GroundTag, kr::ps::ext::StorageStoredState<Storage>>;
        return !requires(const kr::ps::ext::detail::ActionRuleEvaluator<tyr::GroundTag>& evaluator,
                         Context& context,
                         State state,
                         bool (*emit)(Step),
                         bool (*stop)()) { evaluator.emit(context, state, emit, stop); };
    }.template operator()<kr::ps::ext::ProgramStateView<tyr::GroundTag>>());

}  // namespace

TEST(RunirTests, ExtDistanceFeatureEvaluationDoesNotRetainTaskDenotations)
{
    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
    auto task_context = kr::TaskContext<tyr::GroundTag>::create(kr::DomainContext::create(task->get_domain()), search_context);

    auto dl_repository = task_context->domain_context->ext_repository->get_dl_repository_ptr();
    auto repository = task_context->domain_context->ext_repository;
    const auto module_ =
        kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/executor/ext_distance_feature_evaluation_reuses_task_context_cache/distance.module"),
                                      task->get_domain().get_domain(),
                                      *repository);
    const auto program = create_program(*repository, module_, { module_ });
    auto expander = kr::ps::ext::SuccessorExpander<tyr::GroundTag>(task_context, program);
    const auto planning_node = initial_planning_node(expander);
    const auto initial_state = expander.initial_state(planning_node.get_state());
    const auto first = collect_steps(expander, initial_state);
    ASSERT_EQ(first.size(), 1);
    EXPECT_EQ(first.front().status, kr::ps::ProgramOutcome::APPLIED);
    EXPECT_EQ(task_context->dl_denotation_repository->size<kr::dl::semantics::Denotation<kr::dl::NumericalTag>>(), 0);

    const auto second = collect_steps(expander, initial_state);
    ASSERT_EQ(second.size(), 1);
    EXPECT_EQ(second.front().get_target().index(), first.front().get_target().index());
    EXPECT_EQ(task_context->dl_denotation_repository->size<kr::dl::semantics::Denotation<kr::dl::NumericalTag>>(), 0);
    EXPECT_EQ(initial_state.get_module_state().get_memory_state().get_name(), "source");
}

TEST(RunirTests, ExtInitialStatesUseExpanderRepositoryGround) { expect_initial_program_state_uses_expander_repository<tyr::GroundTag>(); }

TEST(RunirTests, ExtTransientProgramStatesRetainValuesAndInternCallersGround)
{ expect_transient_program_states_retain_values_and_intern_callers<tyr::GroundTag>(); }

TEST(RunirTests, ExtChooseUsesNaturalBindingOrder)
{
    using Expander = kr::ps::ext::SuccessorExpander<tyr::GroundTag>;
    auto search = make_gripper_ground_context();
    auto context = kr::TaskContext<tyr::GroundTag>::create(kr::DomainContext::create(search->task->get_domain()), search);
    auto& repository = *context->domain_context->ext_repository;
    const auto module_ = kr::ps::ext::dl::parse_module(R"(
(:module (:symbol bindings) (:arguments) (:registers (:concept c) (:role r))
  (:entry source) (:memory source target)
  (:features
    (:concept (:symbol Balls) (:expression (c_atomic_state "ball")))
    (:concept (:symbol Empty) (:expression (c_bot)))
    (:concept (:symbol One) (:expression (c_some (r_atomic_goal "at" true) (c_top))))
    (:role (:symbol At) (:expression (r_atomic_state "at")))
    (:role (:symbol NoPairs) (:expression (r_restriction (r_atomic_state "at") (c_bot))))
    (:role (:symbol OnePair) (:expression (r_restriction (r_inverse (r_atomic_state "at")) (c_some (r_atomic_goal "at" true) (c_top))))))
  (:rules
    (:rule (:symbol balls) (:expression (:source-memory source) (:target-memory target) (:choose (:conditions) (:concept Balls) (:register (:concept c)))))
    (:rule (:symbol empty) (:expression (:source-memory source) (:target-memory target) (:choose (:conditions) (:concept Empty) (:register (:concept c)))))
    (:rule (:symbol one) (:expression (:source-memory source) (:target-memory target) (:choose (:conditions) (:concept One) (:register (:concept c)))))
    (:rule (:symbol at) (:expression (:source-memory source) (:target-memory target) (:choose (:conditions) (:role At) (:register (:role r)))))
    (:rule (:symbol no-pairs) (:expression (:source-memory source) (:target-memory target) (:choose (:conditions) (:role NoPairs) (:register (:role r)))))
    (:rule (:symbol one-pair) (:expression (:source-memory source) (:target-memory target) (:choose (:conditions) (:role OnePair) (:register (:role r)))))))
)",
                                                       search->task->get_domain().get_domain(),
                                                       repository);
    const auto program = create_program(repository, module_, { module_ });
    auto expander = Expander(context, program);
    const auto node = initial_planning_node(expander);
    const auto state = expander.initial_state(node.get_state());
    auto statistics = kr::ps::ext::ProgramSearchStatistics {};
    auto counts = urd::Store<urd::sequence<std::size_t>> {};
    ASSERT_TRUE(expander.for_each_successor(
        state,
        statistics,
        [&](auto&& choice)
        {
            if constexpr (std::same_as<std::remove_cvref_t<decltype(choice)>, kr::ps::ProgramStep<kr::ExtFamilyTag, tyr::GroundTag>>)
                ADD_FAILURE() << "Expected a compact Choice descriptor";
            else
            {
                counts.push_back(choice.count());
                EXPECT_EQ(choice.has_alternatives(), choice.count() > 1);
                auto expected = choice.bindings().begin();
                std::size_t visited = 0;
                while (!choice.exhausted())
                {
                    EXPECT_EQ(choice.current(), *expected);
                    ++expected;
                    ++visited;
                    choice.advance();
                }
                EXPECT_EQ(expected, choice.bindings().end());
                EXPECT_EQ(visited, choice.count());
            }
            return true;
        },
        [] { return false; }));
    EXPECT_EQ(statistics.num_generated, 0);
    std::ranges::sort(counts);
    EXPECT_EQ(counts, (urd::Store<urd::sequence<std::size_t>> { 0, 0, 1, 1, 2, 2 }));
}

TEST(RunirTests, ExtChoicesRetainBindingsAcrossChildEvaluationAndPoolReuse)
{
    using Choice = kr::ps::ext::detail::Choice<kr::dl::ConceptTag>;
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../../fixtures/kr/ps/ext/choose";
    const auto context = create_task_context<tyr::GroundTag>(directory / "domain.pddl", directory / "task.pddl");
    auto& search = *context->search_context;
    auto& repository = *context->domain_context->ext_repository;
    const auto module_ = kr::ps::ext::dl::parse_module(R"(
(:module (:symbol retained-bindings) (:arguments) (:registers (:concept selected))
  (:entry source) (:memory source child done)
  (:features
    (:concept (:symbol Candidates) (:expression (c_or (c_atomic_state "candidate") (c_atomic_state "at"))))
    (:concept (:symbol Here) (:expression (c_atomic_state "at"))))
  (:rules
    (:rule (:symbol parent) (:expression (:source-memory source) (:target-memory child) (:choose (:conditions) (:concept Candidates) (:register (:concept selected)))))
    (:rule (:symbol child) (:expression (:source-memory child) (:target-memory done) (:choose (:conditions) (:concept Here) (:register (:concept selected)))))))
)",
                                                       search.task->get_domain().get_domain(),
                                                       repository);
    const auto program = create_program(repository, module_, { module_ });
    const auto check = [&]<typename Storage>()
    {
        auto expander = kr::ps::ext::SuccessorExpander<tyr::GroundTag, Storage>(context, program);
        const auto node = search.successor_generator->get_initial_node(*search.state_repository, *search.axiom_evaluator);
        const auto initial = expander.initial_state(node.get_state());
        auto statistics = kr::ps::ext::ProgramSearchStatistics {};
        const auto choices = [&](const auto& state)
        {
            auto result = urd::Store<urd::sequence<Choice>> {};
            EXPECT_TRUE(expander.for_each_successor(
                expander.view(state),
                statistics,
                [&](auto&& expansion)
                {
                    if constexpr (std::same_as<std::remove_cvref_t<decltype(expansion)>, Choice>)
                        result.push_back(std::forward<decltype(expansion)>(expansion));
                    else
                        ADD_FAILURE() << "Expected a concept choice";
                    return true;
                },
                [] { return false; }));
            return result;
        };
        auto parent = choices(initial);
        ASSERT_EQ(parent.size(), 1);
        const auto expected = parent.front().bindings();
        ASSERT_EQ(expected.size(), 3);
        const auto* parent_buffer = parent.front().bindings().data();
        const auto child = expander.apply_choice(expander.view(initial), parent.front(), statistics).get_target();

        // Child expansion clears the evaluator cache. Its released binding buffer can be reused while the parent remains live.
        const typename decltype(expected)::value_type* child_buffer = nullptr;
        for (size_t attempt = 0; attempt < 2; ++attempt)
        {
            const auto next = choices(child);
            ASSERT_EQ(next.size(), 1);
            ASSERT_EQ(next.front().count(), 1);
            EXPECT_EQ(next.front().current().get_name(), "start");
            EXPECT_NE(next.front().bindings().data(), parent_buffer);
            if (child_buffer)
            {
                EXPECT_EQ(next.front().bindings().data(), child_buffer);
            }
            child_buffer = next.front().bindings().data();
        }
        EXPECT_EQ(parent.front().bindings().data(), parent_buffer);
        EXPECT_TRUE(std::ranges::equal(parent.front().bindings(), expected));
        for (const auto binding : expected)
        {
            const auto target = expander.apply_choice(expander.view(initial), parent.front(), statistics).get_target();
            const auto selected = expander.view(target).get_module_state().get_registers().template values<kr::dl::ConceptTag>()[0];
            ASSERT_TRUE(selected);
            EXPECT_EQ(*selected, binding);
            parent.front().advance();
        }
        EXPECT_TRUE(parent.front().exhausted());
    };
    check.template operator()<kr::ps::ext::InternedExecutionStorage<tyr::GroundTag>>();
    check.template operator()<kr::ps::ext::TransientExecutionStorage<tyr::GroundTag>>();
}

TEST(RunirTests, ExtModuleReturnsPreserveCalleePlanningStateGround) { expect_module_return_preserves_callee_planning_state<tyr::GroundTag>(); }

TEST(RunirTests, ExtQueryEvaluationBorrowsIndependentSourceAndTargetCachesGround) { expect_borrowed_query_evaluation<tyr::GroundTag>(); }

TEST(RunirTests, ExtBindingEffectsFilterLoadsAndChoicesGround) { expect_binding_effects_and_empty_choices<tyr::GroundTag>(); }

TEST(RunirTests, ExtDoSuccessorsFilterArgumentsBeforeConstructingStatesGround) { expect_lazy_do_successors<tyr::GroundTag>(); }

TEST(RunirTests, ExtControlOnlyStepsDoNotGeneratePlanningSuccessorsGround) { expect_control_only_steps_do_not_generate_planning_successors<tyr::GroundTag>(); }

TEST(RunirTests, ExtLoadRuleEnumeratesAllObjectsAndAdvancesMemory)
{
    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
    auto task_context = kr::TaskContext<tyr::GroundTag>::create(kr::DomainContext::create(task->get_domain()), search_context);

    auto dl_repository = task_context->domain_context->ext_repository->get_dl_repository_ptr();
    auto repository = task_context->domain_context->ext_repository;

    const auto source = create_memory_state(*repository, "source");
    const auto target = create_memory_state(*repository, "target");
    const auto reg = create_register(*repository, "r0", 0);
    const auto top_concept = create_top_concept(*dl_repository);
    const auto top_feature = create_concept_feature(*repository, top_concept.index(), "top");

    auto load_data = urd::data<kr::ps::ext::Rule<kr::ps::ext::LoadTag<kr::dl::ConceptTag>>>();
    load_data.source = source.index();
    load_data.target = target.index();
    load_data.feature = top_feature.index();
    load_data.reg = reg.index();
    kr::ps::ext::canonicalize(load_data);
    const auto load = repository->insert(load_data).first;

    auto variant_data = urd::data<kr::ps::Rule<kr::ExtFamilyTag>>("", load.index());
    const auto variant = repository->insert(variant_data).first;

    auto module_data = make_module_data(*repository, "module");
    module_data.entry_memory_state = source.index();
    module_data.memory_states.push_back(source.index());
    module_data.memory_states.push_back(target.index());
    module_data.concept_registers.push_back(reg.index());
    module_data.concept_features.push_back(top_feature.index());
    auto transition = urd::flat_vector<urd::index<kr::ps::Rule<kr::ExtFamilyTag>>>();
    transition.push_back(variant.index());
    ygg::canonicalize(transition);
    module_data.memory_transitions.push_back(std::move(transition));
    kr::ps::ext::canonicalize(module_data);
    const auto module_ = repository->insert(module_data).first;

    const auto formatted = fmt::format("{}", module_);
    EXPECT_NE(formatted.find("(:symbol module)"), std::string::npos) << formatted;
    EXPECT_NE(formatted.find("(:load"), std::string::npos);
    EXPECT_NE(formatted.find("(:expression"), std::string::npos);
    EXPECT_NE(formatted.find("(:concept top)"), std::string::npos) << formatted;
    EXPECT_NE(formatted.find(formatter_fragment("concept_load_register")), std::string::npos) << formatted;

    const auto program = create_program(*repository, module_, { module_ });
    auto expander = kr::ps::ext::SuccessorExpander<tyr::GroundTag>(task_context, program);
    const auto planning_node = initial_planning_node(expander);
    const auto initial_state = expander.initial_state(planning_node.get_state());
    const auto steps = collect_steps(expander, initial_state);
    ASSERT_GT(steps.size(), 1);

    auto loaded_objects = urd::Store<urd::set<ygg::uint_t>> {};
    for (const auto& step : steps)
    {
        EXPECT_EQ(step.status, kr::ps::ProgramOutcome::APPLIED);
        const auto target_state = step.get_target();
        EXPECT_EQ(target_state.get_state().index(), initial_state.get_state().index());
        EXPECT_EQ(target_state.get_module_state().get_memory_state().index(), target.index());
        const auto loaded = target_state.get_module_state().get_registers().template values<kr::dl::ConceptTag>()[0];
        ASSERT_TRUE(loaded);
        loaded_objects.insert(static_cast<ygg::uint_t>(loaded.value().index().value));
    }
    EXPECT_EQ(loaded_objects.size(), steps.size());

    auto greedy_options = kr::ps::ext::ProgramSearchOptions<tyr::GroundTag> {};
    const auto greedy = kr::ps::ext::find_solution(task_context, program, greedy_options);
    auto universal_options = kr::ps::ext::ProgramSearchOptions<tyr::GroundTag> {};
    universal_options.universal = true;
    const auto universal = kr::ps::ext::find_solution(task_context, program, universal_options);
    ASSERT_TRUE(greedy.graph);
    ASSERT_TRUE(universal.graph);
    ASSERT_EQ(greedy.graph->out_degree(0), 1);
    EXPECT_EQ(universal.graph->out_degree(0), steps.size());

    const auto edge = greedy.graph->out_edges(0).front();
    const auto selected = greedy.graph->vertex_value(greedy.graph->target(edge)).program_state;
    const auto actual_loaded = selected.get_module_state().get_registers().template values<kr::dl::ConceptTag>()[0];
    const auto expected_loaded = steps.front().get_target().get_module_state().get_registers().template values<kr::dl::ConceptTag>()[0];
    ASSERT_TRUE(actual_loaded);
    ASSERT_TRUE(expected_loaded);
    EXPECT_EQ(actual_loaded.value().index(), expected_loaded.value().index());

    auto options = kr::ps::ext::ProgramSearchOptions<tyr::GroundTag> {};
    options.max_num_states = 1;
    options.universal = true;
    const auto bounded = kr::ps::ext::find_solution(task_context, program, options);
    EXPECT_EQ(bounded.status, kr::ps::ProofStatus::OUT_OF_STATES);
    ASSERT_TRUE(bounded.graph);
    EXPECT_EQ(bounded.graph->num_vertices(), 1);
}

TEST(RunirTests, ExtRoleLoadRuleEnumeratesAllPairsAndAdvancesMemory)
{
    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
    auto task_context = kr::TaskContext<tyr::GroundTag>::create(kr::DomainContext::create(task->get_domain()), search_context);

    auto dl_repository = task_context->domain_context->ext_repository->get_dl_repository_ptr();
    auto repository = task_context->domain_context->ext_repository;

    const auto module_ =
        kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/executor/ext_role_load_rule_enumerates_all_pairs_and_advances_memory/module.module"),
                                      task->get_domain().get_domain(),
                                      *repository);

    ASSERT_EQ(module_.get_registers<kr::dl::RoleTag>().size(), 1);
    const auto transitions = module_.get_memory_transitions();
    ASSERT_EQ(transitions.size(), 1);
    ASSERT_EQ(transitions[0].size(), 1);

    const auto formatted = fmt::format("{}", module_);
    EXPECT_NE(formatted.find("(:role At)"), std::string::npos) << formatted;
    EXPECT_NE(formatted.find(formatter_fragment("role_load_register")), std::string::npos) << formatted;

    const auto program = create_program(*repository, module_, { module_ });
    auto expander = kr::ps::ext::SuccessorExpander<tyr::GroundTag>(task_context, program);
    const auto planning_node = initial_planning_node(expander);
    const auto initial_state = expander.initial_state(planning_node.get_state());
    const auto steps = collect_steps(expander, initial_state);
    ASSERT_GT(steps.size(), 1);

    auto loaded_pairs = urd::Store<urd::set<std::pair<ygg::uint_t, ygg::uint_t>>> {};
    for (const auto& step : steps)
    {
        EXPECT_EQ(step.status, kr::ps::ProgramOutcome::APPLIED);
        const auto target_state = step.get_target();
        EXPECT_EQ(target_state.get_state().index(), initial_state.get_state().index());
        EXPECT_EQ(target_state.get_module_state().get_memory_state().get_name(), "target");
        const auto loaded = target_state.get_module_state().get_registers().template values<kr::dl::RoleTag>()[0];
        ASSERT_TRUE(loaded);
        const auto pair = loaded.value();
        loaded_pairs.emplace(static_cast<ygg::uint_t>(pair.first.index().value), static_cast<ygg::uint_t>(pair.second.index().value));
    }
    EXPECT_EQ(loaded_pairs.size(), steps.size());
}

TEST(RunirTests, ExtSuccessorEnumerationCombinesAllApplicableRuleKinds)
{
    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
    auto task_context = kr::TaskContext<tyr::GroundTag>::create(kr::DomainContext::create(task->get_domain()), search_context);

    auto repository = task_context->domain_context->ext_repository;
    const auto module_ =
        kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/executor/ext_successor_enumeration_combines_all_applicable_rule_kinds/module.module"),
                                      task->get_domain().get_domain(),
                                      *repository);
    const auto program = create_program(*repository, module_, { module_ });
    auto expander = kr::ps::ext::SuccessorExpander<tyr::GroundTag>(task_context, program);
    const auto planning_node = initial_planning_node(expander);
    const auto initial_state = expander.initial_state(planning_node.get_state());
    const auto steps = collect_steps(expander, initial_state);

    auto load_steps = static_cast<std::size_t>(0);
    auto do_steps = static_cast<std::size_t>(0);
    auto sketch_steps = static_cast<std::size_t>(0);
    for (const auto& step : steps)
    {
        EXPECT_EQ(step.status, kr::ps::ProgramOutcome::APPLIED);
        const auto& target = step.get_target();
        const auto memory = target.get_module_state().get_memory_state().get_name();
        if (memory == "load-target")
        {
            ++load_steps;
        }
        else if (memory == "do-target")
        {
            ++do_steps;
        }
        else if (memory == "sketch-target")
        {
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

    auto greedy_options = kr::ps::ext::ProgramSearchOptions<tyr::GroundTag> {};
    const auto greedy = kr::ps::ext::find_solution(task_context, program, greedy_options);
    auto universal_options = kr::ps::ext::ProgramSearchOptions<tyr::GroundTag> {};
    universal_options.universal = true;
    const auto universal = kr::ps::ext::find_solution(task_context, program, universal_options);
    ASSERT_TRUE(greedy.graph);
    ASSERT_TRUE(universal.graph);
    ASSERT_EQ(greedy.graph->out_degree(0), 1);
    EXPECT_EQ(universal.graph->out_degree(0), steps.size());
}

TEST(RunirTests, ExtCallRulePassesArgumentDenotationsToCallee)
{
    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
    auto task_context = kr::TaskContext<tyr::GroundTag>::create(kr::DomainContext::create(task->get_domain()), search_context);

    auto dl_repository = task_context->domain_context->ext_repository->get_dl_repository_ptr();
    auto repository = task_context->domain_context->ext_repository;

    const auto caller_entry = create_memory_state(*repository, "caller_entry");
    const auto caller_return = create_memory_state(*repository, "caller_return");
    const auto callee_entry = create_memory_state(*repository, "callee_entry");
    auto concept_arg_data = urd::data<kr::dl::Argument<kr::dl::ConceptTag>>(std::string("x"), kr::dl::ArgumentIdentifier<kr::dl::ConceptTag>(0));
    const auto concept_arg = dl_repository->insert(concept_arg_data).first;
    auto role_arg_data = urd::data<kr::dl::Argument<kr::dl::RoleTag>>(std::string("r"), kr::dl::ArgumentIdentifier<kr::dl::RoleTag>(0));
    const auto role_arg = dl_repository->insert(role_arg_data).first;
    auto boolean_arg_data = urd::data<kr::dl::Argument<kr::dl::BooleanTag>>(std::string("b"), kr::dl::ArgumentIdentifier<kr::dl::BooleanTag>(0));
    const auto boolean_arg = dl_repository->insert(boolean_arg_data).first;
    auto numerical_arg_data = urd::data<kr::dl::Argument<kr::dl::NumericalTag>>(std::string("n"), kr::dl::ArgumentIdentifier<kr::dl::NumericalTag>(0));
    const auto numerical_arg = dl_repository->insert(numerical_arg_data).first;

    auto callee_data = make_module_data(*repository, "callee");
    callee_data.entry_memory_state = callee_entry.index();
    callee_data.memory_states.push_back(callee_entry.index());
    callee_data.concept_arguments.push_back(concept_arg.index());
    callee_data.role_arguments.push_back(role_arg.index());
    callee_data.boolean_arguments.push_back(boolean_arg.index());
    callee_data.numerical_arguments.push_back(numerical_arg.index());
    kr::ps::ext::canonicalize(callee_data);
    const auto callee = repository->insert(callee_data).first;

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
    auto call_data = urd::data<kr::ps::ext::Rule<kr::ps::ext::CallTag>>();
    call_data.source = caller_entry.index();
    call_data.target = caller_return.index();
    const auto top_feature = create_feature<kr::dl::ConceptTag>(*repository, top_concept.index(), "top");
    const auto universal_feature = create_feature<kr::dl::RoleTag>(*repository, universal_role.index(), "universal");
    const auto true_feature = create_feature<kr::dl::BooleanTag>(*repository, true_boolean.index(), "true");
    const auto count_feature = create_feature<kr::dl::NumericalTag>(*repository, object_count.index(), "count");
    call_data.callee = callee.get_symbol().index();
    call_data.arguments.push_back(top_feature.index());
    call_data.arguments.push_back(universal_feature.index());
    call_data.arguments.push_back(true_feature.index());
    call_data.arguments.push_back(count_feature.index());
    kr::ps::ext::canonicalize(call_data);
    const auto call = repository->insert(call_data).first;

    auto variant_data = urd::data<kr::ps::Rule<kr::ExtFamilyTag>>("", call.index());
    const auto variant = repository->insert(variant_data).first;
    auto caller_data = make_module_data(*repository, "caller");
    caller_data.entry_memory_state = caller_entry.index();
    caller_data.memory_states.push_back(caller_entry.index());
    caller_data.memory_states.push_back(caller_return.index());
    caller_data.concept_features.push_back(top_feature.index());
    caller_data.role_features.push_back(universal_feature.index());
    caller_data.boolean_features.push_back(true_feature.index());
    caller_data.numerical_features.push_back(count_feature.index());
    auto transition = urd::flat_vector<urd::index<kr::ps::Rule<kr::ExtFamilyTag>>> {};
    transition.push_back(variant.index());
    ygg::canonicalize(transition);
    caller_data.memory_transitions.push_back(std::move(transition));
    kr::ps::ext::canonicalize(caller_data);
    const auto caller = repository->insert(caller_data).first;

    const auto program = create_program(*repository, caller, { caller, callee });
    auto expander = kr::ps::ext::SuccessorExpander<tyr::GroundTag>(task_context, program);
    const auto planning_node = initial_planning_node(expander);
    const auto initial_state = expander.initial_state(planning_node.get_state());
    const auto call_steps = collect_steps(expander, initial_state);
    ASSERT_EQ(call_steps.size(), 1);
    EXPECT_EQ(call_steps.front().status, kr::ps::ProgramOutcome::APPLIED);
    const auto call_target = call_steps.front().get_target();
    const auto module_state = call_target.get_module_state();
    EXPECT_EQ(module_state.get_module().index(), callee.index());
    EXPECT_EQ(module_state.get_memory_state().index(), callee_entry.index());
    const auto arguments = module_state.get_arguments();
    const auto concept_arguments = arguments.arguments<kr::dl::ConceptTag>();
    const auto role_arguments = arguments.arguments<kr::dl::RoleTag>();
    const auto boolean_arguments = arguments.arguments<kr::dl::BooleanTag>();
    const auto numerical_arguments = arguments.arguments<kr::dl::NumericalTag>();
    ASSERT_EQ(concept_arguments.size(), 1);
    ASSERT_EQ(role_arguments.size(), 1);
    ASSERT_EQ(boolean_arguments.size(), 1);
    ASSERT_EQ(numerical_arguments.size(), 1);

    const auto concept_denotation = concept_arguments[0];
    const auto concept_first = concept_denotation.begin();
    ASSERT_NE(concept_first, concept_denotation.end());
    EXPECT_EQ((*concept_first).index(), urd::index<tyr::formalism::Object>(0));

    const auto role_denotation = role_arguments[0];
    const auto role_first = role_denotation.begin();
    ASSERT_NE(role_first, role_denotation.end());
    const auto role_pair = *role_first;
    EXPECT_EQ(role_pair.first.index(), urd::index<tyr::formalism::Object>(0));
    EXPECT_EQ(role_pair.second.index(), urd::index<tyr::formalism::Object>(0));

    EXPECT_TRUE(boolean_arguments[0].value());
    EXPECT_GT(numerical_arguments[0].value(), 0);

    // Contexts borrow the persistent data; preparing another context does not replace it.
    auto environment = kr::ps::ext::EvaluationEnvironment<tyr::GroundTag, kr::dl::semantics::FullEvaluationPolicy<tyr::GroundTag>>(*task_context, program);
    auto evaluation_context = environment.make_dl_context(call_target);
    const auto initial_context = environment.make_dl_context(initial_state);
    EXPECT_TRUE(initial_context.arguments().template arguments<kr::dl::ConceptTag>().empty());
    EXPECT_EQ(&evaluation_context.arguments().data(), &arguments.data());
    EXPECT_EQ(&evaluation_context.registers().data(), &module_state.get_registers().data());
    EXPECT_EQ(evaluation_context.arguments().template arguments<kr::dl::ConceptTag>()[0].index(), concept_arguments[0].index());
    EXPECT_TRUE(evaluation_context.arguments().template arguments<kr::dl::BooleanTag>()[0].value());
    EXPECT_EQ(evaluation_context.arguments().template arguments<kr::dl::NumericalTag>()[0].value(), numerical_arguments[0].value());

    auto evaluation_transition = environment.make_dl_transition_context(call_target.get_state(),
                                                                        call_target.get_state(),
                                                                        arguments,
                                                                        module_state.get_registers(),
                                                                        module_state.get_registers());
    EXPECT_EQ(&evaluation_transition.get_source_context().arguments().data(), &arguments.data());
    EXPECT_EQ(&evaluation_transition.get_target_context().arguments().data(), &arguments.data());

    const auto caller_frame = call_target.get_call_stack();
    ASSERT_TRUE(caller_frame);
    const auto return_steps = collect_steps(expander, call_target);
    ASSERT_EQ(return_steps.size(), 1);
    EXPECT_EQ(return_steps.front().status, kr::ps::ProgramOutcome::RESTORED_CALLER);
    EXPECT_EQ(return_steps.front().get_target().get_call_stack(), caller_frame.value().get_caller());
    EXPECT_EQ(return_steps.front().get_target().get_module_state().get_module().index(), caller.index());
    EXPECT_EQ(return_steps.front().get_target().get_module_state().get_memory_state().index(), caller_return.index());

    const auto repeated_call_steps = collect_steps(expander, initial_state);
    ASSERT_EQ(repeated_call_steps.size(), 1);
    EXPECT_EQ(repeated_call_steps.front().get_target().index(), call_target.index());
}

TEST(RunirTests, ExtCallRuleResolvesNamedCalleeFromModuleRegistry)
{
    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
    auto task_context = kr::TaskContext<tyr::GroundTag>::create(kr::DomainContext::create(task->get_domain()), search_context);

    auto dl_repository = task_context->domain_context->ext_repository->get_dl_repository_ptr();
    auto repository = task_context->domain_context->ext_repository;

    const auto caller_entry = create_memory_state(*repository, "caller_entry");
    const auto caller_return = create_memory_state(*repository, "caller_return");
    const auto callee_entry = create_memory_state(*repository, "callee_entry");
    const auto callee = create_module(*repository, "callee", callee_entry, { callee_entry });

    auto call_data = urd::data<kr::ps::ext::Rule<kr::ps::ext::CallTag>>();
    call_data.source = caller_entry.index();
    call_data.target = caller_return.index();
    call_data.callee = callee.get_symbol().index();
    kr::ps::ext::canonicalize(call_data);
    const auto call = repository->insert(call_data).first;

    auto variant_data = urd::data<kr::ps::Rule<kr::ExtFamilyTag>>("", call.index());
    const auto variant = repository->insert(variant_data).first;
    auto caller_data = make_module_data(*repository, "caller");
    caller_data.entry_memory_state = caller_entry.index();
    caller_data.memory_states.push_back(caller_entry.index());
    caller_data.memory_states.push_back(caller_return.index());
    auto transition = urd::flat_vector<urd::index<kr::ps::Rule<kr::ExtFamilyTag>>> {};
    transition.push_back(variant.index());
    ygg::canonicalize(transition);
    caller_data.memory_transitions.push_back(std::move(transition));
    kr::ps::ext::canonicalize(caller_data);
    const auto caller = repository->insert(caller_data).first;

    const auto program = create_program(*repository, caller, { caller, callee });
    auto expander = kr::ps::ext::SuccessorExpander<tyr::GroundTag>(task_context, program);
    const auto planning_node = initial_planning_node(expander);
    const auto steps = collect_steps(expander, expander.initial_state(planning_node.get_state()));
    ASSERT_EQ(steps.size(), 1);
    EXPECT_EQ(steps.front().status, kr::ps::ProgramOutcome::APPLIED);
    EXPECT_EQ(steps.front().get_target().get_module_state().get_module().index(), callee.index());
    EXPECT_EQ(steps.front().get_target().get_module_state().get_memory_state().index(), callee_entry.index());
}

TEST(RunirTests, ExtDoRuleAppliesMatchingActionAndAdvancesMemory)
{
    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
    auto task_context = kr::TaskContext<tyr::GroundTag>::create(kr::DomainContext::create(task->get_domain()), search_context);

    auto dl_repository = task_context->domain_context->ext_repository->get_dl_repository_ptr();
    auto repository = task_context->domain_context->ext_repository;

    const auto source = create_memory_state(*repository, "source");
    const auto target = create_memory_state(*repository, "target");
    const auto ball_feature =
        create_concept_feature(*repository,
                               kr::ps::ext::dl::parse_concept("(c_atomic_state \"ball\")", task->get_domain().get_domain(), *dl_repository).index(),
                               "ball");
    const auto room_feature =
        create_concept_feature(*repository,
                               kr::ps::ext::dl::parse_concept("(c_atomic_state \"room\")", task->get_domain().get_domain(), *dl_repository).index(),
                               "room");
    const auto gripper_feature =
        create_concept_feature(*repository,
                               kr::ps::ext::dl::parse_concept("(c_atomic_state \"gripper\")", task->get_domain().get_domain(), *dl_repository).index(),
                               "gripper");

    auto do_data = urd::data<kr::ps::ext::Rule<kr::ps::ext::DoTag>>();
    do_data.action_name = "pick";
    do_data.source = source.index();
    do_data.target = target.index();
    do_data.arguments.push_back(ball_feature.index());
    do_data.arguments.push_back(room_feature.index());
    do_data.arguments.push_back(gripper_feature.index());
    kr::ps::ext::canonicalize(do_data);
    const auto rule = repository->insert(do_data).first;
    auto variant_data = urd::data<kr::ps::Rule<kr::ExtFamilyTag>>("", rule.index());
    const auto variant = repository->insert(variant_data).first;

    auto module_data = make_module_data(*repository, "module");
    module_data.entry_memory_state = source.index();
    module_data.memory_states.push_back(source.index());
    module_data.memory_states.push_back(target.index());
    module_data.concept_features.push_back(ball_feature.index());
    module_data.concept_features.push_back(room_feature.index());
    module_data.concept_features.push_back(gripper_feature.index());
    auto transition = urd::flat_vector<urd::index<kr::ps::Rule<kr::ExtFamilyTag>>> {};
    transition.push_back(variant.index());
    ygg::canonicalize(transition);
    module_data.memory_transitions.push_back(std::move(transition));
    kr::ps::ext::canonicalize(module_data);
    const auto module_ = repository->insert(module_data).first;
    const auto program = create_program(*repository, module_, { module_ });
    auto expander = kr::ps::ext::SuccessorExpander<tyr::GroundTag>(task_context, program);
    const auto planning_node = initial_planning_node(expander);
    const auto initial_state = expander.initial_state(planning_node.get_state());
    const auto steps = collect_steps(expander, initial_state);
    ASSERT_GT(steps.size(), 1);

    for (const auto& step : steps)
    {
        EXPECT_EQ(step.status, kr::ps::ProgramOutcome::APPLIED);
        ASSERT_TRUE(step.planning_successor.has_value());
        EXPECT_EQ(step.planning_successor.value().label.get_relation().get_name(), "pick");
        EXPECT_EQ(step.get_target().get_module_state().get_memory_state().index(), target.index());
        EXPECT_NE(step.get_target().get_state().index(), initial_state.get_state().index());
    }

    auto greedy_options = kr::ps::ext::ProgramSearchOptions<tyr::GroundTag> {};
    const auto greedy = kr::ps::ext::find_solution(task_context, program, greedy_options);
    auto universal_options = kr::ps::ext::ProgramSearchOptions<tyr::GroundTag> {};
    universal_options.universal = true;
    const auto universal = kr::ps::ext::find_solution(task_context, program, universal_options);
    ASSERT_TRUE(greedy.graph);
    ASSERT_TRUE(universal.graph);
    ASSERT_EQ(greedy.graph->out_degree(0), 1);
    EXPECT_EQ(universal.graph->out_degree(0), steps.size());

    const auto edge = greedy.graph->out_edges(0).front();
    const auto selected = greedy.graph->vertex_value(greedy.graph->target(edge)).program_state;
    EXPECT_EQ(selected.get_state().index(), steps.front().get_target().get_state().index());
    EXPECT_EQ(selected.get_module_state().get_memory_state().index(), steps.front().get_target().get_module_state().get_memory_state().index());
}

TEST(RunirTests, ExtDoRuleRejectsActionWithIncompatibleDeclaredEffects)
{
    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
    auto task_context = kr::TaskContext<tyr::GroundTag>::create(kr::DomainContext::create(task->get_domain()), search_context);

    auto dl_repository = task_context->domain_context->ext_repository->get_dl_repository_ptr();
    auto repository = task_context->domain_context->ext_repository;

    const auto module_ =
        kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/executor/ext_do_rule_rejects_action_with_incompatible_declared_effects/module.module"),
                                      task->get_domain().get_domain(),
                                      *repository);
    const auto program = create_program(*repository, module_, { module_ });
    auto expander = kr::ps::ext::SuccessorExpander<tyr::GroundTag>(task_context, program);
    const auto planning_node = initial_planning_node(expander);
    const auto initial_state = expander.initial_state(planning_node.get_state());
    const auto steps = collect_steps(expander, initial_state);
    ASSERT_EQ(steps.size(), 1);
    EXPECT_EQ(steps.front().status, kr::ps::ProgramOutcome::NO_APPLICABLE_ACTION);
    EXPECT_EQ(steps.front().get_target().get_state().index(), initial_state.get_state().index());

    auto options = kr::ps::ext::ProgramSearchOptions<tyr::GroundTag> {};
    options.universal = true;
    const auto result = kr::ps::ext::find_solution(task_context, program, options);
    EXPECT_EQ(result.status, kr::ps::ProofStatus::FAILURE);
    ASSERT_TRUE(result.graph);
    EXPECT_EQ(result.graph->num_vertices(), 1);
    EXPECT_EQ(result.graph->num_edges(), 0);
    EXPECT_TRUE(result.deadend_states.empty());
    EXPECT_FALSE(result.open_states.empty());
}

TEST(RunirTests, ExtImmediateExternalRulesUseCanonicalFirstApplicableRule)
{
    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
    auto task_context = kr::TaskContext<tyr::GroundTag>::create(kr::DomainContext::create(task->get_domain()), search_context);

    auto dl_repository = task_context->domain_context->ext_repository->get_dl_repository_ptr();
    auto repository = task_context->domain_context->ext_repository;

    const auto source = create_memory_state(*repository, "source");
    const auto move_target = create_memory_state(*repository, "move_target");
    const auto pick_target = create_memory_state(*repository, "pick_target");
    const auto ball_feature =
        create_concept_feature(*repository,
                               kr::ps::ext::dl::parse_concept("(c_atomic_state \"ball\")", task->get_domain().get_domain(), *dl_repository).index(),
                               "ball");
    const auto room_feature =
        create_concept_feature(*repository,
                               kr::ps::ext::dl::parse_concept("(c_atomic_state \"room\")", task->get_domain().get_domain(), *dl_repository).index(),
                               "room");
    const auto gripper_feature =
        create_concept_feature(*repository,
                               kr::ps::ext::dl::parse_concept("(c_atomic_state \"gripper\")", task->get_domain().get_domain(), *dl_repository).index(),
                               "gripper");

    auto move_data = urd::data<kr::ps::ext::Rule<kr::ps::ext::DoTag>>();
    move_data.action_name = "pick";
    move_data.source = source.index();
    move_data.target = move_target.index();
    move_data.arguments.push_back(ball_feature.index());
    move_data.arguments.push_back(room_feature.index());
    move_data.arguments.push_back(gripper_feature.index());
    kr::ps::ext::canonicalize(move_data);
    const auto move_rule = repository->insert(move_data).first;
    auto move_variant_data = urd::data<kr::ps::Rule<kr::ExtFamilyTag>>("", move_rule.index());
    const auto move_variant = repository->insert(move_variant_data).first;

    auto pick_data = urd::data<kr::ps::ext::Rule<kr::ps::ext::DoTag>>();
    pick_data.action_name = "pick";
    pick_data.source = source.index();
    pick_data.target = pick_target.index();
    pick_data.arguments.push_back(ball_feature.index());
    pick_data.arguments.push_back(room_feature.index());
    pick_data.arguments.push_back(gripper_feature.index());
    kr::ps::ext::canonicalize(pick_data);
    const auto pick_rule = repository->insert(pick_data).first;
    auto pick_variant_data = urd::data<kr::ps::Rule<kr::ExtFamilyTag>>("", pick_rule.index());
    const auto pick_variant = repository->insert(pick_variant_data).first;

    auto module_data = make_module_data(*repository, "module");
    module_data.entry_memory_state = source.index();
    module_data.memory_states.push_back(source.index());
    module_data.memory_states.push_back(move_target.index());
    module_data.memory_states.push_back(pick_target.index());
    module_data.concept_features.push_back(ball_feature.index());
    module_data.concept_features.push_back(room_feature.index());
    module_data.concept_features.push_back(gripper_feature.index());

    auto move_transition = urd::flat_vector<urd::index<kr::ps::Rule<kr::ExtFamilyTag>>>();
    move_transition.push_back(move_variant.index());
    ygg::canonicalize(move_transition);
    module_data.memory_transitions.push_back(std::move(move_transition));

    auto pick_transition = urd::flat_vector<urd::index<kr::ps::Rule<kr::ExtFamilyTag>>>();
    pick_transition.push_back(pick_variant.index());
    ygg::canonicalize(pick_transition);
    module_data.memory_transitions.push_back(std::move(pick_transition));

    kr::ps::ext::canonicalize(module_data);
    const auto module_ = repository->insert(module_data).first;

    const auto program = create_program(*repository, module_, { module_ });
    auto expander = kr::ps::ext::SuccessorExpander<tyr::GroundTag>(task_context, program);
    const auto planning_node = initial_planning_node(expander);
    const auto steps = collect_steps(expander, expander.initial_state(planning_node.get_state()));
    ASSERT_GT(steps.size(), 2);
    EXPECT_EQ(steps.front().status, kr::ps::ProgramOutcome::APPLIED);
    EXPECT_EQ(steps.front().get_target().get_module_state().get_memory_state().index(), move_target.index());

    auto reached_move_target = false;
    auto reached_pick_target = false;
    for (const auto& step : steps)
    {
        ASSERT_TRUE(step.planning_successor.has_value());
        reached_move_target |= step.get_target().get_module_state().get_memory_state().index() == move_target.index();
        reached_pick_target |= step.get_target().get_module_state().get_memory_state().index() == pick_target.index();
    }
    EXPECT_TRUE(reached_move_target);
    EXPECT_TRUE(reached_pick_target);

    // Explicit application also accepts a rule created after the program was prepared.
    auto external_data = urd::data<kr::ps::Rule<kr::ExtFamilyTag>>(std::string("external"), pick_rule.index());
    const auto external_rule = repository->insert(external_data).first;
    const auto external = expander.apply(expander.initial_state(planning_node.get_state()), external_rule, steps.front().planning_successor.value().unpack());
    ASSERT_TRUE(external);
    ASSERT_TRUE(external.value().rule);
    EXPECT_EQ(external.value().rule.value().index(), external_rule.index());
    EXPECT_EQ(external.value().get_target().get_module_state().get_memory_state().index(), pick_target.index());

    auto greedy_options = kr::ps::ext::ProgramSearchOptions<tyr::GroundTag> {};
    const auto greedy = kr::ps::ext::find_solution(task_context, program, greedy_options);
    auto universal_options = kr::ps::ext::ProgramSearchOptions<tyr::GroundTag> {};
    universal_options.universal = true;
    const auto universal = kr::ps::ext::find_solution(task_context, program, universal_options);
    ASSERT_TRUE(greedy.graph);
    ASSERT_TRUE(universal.graph);
    ASSERT_EQ(greedy.graph->out_degree(0), 1);
    EXPECT_EQ(universal.graph->out_degree(0), steps.size());
}

TEST(RunirTests, ExtExplicitApplicationDistinguishesCollidingRuleIdsAcrossRepositories)
{
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../../fixtures/kr/ps/ext/choose";
    const auto context = create_task_context<tyr::GroundTag>(directory / "domain.pddl", directory / "task.pddl");
    auto foreign_factory = kr::ps::ext::RepositoryFactory {};
    const auto foreign_repository = foreign_factory.create(context->domain_context->ext_repository->get_dl_repository_ptr());
    const auto distinct_repository = foreign_factory.create(context->domain_context->ext_repository->get_dl_repository_ptr());
    const auto make_module = [&](kr::ps::ext::Repository& repository, const char* target)
    {
        return kr::ps::ext::dl::parse_module(fmt::format(R"((:module (:symbol policy) (:arguments) (:registers)
                (:entry source) (:memory source prepared external)
                (:features)
                (:rules (:rule (:symbol advance) (:expression
                    (:source-memory source) (:target-memory {})
                    (:sketch (:conditions) (:effects)))))))",
                                                         target),
                                             context->search_context->task->get_domain().get_domain(),
                                             repository);
    };
    auto& repository = *context->domain_context->ext_repository;
    const auto prepared_module = make_module(repository, "prepared");
    const auto foreign_module = make_module(*foreign_repository, "external");
    const auto distinct_module = make_module(*distinct_repository, "external");
    const auto prepared_rule = prepared_module.get_memory_transitions()[0][0];
    const auto foreign_rule = foreign_module.get_memory_transitions()[0][0];
    ASSERT_EQ(prepared_rule.index(), foreign_rule.index());
    ASSERT_NE(&prepared_rule.context(), &foreign_rule.context());

    const auto program = create_program(repository, prepared_module, { prepared_module });
    auto expander = kr::ps::ext::SuccessorExpander<tyr::GroundTag>(context, program);
    const auto planning_node = initial_planning_node(expander);
    const auto initial = expander.initial_state(planning_node.get_state());
    const auto prepared = expander.apply(initial, prepared_rule);
    const auto foreign = expander.apply(initial, foreign_rule);
    ASSERT_TRUE(prepared);
    ASSERT_TRUE(foreign);
    ASSERT_TRUE(foreign.value().rule);
    EXPECT_EQ(prepared.value().get_target().get_module_state().get_memory_state().get_name(), "prepared");
    EXPECT_EQ(foreign.value().get_target().get_module_state().get_memory_state().get_name(), "external");
    EXPECT_EQ(&foreign.value().rule.value().context(), &foreign_rule.context());

    // A colliding memory index from a different repository identity cannot
    // make a rule applicable to the current program state.
    ASSERT_EQ(distinct_module.get_entry_memory_state().index(), initial.get_module_state().get_memory_state().index());
    ASSERT_NE(distinct_module.get_entry_memory_state(), initial.get_module_state().get_memory_state());
    EXPECT_FALSE(expander.apply(initial, distinct_module.get_memory_transitions()[0][0]));
}

TEST(RunirTests, ExtQueryActionContractsGround) { expect_query_action_contracts<tyr::GroundTag>(); }

TEST(RunirTests, ExtUnknownActionRejectedDuringPreparationGround)
{
    expect_unknown_action_rejected_during_preparation<tyr::GroundTag, kr::ps::ext::ActionTag>();
    expect_unknown_action_rejected_during_preparation<tyr::GroundTag, kr::ps::ext::DoTag>();
}

TEST(RunirTests, ExtCallbackSelectionStopsAfterSelectedRuleGround) { expect_callback_selection_stops_after_selected_rule<tyr::GroundTag>(); }

TEST(RunirTests, ExtRuleOccurrenceOrderGround) { expect_rule_occurrence_order<tyr::GroundTag>(); }

TEST(RunirTests, ExtCallbackSketchOrderAndCancellationGround) { expect_callback_sketch_order_and_cancellation<tyr::GroundTag>(); }

TEST(RunirTests, ExtQueryActionExistentialBindingGround) { expect_query_action_existential_binding<tyr::GroundTag>(); }

TEST(RunirTests, ExtQueryActionNullaryBindingGround) { expect_query_action_nullary_binding<tyr::GroundTag>(); }

TEST(RunirTests, ExtCallbackExpansionCountsGround) { expect_callback_expansion_counts<tyr::GroundTag>(); }

}  // namespace runir::tests
