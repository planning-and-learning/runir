#ifndef RUNIR_TESTS_EXT_SUCCESSOR_EXPANDER_FIXTURES_HPP_
#define RUNIR_TESTS_EXT_SUCCESSOR_EXPANDER_FIXTURES_HPP_

#include "fixtures.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include "module_fixtures.hpp"
#include "planning_fixtures.hpp"
#include "successor_fixtures.hpp"

#include <algorithm>
#include <array>
#include <concepts>
#include <filesystem>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <tyr/datalog/lifted/database/syntax/query_repository.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/ext/detail/rule_evaluation/context.hpp>
#include <runir/kr/ps/ext/detail/successor_expander_impl.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <runir/kr/ps/execution_repository.hpp>
#include <runir/kr/ps/ext/execution_data.hpp>
#include <runir/kr/ps/ext/execution_storage.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/program_executor.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/rule_evaluator_concepts.hpp>
#include <runir/kr/task_context.hpp>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <tyr/planning/ground/successor_generator.hpp>
#include <tyr/planning/lifted/successor_generator.hpp>
#include <utility>

namespace runir::tests
{

inline auto create_register(kr::ps::ext::Repository& repository, const std::string& name, ygg::uint_t identifier)
{
    auto data = urd::data<kr::dl::Register<kr::dl::ConceptTag>>(name, kr::dl::RegisterIdentifier<kr::dl::ConceptTag>(identifier));
    return repository.get_dl_repository().insert(data).first;
}

template<typename FeatureTag, kr::dl::CategoryTag Category>
auto create_feature(kr::ps::ext::Repository& repository, urd::index<kr::dl::Constructor<Category>> expression, const std::string& name)
{
    auto concrete_data = urd::data<kr::ps::ConcreteFeature<kr::ExtFamilyTag, kr::DlTag, FeatureTag>>(expression, name);
    const auto concrete = repository.insert(concrete_data).first;
    auto feature_data = urd::data<kr::ps::Feature<kr::ExtFamilyTag, FeatureTag>>(concrete.index());
    return repository.insert(feature_data).first;
}

inline auto create_concept_feature(kr::ps::ext::Repository& repository, urd::index<kr::dl::Constructor<kr::dl::ConceptTag>> expression, const std::string& name)
{ return create_feature<kr::dl::ConceptTag>(repository, expression, name); }

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
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../../fixtures/kr/ps/ext/choose";
    const auto context = create_task_context<Kind>(directory / "domain.pddl", directory / "task.pddl");
    const auto program = kr::ps::ext::dl::parse_program(R"(
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
    auto expander = kr::ps::ext::SuccessorExpander<Kind>(context, program);
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
        ASSERT_NE(moved.get_state().index(), initial.get_state().index());
        EXPECT_EQ(move.planning_successor->node.get_state(), moved.get_state().pack());
        const auto returns = collect_steps(expander, moved);
        ASSERT_EQ(returns.size(), 1);
        EXPECT_EQ(returns.front().status, kr::ps::ProgramOutcome::RESTORED_CALLER);
        EXPECT_FALSE(returns.front().planning_successor);
        const auto returned = returns.front().get_target();
        EXPECT_EQ(returned.get_state().index(), moved.get_state().index());
        EXPECT_EQ(returned.get_module_state().get_module(), suspended_callee->get_module());
        EXPECT_EQ(returned.get_module_state().get_memory_state(), suspended_callee->get_return_memory_state());
        EXPECT_EQ(returned.get_module_state().get_registers(), suspended_callee->get_registers());
        EXPECT_EQ(returned.get_module_state().get_arguments(), callee.get_module_state().get_arguments());
        EXPECT_EQ(returned.get_call_stack(), suspended);

        const auto outer_returns = collect_steps(expander, returned);
        ASSERT_EQ(outer_returns.size(), 1);
        EXPECT_EQ(outer_returns.front().status, kr::ps::ProgramOutcome::RESTORED_CALLER);
        EXPECT_FALSE(outer_returns.front().planning_successor);
        const auto caller = outer_returns.front().get_target();
        EXPECT_EQ(caller.get_state().index(), moved.get_state().index());
        EXPECT_EQ(caller.get_module_state().get_module(), suspended->get_module());
        EXPECT_EQ(caller.get_module_state().get_memory_state(), suspended->get_return_memory_state());
        EXPECT_EQ(caller.get_module_state().get_registers(), suspended->get_registers());
        EXPECT_EQ(caller.get_module_state().get_arguments(), suspended->get_arguments());
        EXPECT_FALSE(caller.get_call_stack());
    }
    EXPECT_EQ(callee.get_state().index(), initial.get_state().index());
}

template<tyr::TaskKind Kind>
void expect_borrowed_query_evaluation()
{
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../../fixtures/kr/ps/ext/choose";
    const auto task_context = create_task_context<Kind>(directory / "domain.pddl", directory / "task.pddl");
    auto& repository = *task_context->domain_context->ext_repository;
    auto& search = *task_context->search_context;
    const auto module_ = kr::ps::ext::dl::parse_module(
        R"((:module (:symbol queries) (:arguments) (:registers)
            (:entry source) (:memory source)
            (:features (:query (:symbol At) (:expression (q_rename (location) (q_atomic_state "at" (x))))))
            (:rules)))",
        search.task->get_domain().get_domain(),
        repository);
    const auto program = create_program(repository, module_, { module_ });
    const auto initial = search.successor_generator->get_initial_node(*search.state_repository, *search.axiom_evaluator);
    auto expander = kr::ps::ext::SuccessorExpander<Kind>(task_context, program);
    const auto planning_node = initial_planning_node(expander);
    const auto state = expander.initial_state(planning_node.get_state());
    auto environment = kr::ps::ext::EvaluationEnvironment<Kind, kr::dl::semantics::FullEvaluationPolicy<Kind>>(*task_context, program);
    auto state_context = environment.make_dl_context(state);
    const auto feature = module_.get_query_features()[0];
    const auto query = feature.get_expression();
    {
        const auto rows = kr::ps::evaluate<Kind>(feature, state_context);
        ASSERT_EQ(rows.size(), 1);
        EXPECT_EQ(kr::ps::evaluate<Kind>(feature, state_context).data().row_set, rows.data().row_set);
        const auto& cached = environment.get_dl_caches().get_queries(false).at(query);
        EXPECT_EQ(rows.data().row_set, cached.data().row_set);
        EXPECT_EQ(rows.columns().data(), cached.columns().data());
        EXPECT_TRUE(std::ranges::equal(rows.columns(), query.get_schema()));

        const auto successors = search.successor_generator->get_labeled_successor_nodes(initial, *search.state_repository, *search.axiom_evaluator);
        ASSERT_FALSE(successors.empty());
        const auto stack = state.get_module_state();
        auto transition = environment.make_dl_transition_context(state.get_state(),
                                                                 successors.front().node.get_state(),
                                                                 stack.get_arguments(),
                                                                 stack.get_registers(),
                                                                 stack.get_registers());
        {
            const auto target_rows = kr::ps::evaluate<Kind>(feature, transition.get_target_context());
            ASSERT_EQ(target_rows.size(), 1);
            EXPECT_NE(urd::index<tyr::formalism::Object>(ygg::decode_value<ygg::uint_t>(target_rows.row(0).value(0))),
                      urd::index<tyr::formalism::Object>(ygg::decode_value<ygg::uint_t>(rows.row(0).value(0))));
            EXPECT_NE(target_rows.relation_identity(), rows.relation_identity());  // independent caches: other layers

            // The indexed program state supplies bindings; its prepared planning view may use builder storage.
            const auto source_builder = urd::builder<tyr::planning::State<Kind>> { planning_node.get_state().data() };
            const auto target_builder = urd::builder<tyr::planning::State<Kind>> { successors.front().node.get_state().data() };
            const auto prepared_source = urd::resolve(source_builder, *search.state_repository);
            const auto prepared_target = urd::resolve(target_builder, *search.state_repository);
            auto storage = kr::ps::ext::InternedExecutionStorage<Kind>(task_context, program);
            auto evaluators = kr::ps::ext::detail::RuleEvaluators<Kind, kr::dl::semantics::FullEvaluationPolicy<Kind>>(task_context, program);
            auto context = evaluators.make_context(storage, prepared_source);
            auto prepared_context = context.make_dl_context(state);
            static_assert(std::same_as<std::remove_cvref_t<decltype(prepared_context.get_state())>, tyr::planning::BuilderStateView<Kind>>);
            const auto prepared_rows = kr::ps::evaluate<Kind>(feature, prepared_context);
            ASSERT_EQ(prepared_rows.size(), rows.size());
            EXPECT_EQ(urd::index<tyr::formalism::Object>(ygg::decode_value<ygg::uint_t>(prepared_rows.row(0).value(0))),
                      urd::index<tyr::formalism::Object>(ygg::decode_value<ygg::uint_t>(rows.row(0).value(0))));
            const auto candidate = tyr::planning::LabeledNode<Kind, tyr::planning::BuilderStateView<Kind>> {
                successors.front().label,
                tyr::planning::Node<Kind, tyr::planning::BuilderStateView<Kind>>(prepared_target, successors.front().node.get_metric())
            };
            auto prepared_transition = context.make_dl_transition_context(state, candidate);
            const auto prepared_target_rows = kr::ps::evaluate<Kind>(feature, prepared_transition.get_target_context());
            ASSERT_EQ(prepared_target_rows.size(), target_rows.size());
            EXPECT_EQ(urd::index<tyr::formalism::Object>(ygg::decode_value<ygg::uint_t>(prepared_target_rows.row(0).value(0))),
                      urd::index<tyr::formalism::Object>(ygg::decode_value<ygg::uint_t>(target_rows.row(0).value(0))));
            EXPECT_EQ(urd::index<tyr::formalism::Object>(
                          ygg::decode_value<ygg::uint_t>(kr::ps::evaluate<Kind>(feature, prepared_transition.get_source_context()).row(0).value(0))),
                      urd::index<tyr::formalism::Object>(ygg::decode_value<ygg::uint_t>(rows.row(0).value(0))));
        }
        EXPECT_TRUE(environment.get_dl_target_caches().get_queries(false).contains(query));
        environment.reset_target();
        EXPECT_TRUE(environment.get_dl_caches().get_queries(false).contains(query));
        EXPECT_EQ(rows.size(), 1);
        EXPECT_EQ(kr::ps::evaluate<Kind>(feature, state_context).data().row_set, rows.data().row_set);
    }
    environment.reset_source();
    EXPECT_TRUE(environment.get_dl_caches().get_queries(false).empty());
    const auto rebuilt = kr::ps::evaluate<Kind>(feature, state_context);
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
                        EXPECT_EQ(step.status, kr::ps::ProgramOutcome::APPLIED);
                        const auto target = step.get_target();
                        EXPECT_EQ(target.get_state().index(), initial.get_state().index());
                        EXPECT_EQ(target.get_module_state().get_memory_state().get_name(), "target");
                        EXPECT_FALSE(step.planning_successor.has_value());
                        ASSERT_TRUE(step.rule);
                        const auto values = target.get_module_state().get_registers();
                        if (role)
                        {
                            ASSERT_TRUE(values.template values<kr::dl::RoleTag>()[0]);
                            const auto pair = values.template values<kr::dl::RoleTag>()[0].value();
                            EXPECT_EQ(pair.second.get_name(), "rooma");
                            if (scenario == 1)
                            {
                                EXPECT_EQ(pair.first.get_name(), "ball2");
                            }
                        }
                        else
                        {
                            ASSERT_TRUE(values.template values<kr::dl::ConceptTag>()[0]);
                            if (scenario == 1)
                            {
                                EXPECT_EQ(values.template values<kr::dl::ConceptTag>()[0].value().get_name(), "ball2");
                            }
                        }
                    }
                    const auto applied = expander.apply(initial, *steps.front().rule);
                    ASSERT_TRUE(applied);
                    EXPECT_EQ(applied->get_target().index(), steps.front().get_target().index());
                }
                else if (choose && scenario != 4)
                {
                    ASSERT_EQ(steps.size(), 1);
                    EXPECT_EQ(steps.front().status, kr::ps::ProgramOutcome::FAILURE);
                    EXPECT_EQ(steps.front().get_target().index(), initial.index());
                    ASSERT_TRUE(steps.front().rule);
                }
                else
                {
                    ASSERT_EQ(steps.size(), 1);
                    EXPECT_EQ(steps.front().status, kr::ps::ProgramOutcome::NO_APPLICABLE_ACTION);
                }
                for (const auto value : initial.get_module_state().get_registers().template values<kr::dl::ConceptTag>())
                    EXPECT_FALSE(value);
                for (const auto value : initial.get_module_state().get_registers().template values<kr::dl::RoleTag>())
                    EXPECT_FALSE(value);
                EXPECT_EQ(initial.get_module_state().get_memory_state().get_name(), "source");
                EXPECT_TRUE(collect_steps(expander, initial, false, [] { return true; }).empty());
            }
}

template<tyr::TaskKind Kind>
void expect_lazy_do_successors()
{
    for (const auto scenario : { 0, 1, 2, 3, 4, 5 })
    {
        SCOPED_TRACE(fmt::format("scenario={}", scenario));
        const auto task_context =
            create_task_context<Kind>(benchmark_path("classical/tests/gripper/domain.pddl"), benchmark_path("classical/tests/gripper/test-1.pddl"));
        auto& repository = *task_context->domain_context->ext_repository;
        auto& states = *task_context->search_context->state_repository;
        const auto domain = task_context->search_context->task->get_domain().get_domain();
        const auto selected_ball = scenario == 2 ? "(c_bot)" : scenario == 5 ? "(c_atomic_state \"room\")" : "(c_some (r_atomic_goal \"at\" true) (c_top))";
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
        const auto module_ = kr::ps::ext::dl::parse_module(source, domain, repository);
        const auto program = create_program(repository, module_, { module_ });
        auto expander = kr::ps::ext::SuccessorExpander<Kind>(task_context, program);
        const auto planning_node = initial_planning_node(expander);
        const auto initial = expander.initial_state(planning_node.get_state());
        ASSERT_EQ(states.num_states(), 1);
        const auto count_bindings = [&]
        {
            size_t count = 0;
            for (const auto action : task_context->search_context->task->get_task().get_domain().get_actions())
                count += task_context->search_context->task->get_repository()->relations().size(action.index());
            return count;
        };
        const auto initial_binding_count = count_bindings();
        EXPECT_TRUE(collect_steps(expander, initial, false, [] { return true; }).empty());
        EXPECT_EQ(states.num_states(), 1);

        const auto steps = collect_steps(expander, initial);
        if constexpr (std::same_as<Kind, tyr::LiftedTag>)
            EXPECT_EQ(count_bindings(), initial_binding_count + (scenario == 0 ? 2 : scenario == 4 ? 4 : 0));
        else
            EXPECT_EQ(count_bindings(), initial_binding_count);
        if (scenario < 2)
        {
            // Only ball2's two pick bindings construct states. Other balls and
            // the unrelated move schema must not be expanded before filtering.
            EXPECT_EQ(states.num_states(), 3);
        }
        else if (scenario < 4 || scenario == 5)
        {
            // A nonempty argument denotation can still reject every candidate.
            EXPECT_EQ(states.num_states(), 1);
        }
        if (scenario == 0 || scenario == 4)
        {
            ASSERT_EQ(steps.size(), scenario == 4 ? 4 : 2);
            auto moves = static_cast<std::size_t>(0);
            for (const auto& step : steps)
            {
                EXPECT_EQ(step.status, kr::ps::ProgramOutcome::APPLIED);
                ASSERT_TRUE(step.planning_successor.has_value());
                const auto action = step.planning_successor->label;
                if (action.get_relation().get_name() == "pick")
                {
                    EXPECT_EQ(objects_of(action)[0].get_name(), "ball2");
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
            EXPECT_EQ(steps.front().status, kr::ps::ProgramOutcome::NO_APPLICABLE_ACTION);
        }

        const auto same_step = [](const auto& left, const auto& right)
        {
            return left.status == right.status && left.get_target().index() == right.get_target().index()
                   && left.rule.has_value() == right.rule.has_value() && (!left.rule || left.rule->index() == right.rule->index())
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
    const auto task_context =
        create_task_context<Kind>(benchmark_path("classical/tests/gripper/domain.pddl"), benchmark_path("classical/tests/gripper/test-1.pddl"));
    auto& repository = *task_context->domain_context->ext_repository;
    auto& states = *task_context->search_context->state_repository;
    const auto program = kr::ps::ext::dl::parse_program(R"(
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
    auto expander = kr::ps::ext::SuccessorExpander<Kind>(task_context, program);
    const auto planning_node = initial_planning_node(expander);
    const auto initial = expander.initial_state(planning_node.get_state());
    ASSERT_EQ(states.num_states(), 1);
    const auto steps = collect_steps(expander, initial);
    ASSERT_EQ(steps.size(), 4);
    for (const auto& step : steps)
    {
        EXPECT_EQ(step.status, kr::ps::ProgramOutcome::APPLIED);
        EXPECT_EQ(step.get_target().get_state().index(), initial.get_state().index());
        EXPECT_FALSE(step.planning_successor.has_value());
        ASSERT_TRUE(step.rule);
        const auto applied = expander.apply(initial, step.rule.value());
        ASSERT_TRUE(applied);
        EXPECT_EQ(applied.value().get_target().index(), step.get_target().index());
        if (step.get_target().get_module_state().get_memory_state().get_name() == "skipped")
        {
            EXPECT_FALSE(expander.apply(step.get_target(), step.rule.value()));
        }
    }
    EXPECT_EQ(states.num_states(), 1);
    for (const auto universal : { false, true })
    {
        auto options = kr::ps::ext::ProgramSearchOptions<Kind> {};
        options.universal = universal;
        EXPECT_EQ(kr::ps::ext::find_solution(task_context, program, options).status, kr::ps::ProofStatus::FAILURE);
        EXPECT_EQ(states.num_states(), 1);
    }
    auto& search = *task_context->search_context;
    const auto successors = search.successor_generator->get_labeled_successor_nodes(planning_node, states, *search.axiom_evaluator);
    ASSERT_FALSE(successors.empty());
    EXPECT_FALSE(expander.matching_rule(initial, successors.front()));
}


template<tyr::TaskKind Kind>
void expect_transient_program_states_retain_values_and_intern_callers()
{
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../../fixtures/kr/ps/ext/choose";
    const auto context = create_task_context<Kind>(directory / "domain.pddl", directory / "task.pddl");
    auto& repository = *context->domain_context->ext_repository;
    auto& search = *context->search_context;
    auto& executions = *context->execution_repository;
    const auto entry = create_memory_state(repository, "entry");
    const auto outer_return = create_memory_state(repository, "outer-return");
    const auto inner_return = create_memory_state(repository, "inner-return");
    const auto module_ = create_module(repository, "module", entry, { entry, outer_return, inner_return });
    const auto program = create_program(repository, module_, { module_ });
    auto storage = kr::ps::ext::TransientExecutionStorage<Kind>(context, program);
    const auto initial_node = search.successor_generator->get_initial_node(*search.state_repository, *search.axiom_evaluator);
    const auto initial = storage.initial_state(initial_node.get_state());
    EXPECT_FALSE(storage.view(initial).get_call_stack());
    EXPECT_EQ(executions.template size<kr::ps::ext::CallStack>(), 0);
    const auto source = storage.view(initial).get_state();
    const auto bindings = search.successor_generator->get_applicable_action_bindings(initial_node);
    ASSERT_EQ(bindings.size(), 2);

    const auto outer_caller = storage.save_caller(storage.view(initial), outer_return);
    EXPECT_EQ(storage.save_caller(storage.view(initial), outer_return).index(), outer_caller.index());
    EXPECT_EQ(executions.template size<kr::ps::ext::CallStack>(), 1);
    const auto initial_module = storage.view(initial).get_module_state();
    const auto callee = storage.store(source, module_, entry, initial_module.get_registers(), initial_module.get_arguments(), outer_caller);
    const auto inner_caller = storage.save_caller(storage.view(callee), inner_return);
    EXPECT_EQ(storage.save_caller(storage.view(callee), inner_return).index(), inner_caller.index());
    EXPECT_EQ(executions.template size<kr::ps::ext::CallStack>(), 2);
    ASSERT_TRUE(inner_caller.get_caller());
    EXPECT_EQ(inner_caller.get_caller()->index(), outer_caller.index());
    EXPECT_FALSE(outer_caller.get_caller());

    const auto first = storage.successor(source, bindings.front());
    const auto scratch = &first.get_state().data();
    const auto expected = *scratch;
    const auto retained = storage.store(first.get_state(), module_, entry, initial_module.get_registers(), initial_module.get_arguments(), inner_caller);
    ASSERT_TRUE(retained->call_stack);
    EXPECT_EQ(*retained->call_stack, inner_caller.index());

    // Generating another candidate overwrites scratch, but not the accepted program state.
    const auto second = storage.successor(source, bindings.back());
    EXPECT_EQ(&second.get_state().data(), scratch);
    EXPECT_TRUE(urd::EqualTo<urd::builder<tyr::planning::State<Kind>>> {}(retained->module_state.state, expected));
    EXPECT_FALSE(urd::EqualTo<urd::builder<tyr::planning::State<Kind>>> {}(retained->module_state.state, second.get_state().data()));
    EXPECT_TRUE(urd::EqualTo<urd::builder<tyr::planning::State<Kind>>> {}(initial->module_state.state, urd::builder<tyr::planning::State<Kind>> { initial_node.get_state().data() }));

    const auto copy = storage.retain(storage.view(retained));
    ASSERT_TRUE(copy->call_stack);
    EXPECT_EQ(*copy->call_stack, inner_caller.index());
    const auto caller = storage.view(copy).get_call_stack();
    ASSERT_TRUE(caller);
    EXPECT_EQ(caller->index(), inner_caller.index());
    const auto returned = storage.store(storage.view(copy).get_state(),
                                        caller->get_module(),
                                        caller->get_return_memory_state(),
                                        caller->get_registers(),
                                        caller->get_arguments(),
                                        caller->get_caller());
    ASSERT_TRUE(returned->call_stack);
    EXPECT_EQ(*returned->call_stack, outer_caller.index());
    EXPECT_TRUE(urd::EqualTo<kr::ps::ext::MemoryStateView> {}(storage.view(returned).get_module_state().get_memory_state(), inner_return));

    const auto materialized = storage.materialize(storage.view(retained));
    const auto materialized_inner = materialized.get_call_stack();
    ASSERT_TRUE(materialized_inner);
    EXPECT_EQ(materialized_inner->index(), inner_caller.index());
    EXPECT_TRUE(urd::EqualTo<kr::ps::ext::MemoryStateView> {}(materialized_inner->get_return_memory_state(), inner_return));
    const auto materialized_outer = materialized_inner->get_caller();
    ASSERT_TRUE(materialized_outer);
    EXPECT_EQ(materialized_outer->index(), outer_caller.index());
    EXPECT_TRUE(urd::EqualTo<kr::ps::ext::MemoryStateView> {}(materialized_outer->get_return_memory_state(), outer_return));
    EXPECT_FALSE(materialized_outer->get_caller());
    EXPECT_EQ(executions.template size<kr::ps::ext::CallStack>(), 2);

    // Reusing a released slot must replace its values without replacing its buffers.
    auto planning_values = expected;
    planning_values.numeric_storage.values = { 3.0 };
    const auto planning = urd::resolve(planning_values, *search.state_repository);
    auto register_values = urd::data<kr::dl::semantics::RegisterValues>();
    register_values.concept_values.emplace_back(urd::index<tyr::formalism::Object>(0));
    register_values.role_values.emplace_back(::cista::pair { urd::index<tyr::formalism::Object>(0), urd::index<tyr::formalism::Object>(1) });
    const auto registers = storage.registers(register_values);
    auto recycled = storage.store(planning, module_, inner_return, registers, initial_module.get_arguments(), inner_caller);
    const auto* slot = recycled.get();
    const auto numeric_buffer = std::ranges::data(recycled->module_state.state.numeric_storage.values);
    const auto numeric_capacity = recycled->module_state.state.numeric_storage.values.capacity();
    const auto concept_buffer = recycled->module_state.registers.concept_values.data();
    const auto concept_capacity = recycled->module_state.registers.concept_values.allocated_size_;
    const auto role_buffer = recycled->module_state.registers.role_values.data();
    const auto role_capacity = recycled->module_state.registers.role_values.allocated_size_;
    recycled = {};
    planning_values.assign(initial_node.get_state().data());
    planning_values.numeric_storage.values = { 4.0 };
    register_values.concept_values.front() = urd::index<tyr::formalism::Object>(1);
    register_values.role_values.front().value().first = urd::index<tyr::formalism::Object>(1);
    register_values.role_values.front().value().second = urd::index<tyr::formalism::Object>(0);
    const auto reused = storage.store(planning, module_, entry, registers, initial_module.get_arguments(), {});
    EXPECT_EQ(reused.get(), slot);
    EXPECT_EQ(std::ranges::data(reused->module_state.state.numeric_storage.values), numeric_buffer);
    EXPECT_EQ(reused->module_state.state.numeric_storage.values.capacity(), numeric_capacity);
    EXPECT_EQ(reused->module_state.registers.concept_values.data(), concept_buffer);
    EXPECT_EQ(reused->module_state.registers.concept_values.allocated_size_, concept_capacity);
    EXPECT_EQ(reused->module_state.registers.role_values.data(), role_buffer);
    EXPECT_EQ(reused->module_state.registers.role_values.allocated_size_, role_capacity);
    EXPECT_EQ(reused->module_state.state.numeric_storage.values.front(), 4.0);
    EXPECT_TRUE(urd::EqualTo<urd::data<kr::dl::semantics::RegisterValues>> {}(reused->module_state.registers, register_values));
    EXPECT_FALSE(storage.view(reused).get_call_stack());
    EXPECT_EQ(storage.view(reused).get_module_state().get_memory_state().index(), entry.index());
    EXPECT_TRUE(urd::EqualTo<urd::builder<tyr::planning::State<Kind>>> {}(reused->module_state.state, planning_values));
    EXPECT_TRUE(urd::EqualTo<urd::builder<tyr::planning::State<Kind>>> {}(retained->module_state.state, expected));
}


template<tyr::TaskKind Kind, kr::ps::ext::RuleKind Tag>
void expect_unknown_action_rejected_during_preparation()
{
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../../fixtures/kr/ps/ext/choose";
    const auto task_context = create_task_context<Kind>(directory / "domain.pddl", directory / "task.pddl");
    auto& repository = *task_context->domain_context->ext_repository;
    const auto parsed = kr::ps::ext::dl::parse_module(R"(
(:module (:symbol actions) (:arguments) (:registers)
  (:entry source) (:memory source unreachable)
  (:features (:query (:symbol Moves) (:expression (q_atomic_state "edge" (from to)))))
  (:rules))
)",
                                                      task_context->search_context->task->get_domain().get_domain(),
                                                      repository);
    // Programmatic rules can bypass the parser's schema lookup. Even an
    // unreachable rule must have its action resolved when preparing the program.
    auto action_data = urd::data<kr::ps::ext::Rule<Tag>> {};
    action_data.source = create_memory_state(repository, "unreachable").index();
    action_data.target = parsed.get_entry_memory_state().index();
    action_data.action_name = "missing-action";
    if constexpr (std::same_as<Tag, kr::ps::ext::ActionTag>)
        action_data.query_feature = parsed.get_query_features()[0].index();
    const auto action_rule = repository.insert(action_data).first;
    auto variant_data = urd::data<kr::ps::Rule<kr::ExtFamilyTag>>(std::string("missing-schema"), action_rule.index());
    const auto variant = repository.insert(variant_data).first;
    auto module_data = parsed.data();
    auto transition = urd::flat_vector<urd::index<kr::ps::Rule<kr::ExtFamilyTag>>> {};
    transition.push_back(variant.index());
    module_data.memory_transitions.push_back(std::move(transition));
    kr::ps::ext::canonicalize(module_data);
    const auto module_ = repository.insert(module_data).first;
    const auto program = create_program(repository, module_, { module_ });
    const auto num_states = task_context->search_context->state_repository->num_states();
    try
    {
        auto expander = kr::ps::ext::SuccessorExpander<Kind>(task_context, program);
        FAIL() << "Preparing an unknown action schema must fail before state evaluation";
    }
    catch (const std::logic_error& error)
    {
        if constexpr (std::same_as<Tag, kr::ps::ext::ActionTag>)
            EXPECT_EQ(std::string(error.what()), "Action rule: action schema does not exist in the domain.");
        else
            EXPECT_EQ(std::string(error.what()), "Do rule: action schema does not exist in the domain.");
    }
    EXPECT_EQ(task_context->search_context->state_repository->num_states(), num_states);
}

template<tyr::TaskKind Kind>
void expect_query_action_contracts()
{
    const auto valid_query = std::string(R"((q_join (q_atomic_state "edge" (from to)) (q_atomic_state "at" (from))))");
    for (const auto scenario : { 0, 1, 2, 3, 4 })
    {
        SCOPED_TRACE(fmt::format("query action scenario={}", scenario));
        const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../../fixtures/kr/ps/ext/choose";
        const auto task_context = create_task_context<Kind>(directory / "domain.pddl", directory / "task.pddl");
        auto& repository = *task_context->domain_context->ext_repository;
        const auto query = scenario == 1 ? fmt::format("(q_difference {} {})", valid_query, valid_query) :
                           scenario == 3 ? fmt::format(R"((q_union {} (q_atomic_state "edge" (from to))))", valid_query) :
                           scenario == 4 ? fmt::format("(q_project (to from) {})", valid_query) :
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
        const auto module_ = kr::ps::ext::dl::parse_module(source, task_context->search_context->task->get_domain().get_domain(), repository);
        const auto program = create_program(repository, module_, { module_ });
        auto expander = kr::ps::ext::SuccessorExpander<Kind>(task_context, program);
        const auto planning_node = initial_planning_node(expander);
        const auto initial = expander.initial_state(planning_node.get_state());
        const auto rule = module_.get_memory_transitions()[0][0];
        auto lazy = urd::Store<urd::sequence<kr::ps::ProgramStep<kr::ExtFamilyTag, Kind>>> {};
        if (scenario == 2)
        {
            // Effects filter Action transitions: no move decreases N, so none is offered or matched.
            for (const auto first_only : { false, true })
            {
                const auto steps = collect_steps(expander, initial, first_only);
                ASSERT_EQ(steps.size(), 1);
                EXPECT_EQ(steps.front().status, kr::ps::ProgramOutcome::NO_APPLICABLE_ACTION);
            }
            auto& search = *task_context->search_context;
            const auto successors = search.successor_generator->get_labeled_successor_nodes(planning_node, *search.state_repository, *search.axiom_evaluator);
            ASSERT_FALSE(successors.empty());
            EXPECT_FALSE(expander.matching_rule(initial, successors.front()).has_value());
            continue;
        }
        if (scenario == 4)
        {
            EXPECT_THROW(collect_steps(expander, initial), std::logic_error);
            EXPECT_THROW(lazy = collect_steps(expander, initial, true), std::logic_error);
            continue;
        }
        if (scenario == 3)
        {
            // Only the first two rows are applicable; the third is unvisited by
            // greedy execution and remains a contract violation exhaustively.
            lazy = collect_steps(expander, initial, true);
            ASSERT_EQ(lazy.size(), 1);
            EXPECT_EQ(lazy.front().status, kr::ps::ProgramOutcome::APPLIED);
            auto options = kr::ps::ext::ProgramSearchOptions<Kind> {};
            EXPECT_NO_THROW(kr::ps::ext::find_solution(task_context, program, options));
            options.universal = true;
            EXPECT_THROW(kr::ps::ext::find_solution(task_context, program, options), std::logic_error);
            EXPECT_THROW(collect_steps(expander, initial), std::logic_error);
            continue;
        }
        if (scenario == 0)
        {
            auto storage = kr::ps::ext::InternedExecutionStorage<Kind>(task_context, program);
            auto evaluators = kr::ps::ext::detail::RuleEvaluators<Kind>(task_context, program);
            auto context = evaluators.make_context(storage, planning_node.get_state());
            auto& generator = *task_context->search_context->successor_generator;
            std::visit(
                [&]<kr::ps::ext::RuleKind Tag>(kr::ps::ext::RuleView<Tag> concrete)
                {
                    if constexpr (std::same_as<Tag, kr::ps::ext::ActionTag>)
                        for (const auto action : task_context->search_context->task->get_task().get_domain().get_actions())
                            if (action.get_name().str() == concrete.get_action_name())
                            {
                                const auto evaluator = kr::ps::ext::detail::ActionRuleEvaluator<Kind>(concrete, rule, action);
                                const auto count = task_context->search_context->task->get_repository()->relations().size(action.index());
                                auto matched = static_cast<size_t>(0);
                                EXPECT_TRUE(generator.for_each_borrowed_applicable_action_binding(
                                    planning_node,
                                    action,
                                    [&](tyr::planning::BorrowedActionRowView<Kind> binding)
                                    {
                                        const auto target = storage.successor(planning_node.get_state(), binding);
                                        EXPECT_TRUE(evaluator.matches(
                                            context,
                                            initial,
                                            tyr::planning::LabeledNode<Kind, tyr::planning::StateView<Kind>, tyr::planning::BorrowedActionRowView<Kind>> {
                                                binding,
                                                target }));
                                        ++matched;
                                        return true;
                                    }));
                                EXPECT_EQ(matched, 2);
                                EXPECT_EQ(task_context->search_context->task->get_repository()->relations().size(action.index()), count);
                            }
                },
                rule.get_variant());
        }
        const auto steps = collect_steps(expander, initial);
        if (scenario == 1)
        {
            ASSERT_EQ(steps.size(), 1);
            EXPECT_EQ(steps.front().status, kr::ps::ProgramOutcome::NO_APPLICABLE_ACTION);
            EXPECT_EQ(task_context->search_context->state_repository->num_states(), 1);
            continue;
        }
        ASSERT_EQ(steps.size(), 2);
        for (const auto& step : steps)
        {
            ASSERT_TRUE(step.planning_successor.has_value());
            const auto candidate = step.planning_successor.value().unpack();
            EXPECT_EQ(objects_of(candidate.label)[0].get_name(), "start");
            EXPECT_EQ(step.get_target().get_module_state().get_memory_state().get_name(), "target");
            const auto matching = expander.matching_rule(initial, candidate);
            ASSERT_TRUE(matching);
            EXPECT_EQ(matching.value().index(), rule.index());
            const auto applied = expander.apply(initial, rule, candidate);
            ASSERT_TRUE(applied);
            EXPECT_EQ(applied.value().get_target().index(), step.get_target().index());
        }
        EXPECT_FALSE(expander.apply(initial, rule));
    }
}

template<tyr::TaskKind Kind>
void expect_callback_selection_stops_after_selected_rule()
{
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
        const auto module_ = kr::ps::ext::dl::parse_module(source, task_context->search_context->task->get_domain().get_domain(), repository);
        const auto program = create_program(repository, module_, { module_ });
        auto expander = kr::ps::ext::SuccessorExpander<Kind>(task_context, program);
        const auto planning_node = initial_planning_node(expander);
        const auto initial = expander.initial_state(planning_node.get_state());
        const auto steps = collect_steps(expander, initial, true);
        ASSERT_EQ(steps.size(), choose ? 2 : 1);
        for (const auto& step : steps)
            EXPECT_EQ(step.get_target().get_module_state().get_memory_state().get_name(), "selected");
        EXPECT_EQ(task_context->search_context->state_repository->num_states(), 1);
        EXPECT_THROW(collect_steps(expander, initial), std::logic_error);
    }
}

template<tyr::TaskKind Kind>
void expect_rule_occurrence_order()
{
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../../fixtures/kr/ps/ext/choose";
    const auto task_context = create_task_context<Kind>(directory / "domain.pddl", directory / "task.pddl");
    auto& repository = *task_context->domain_context->ext_repository;
    const auto domain = task_context->search_context->task->get_domain().get_domain();
    const auto parsed = kr::ps::ext::dl::parse_module(R"(
(:module (:symbol ordered) (:arguments) (:registers)
  (:entry source) (:memory source other first second)
  (:features)
  (:rules
    (:rule (:symbol first) (:expression (:source-memory source) (:target-memory first)
      (:sketch (:conditions) (:effects))))
    (:rule (:symbol second) (:expression (:source-memory source) (:target-memory second)
      (:sketch (:conditions) (:effects))))
    (:rule (:symbol inactive) (:expression (:source-memory other) (:target-memory first)
      (:sketch (:conditions) (:effects))))))
)",
                                                      domain,
                                                      repository);
    auto data = parsed.data();
    data.memory_transitions.clear();
    // Occurrences, including repetitions, retain module order rather than rule-index order.
    for (const auto row : { 1, 2, 0, 1 })
        data.memory_transitions.push_back(parsed.data().memory_transitions[row]);
    const auto module_ = repository.insert(data).first;
    const auto other = kr::ps::ext::dl::parse_module(R"(
(:module (:symbol other) (:arguments) (:registers)
  (:entry source) (:memory source outside)
  (:features)
  (:rules (:rule (:symbol outside) (:expression (:source-memory source) (:target-memory outside)
    (:sketch (:conditions) (:effects))))))
)",
                                                     domain,
                                                     repository);
    const auto program = create_program(repository, module_, { module_, other, module_ });
    auto expander = kr::ps::ext::SuccessorExpander<Kind>(task_context, program);
    const auto planning_node = initial_planning_node(expander);
    const auto initial = expander.initial_state(planning_node.get_state());
    const auto steps = collect_steps(expander, initial);
    ASSERT_EQ(steps.size(), 3);
    const auto expected = std::array { parsed.get_memory_transitions()[1][0], parsed.get_memory_transitions()[0][0], parsed.get_memory_transitions()[1][0] };
    for (size_t i = 0; i < steps.size(); ++i)
    {
        ASSERT_TRUE(steps[i].rule);
        EXPECT_EQ(steps[i].rule.value(), expected[i]);
    }
}

template<tyr::TaskKind Kind>
void expect_callback_sketch_order_and_cancellation()
{
    const auto source = std::string(R"(
(:module (:symbol order) (:arguments) (:registers)
  (:entry source) (:memory source impossible picked)
  (:features
    (:concept (:symbol Ball) (:expression (c_atomic_state "ball")))
    (:concept (:symbol Room) (:expression (c_atomic_state "room")))
    (:concept (:symbol Gripper) (:expression (c_atomic_state "gripper")))
    (:numerical (:symbol Free) (:expression (n_count (c_atomic_state "free")))))
  (:rules
    (:rule (:symbol next) (:expression (:source-memory source) (:target-memory picked)
      (:sketch (:conditions) (:effects (decreases Free)))))
    (:rule (:symbol rejected) (:expression (:source-memory source) (:target-memory impossible)
      (:do (:conditions) (:action "pick") (:arguments Ball Room Gripper) (:effects (increases Free)))))))
)");
    // Sketch rules with effects are expanded after all other rules, so the rejecting Do rule runs first despite its position.
    for (const auto cancelled : { true, false })
    {
        const auto task_context =
            create_task_context<Kind>(benchmark_path("classical/tests/gripper/domain.pddl"), benchmark_path("classical/tests/gripper/test-1.pddl"));
        auto& repository = *task_context->domain_context->ext_repository;
        auto& states = *task_context->search_context->state_repository;
        const auto module_ = kr::ps::ext::dl::parse_module(source, task_context->search_context->task->get_domain().get_domain(), repository);
        const auto program = create_program(repository, module_, { module_ });
        auto expander = kr::ps::ext::SuccessorExpander<Kind>(task_context, program);
        const auto planning_node = initial_planning_node(expander);
        const auto initial = expander.initial_state(planning_node.get_state());
        const auto count_bindings = [&]
        {
            size_t count = 0;
            for (const auto action : task_context->search_context->task->get_task().get_domain().get_actions())
                count += task_context->search_context->task->get_repository()->relations().size(action.index());
            return count;
        };
        const auto initial_binding_count = count_bindings();
        using Step = kr::ps::ProgramStep<kr::ExtFamilyTag, Kind>;
        auto selected = urd::Store<urd::sequence<Step>> {};
        auto statistics = kr::ps::ext::ProgramSearchStatistics {};
        const auto emit = [&](const auto& expansion)
        {
            EXPECT_GT(statistics.num_generated, 0);
            if constexpr (std::same_as<std::remove_cvref_t<decltype(expansion)>, Step>)
                selected.push_back(expansion);
            else
                ADD_FAILURE() << "Expected a program step";
            return false;
        };
        EXPECT_FALSE(expander.for_each_successor(initial, statistics, emit, [] { return true; }));
        EXPECT_TRUE(selected.empty());
        EXPECT_EQ(statistics.num_generated, 0);
        EXPECT_EQ(states.num_states(), 1);
        EXPECT_EQ(count_bindings(), initial_binding_count);
        const auto stop = [&] { return cancelled && states.num_states() > 1; };
        EXPECT_FALSE(expander.for_each_successor(initial, statistics, emit, stop));
        if (cancelled)
        {
            EXPECT_TRUE(selected.empty());
            EXPECT_EQ(states.num_states(), 2);
            EXPECT_EQ(count_bindings(), initial_binding_count);
            continue;
        }
        ASSERT_EQ(selected.size(), 1);
        EXPECT_EQ(count_bindings(), initial_binding_count + (std::same_as<Kind, tyr::LiftedTag> ? 1 : 0));
        auto complete = urd::Store<urd::sequence<Step>> {};
        EXPECT_TRUE(expander.for_each_successor(
            initial,
            statistics,
            [&](const auto& expansion)
            {
                if constexpr (std::same_as<std::remove_cvref_t<decltype(expansion)>, Step>)
                    complete.push_back(expansion);
                else
                    ADD_FAILURE() << "Expected a program step";
                return true;
            },
            [] { return false; }));
        ASSERT_GT(complete.size(), 1);
        EXPECT_EQ(selected.front().get_target().index(), complete.front().get_target().index());
        EXPECT_EQ(selected.front().planning_successor.value().label, complete.front().planning_successor.value().label);
        EXPECT_EQ(statistics.num_expanded, 0);
    }
}

template<tyr::TaskKind Kind>
void expect_query_action_existential_binding()
{
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../../fixtures/kr/ps/ext/action_existential";
    const auto task_context = create_task_context<Kind>(directory / "domain.pddl", directory / "task.pddl");
    auto& repository = *task_context->domain_context->ext_repository;
    const auto domain = task_context->search_context->task->get_domain().get_domain();
    ASSERT_EQ(domain.get_actions().size(), 1);
    const auto action = domain.get_actions()[0];
    ASSERT_EQ(action.get_arity(), 2);
    const auto module_ = kr::ps::ext::dl::parse_module(R"(
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
    auto expander = kr::ps::ext::SuccessorExpander<Kind>(task_context, program);
    const auto planning_node = initial_planning_node(expander);
    const auto steps = collect_steps(expander, expander.initial_state(planning_node.get_state()));
    ASSERT_EQ(steps.size(), 1);
    ASSERT_TRUE(steps.front().planning_successor.has_value());
    const auto objects = objects_of(steps.front().planning_successor.value().label);
    ASSERT_EQ(objects.size(), 2);
    EXPECT_EQ(objects[0].get_name(), "start");
    EXPECT_EQ(objects[1].get_name(), "proof");
    EXPECT_TRUE(is_planning_goal(expander, steps.front().get_target().get_state()));
    EXPECT_TRUE(kr::ps::ext::find_solution(task_context, program, kr::ps::ext::ProgramSearchOptions<Kind> {}).is_successful());
}

template<tyr::TaskKind Kind>
void expect_query_action_nullary_binding()
{
    for (const auto empty : { false, true })
    {
        const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../../fixtures/kr/ps/ext/action_nullary";
        const auto task_context = create_task_context<Kind>(directory / "domain.pddl", directory / "task.pddl");
        auto& repository = *task_context->domain_context->ext_repository;
        const auto truth = std::string(R"((q_atomic_state "truth" ()))");
        const auto query = empty ? fmt::format("(q_difference {} {})", truth, truth) : truth;
        const auto source = fmt::format(R"(
(:module (:symbol nullary) (:arguments) (:registers)
  (:entry source) (:memory source)
  (:features (:query (:symbol Bindings) (:expression {0})))
  (:rules (:rule (:symbol finish) (:expression
    (:source-memory source) (:target-memory source)
    (:action (:conditions) (:action "finish") (:query Bindings) (:effects))))))
)",
                                        query);
        const auto module_ = kr::ps::ext::dl::parse_module(source, task_context->search_context->task->get_domain().get_domain(), repository);
        const auto program = create_program(repository, module_, { module_ });
        auto expander = kr::ps::ext::SuccessorExpander<Kind>(task_context, program);
        const auto planning_node = initial_planning_node(expander);
        const auto initial = expander.initial_state(planning_node.get_state());
        const auto steps = collect_steps(expander, initial);
        ASSERT_EQ(steps.size(), 1);
        if (empty)
        {
            EXPECT_EQ(steps.front().status, kr::ps::ProgramOutcome::NO_APPLICABLE_ACTION);
            EXPECT_EQ(task_context->search_context->state_repository->num_states(), 1);
        }
        else
        {
            EXPECT_EQ(steps.front().status, kr::ps::ProgramOutcome::APPLIED);
            ASSERT_TRUE(steps.front().planning_successor.has_value());
            EXPECT_TRUE(objects_of(steps.front().planning_successor.value().label).empty());
            EXPECT_TRUE(is_planning_goal(expander, steps.front().get_target().get_state()));
            EXPECT_THROW(collect_steps(expander, steps.front().get_target()), std::logic_error);
            EXPECT_THROW(kr::ps::ext::find_solution(task_context, program, kr::ps::ext::ProgramSearchOptions<Kind> {}), std::invalid_argument);
        }
    }
}

template<tyr::TaskKind Kind>
void expect_callback_expansion_counts()
{
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
        auto counts = urd::Store<urd::sequence<size_t>> {};
        auto state_counts = urd::Store<urd::sequence<size_t>> {};
        auto binding_counts = urd::Store<urd::sequence<size_t>> {};
        auto first_bindings = urd::Store<urd::sequence<urd::Store<urd::sequence<std::string>>>> {};
        for (const auto complete : { true, false })
        {
            const auto task_context =
                create_task_context<Kind>(benchmark_path("classical/tests/gripper/domain.pddl"), benchmark_path("classical/tests/gripper/test-1.pddl"));
            auto& repository = *task_context->domain_context->ext_repository;
            const auto module_ = kr::ps::ext::dl::parse_module(source, task_context->search_context->task->get_domain().get_domain(), repository);
            const auto program = create_program(repository, module_, { module_ });
            auto expander = kr::ps::ext::SuccessorExpander<Kind>(task_context, program);
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
                    count += task->get_repository()->relations().size(action.index());
                binding_counts.push_back(count);
            }
            ASSERT_FALSE(steps.empty());
            EXPECT_EQ(steps.front().status, kr::ps::ProgramOutcome::APPLIED);
            auto first = urd::Store<urd::sequence<std::string>> {};
            if (binding)
                first.push_back(
                    std::string(steps.front().get_target().get_module_state().get_registers().template values<kr::dl::ConceptTag>()[0].value().get_name().str()));
            else
                for (const auto object : objects_of(steps.front().planning_successor->label))
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

}  // namespace runir::tests

#endif
