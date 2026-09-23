#include "fixtures.hpp"
#include "module_fixtures.hpp"
#include "planning_fixtures.hpp"
#include "successor_fixtures.hpp"

#include <algorithm>
#include <concepts>
#include <filesystem>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <runir/kr/ps/ext/execution_repository.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/program_executor.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/ext/successor_expander.hpp>
#include <runir/kr/task_context.hpp>
#include <set>
#include <string>
#include <type_traits>
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

template<tyr::TaskKind Kind>
auto create_task_context(const std::filesystem::path& domain, const std::filesystem::path& task_file)
{
    datasets::TaskSearchContextPtr<Kind> search_context;
    if constexpr (std::same_as<Kind, tyr::GroundTag>)
        search_context = make_ground_context(domain, task_file);
    else
        search_context = make_lifted_context(domain, task_file);
    return kr::TaskContext<Kind>::create(kr::DomainContext::create(search_context->task->get_domain()), search_context);
}

template<tyr::TaskKind Kind>
void expect_initial_program_state_uses_expander_repository()
{
    const auto domain = benchmark_path("classical/tests/gripper/domain.pddl");
    const auto task_file = benchmark_path("classical/tests/gripper/test-1.pddl");
    auto task_context = create_task_context<Kind>(domain, task_file);
    auto dl_repository = task_context->domain_context->ext_repository->get_dl_repository_ptr();
    auto repository = task_context->domain_context->ext_repository;
    const auto entry = create_memory_state(*repository, "entry");
    const auto module_ = create_module(*repository, "module", entry, { entry });
    const auto program = create_program(*repository, module_, { module_ });
    auto expander = kr::ps::ext::SuccessorExpander<Kind>(task_context, program);
    const auto planning_node = initial_planning_node(expander);
    const auto state = expander.initial_state(planning_node.get_state());

    task_context.reset();
    repository.reset();
    dl_repository.reset();
    EXPECT_EQ(state.get_module_state().get_module().get_name(), "module");
    EXPECT_EQ(state.get_module_state().get_memory_state().get_name(), "entry");
    EXPECT_EQ(state.get_program().get_entry_module().get_name(), "module");
    EXPECT_FALSE(state.get_call_stack());
}

template<tyr::TaskKind Kind>
void expect_module_return_preserves_callee_planning_state()
{
    namespace ext = kr::ps::ext;
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../../fixtures/kr/ps/ext/choose";
    const auto context = create_task_context<Kind>(directory / "domain.pddl", directory / "task.pddl");
    const auto program = ext::dl::parse_program(R"(
(:program (:entry caller)
  (:module (:symbol caller) (:arguments) (:registers (:concept saved))
    (:entry save) (:memory save call returned)
    (:features (:concept (:symbol Goal) (:expression (c_atomic_goal "at" true))))
    (:rules
      (:rule (:symbol save-goal) (:expression (:source-memory save) (:target-memory call)
        (:load (:conditions) (:concept Goal) (:register (:concept saved)))))
      (:rule (:symbol call) (:expression (:source-memory call) (:target-memory returned)
        (:call (:conditions) (:callee callee) (:arguments Goal))))))
  (:module (:symbol callee) (:arguments (:concept desired)) (:registers)
    (:entry call) (:memory call returned)
    (:features (:concept (:symbol Here) (:expression (c_atomic_state "at"))))
    (:rules (:rule (:symbol call) (:expression (:source-memory call) (:target-memory returned)
      (:call (:conditions) (:callee leaf) (:arguments Here))))))
  (:module (:symbol leaf) (:arguments (:concept origin)) (:registers)
    (:entry move) (:memory move done)
    (:features
      (:concept (:symbol Here) (:expression (c_argument origin)))
      (:concept (:symbol Candidates) (:expression (c_atomic_state "candidate"))))
    (:rules (:rule (:symbol move) (:expression (:source-memory move) (:target-memory done)
      (:do (:conditions) (:action "move") (:arguments Here Candidates) (:effects)))))))
)",
                                                       context->search_context->task->get_domain().get_domain(),
                                                       *context->domain_context->ext_repository);
    auto expander = ext::SuccessorExpander<Kind>(context, program);
    const auto planning_node = initial_planning_node(expander);
    const auto initial = expander.initial_state(planning_node.get_state());
    EXPECT_FALSE(initial.get_call_stack());
    const auto loads = collect_steps(expander, initial);
    ASSERT_EQ(loads.size(), 1);
    const auto calls = collect_steps(expander, loads.front().get_target());
    ASSERT_EQ(calls.size(), 1);
    const auto callee = calls.front().get_target();
    const auto suspended = callee.get_call_stack();
    ASSERT_TRUE(suspended);
    EXPECT_FALSE(suspended->get_caller());
    const auto nested_calls = collect_steps(expander, callee);
    ASSERT_EQ(nested_calls.size(), 1);
    const auto leaf = nested_calls.front().get_target();
    const auto suspended_callee = leaf.get_call_stack();
    ASSERT_TRUE(suspended_callee);
    EXPECT_EQ(suspended_callee->get_caller(), suspended);
    EXPECT_NE(leaf.get_module_state().get_arguments(), callee.get_module_state().get_arguments());
    const auto moves = collect_steps(expander, leaf);
    ASSERT_EQ(moves.size(), 2);
    for (const auto& move : moves)
    {
        const auto moved = move.get_target();
        ASSERT_TRUE(move.planning_successor);
        ASSERT_NE(moved.get_state().get_index(), initial.get_state().get_index());
        EXPECT_EQ(move.planning_successor->node.get_state(), moved.get_state().pack());
        const auto returns = collect_steps(expander, moved);
        ASSERT_EQ(returns.size(), 1);
        EXPECT_EQ(returns.front().status, ext::detail::ProgramOutcome::RESTORED_CALLER);
        EXPECT_FALSE(returns.front().planning_successor);
        const auto returned = returns.front().get_target();
        EXPECT_EQ(returned.get_state().get_index(), moved.get_state().get_index());
        EXPECT_EQ(returned.get_module_state().get_module(), suspended_callee->get_module());
        EXPECT_EQ(returned.get_module_state().get_memory_state(), suspended_callee->get_return_memory_state());
        EXPECT_EQ(returned.get_module_state().get_registers(), suspended_callee->get_registers());
        EXPECT_EQ(returned.get_module_state().get_arguments(), callee.get_module_state().get_arguments());
        EXPECT_EQ(returned.get_call_stack(), suspended);

        const auto outer_returns = collect_steps(expander, returned);
        ASSERT_EQ(outer_returns.size(), 1);
        EXPECT_EQ(outer_returns.front().status, ext::detail::ProgramOutcome::RESTORED_CALLER);
        EXPECT_FALSE(outer_returns.front().planning_successor);
        const auto caller = outer_returns.front().get_target();
        EXPECT_EQ(caller.get_state().get_index(), moved.get_state().get_index());
        EXPECT_EQ(caller.get_module_state().get_module(), suspended->get_module());
        EXPECT_EQ(caller.get_module_state().get_memory_state(), suspended->get_return_memory_state());
        EXPECT_EQ(caller.get_module_state().get_registers(), suspended->get_registers());
        EXPECT_EQ(caller.get_module_state().get_arguments(), suspended->get_arguments());
        EXPECT_FALSE(caller.get_call_stack());
    }
    EXPECT_EQ(callee.get_state().get_index(), initial.get_state().get_index());
}

template<tyr::TaskKind Kind>
void expect_borrowed_query_evaluation()
{
    namespace ext = kr::ps::ext;
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../../fixtures/kr/ps/ext/choose";
    const auto task_context = create_task_context<Kind>(directory / "domain.pddl", directory / "task.pddl");
    auto& repository = *task_context->domain_context->ext_repository;
    auto& search = *task_context->search_context;
    const auto module_ = ext::dl::parse_module(
        R"((:module (:symbol queries) (:arguments) (:registers)
            (:entry source) (:memory source)
            (:features (:query (:symbol At) (:expression (q_rename (location) (q_atomic_state "at" (x))))))
            (:rules)))",
        search.task->get_domain().get_domain(),
        repository);
    const auto program = create_program(repository, module_, { module_ });
    const auto initial = search.successor_generator->get_initial_node(*search.state_repository, *search.axiom_evaluator);
    auto expander = ext::SuccessorExpander<Kind>(task_context, program);
    const auto planning_node = initial_planning_node(expander);
    const auto state = expander.initial_state(planning_node.get_state());
    auto environment = ext::EvaluationEnvironment<Kind>(*task_context, program);
    auto state_context = environment.make_dl_context(state);
    const auto feature = module_.get_query_features()[0];
    const auto query = feature.get_expression();
    {
        const auto rows = kr::ps::evaluate(feature, state_context);
        ASSERT_EQ(rows.size(), 1);
        EXPECT_EQ(&kr::ps::evaluate(feature, state_context).storage(), &rows.storage());
        const auto& cached = environment.get_dl_caches().get_queries(false).at(query);
        EXPECT_EQ(&rows.storage(), &cached.storage());
        EXPECT_EQ(rows.columns().data(), cached.columns().data());
        EXPECT_TRUE(std::ranges::equal(rows.columns(), query.get_schema()));

        const auto successors = search.successor_generator->get_successor_nodes(initial, *search.state_repository, *search.axiom_evaluator);
        ASSERT_FALSE(successors.empty());
        const auto stack = state.get_module_state();
        auto transition = environment.make_dl_transition_context(state.get_state(),
                                                                 successors.front().get_state(),
                                                                 stack.get_arguments(),
                                                                 stack.get_registers(),
                                                                 stack.get_registers());
        {
            const auto target_rows = kr::ps::evaluate(feature, transition.get_target_context());
            ASSERT_EQ(target_rows.size(), 1);
            EXPECT_NE(target_rows[0][0], rows[0][0]);
            EXPECT_NE(&target_rows.storage(), &rows.storage());
        }
        EXPECT_TRUE(environment.get_dl_target_caches().get_queries(false).contains(query));
        environment.get_dl_target_caches().clear(false);
        EXPECT_TRUE(environment.get_dl_caches().get_queries(false).contains(query));
        EXPECT_EQ(rows.size(), 1);
        EXPECT_EQ(&kr::ps::evaluate(feature, state_context).storage(), &rows.storage());
    }
    environment.get_dl_caches().clear(false);
    EXPECT_TRUE(environment.get_dl_caches().get_queries(false).empty());
    const auto rebuilt = kr::ps::evaluate(feature, state_context);
    EXPECT_EQ(rebuilt.size(), 1);
    EXPECT_TRUE(std::ranges::equal(rebuilt.columns(), query.get_schema()));
}

template<tyr::TaskKind Kind>
void expect_binding_effects_and_empty_choices()
{
    const auto task_context =
        create_task_context<Kind>(benchmark_path("classical/tests/gripper/domain.pddl"), benchmark_path("classical/tests/gripper/test-1.pddl"));
    auto& repository = *task_context->domain_context->ext_repository;
    const auto domain = task_context->search_context->task->get_domain().get_domain();
    for (const auto role : { false, true })
        for (const auto choose : { false, true })
            for (const auto scenario : { 0, 1, 2, 3, 4 })
            {
                SCOPED_TRACE(fmt::format("role={}, choose={}, scenario={}", role, choose, scenario));
                const auto category = role ? "role" : "concept";
                const auto selected = role ? "(c_some (r_register r) (c_top))" : "(c_register r)";
                const auto candidates = scenario == 3 ? (role ? "(r_restriction (r_atomic_state \"at\") (c_bot))" : "(c_bot)") :
                                                        (role ? "(r_atomic_state \"at\")" : "(c_atomic_state \"ball\")");
                const auto effects =
                    scenario == 1 ? "(:effects (positive goal) (increases size) (unchanged total))" : (scenario == 2 ? "(:effects (decreases size))" : "");
                const auto source = fmt::format(R"(
(:module (:symbol binding) (:arguments) (:registers (:{0} r))
  (:entry source) (:memory source target)
  (:features
    (:{0} (:symbol candidates) (:expression {1}))
    (:boolean (:symbol goal) (:expression
      (b_nonempty (c_and {2} (c_some (r_atomic_goal "at" true) (c_top))))))
    (:numerical (:symbol size) (:expression (n_count {2})))
    (:numerical (:symbol total) (:expression (n_count (c_top)))))
  (:rules (:rule (:symbol bind) (:expression
    (:source-memory source) (:target-memory target)
    (:{3} (:conditions ({4} size) (negative goal))
      (:{0} candidates) (:register (:{0} r)) {5})))))
)",
                                                category,
                                                candidates,
                                                selected,
                                                choose ? "choose" : "load",
                                                scenario == 4 ? "greater_zero" : "equal_zero",
                                                effects);
                const auto module_ = kr::ps::ext::dl::parse_module(source, domain, repository);
                const auto program = create_program(repository, module_, { module_ });
                auto expander = kr::ps::ext::SuccessorExpander<Kind>(task_context, program);
                const auto planning_node = initial_planning_node(expander);
                const auto initial = expander.initial_state(planning_node.get_state());
                const auto steps = collect_steps(expander, initial);
                const auto successful = scenario < 2;
                if (successful)
                {
                    ASSERT_EQ(steps.size(), scenario == 0 ? 2 : 1);
                    for (const auto& step : steps)
                    {
                        EXPECT_EQ(step.status, kr::ps::ext::detail::ProgramOutcome::APPLIED);
                        const auto target = step.get_target();
                        EXPECT_EQ(target.get_state().get_index(), initial.get_state().get_index());
                        EXPECT_EQ(target.get_module_state().get_memory_state().get_name(), "target");
                        EXPECT_FALSE(step.planning_successor.has_value());
                        ASSERT_TRUE(step.rule);
                        const auto values = target.get_module_state().get_registers();
                        if (role)
                        {
                            ASSERT_TRUE(values.get_role_values()[0]);
                            const auto pair = values.get_role_values()[0].value();
                            EXPECT_EQ(pair.get_second().get_name(), "rooma");
                            if (scenario == 1)
                            {
                                EXPECT_EQ(pair.get_first().get_name(), "ball2");
                            }
                        }
                        else
                        {
                            ASSERT_TRUE(values.get_concept_values()[0]);
                            if (scenario == 1)
                            {
                                EXPECT_EQ(values.get_concept_values()[0].value().get_name(), "ball2");
                            }
                        }
                    }
                    const auto applied = expander.apply(initial, *steps.front().rule);
                    ASSERT_TRUE(applied);
                    EXPECT_EQ(applied->get_target().get_index(), steps.front().get_target().get_index());
                }
                else if (choose && scenario != 4)
                {
                    ASSERT_EQ(steps.size(), 1);
                    EXPECT_EQ(steps.front().status, kr::ps::ext::detail::ProgramOutcome::FAILURE);
                    EXPECT_EQ(steps.front().get_target().get_index(), initial.get_index());
                    ASSERT_TRUE(steps.front().rule);
                }
                else
                {
                    ASSERT_EQ(steps.size(), 1);
                    EXPECT_EQ(steps.front().status, kr::ps::ext::detail::ProgramOutcome::NO_APPLICABLE_ACTION);
                }
                for (const auto value : initial.get_module_state().get_registers().get_concept_values())
                    EXPECT_FALSE(value);
                for (const auto value : initial.get_module_state().get_registers().get_role_values())
                    EXPECT_FALSE(value);
                EXPECT_EQ(initial.get_module_state().get_memory_state().get_name(), "source");
                EXPECT_TRUE(collect_steps(expander, initial, false, [] { return true; }).empty());
            }
}

template<tyr::TaskKind Kind>
void expect_lazy_do_successors()
{
    namespace ext = kr::ps::ext;
    for (const auto scenario : { 0, 1, 2, 3, 4 })
    {
        SCOPED_TRACE(fmt::format("scenario={}", scenario));
        const auto task_context =
            create_task_context<Kind>(benchmark_path("classical/tests/gripper/domain.pddl"), benchmark_path("classical/tests/gripper/test-1.pddl"));
        auto& repository = *task_context->domain_context->ext_repository;
        auto& states = *task_context->search_context->state_repository;
        const auto domain = task_context->search_context->task->get_domain().get_domain();
        const auto selected_ball = scenario == 2 ? "(c_bot)" : "(c_some (r_atomic_goal \"at\" true) (c_top))";
        const auto sketch = scenario == 4 ? R"(
    (:rule (:symbol move) (:expression
      (:source-memory source) (:target-memory sketch-target)
      (:sketch (:conditions) (:effects (unchanged Free)))))
)" :
                                            "";
        const auto source = fmt::format(R"(
(:module (:symbol lazy) (:arguments) (:registers)
  (:entry source) (:memory source do-target sketch-target)
  (:features
    (:concept (:symbol Ball) (:expression {0}))
    (:concept (:symbol Room) (:expression (c_atomic_state "room")))
    (:concept (:symbol Gripper) (:expression (c_atomic_state "gripper")))
    (:numerical (:symbol Free) (:expression (n_count (c_atomic_state "free")))))
  (:rules
    (:rule (:symbol pick) (:expression
      (:source-memory source) (:target-memory do-target)
      (:do (:conditions ({1} Free)) (:action "pick")
        (:arguments Ball Room Gripper) (:effects ({2} Free)))))
    {3}))
)",
                                        selected_ball,
                                        scenario == 3 ? "equal_zero" : "greater_zero",
                                        scenario == 1 ? "increases" : "decreases",
                                        sketch);
        const auto module_ = ext::dl::parse_module(source, domain, repository);
        const auto program = create_program(repository, module_, { module_ });
        auto expander = ext::SuccessorExpander<Kind>(task_context, program);
        const auto planning_node = initial_planning_node(expander);
        const auto initial = expander.initial_state(planning_node.get_state());
        ASSERT_EQ(states.num_states(), 1);
        EXPECT_TRUE(collect_steps(expander, initial, false, [] { return true; }).empty());
        EXPECT_EQ(states.num_states(), 1);

        const auto steps = collect_steps(expander, initial);
        if (scenario < 2)
        {
            // Only ball2's two pick bindings construct states. Other balls and
            // the unrelated move schema must not be expanded before filtering.
            EXPECT_EQ(states.num_states(), 3);
        }
        else if (scenario < 4)
        {
            EXPECT_EQ(states.num_states(), 1);
        }
        if (scenario == 0 || scenario == 4)
        {
            ASSERT_EQ(steps.size(), scenario == 4 ? 4 : 2);
            auto moves = std::size_t(0);
            for (const auto& step : steps)
            {
                EXPECT_EQ(step.status, ext::detail::ProgramOutcome::APPLIED);
                ASSERT_TRUE(step.planning_successor.has_value());
                const auto action = step.planning_successor->label;
                if (action.get_relation().get_name() == "pick")
                {
                    EXPECT_EQ(action.get_objects()[0].get_name(), "ball2");
                    EXPECT_EQ(step.get_target().get_module_state().get_memory_state().get_name(), "do-target");
                }
                else
                {
                    EXPECT_EQ(action.get_relation().get_name(), "move");
                    EXPECT_EQ(step.get_target().get_module_state().get_memory_state().get_name(), "sketch-target");
                    ++moves;
                }
            }
            EXPECT_EQ(moves, scenario == 4 ? 2 : 0);
        }
        else
        {
            ASSERT_EQ(steps.size(), 1);
            EXPECT_EQ(steps.front().status, ext::detail::ProgramOutcome::NO_APPLICABLE_ACTION);
        }

        const auto same_step = [](const auto& left, const auto& right)
        {
            return left.status == right.status && left.get_target().get_index() == right.get_target().get_index()
                   && left.rule.has_value() == right.rule.has_value() && (!left.rule || left.rule->get_index() == right.rule->get_index())
                   && left.planning_successor.has_value() == right.planning_successor.has_value()
                   && (!left.planning_successor.has_value() || left.planning_successor->label == right.planning_successor->label);
        };
        const auto repeated = collect_steps(expander, initial);
        EXPECT_TRUE(std::ranges::equal(steps, repeated, same_step));
        EXPECT_TRUE(collect_steps(expander, initial, false, [] { return true; }).empty());
        if (scenario == 0)
        {
            ASSERT_TRUE(steps.front().rule);
            EXPECT_FALSE(expander.apply(initial, *steps.front().rule));
        }
    }
}

template<tyr::TaskKind Kind>
void expect_control_only_steps_do_not_generate_planning_successors()
{
    namespace ext = kr::ps::ext;
    const auto task_context =
        create_task_context<Kind>(benchmark_path("classical/tests/gripper/domain.pddl"), benchmark_path("classical/tests/gripper/test-1.pddl"));
    auto& repository = *task_context->domain_context->ext_repository;
    auto& states = *task_context->search_context->state_repository;
    const auto program = ext::dl::parse_program(R"(
(:program (:entry root)
  (:module (:symbol root) (:arguments) (:registers (:concept r))
    (:entry source) (:memory source loaded chosen called skipped)
    (:features
      (:concept (:symbol Ball) (:expression (c_some (r_atomic_goal "at" true) (c_top)))))
    (:rules
      (:rule (:symbol load) (:expression
        (:source-memory source) (:target-memory loaded)
        (:load (:conditions) (:concept Ball) (:register (:concept r)))))
      (:rule (:symbol choose) (:expression
        (:source-memory source) (:target-memory chosen)
        (:choose (:conditions) (:concept Ball) (:register (:concept r)))))
      (:rule (:symbol call) (:expression
        (:source-memory source) (:target-memory called)
        (:call (:conditions) (:callee leaf) (:arguments))))
      (:rule (:symbol skip) (:expression
        (:source-memory source) (:target-memory skipped)
        (:sketch (:conditions) (:effects))))))
  (:module (:symbol leaf) (:arguments) (:registers)
    (:entry source) (:memory source) (:features) (:rules)))
)",
                                                       task_context->search_context->task->get_domain().get_domain(),
                                                       repository);
    auto expander = ext::SuccessorExpander<Kind>(task_context, program);
    const auto planning_node = initial_planning_node(expander);
    const auto initial = expander.initial_state(planning_node.get_state());
    ASSERT_EQ(states.num_states(), 1);
    const auto steps = collect_steps(expander, initial);
    ASSERT_EQ(steps.size(), 4);
    for (const auto& step : steps)
    {
        EXPECT_EQ(step.status, ext::detail::ProgramOutcome::APPLIED);
        EXPECT_EQ(step.get_target().get_state().get_index(), initial.get_state().get_index());
        EXPECT_FALSE(step.planning_successor.has_value());
    }
    EXPECT_EQ(states.num_states(), 1);
    for (const auto universal : { false, true })
    {
        auto options = ext::ProgramSearchOptions<Kind> {};
        options.universal = universal;
        EXPECT_EQ(ext::find_solution(task_context, program, options).status, ext::ProgramProofStatus::FAILURE);
        EXPECT_EQ(states.num_states(), 1);
    }
    auto& search = *task_context->search_context;
    const auto successors = search.successor_generator->get_labeled_successor_nodes(planning_node, states, *search.axiom_evaluator);
    ASSERT_FALSE(successors.empty());
    EXPECT_FALSE(expander.matching_rule(initial, successors.front()));
}

}  // namespace

TEST(RunirTests, ExtDistanceFeatureEvaluationReusesTaskContextCache)
{
    namespace p = tyr::planning;

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
    EXPECT_EQ(first.front().status, kr::ps::ext::detail::ProgramOutcome::APPLIED);
    EXPECT_GT(task_context->dl_denotation_repository->size<kr::dl::semantics::Denotation<kr::dl::NumericalTag>>(), 0);
    const auto cached_denotations = task_context->dl_denotation_repository->size<kr::dl::semantics::Denotation<kr::dl::NumericalTag>>();

    const auto second = collect_steps(expander, initial_state);
    ASSERT_EQ(second.size(), 1);
    EXPECT_EQ(second.front().get_target().get_index(), first.front().get_target().get_index());
    EXPECT_EQ(task_context->dl_denotation_repository->size<kr::dl::semantics::Denotation<kr::dl::NumericalTag>>(), cached_denotations);
    EXPECT_EQ(initial_state.get_module_state().get_memory_state().get_name(), "source");
}

TEST(RunirTests, ExtGroundAndLiftedInitialStatesUseExpanderRepository)
{
    expect_initial_program_state_uses_expander_repository<tyr::GroundTag>();
    expect_initial_program_state_uses_expander_repository<tyr::LiftedTag>();
}

TEST(RunirTests, ExtChooseUsesNaturalDenotationCursors)
{
    namespace ext = kr::ps::ext;
    using Expander = ext::SuccessorExpander<tyr::GroundTag>;
    auto search = make_gripper_ground_context();
    auto context = kr::TaskContext<tyr::GroundTag>::create(kr::DomainContext::create(search->task->get_domain()), search);
    auto& repository = *context->domain_context->ext_repository;
    const auto module_ = ext::dl::parse_module(R"(
(:module (:symbol cursors) (:arguments) (:registers (:concept c) (:role r))
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
)", search->task->get_domain().get_domain(), repository);
    const auto program = create_program(repository, module_, { module_ });
    auto expander = Expander(context, program);
    const auto node = initial_planning_node(expander);
    const auto state = expander.initial_state(node.get_state());
    auto statistics = ext::ProgramSearchStatistics {};
    auto counts = std::vector<std::size_t> {};
    ASSERT_TRUE(expander.for_each_successor(
        state,
        statistics,
        [&](auto expansion)
        {
            std::visit(
                [&](auto choice)
                {
                    if constexpr (std::same_as<decltype(choice), Expander::Step>)
                        ADD_FAILURE() << "Expected a compact Choice descriptor";
                    else
                    {
                        counts.push_back(choice.count());
                        EXPECT_EQ(choice.has_alternatives(), choice.count() > 1);
                        auto expected = choice.denotation.begin();
                        std::size_t visited = 0;
                        while (!choice.exhausted())
                        {
                            EXPECT_EQ(choice.cursor, expected);
                            ++expected;
                            ++visited;
                            choice.advance();
                        }
                        EXPECT_EQ(expected, choice.denotation.end());
                        EXPECT_EQ(visited, choice.count());
                    }
                },
                std::move(expansion));
            return true;
        },
        [] { return false; }));
    EXPECT_EQ(statistics.num_generated, 0);
    std::ranges::sort(counts);
    EXPECT_EQ(counts, (std::vector<std::size_t> { 0, 0, 1, 1, 2, 2 }));
}

TEST(RunirTests, ExtGroundAndLiftedModuleReturnsPreserveCalleePlanningState)
{
    expect_module_return_preserves_callee_planning_state<tyr::GroundTag>();
    expect_module_return_preserves_callee_planning_state<tyr::LiftedTag>();
}

TEST(RunirTests, ExtGroundAndLiftedQueryEvaluationBorrowsIndependentSourceAndTargetCaches)
{
    expect_borrowed_query_evaluation<tyr::GroundTag>();
    expect_borrowed_query_evaluation<tyr::LiftedTag>();
}

TEST(RunirTests, ExtBindingEffectsFilterGroundAndLiftedLoadsAndChoices)
{
    expect_binding_effects_and_empty_choices<tyr::GroundTag>();
    expect_binding_effects_and_empty_choices<tyr::LiftedTag>();
}

TEST(RunirTests, ExtDoSuccessorsFilterArgumentsBeforeConstructingGroundAndLiftedStates)
{
    expect_lazy_do_successors<tyr::GroundTag>();
    expect_lazy_do_successors<tyr::LiftedTag>();
}

TEST(RunirTests, ExtControlOnlyGroundAndLiftedStepsDoNotGeneratePlanningSuccessors)
{
    expect_control_only_steps_do_not_generate_planning_successors<tyr::GroundTag>();
    expect_control_only_steps_do_not_generate_planning_successors<tyr::LiftedTag>();
}

TEST(RunirTests, ExtLoadRuleEnumeratesAllObjectsAndAdvancesMemory)
{
    namespace p = tyr::planning;

    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
    auto task_context = kr::TaskContext<tyr::GroundTag>::create(kr::DomainContext::create(task->get_domain()), search_context);

    auto dl_repository = task_context->domain_context->ext_repository->get_dl_repository_ptr();
    auto repository = task_context->domain_context->ext_repository;

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

    auto variant_data = ygg::Data<kr::ps::Rule<kr::ExtFamilyTag>>(load.get_index());
    const auto variant = repository->get_or_create(variant_data).first;

    auto module_data = make_module_data(*repository, "module");
    module_data.entry_memory_state = source.get_index();
    module_data.memory_states.push_back(source.get_index());
    module_data.memory_states.push_back(target.get_index());
    module_data.concept_registers.push_back(reg.get_index());
    module_data.concept_features.push_back(top_feature.get_index());
    auto transition = ygg::IndexList<kr::ps::Rule<kr::ExtFamilyTag>>();
    transition.push_back(variant.get_index());
    ygg::canonicalize(transition);
    module_data.memory_transitions.push_back(std::move(transition));
    kr::ps::ext::canonicalize(module_data);
    const auto module_ = repository->get_or_create(module_data).first;

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

    auto loaded_objects = std::set<ygg::uint_t> {};
    for (const auto& step : steps)
    {
        EXPECT_EQ(step.status, kr::ps::ext::detail::ProgramOutcome::APPLIED);
        const auto target_state = step.get_target();
        EXPECT_EQ(target_state.get_state().get_index(), initial_state.get_state().get_index());
        EXPECT_EQ(target_state.get_module_state().get_memory_state().get_index(), target.get_index());
        const auto loaded = target_state.get_module_state().get_registers().get_concept_values()[0];
        ASSERT_TRUE(loaded);
        loaded_objects.insert(ygg::uint_t(loaded.value().get_index()));
    }
    EXPECT_EQ(loaded_objects.size(), steps.size());

    auto greedy_options = kr::ps::ext::ProgramSearchOptions<tyr::GroundTag> {};
    const auto greedy = kr::ps::ext::find_solution(task_context, program, greedy_options);
    auto universal_options = kr::ps::ext::ProgramSearchOptions<tyr::GroundTag> {};
    universal_options.universal = true;
    const auto universal = kr::ps::ext::find_solution(task_context, program, universal_options);
    ASSERT_TRUE(greedy.graph);
    ASSERT_TRUE(universal.graph);
    ASSERT_EQ(greedy.graph->get_out_degree(0), 1);
    EXPECT_EQ(universal.graph->get_out_degree(0), steps.size());

    const auto edge = greedy.graph->get_out_edge_indices(0).front();
    const auto selected = greedy.graph->get_vertex(greedy.graph->get_target(edge)).get_property().program_state;
    const auto actual_loaded = selected.get_module_state().get_registers().get_concept_values()[0];
    const auto expected_loaded = steps.front().get_target().get_module_state().get_registers().get_concept_values()[0];
    ASSERT_TRUE(actual_loaded);
    ASSERT_TRUE(expected_loaded);
    EXPECT_EQ(actual_loaded.value().get_index(), expected_loaded.value().get_index());

    auto options = kr::ps::ext::ProgramSearchOptions<tyr::GroundTag> {};
    options.max_num_states = 1;
    options.universal = true;
    const auto bounded = kr::ps::ext::find_solution(task_context, program, options);
    EXPECT_EQ(bounded.status, kr::ps::ext::ProgramProofStatus::OUT_OF_STATES);
    ASSERT_TRUE(bounded.graph);
    EXPECT_EQ(bounded.graph->get_num_vertices(), 1);
}

TEST(RunirTests, ExtRoleLoadRuleEnumeratesAllPairsAndAdvancesMemory)
{
    namespace p = tyr::planning;

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

    auto loaded_pairs = std::set<std::pair<ygg::uint_t, ygg::uint_t>> {};
    for (const auto& step : steps)
    {
        EXPECT_EQ(step.status, kr::ps::ext::detail::ProgramOutcome::APPLIED);
        const auto target_state = step.get_target();
        EXPECT_EQ(target_state.get_state().get_index(), initial_state.get_state().get_index());
        EXPECT_EQ(target_state.get_module_state().get_memory_state().get_name(), "target");
        const auto loaded = target_state.get_module_state().get_registers().get_role_values()[0];
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

    auto load_steps = std::size_t(0);
    auto do_steps = std::size_t(0);
    auto sketch_steps = std::size_t(0);
    for (const auto& step : steps)
    {
        EXPECT_EQ(step.status, kr::ps::ext::detail::ProgramOutcome::APPLIED);
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
    ASSERT_EQ(greedy.graph->get_out_degree(0), 1);
    EXPECT_EQ(universal.graph->get_out_degree(0), steps.size());
}

TEST(RunirTests, ExtCallRulePassesArgumentDenotationsToCallee)
{
    namespace p = tyr::planning;

    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
    auto task_context = kr::TaskContext<tyr::GroundTag>::create(kr::DomainContext::create(task->get_domain()), search_context);

    auto dl_repository = task_context->domain_context->ext_repository->get_dl_repository_ptr();
    auto repository = task_context->domain_context->ext_repository;

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

    auto variant_data = ygg::Data<kr::ps::Rule<kr::ExtFamilyTag>>(call.get_index());
    const auto variant = repository->get_or_create(variant_data).first;
    auto caller_data = make_module_data(*repository, "caller");
    caller_data.entry_memory_state = caller_entry.get_index();
    caller_data.memory_states.push_back(caller_entry.get_index());
    caller_data.memory_states.push_back(caller_return.get_index());
    auto transition = ygg::IndexList<kr::ps::Rule<kr::ExtFamilyTag>> {};
    transition.push_back(variant.get_index());
    ygg::canonicalize(transition);
    caller_data.memory_transitions.push_back(std::move(transition));
    kr::ps::ext::canonicalize(caller_data);
    const auto caller = repository->get_or_create(caller_data).first;

    const auto program = create_program(*repository, caller, { caller, callee });
    auto expander = kr::ps::ext::SuccessorExpander<tyr::GroundTag>(task_context, program);
    const auto planning_node = initial_planning_node(expander);
    const auto initial_state = expander.initial_state(planning_node.get_state());
    const auto call_steps = collect_steps(expander, initial_state);
    ASSERT_EQ(call_steps.size(), 1);
    EXPECT_EQ(call_steps.front().status, kr::ps::ext::detail::ProgramOutcome::APPLIED);
    const auto call_target = call_steps.front().get_target();
    const auto module_state = call_target.get_module_state();
    EXPECT_EQ(module_state.get_module().get_index(), callee.get_index());
    EXPECT_EQ(module_state.get_memory_state().get_index(), callee_entry.get_index());
    const auto arguments = module_state.get_arguments();
    const auto concept_arguments = arguments.get<kr::dl::ConceptTag>();
    const auto role_arguments = arguments.get<kr::dl::RoleTag>();
    const auto boolean_arguments = arguments.get<kr::dl::BooleanTag>();
    const auto numerical_arguments = arguments.get<kr::dl::NumericalTag>();
    ASSERT_EQ(concept_arguments.size(), 1);
    ASSERT_EQ(role_arguments.size(), 1);
    ASSERT_EQ(boolean_arguments.size(), 1);
    ASSERT_EQ(numerical_arguments.size(), 1);

    const auto concept_denotation = concept_arguments[0];
    const auto concept_first = concept_denotation.begin();
    ASSERT_NE(concept_first, concept_denotation.end());
    EXPECT_EQ((*concept_first).get_index(), ygg::Index<tyr::formalism::Object>(0));

    const auto role_denotation = role_arguments[0];
    const auto role_first = role_denotation.begin();
    ASSERT_NE(role_first, role_denotation.end());
    const auto role_pair = *role_first;
    EXPECT_EQ(role_pair.first.get_index(), ygg::Index<tyr::formalism::Object>(0));
    EXPECT_EQ(role_pair.second.get_index(), ygg::Index<tyr::formalism::Object>(0));

    EXPECT_TRUE(boolean_arguments[0].get());
    EXPECT_GT(numerical_arguments[0].get(), 0);

    // Contexts borrow the persistent data; preparing another context does not replace it.
    auto environment = kr::ps::ext::EvaluationEnvironment<tyr::GroundTag>(*task_context, program);
    auto evaluation_context = environment.make_dl_context(call_target);
    const auto initial_context = environment.make_dl_context(initial_state);
    EXPECT_TRUE(initial_context.arguments().template get<kr::dl::ConceptTag>().empty());
    EXPECT_EQ(&evaluation_context.arguments().get_data(), &arguments.get_data());
    EXPECT_EQ(&evaluation_context.registers().get_data(), &module_state.get_registers().get_data());
    EXPECT_EQ(evaluation_context.arguments().template get<kr::dl::ConceptTag>()[0].get_index(), concept_arguments[0].get_index());
    EXPECT_TRUE(evaluation_context.arguments().template get<kr::dl::BooleanTag>()[0].get());
    EXPECT_EQ(evaluation_context.arguments().template get<kr::dl::NumericalTag>()[0].get(), numerical_arguments[0].get());

    auto evaluation_transition = environment.make_dl_transition_context(call_target.get_state(),
                                                                        call_target.get_state(),
                                                                        arguments,
                                                                        module_state.get_registers(),
                                                                        module_state.get_registers());
    EXPECT_EQ(&evaluation_transition.get_source_context().arguments().get_data(), &arguments.get_data());
    EXPECT_EQ(&evaluation_transition.get_target_context().arguments().get_data(), &arguments.get_data());

    const auto caller_frame = call_target.get_call_stack();
    ASSERT_TRUE(caller_frame);
    const auto return_steps = collect_steps(expander, call_target);
    ASSERT_EQ(return_steps.size(), 1);
    EXPECT_EQ(return_steps.front().status, kr::ps::ext::detail::ProgramOutcome::RESTORED_CALLER);
    EXPECT_EQ(return_steps.front().get_target().get_call_stack(), caller_frame->get_caller());
    EXPECT_EQ(return_steps.front().get_target().get_module_state().get_module().get_index(), caller.get_index());
    EXPECT_EQ(return_steps.front().get_target().get_module_state().get_memory_state().get_index(), caller_return.get_index());

    const auto repeated_call_steps = collect_steps(expander, initial_state);
    ASSERT_EQ(repeated_call_steps.size(), 1);
    EXPECT_EQ(repeated_call_steps.front().get_target().get_index(), call_target.get_index());
}

TEST(RunirTests, ExtCallRuleResolvesNamedCalleeFromModuleRegistry)
{
    namespace p = tyr::planning;

    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
    auto task_context = kr::TaskContext<tyr::GroundTag>::create(kr::DomainContext::create(task->get_domain()), search_context);

    auto dl_repository = task_context->domain_context->ext_repository->get_dl_repository_ptr();
    auto repository = task_context->domain_context->ext_repository;

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

    auto variant_data = ygg::Data<kr::ps::Rule<kr::ExtFamilyTag>>(call.get_index());
    const auto variant = repository->get_or_create(variant_data).first;
    auto caller_data = make_module_data(*repository, "caller");
    caller_data.entry_memory_state = caller_entry.get_index();
    caller_data.memory_states.push_back(caller_entry.get_index());
    caller_data.memory_states.push_back(caller_return.get_index());
    auto transition = ygg::IndexList<kr::ps::Rule<kr::ExtFamilyTag>> {};
    transition.push_back(variant.get_index());
    ygg::canonicalize(transition);
    caller_data.memory_transitions.push_back(std::move(transition));
    kr::ps::ext::canonicalize(caller_data);
    const auto caller = repository->get_or_create(caller_data).first;

    const auto program = create_program(*repository, caller, { caller, callee });
    auto expander = kr::ps::ext::SuccessorExpander<tyr::GroundTag>(task_context, program);
    const auto planning_node = initial_planning_node(expander);
    const auto steps = collect_steps(expander, expander.initial_state(planning_node.get_state()));
    ASSERT_EQ(steps.size(), 1);
    EXPECT_EQ(steps.front().status, kr::ps::ext::detail::ProgramOutcome::APPLIED);
    EXPECT_EQ(steps.front().get_target().get_module_state().get_module().get_index(), callee.get_index());
    EXPECT_EQ(steps.front().get_target().get_module_state().get_memory_state().get_index(), callee_entry.get_index());
}

TEST(RunirTests, ExtDoRuleAppliesMatchingActionAndAdvancesMemory)
{
    namespace p = tyr::planning;

    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
    auto task_context = kr::TaskContext<tyr::GroundTag>::create(kr::DomainContext::create(task->get_domain()), search_context);

    auto dl_repository = task_context->domain_context->ext_repository->get_dl_repository_ptr();
    auto repository = task_context->domain_context->ext_repository;

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
    auto variant_data = ygg::Data<kr::ps::Rule<kr::ExtFamilyTag>>(rule.get_index());
    const auto variant = repository->get_or_create(variant_data).first;

    auto module_data = make_module_data(*repository, "module");
    module_data.entry_memory_state = source.get_index();
    module_data.memory_states.push_back(source.get_index());
    module_data.memory_states.push_back(target.get_index());
    auto transition = ygg::IndexList<kr::ps::Rule<kr::ExtFamilyTag>> {};
    transition.push_back(variant.get_index());
    ygg::canonicalize(transition);
    module_data.memory_transitions.push_back(std::move(transition));
    kr::ps::ext::canonicalize(module_data);
    const auto module_ = repository->get_or_create(module_data).first;
    const auto program = create_program(*repository, module_, { module_ });
    auto expander = kr::ps::ext::SuccessorExpander<tyr::GroundTag>(task_context, program);
    const auto planning_node = initial_planning_node(expander);
    const auto initial_state = expander.initial_state(planning_node.get_state());
    const auto steps = collect_steps(expander, initial_state);
    ASSERT_GT(steps.size(), 1);

    for (const auto& step : steps)
    {
        EXPECT_EQ(step.status, kr::ps::ext::detail::ProgramOutcome::APPLIED);
        ASSERT_TRUE(step.planning_successor.has_value());
        EXPECT_EQ(step.planning_successor->label.get_relation().get_name(), "pick");
        EXPECT_EQ(step.get_target().get_module_state().get_memory_state().get_index(), target.get_index());
        EXPECT_NE(step.get_target().get_state().get_index(), initial_state.get_state().get_index());
    }

    auto greedy_options = kr::ps::ext::ProgramSearchOptions<tyr::GroundTag> {};
    const auto greedy = kr::ps::ext::find_solution(task_context, program, greedy_options);
    auto universal_options = kr::ps::ext::ProgramSearchOptions<tyr::GroundTag> {};
    universal_options.universal = true;
    const auto universal = kr::ps::ext::find_solution(task_context, program, universal_options);
    ASSERT_TRUE(greedy.graph);
    ASSERT_TRUE(universal.graph);
    ASSERT_EQ(greedy.graph->get_out_degree(0), 1);
    EXPECT_EQ(universal.graph->get_out_degree(0), steps.size());

    const auto edge = greedy.graph->get_out_edge_indices(0).front();
    const auto selected = greedy.graph->get_vertex(greedy.graph->get_target(edge)).get_property().program_state;
    EXPECT_EQ(selected.get_state().get_index(), steps.front().get_target().get_state().get_index());
    EXPECT_EQ(selected.get_module_state().get_memory_state().get_index(), steps.front().get_target().get_module_state().get_memory_state().get_index());
}

TEST(RunirTests, ExtDoRuleRejectsActionWithIncompatibleDeclaredEffects)
{
    namespace p = tyr::planning;

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
    EXPECT_EQ(steps.front().status, kr::ps::ext::detail::ProgramOutcome::NO_APPLICABLE_ACTION);
    EXPECT_EQ(steps.front().get_target().get_state().get_index(), initial_state.get_state().get_index());

    auto options = kr::ps::ext::ProgramSearchOptions<tyr::GroundTag> {};
    options.universal = true;
    const auto result = kr::ps::ext::find_solution(task_context, program, options);
    EXPECT_EQ(result.status, kr::ps::ext::ProgramProofStatus::FAILURE);
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
    auto task_context = kr::TaskContext<tyr::GroundTag>::create(kr::DomainContext::create(task->get_domain()), search_context);

    auto dl_repository = task_context->domain_context->ext_repository->get_dl_repository_ptr();
    auto repository = task_context->domain_context->ext_repository;

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
    auto move_variant_data = ygg::Data<kr::ps::Rule<kr::ExtFamilyTag>>(move_rule.get_index());
    const auto move_variant = repository->get_or_create(move_variant_data).first;

    auto pick_data = ygg::Data<kr::ps::ext::Rule<kr::ps::ext::DoTag>>(std::string("pick"));
    pick_data.source = source.get_index();
    pick_data.target = pick_target.get_index();
    pick_data.arguments.push_back(ball_feature.get_index());
    pick_data.arguments.push_back(room_feature.get_index());
    pick_data.arguments.push_back(gripper_feature.get_index());
    kr::ps::ext::canonicalize(pick_data);
    const auto pick_rule = repository->get_or_create(pick_data).first;
    auto pick_variant_data = ygg::Data<kr::ps::Rule<kr::ExtFamilyTag>>(pick_rule.get_index());
    const auto pick_variant = repository->get_or_create(pick_variant_data).first;

    auto module_data = make_module_data(*repository, "module");
    module_data.entry_memory_state = source.get_index();
    module_data.memory_states.push_back(source.get_index());
    module_data.memory_states.push_back(move_target.get_index());
    module_data.memory_states.push_back(pick_target.get_index());

    auto move_transition = ygg::IndexList<kr::ps::Rule<kr::ExtFamilyTag>>();
    move_transition.push_back(move_variant.get_index());
    ygg::canonicalize(move_transition);
    module_data.memory_transitions.push_back(std::move(move_transition));

    auto pick_transition = ygg::IndexList<kr::ps::Rule<kr::ExtFamilyTag>>();
    pick_transition.push_back(pick_variant.get_index());
    ygg::canonicalize(pick_transition);
    module_data.memory_transitions.push_back(std::move(pick_transition));

    kr::ps::ext::canonicalize(module_data);
    const auto module_ = repository->get_or_create(module_data).first;

    const auto program = create_program(*repository, module_, { module_ });
    auto expander = kr::ps::ext::SuccessorExpander<tyr::GroundTag>(task_context, program);
    const auto planning_node = initial_planning_node(expander);
    const auto steps = collect_steps(expander, expander.initial_state(planning_node.get_state()));
    ASSERT_GT(steps.size(), 2);
    EXPECT_EQ(steps.front().status, kr::ps::ext::detail::ProgramOutcome::APPLIED);
    EXPECT_EQ(steps.front().get_target().get_module_state().get_memory_state().get_index(), move_target.get_index());

    auto reached_move_target = false;
    auto reached_pick_target = false;
    for (const auto& step : steps)
    {
        ASSERT_TRUE(step.planning_successor.has_value());
        reached_move_target |= step.get_target().get_module_state().get_memory_state().get_index() == move_target.get_index();
        reached_pick_target |= step.get_target().get_module_state().get_memory_state().get_index() == pick_target.get_index();
    }
    EXPECT_TRUE(reached_move_target);
    EXPECT_TRUE(reached_pick_target);

    auto greedy_options = kr::ps::ext::ProgramSearchOptions<tyr::GroundTag> {};
    const auto greedy = kr::ps::ext::find_solution(task_context, program, greedy_options);
    auto universal_options = kr::ps::ext::ProgramSearchOptions<tyr::GroundTag> {};
    universal_options.universal = true;
    const auto universal = kr::ps::ext::find_solution(task_context, program, universal_options);
    ASSERT_TRUE(greedy.graph);
    ASSERT_TRUE(universal.graph);
    ASSERT_EQ(greedy.graph->get_out_degree(0), 1);
    EXPECT_EQ(universal.graph->get_out_degree(0), steps.size());
}

namespace
{

template<tyr::TaskKind Kind>
void expect_query_action_contracts()
{
    namespace ext = kr::ps::ext;
    const auto valid_query = std::string(R"((q_join (q_atomic_state "edge" (from to)) (q_atomic_state "at" (from))))");
    for (const auto scenario : { 0, 1, 2, 3, 4 })
    {
        SCOPED_TRACE(fmt::format("query action scenario={}", scenario));
        const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../../fixtures/kr/ps/ext/choose";
        const auto task_context = create_task_context<Kind>(directory / "domain.pddl", directory / "task.pddl");
        auto& repository = *task_context->domain_context->ext_repository;
        const auto query = scenario == 1 ? "(q_difference " + valid_query + " " + valid_query + ")" :
                           scenario == 3 ? "(q_union " + valid_query + R"( (q_atomic_state "edge" (from to))))" :
                           scenario == 4 ? "(q_project (to from) " + valid_query + ")" :
                                           valid_query;
        const auto source = fmt::format(R"(
(:module (:symbol actions) (:arguments) (:registers)
  (:entry source) (:memory source target)
  (:features
    (:query (:symbol Moves) (:expression {0}))
    (:numerical (:symbol N) (:expression (n_count (c_atomic_state "at")))))
  (:rules (:rule (:symbol move) (:expression
    (:source-memory source) (:target-memory target)
    (:action (:conditions) (:action "move") (:query Moves) (:effects ({1} N)))))))
)",
                                        query,
                                        scenario == 2 ? "decreases" : "unchanged");
        const auto module_ = ext::dl::parse_module(source, task_context->search_context->task->get_domain().get_domain(), repository);
        const auto program = create_program(repository, module_, { module_ });
        auto expander = ext::SuccessorExpander<Kind>(task_context, program);
        const auto planning_node = initial_planning_node(expander);
        const auto initial = expander.initial_state(planning_node.get_state());
        const auto rule = module_.get_memory_transitions()[0][0];
        auto lazy = std::vector<typename ext::SuccessorExpander<Kind>::Step> {};
        if (scenario == 2 || scenario == 4)
        {
            EXPECT_THROW(collect_steps(expander, initial), ext::ActionRuleContractError);
            EXPECT_THROW(lazy = collect_steps(expander, initial, true), ext::ActionRuleContractError);
            if (scenario == 2)
            {
                auto& search = *task_context->search_context;
                const auto successors =
                    search.successor_generator->get_labeled_successor_nodes(planning_node, *search.state_repository, *search.axiom_evaluator);
                ASSERT_FALSE(successors.empty());
                const auto& candidate = successors.front();
                EXPECT_THROW(expander.matching_rule(initial, candidate), ext::ActionRuleContractError);
                EXPECT_THROW(expander.apply(initial, rule, candidate), ext::ActionRuleContractError);
            }
            continue;
        }
        if (scenario == 3)
        {
            // Only the first two rows are applicable; the third is unvisited by
            // greedy execution and remains a contract violation exhaustively.
            lazy = collect_steps(expander, initial, true);
            ASSERT_EQ(lazy.size(), 1);
            EXPECT_EQ(lazy.front().status, ext::detail::ProgramOutcome::APPLIED);
            auto options = ext::ProgramSearchOptions<Kind> {};
            EXPECT_NO_THROW(ext::find_solution(task_context, program, options));
            options.universal = true;
            EXPECT_THROW(ext::find_solution(task_context, program, options), ext::ActionRuleContractError);
            EXPECT_THROW(collect_steps(expander, initial), ext::ActionRuleContractError);
            continue;
        }
        const auto steps = collect_steps(expander, initial);
        if (scenario == 1)
        {
            ASSERT_EQ(steps.size(), 1);
            EXPECT_EQ(steps.front().status, ext::detail::ProgramOutcome::NO_APPLICABLE_ACTION);
            EXPECT_EQ(task_context->search_context->state_repository->num_states(), 1);
            continue;
        }
        ASSERT_EQ(steps.size(), 2);
        for (const auto& step : steps)
        {
            ASSERT_TRUE(step.planning_successor.has_value());
            const auto candidate = step.planning_successor->unpack();
            EXPECT_EQ(candidate.label.get_objects()[0].get_name(), "start");
            EXPECT_EQ(step.get_target().get_module_state().get_memory_state().get_name(), "target");
            const auto matching = expander.matching_rule(initial, candidate);
            ASSERT_TRUE(matching);
            EXPECT_EQ(matching->get_index(), rule.get_index());
            const auto applied = expander.apply(initial, rule, candidate);
            ASSERT_TRUE(applied);
            EXPECT_EQ(applied->get_target().get_index(), step.get_target().get_index());
        }
        EXPECT_FALSE(expander.apply(initial, rule));
    }
}

template<tyr::TaskKind Kind>
void expect_callback_selection_stops_after_selected_rule()
{
    namespace ext = kr::ps::ext;
    for (const auto choose : { false, true })
    {
        const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../../fixtures/kr/ps/ext/choose";
        const auto task_context = create_task_context<Kind>(directory / "domain.pddl", directory / "task.pddl");
        auto& repository = *task_context->domain_context->ext_repository;
        const auto source = fmt::format(R"(
(:module (:symbol selection) (:arguments) (:registers (:concept r))
  (:entry source) (:memory source selected invalid)
  (:features
    (:concept (:symbol Candidates) (:expression (c_atomic_state "candidate")))
    (:query (:symbol Invalid) (:expression (q_atomic_state "edge" (from to)))))
  (:rules
    (:rule (:symbol first) (:expression (:source-memory source) (:target-memory selected)
      (:{0} (:conditions) (:concept Candidates) (:register (:concept r)))))
    (:rule (:symbol later) (:expression (:source-memory source) (:target-memory invalid)
      (:action (:conditions) (:action "move") (:query Invalid) (:effects))))))
)",
                                        choose ? "choose" : "load");
        const auto module_ = ext::dl::parse_module(source, task_context->search_context->task->get_domain().get_domain(), repository);
        const auto program = create_program(repository, module_, { module_ });
        auto expander = ext::SuccessorExpander<Kind>(task_context, program);
        const auto planning_node = initial_planning_node(expander);
        const auto initial = expander.initial_state(planning_node.get_state());
        const auto steps = collect_steps(expander, initial, true);
        ASSERT_EQ(steps.size(), choose ? 2 : 1);
        for (const auto& step : steps)
            EXPECT_EQ(step.get_target().get_module_state().get_memory_state().get_name(), "selected");
        EXPECT_EQ(task_context->search_context->state_repository->num_states(), 1);
        EXPECT_THROW(collect_steps(expander, initial), ext::ActionRuleContractError);
    }
}

template<tyr::TaskKind Kind>
void expect_callback_sketch_order_and_cancellation()
{
    namespace ext = kr::ps::ext;
    const auto source = std::string(R"(
(:module (:symbol order) (:arguments) (:registers)
  (:entry source) (:memory source impossible picked)
  (:features
    (:concept (:symbol Ball) (:expression (c_atomic_state "ball")))
    (:concept (:symbol Room) (:expression (c_atomic_state "room")))
    (:concept (:symbol Gripper) (:expression (c_atomic_state "gripper")))
    (:numerical (:symbol Free) (:expression (n_count (c_atomic_state "free")))))
  (:rules
    (:rule (:symbol rejected) (:expression (:source-memory source) (:target-memory impossible)
      (:sketch (:conditions) (:effects (increases Free)))))
    (:rule (:symbol next) (:expression (:source-memory source) (:target-memory picked)
      (:do (:conditions) (:action "pick") (:arguments Ball Room Gripper) (:effects (decreases Free)))))))
)");
    for (const auto cancelled : { true, false })
    {
        const auto task_context =
            create_task_context<Kind>(benchmark_path("classical/tests/gripper/domain.pddl"), benchmark_path("classical/tests/gripper/test-1.pddl"));
        auto& repository = *task_context->domain_context->ext_repository;
        auto& states = *task_context->search_context->state_repository;
        const auto module_ = ext::dl::parse_module(source, task_context->search_context->task->get_domain().get_domain(), repository);
        const auto program = create_program(repository, module_, { module_ });
        auto expander = ext::SuccessorExpander<Kind>(task_context, program);
        const auto planning_node = initial_planning_node(expander);
        const auto initial = expander.initial_state(planning_node.get_state());
        using Step = typename ext::SuccessorExpander<Kind>::Step;
        auto selected = std::vector<Step> {};
        auto statistics = ext::ProgramSearchStatistics {};
        const auto emit = [&](const auto& expansion)
        {
            EXPECT_GT(statistics.num_generated, 0);
            selected.push_back(std::get<Step>(expansion));
            return false;
        };
        EXPECT_FALSE(expander.for_each_successor(initial, statistics, emit, [] { return true; }));
        EXPECT_TRUE(selected.empty());
        EXPECT_EQ(statistics.num_generated, 0);
        EXPECT_EQ(states.num_states(), 1);
        const auto stop = [&] { return cancelled && states.num_states() > 1; };
        EXPECT_FALSE(expander.for_each_successor(initial, statistics, emit, stop));
        if (cancelled)
        {
            EXPECT_TRUE(selected.empty());
            EXPECT_EQ(states.num_states(), 2);
            continue;
        }
        auto complete = std::vector<Step> {};
        EXPECT_TRUE(expander.for_each_successor(
            initial,
            statistics,
            [&](const auto& expansion)
            {
                complete.push_back(std::get<Step>(expansion));
                return true;
            },
            [] { return false; }));
        ASSERT_GT(complete.size(), 1);
        ASSERT_EQ(selected.size(), 1);
        EXPECT_EQ(selected.front().get_target().get_index(), complete.front().get_target().get_index());
        EXPECT_EQ(selected.front().planning_successor->label, complete.front().planning_successor->label);
        EXPECT_EQ(statistics.num_expanded, 0);
    }
}

template<tyr::TaskKind Kind>
void expect_query_action_existential_binding()
{
    namespace ext = kr::ps::ext;
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../../fixtures/kr/ps/ext/action_existential";
    const auto task_context = create_task_context<Kind>(directory / "domain.pddl", directory / "task.pddl");
    auto& repository = *task_context->domain_context->ext_repository;
    const auto domain = task_context->search_context->task->get_domain().get_domain();
    ASSERT_EQ(domain.get_actions().size(), 1);
    const auto action = domain.get_actions()[0];
    ASSERT_EQ(action.get_arity(), 2);
    const auto module_ = ext::dl::parse_module(R"(
(:module (:symbol witness) (:arguments) (:registers)
  (:entry source) (:memory source target)
  (:features (:query (:symbol Bindings) (:expression (q_atomic_state "supports" (x witness)))))
  (:rules (:rule (:symbol finish) (:expression
    (:source-memory source) (:target-memory target)
    (:action (:conditions) (:action "finish") (:query Bindings) (:effects))))))
)",
                                              domain,
                                              repository);
    const auto program = create_program(repository, module_, { module_ });
    auto expander = ext::SuccessorExpander<Kind>(task_context, program);
    const auto planning_node = initial_planning_node(expander);
    const auto steps = collect_steps(expander, expander.initial_state(planning_node.get_state()));
    ASSERT_EQ(steps.size(), 1);
    ASSERT_TRUE(steps.front().planning_successor.has_value());
    const auto objects = steps.front().planning_successor->label.get_objects();
    ASSERT_EQ(objects.size(), 2);
    EXPECT_EQ(objects[0].get_name(), "start");
    EXPECT_EQ(objects[1].get_name(), "proof");
    EXPECT_TRUE(is_planning_goal(expander, steps.front().get_target().get_state()));
    EXPECT_TRUE(ext::find_solution(task_context, program, ext::ProgramSearchOptions<Kind> {}).is_successful());
}

template<tyr::TaskKind Kind>
void expect_query_action_nullary_binding()
{
    namespace ext = kr::ps::ext;
    for (const auto empty : { false, true })
    {
        const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../../fixtures/kr/ps/ext/action_nullary";
        const auto task_context = create_task_context<Kind>(directory / "domain.pddl", directory / "task.pddl");
        auto& repository = *task_context->domain_context->ext_repository;
        const auto truth = std::string(R"((q_atomic_state "truth" ()))");
        const auto query = empty ? "(q_difference " + truth + " " + truth + ")" : truth;
        const auto source = fmt::format(R"(
(:module (:symbol nullary) (:arguments) (:registers)
  (:entry source) (:memory source)
  (:features (:query (:symbol Bindings) (:expression {0})))
  (:rules (:rule (:symbol finish) (:expression
    (:source-memory source) (:target-memory source)
    (:action (:conditions) (:action "finish") (:query Bindings) (:effects))))))
)",
                                        query);
        const auto module_ = ext::dl::parse_module(source, task_context->search_context->task->get_domain().get_domain(), repository);
        const auto program = create_program(repository, module_, { module_ });
        auto expander = ext::SuccessorExpander<Kind>(task_context, program);
        const auto planning_node = initial_planning_node(expander);
        const auto initial = expander.initial_state(planning_node.get_state());
        const auto steps = collect_steps(expander, initial);
        ASSERT_EQ(steps.size(), 1);
        if (empty)
        {
            EXPECT_EQ(steps.front().status, ext::detail::ProgramOutcome::NO_APPLICABLE_ACTION);
            EXPECT_EQ(task_context->search_context->state_repository->num_states(), 1);
        }
        else
        {
            EXPECT_EQ(steps.front().status, ext::detail::ProgramOutcome::APPLIED);
            ASSERT_TRUE(steps.front().planning_successor.has_value());
            EXPECT_TRUE(steps.front().planning_successor->label.get_objects().empty());
            EXPECT_TRUE(is_planning_goal(expander, steps.front().get_target().get_state()));
            EXPECT_THROW(collect_steps(expander, steps.front().get_target()), ext::ActionRuleContractError);
            EXPECT_TRUE(ext::find_solution(task_context, program, ext::ProgramSearchOptions<Kind> {}).is_successful());
        }
    }
}

template<tyr::TaskKind Kind>
void expect_callback_expansion_counts()
{
    namespace ext = kr::ps::ext;
    for (const auto rule_kind : { "load", "choose", "do", "sketch" })
    {
        SCOPED_TRACE(rule_kind);
        const auto binding = std::string(rule_kind) == "load" || std::string(rule_kind) == "choose";
        const auto body = binding ? fmt::format("(:{} (:conditions) (:concept Ball) (:register (:concept r)))", rule_kind) :
                          std::string(rule_kind) == "do" ?
                                    R"((:do (:conditions) (:action "pick") (:arguments Ball Room Gripper) (:effects (decreases Free))))" :
                                    R"((:sketch (:conditions) (:effects (decreases Free))))";
        const auto source = fmt::format(R"(
(:module (:symbol selection) (:arguments) (:registers (:concept r))
  (:entry source) (:memory source target)
  (:features
    (:concept (:symbol Ball) (:expression (c_atomic_state "ball")))
    (:concept (:symbol Room) (:expression (c_atomic_state "room")))
    (:concept (:symbol Gripper) (:expression (c_atomic_state "gripper")))
    (:numerical (:symbol Free) (:expression (n_count (c_atomic_state "free")))))
  (:rules (:rule (:symbol select) (:expression (:source-memory source) (:target-memory target) {0}))))
)",
                                        body);
        auto counts = std::vector<size_t> {};
        auto state_counts = std::vector<size_t> {};
        auto binding_counts = std::vector<size_t> {};
        auto first_bindings = std::vector<std::vector<std::string>> {};
        for (const auto complete : { true, false })
        {
            const auto task_context =
                create_task_context<Kind>(benchmark_path("classical/tests/gripper/domain.pddl"), benchmark_path("classical/tests/gripper/test-1.pddl"));
            auto& repository = *task_context->domain_context->ext_repository;
            const auto module_ = ext::dl::parse_module(source, task_context->search_context->task->get_domain().get_domain(), repository);
            const auto program = create_program(repository, module_, { module_ });
            auto expander = ext::SuccessorExpander<Kind>(task_context, program);
            const auto planning_node = initial_planning_node(expander);
            const auto initial = expander.initial_state(planning_node.get_state());
            const auto steps = collect_steps(expander, initial, !complete);
            counts.push_back(steps.size());
            state_counts.push_back(task_context->search_context->state_repository->num_states() - 1);
            if constexpr (std::same_as<Kind, tyr::LiftedTag>)
            {
                const auto& task = task_context->search_context->task;
                size_t count = 0;
                for (const auto action : task->get_domain().get_domain().get_actions())
                    count += task->get_repository()->size(action.get_index());
                binding_counts.push_back(count);
            }
            ASSERT_FALSE(steps.empty());
            EXPECT_EQ(steps.front().status, ext::detail::ProgramOutcome::APPLIED);
            auto first = std::vector<std::string> {};
            if (binding)
                first.push_back(std::string(steps.front().get_target().get_module_state().get_registers().get_concept_values()[0].value().get_name().str()));
            else
                for (const auto object : steps.front().planning_successor->label.get_objects())
                    first.push_back(std::string(object.get_name().str()));
            first_bindings.push_back(std::move(first));
        }
        EXPECT_EQ(first_bindings[0], first_bindings[1]);
        EXPECT_GT(counts[0], 1);
        EXPECT_EQ(counts[1], std::string(rule_kind) == "choose" ? counts[0] : 1);
        if (binding)
        {
            EXPECT_EQ(state_counts[0], 0);
            EXPECT_EQ(state_counts[1], 0);
        }
        else
        {
            EXPECT_LT(state_counts[1], state_counts[0]);
            if constexpr (std::same_as<Kind, tyr::LiftedTag>)
            {
                EXPECT_LT(binding_counts[1], binding_counts[0]);
            }
            if (std::string(rule_kind) == "do")
            {
                EXPECT_EQ(state_counts[0], 4);
                EXPECT_EQ(state_counts[1], 1);
                if constexpr (std::same_as<Kind, tyr::LiftedTag>)
                {
                    EXPECT_EQ(binding_counts[0], 4);
                    EXPECT_EQ(binding_counts[1], 1);
                }
            }
        }
    }
}

}  // namespace

TEST(RunirTests, ExtQueryActionContractsGround) { expect_query_action_contracts<tyr::GroundTag>(); }
TEST(RunirTests, ExtQueryActionContractsLifted) { expect_query_action_contracts<tyr::LiftedTag>(); }
TEST(RunirTests, ExtCallbackSelectionStopsAfterSelectedRuleGround) { expect_callback_selection_stops_after_selected_rule<tyr::GroundTag>(); }
TEST(RunirTests, ExtCallbackSelectionStopsAfterSelectedRuleLifted) { expect_callback_selection_stops_after_selected_rule<tyr::LiftedTag>(); }
TEST(RunirTests, ExtCallbackSketchOrderAndCancellationGround) { expect_callback_sketch_order_and_cancellation<tyr::GroundTag>(); }
TEST(RunirTests, ExtCallbackSketchOrderAndCancellationLifted) { expect_callback_sketch_order_and_cancellation<tyr::LiftedTag>(); }
TEST(RunirTests, ExtQueryActionExistentialBindingGround) { expect_query_action_existential_binding<tyr::GroundTag>(); }
TEST(RunirTests, ExtQueryActionExistentialBindingLifted) { expect_query_action_existential_binding<tyr::LiftedTag>(); }
TEST(RunirTests, ExtQueryActionNullaryBindingGround) { expect_query_action_nullary_binding<tyr::GroundTag>(); }
TEST(RunirTests, ExtQueryActionNullaryBindingLifted) { expect_query_action_nullary_binding<tyr::LiftedTag>(); }
TEST(RunirTests, ExtCallbackExpansionCountsGround) { expect_callback_expansion_counts<tyr::GroundTag>(); }
TEST(RunirTests, ExtCallbackExpansionCountsLifted) { expect_callback_expansion_counts<tyr::LiftedTag>(); }

}  // namespace runir::tests
