#include "module_fixtures.hpp"
#include "planning_fixtures.hpp"
#include "record_copy.hpp"

#include <algorithm>
#include <cista/serialization.h>
#include <gtest/gtest.h>
#include <optional>
#include <runir/kr/ps/execution_repository.hpp>
#include <runir/kr/ps/ext/execution_data.hpp>
#include <runir/kr/ps/ext/execution_view.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/task_context.hpp>
#include <string>
#include <tyr/planning/ground/successor_generator.hpp>
#include <tyr/planning/ground/task.hpp>
#include <urd/elements/flat.hpp>

namespace runir::tests
{

namespace
{

template<typename T>
void expect_cista_round_trip(const T& value)
{
    auto bytes = cista::serialize(value);
    const auto* decoded = cista::deserialize<T>(bytes);
    EXPECT_EQ(value, *decoded);
}

void expect_control_records_round_trip()
{
    auto builder = kr::ps::ext::ControlBuilder();
    {
        auto data = builder.template scratch<kr::ps::ext::ModuleState>();
        data->mod = urd::index<kr::ps::ext::Module>(1);
        data->memory_state = urd::index<kr::ps::MemoryState<kr::ExtFamilyTag>>(2);
        data->registers = urd::index<kr::dl::semantics::RegisterValues>(3);
        data->arguments = urd::index<kr::dl::semantics::CallArguments>(4);
        expect_cista_round_trip(*data);
    }
    {
        auto data = builder.template scratch<kr::ps::ext::CallStack>();
        data->module_state = urd::index<kr::ps::ext::ModuleState>(6);
        expect_cista_round_trip(*data);
        data->caller = urd::index<kr::ps::ext::CallStack>(7);
        expect_cista_round_trip(*data);
    }
}

template<tyr::TaskKind Kind>
void expect_execution_records_round_trip()
{
    auto builder = kr::ps::ext::ExecutionBuilder<Kind>();
    {
        auto data = builder.template scratch<kr::ps::ext::ProgramState<Kind>>();
        data->state = urd::index<tyr::planning::State<Kind>>(5);
        data->module_state = urd::index<kr::ps::ext::ModuleState>(6);
        expect_cista_round_trip(*data);
        data->call_stack = urd::index<kr::ps::ext::CallStack>(9);
        expect_cista_round_trip(*data);
    }
    {
        // A scratch record drawn again starts empty.
        auto data = builder.template scratch<kr::ps::ext::ProgramState<Kind>>();
        EXPECT_EQ(data->module_state, urd::index<kr::ps::ext::ModuleState>());
        EXPECT_FALSE(data->call_stack);
    }
}

}  // namespace

TEST(RunirTests, ExtExecutionRepositoryPersistsRecordsAndSharesCallers)
{
    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
    auto task_context = kr::TaskContext<tyr::GroundTag>::create(kr::DomainContext::create(task->get_domain()), search_context);

    auto dl_repository = task_context->domain_context->ext_repository->get_dl_repository_ptr();
    auto repository = task_context->domain_context->ext_repository;

    const auto caller_entry = create_memory_state(*repository, "caller_entry");
    const auto caller_return = create_memory_state(*repository, "caller_return");
    const auto callee_entry = create_memory_state(*repository, "callee_entry");
    const auto caller = create_module(*repository, "caller", caller_entry, { caller_entry, caller_return });
    const auto callee = create_module(*repository, "callee", callee_entry, { callee_entry });

    (void) create_program(*repository, caller, { caller, callee });
    auto& execution_builder = task_context->execution_builder;
    auto& control_builder = task_context->control_builder;
    auto control_repository = task_context->control_repository;
    auto& dl_builder = task_context->dl_builder;
    auto& denotations = *task_context->dl_denotation_repository;
    auto execution_repository = task_context->execution_repository;

    const void* scratch_address = nullptr;
    {
        auto scratch = dl_builder.scratch<kr::dl::semantics::RegisterValues>();
        scratch_address = scratch.get();
        EXPECT_TRUE(scratch->concept_values.empty());
        EXPECT_TRUE(scratch->role_values.empty());
        EXPECT_TRUE(kr::dl::semantics::is_canonical(*scratch));
        scratch->concept_values.resize(6);
        scratch->concept_values[0] = urd::index<tyr::formalism::Object>(0);
    }
    {
        auto scratch = dl_builder.scratch<kr::dl::semantics::RegisterValues>();
        EXPECT_EQ(scratch.get(), scratch_address);
        EXPECT_TRUE(scratch->concept_values.empty());
        EXPECT_TRUE(scratch->role_values.empty());
    }

    const auto registers = [&]()
    {
        auto data = dl_builder.scratch<kr::dl::semantics::RegisterValues>();
        data->concept_values.resize(6);
        data->role_values.resize(7);
        data->concept_values[0] = urd::index<tyr::formalism::Object>(0);
        auto& role = data->role_values[0].emplace();
        role.first = urd::index<tyr::formalism::Object>(0);
        role.second = urd::index<tyr::formalism::Object>(1);
        const auto* buffer = data->concept_values.data();
        const auto result = ygg::formalism::insert(denotations, *data).first;
        EXPECT_EQ(data->concept_values.data(), buffer);
        return result;
    }();

    {
        auto data = dl_builder.scratch<kr::dl::semantics::RegisterValues>();
        data->concept_values.resize(6);
        data->role_values.resize(7);
        data->concept_values[0] = urd::index<tyr::formalism::Object>(0);
        auto& role = data->role_values[0].emplace();
        role.first = urd::index<tyr::formalism::Object>(0);
        role.second = urd::index<tyr::formalism::Object>(1);
        canonicalize(*data);
        const auto found = denotations.find(*data);
        ASSERT_TRUE(found);
        EXPECT_EQ(found.value().index(), registers.index());
        const auto [duplicate, created] = ygg::formalism::insert(denotations, *data);
        EXPECT_FALSE(created);
        EXPECT_EQ(duplicate.index(), registers.index());
        EXPECT_EQ(&duplicate.context(), &denotations);
        data->concept_values[0] = urd::index<tyr::formalism::Object>(1);
        const auto changed = denotations.insert(*data).first;
        EXPECT_NE(changed.index(), registers.index());
        urd::clear_fields(*data);
        EXPECT_EQ(changed.template values<kr::dl::ConceptTag>()[0].value().index(), urd::index<tyr::formalism::Object>(1));
        EXPECT_EQ(registers.template values<kr::dl::ConceptTag>()[0].value().index(), urd::index<tyr::formalism::Object>(0));
        EXPECT_EQ(denotations.size<kr::dl::semantics::RegisterValues>(), 2);
    }

    const auto arguments = [&]()
    {
        auto data = dl_builder.scratch<kr::dl::semantics::CallArguments>();
        auto boolean = urd::data<kr::dl::semantics::Denotation<kr::dl::BooleanTag>>();
        boolean.value = true;
        data->boolean_arguments.push_back(denotations.insert(boolean).first.index());
        canonicalize(*data);
        return denotations.insert(*data).first;
    }();
    {
        auto data = dl_builder.scratch<kr::dl::semantics::CallArguments>();
        data->boolean_arguments = denotations[arguments.index()].boolean_arguments;
        const auto found = denotations.find(*data);
        ASSERT_TRUE(found);
        EXPECT_EQ(found.value().index(), arguments.index());
        const auto [duplicate, created] = denotations.insert(*data);
        EXPECT_FALSE(created);
        EXPECT_EQ(duplicate.index(), arguments.index());
        EXPECT_EQ(&duplicate.context(), &denotations);
        urd::clear_fields(*data);
        const auto empty = denotations.insert(*data).first;
        EXPECT_TRUE(empty.arguments<kr::dl::BooleanTag>().empty());
        EXPECT_TRUE(arguments.arguments<kr::dl::BooleanTag>()[0].get_value());
        EXPECT_EQ(denotations.size<kr::dl::semantics::CallArguments>(), 2);
    }

    const auto initial = search_context->successor_generator->get_initial_node(*search_context->state_repository, *search_context->axiom_evaluator);
    const auto& state = initial.get_state();
    using CallStackView = kr::ps::ext::CallStackView;

    // An activation: a module, a memory state, registers and arguments, interned once.
    const auto activation = [&](kr::ps::ext::ModuleView module_, kr::ps::ext::MemoryStateView memory_state)
    {
        auto data = control_builder.checkout<kr::ps::ext::ModuleState>();
        urd::store(data->mod, module_);
        urd::store(data->memory_state, memory_state);
        urd::store(data->registers, registers);
        urd::store(data->arguments, arguments);
        return control_repository->insert(*data, control_builder).first;
    };
    const auto caller_activation = activation(caller, caller_return);
    const auto callee_activation = activation(callee, callee_entry);
    EXPECT_EQ(activation(callee, callee_entry), callee_activation);
    EXPECT_EQ(&callee_activation.context(), control_repository.get());
    EXPECT_EQ(&callee_activation.get_mod().context(), repository.get());
    EXPECT_EQ(&callee_activation.get_registers().context(), &denotations);
    EXPECT_EQ(control_repository->size<kr::ps::ext::ModuleState>(), 2);

    const auto program_state =
        [&](const tyr::planning::StateView<tyr::GroundTag>& planning_state, kr::ps::ext::ModuleStateView activation_, std::optional<CallStackView> caller_)
    {
        auto data = execution_builder.checkout<kr::ps::ext::ProgramState<tyr::GroundTag>>();
        data->state = planning_state.pack().index();
        urd::store(data->module_state, activation_);
        if (caller_.has_value())
            urd::store(data->call_stack, *caller_);
        return execution_repository->insert(*data, execution_builder).first;
    };
    {
        // Program states that differ in their planning state share their activation.
        const auto successors =
            search_context->successor_generator->get_successor_nodes(initial, *search_context->state_repository, *search_context->axiom_evaluator);
        const auto changed = std::ranges::find_if(successors, [&](const auto& node) { return node.get_state().pack().index() != state.pack().index(); });
        ASSERT_NE(changed, successors.end());
        const auto first = program_state(state, callee_activation, std::nullopt);
        const auto moved = program_state(changed->get_state(), callee_activation, std::nullopt);
        EXPECT_NE(moved, first);
        EXPECT_EQ(moved.get_module_state(), first.get_module_state());
        EXPECT_EQ(moved.get_state().index(), changed->get_state().pack().index());
        EXPECT_EQ(first.get_state().index(), state.pack().index());
        EXPECT_EQ(program_state(state, callee_activation, std::nullopt), first);
        EXPECT_EQ(control_repository->size<kr::ps::ext::ModuleState>(), 2);
    }

    // A frame: the suspended activation (its memory state the one to return to) and its caller.
    const auto frame = [&](kr::ps::ext::ModuleStateView activation_, std::optional<CallStackView> caller_)
    {
        auto data = control_builder.checkout<kr::ps::ext::CallStack>();
        urd::store(data->module_state, activation_);
        if (caller_.has_value())
            urd::store(data->caller, *caller_);
        return control_repository->insert(*data, control_builder).first;
    };
    const auto caller_frame = frame(caller_activation, std::nullopt);
    const auto callee_frame = frame(callee_activation, caller_frame);
    {
        const auto nested_caller = frame(caller_activation, caller_frame);
        const auto first = program_state(state, callee_activation, caller_frame);
        const auto nested = program_state(state, callee_activation, nested_caller);
        EXPECT_NE(nested, first);
        EXPECT_EQ(nested.get_module_state(), first.get_module_state());
        EXPECT_EQ(nested.get_call_stack().value(), nested_caller);
        EXPECT_EQ(program_state(state, callee_activation, nested_caller), nested);
        EXPECT_EQ(control_repository->size<kr::ps::ext::ModuleState>(), 2);
    }

    ASSERT_TRUE(callee_frame.get_caller());
    EXPECT_EQ(callee_frame.get_caller().value().index(), caller_frame.index());
    const auto saved = callee_frame.get_caller().value().get_module_state().get_registers();
    EXPECT_EQ(saved.template values<kr::dl::ConceptTag>()[0].value().index(), urd::index<tyr::formalism::Object>(0));
    const auto role_value = saved.template values<kr::dl::RoleTag>()[0];
    ASSERT_TRUE(role_value);
    const auto pair = role_value.value();
    EXPECT_EQ(pair.first.index(), urd::index<tyr::formalism::Object>(0));
    EXPECT_EQ(pair.second.index(), urd::index<tyr::formalism::Object>(1));

    // A return: the frame's activation and caller, over the current planning state.
    const auto returned_state = program_state(state, caller_frame.get_module_state(), caller_frame.get_caller());
    EXPECT_EQ(returned_state.get_module_state().get_mod().get_symbol().get_name(), "caller");
    EXPECT_EQ(returned_state.get_module_state(), caller_activation);
    EXPECT_EQ(returned_state.get_state().index(), state.pack().index());
    EXPECT_FALSE(returned_state.get_call_stack());
}

TEST(RunirTests, ExtExecutionRecordsAreCistaCompatible)
{
    auto dl_builder = kr::dl::semantics::Builder();
    {
        auto data = dl_builder.scratch<kr::dl::semantics::RegisterValues>();
        data->concept_values.resize(6);
        data->role_values.resize(7);
        data->concept_values[0] = urd::index<tyr::formalism::Object>(3);
        auto& role = data->role_values[0].emplace();
        role.first = urd::index<tyr::formalism::Object>(4);
        role.second = urd::index<tyr::formalism::Object>(5);
        data->concept_values[5] = urd::index<tyr::formalism::Object>(6);
        data->role_values[6] = role;
        expect_cista_round_trip(*data);
    }
    {
        auto data = dl_builder.scratch<kr::dl::semantics::CallArguments>();
        data->concept_arguments.push_back(urd::index<kr::dl::semantics::Denotation<kr::dl::ConceptTag>>(1));
        data->role_arguments.push_back(urd::index<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(2));
        data->boolean_arguments.push_back(urd::index<kr::dl::semantics::Denotation<kr::dl::BooleanTag>>(3));
        data->numerical_arguments.push_back(urd::index<kr::dl::semantics::Denotation<kr::dl::NumericalTag>>(4));
        expect_cista_round_trip(*data);
    }
    expect_control_records_round_trip();
    expect_execution_records_round_trip<tyr::GroundTag>();
    expect_execution_records_round_trip<tyr::LiftedTag>();
}

}  // namespace runir::tests
