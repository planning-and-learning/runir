#include "module_fixtures.hpp"
#include "planning_fixtures.hpp"

#include <algorithm>
#include <cista/serialization.h>
#include <gtest/gtest.h>
#include <optional>
#include <runir/kr/ps/ext/execution_repository.hpp>
#include <runir/kr/ps/ext/execution_view.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/task_context.hpp>
#include <string>
#include <tyr/planning/ground/task.hpp>

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

template<tyr::TaskKind Kind>
void expect_execution_records_round_trip()
{
    auto builder = kr::ps::ext::ExecutionBuilder<Kind>();
    {
        auto data = builder.template get_builder<kr::ps::ext::ModuleState<Kind>>();
        data->state = ygg::Index<tyr::planning::State<Kind>>(5);
        data->module = ygg::Index<kr::ps::ext::Module>(1);
        data->memory_state = ygg::Index<kr::ps::ext::MemoryState>(2);
        data->registers = ygg::Index<kr::dl::semantics::RegisterValues>(3);
        data->arguments = ygg::Index<kr::dl::semantics::CallArguments>(4);
        expect_cista_round_trip(*data);
    }
    {
        auto data = builder.template get_builder<kr::ps::ext::CallStack>();
        data->module = ygg::Index<kr::ps::ext::Module>(1);
        data->return_memory_state = ygg::Index<kr::ps::ext::MemoryState>(2);
        data->registers = ygg::Index<kr::dl::semantics::RegisterValues>(3);
        data->arguments = ygg::Index<kr::dl::semantics::CallArguments>(4);
        expect_cista_round_trip(*data);
        data->caller = ygg::Index<kr::ps::ext::CallStack>(7);
        expect_cista_round_trip(*data);
    }
    {
        auto data = builder.template get_builder<kr::ps::ext::ProgramState<Kind>>();
        data->program = ygg::Index<kr::ps::ext::Program>(8);
        data->module_state = ygg::Index<kr::ps::ext::ModuleState<Kind>>(6);
        expect_cista_round_trip(*data);
        data->call_stack = ygg::Index<kr::ps::ext::CallStack>(9);
        expect_cista_round_trip(*data);
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

    const auto program = create_program(*repository, caller, { caller, callee });
    auto& execution_builder = task_context->execution_builder;
    auto& dl_builder = task_context->dl_builder;
    auto& denotations = *task_context->dl_denotation_repository;
    auto execution_repository = task_context->execution_repository;

    const void* scratch_address = nullptr;
    {
        auto scratch = kr::dl::semantics::checkout<kr::dl::semantics::RegisterValues>(dl_builder);
        scratch_address = scratch.get();
        EXPECT_TRUE(scratch->concept_values.empty());
        EXPECT_TRUE(scratch->role_values.empty());
        EXPECT_TRUE(kr::dl::semantics::is_canonical(*scratch));
        scratch->concept_values.resize(6);
        scratch->concept_values[0] = ygg::Index<tyr::formalism::Object>(0);
    }
    {
        auto scratch = kr::dl::semantics::checkout<kr::dl::semantics::RegisterValues>(dl_builder);
        EXPECT_EQ(scratch.get(), scratch_address);
        EXPECT_TRUE(scratch->concept_values.empty());
        EXPECT_TRUE(scratch->role_values.empty());
    }

    const auto registers = [&]()
    {
        auto data = kr::dl::semantics::checkout<kr::dl::semantics::RegisterValues>(dl_builder);
        data->concept_values.resize(6);
        data->role_values.resize(7);
        data->concept_values[0] = ygg::Index<tyr::formalism::Object>(0);
        auto& role = data->role_values[0].emplace();
        role.first = ygg::Index<tyr::formalism::Object>(0);
        role.second = ygg::Index<tyr::formalism::Object>(1);
        canonicalize(*data);
        return denotations.get_or_create(*data).first;
    }();

    {
        auto data = kr::dl::semantics::checkout<kr::dl::semantics::RegisterValues>(dl_builder);
        data->concept_values.resize(6);
        data->role_values.resize(7);
        data->concept_values[0] = ygg::Index<tyr::formalism::Object>(0);
        auto& role = data->role_values[0].emplace();
        role.first = ygg::Index<tyr::formalism::Object>(0);
        role.second = ygg::Index<tyr::formalism::Object>(1);
        canonicalize(*data);
        const auto found = denotations.find(*data);
        ASSERT_TRUE(found);
        EXPECT_EQ(found->get_index(), registers.get_index());
        const auto [duplicate, created] = denotations.get_or_create(*data);
        EXPECT_FALSE(created);
        EXPECT_EQ(duplicate.get_index(), registers.get_index());
        EXPECT_EQ(&duplicate.get_context(), &denotations);
        data->concept_values[0] = ygg::Index<tyr::formalism::Object>(1);
        const auto changed = denotations.get_or_create(*data).first;
        EXPECT_NE(changed.get_index(), registers.get_index());
        data->clear();
        EXPECT_EQ(changed.get_concept_values()[0].value().get_index(), ygg::Index<tyr::formalism::Object>(1));
        EXPECT_EQ(registers.get_concept_values()[0].value().get_index(), ygg::Index<tyr::formalism::Object>(0));
        EXPECT_EQ(denotations.size<kr::dl::semantics::RegisterValues>(), 2);
    }

    const auto arguments = [&]()
    {
        auto data = kr::dl::semantics::checkout<kr::dl::semantics::CallArguments>(dl_builder);
        auto boolean = ygg::Data<kr::dl::semantics::Denotation<kr::dl::BooleanTag>>();
        boolean.value = true;
        data->boolean_arguments.push_back(denotations.get_or_create(boolean).first.get_index());
        canonicalize(*data);
        return denotations.get_or_create(*data).first;
    }();
    {
        auto data = kr::dl::semantics::checkout<kr::dl::semantics::CallArguments>(dl_builder);
        data->boolean_arguments = arguments.get_data().boolean_arguments;
        const auto found = denotations.find(*data);
        ASSERT_TRUE(found);
        EXPECT_EQ(found->get_index(), arguments.get_index());
        const auto [duplicate, created] = denotations.get_or_create(*data);
        EXPECT_FALSE(created);
        EXPECT_EQ(duplicate.get_index(), arguments.get_index());
        EXPECT_EQ(&duplicate.get_context(), &denotations);
        data->clear();
        const auto empty = denotations.get_or_create(*data).first;
        EXPECT_TRUE(empty.get<kr::dl::BooleanTag>().empty());
        EXPECT_TRUE(arguments.get<kr::dl::BooleanTag>()[0].get());
        EXPECT_EQ(denotations.size<kr::dl::semantics::CallArguments>(), 2);
    }

    const auto initial = search_context->successor_generator->get_initial_node(*search_context->state_repository, *search_context->axiom_evaluator);
    const auto state = initial.get_state();
    const auto caller_configuration = [&]()
    {
        auto data = execution_builder.get_builder<kr::ps::ext::ModuleState<tyr::GroundTag>>();
        ygg::set(state, data->state);
        ygg::set(caller, data->module);
        ygg::set(caller_return, data->memory_state);
        ygg::set(registers, data->registers);
        ygg::set(arguments, data->arguments);
        canonicalize(*data);
        return execution_repository->get_or_create(*data).first;
    }();

    const auto callee_configuration = [&]()
    {
        auto data = execution_builder.get_builder<kr::ps::ext::ModuleState<tyr::GroundTag>>();
        ygg::set(state, data->state);
        ygg::set(callee, data->module);
        ygg::set(callee_entry, data->memory_state);
        ygg::set(registers, data->registers);
        ygg::set(arguments, data->arguments);
        canonicalize(*data);
        return execution_repository->get_or_create(*data).first;
    }();

    {
        auto data = callee_configuration.get_data();
        const auto [duplicate, created] = execution_repository->get_or_create(data);
        EXPECT_FALSE(created);
        EXPECT_EQ(duplicate, callee_configuration);
        const auto found = execution_repository->find(data);
        ASSERT_TRUE(found);
        EXPECT_EQ(*found, callee_configuration);

        const auto successors =
            search_context->successor_generator->get_successor_nodes(initial, *search_context->state_repository, *search_context->axiom_evaluator);
        const auto changed = std::ranges::find_if(successors, [&](const auto& node) { return node.get_state().get_index() != state.get_index(); });
        ASSERT_NE(changed, successors.end());
        ygg::set(changed->get_state(), data.state);
        const auto [changed_configuration, changed_created] = execution_repository->get_or_create(data);
        EXPECT_TRUE(changed_created);
        EXPECT_NE(changed_configuration, callee_configuration);
        EXPECT_EQ(changed_configuration.get_state().get_index(), changed->get_state().get_index());
        EXPECT_EQ(callee_configuration.get_state().get_index(), state.get_index());
        EXPECT_EQ(execution_repository->size<kr::ps::ext::ModuleState<tyr::GroundTag>>(), 3);
    }

    const auto caller_frame = [&]()
    {
        auto data = execution_builder.get_builder<kr::ps::ext::CallStack>();
        ygg::set(caller, data->module);
        ygg::set(caller_return, data->return_memory_state);
        ygg::set(registers, data->registers);
        ygg::set(arguments, data->arguments);
        canonicalize(*data);
        return execution_repository->get_or_create(*data).first;
    }();
    const auto callee_frame = [&]()
    {
        auto data = execution_builder.get_builder<kr::ps::ext::CallStack>();
        ygg::set(callee, data->module);
        ygg::set(callee_entry, data->return_memory_state);
        ygg::set(registers, data->registers);
        ygg::set(arguments, data->arguments);
        ygg::set(std::optional { caller_frame }, data->caller);
        canonicalize(*data);
        return execution_repository->get_or_create(*data).first;
    }();
    {
        auto nested_caller_data = caller_frame.get_data();
        nested_caller_data.caller = caller_frame.get_index();
        const auto nested_caller = execution_repository->get_or_create(nested_caller_data).first;
        auto data = execution_builder.get_builder<kr::ps::ext::ProgramState<tyr::GroundTag>>();
        ygg::set(program, data->program);
        ygg::set(callee_configuration, data->module_state);
        data->call_stack = caller_frame.get_index();
        const auto first = execution_repository->get_or_create(*data).first;
        data->call_stack = nested_caller.get_index();
        const auto nested = execution_repository->get_or_create(*data).first;
        EXPECT_NE(nested, first);
        EXPECT_EQ(nested.get_module_state(), first.get_module_state());
        EXPECT_EQ(nested.get_call_stack().value(), nested_caller);
        const auto [duplicate, created] = execution_repository->get_or_create(*data);
        EXPECT_FALSE(created);
        EXPECT_EQ(duplicate, nested);
        EXPECT_EQ(execution_repository->size<kr::ps::ext::ModuleState<tyr::GroundTag>>(), 3);
    }

    ASSERT_TRUE(callee_frame.get_caller());
    EXPECT_EQ(callee_frame.get_caller()->get_index(), caller_frame.get_index());
    EXPECT_EQ(callee_frame.get_caller()->get_registers().get_concept_values()[0].value().get_index(), ygg::Index<tyr::formalism::Object>(0));
    const auto role_value = callee_frame.get_caller()->get_registers().get_role_values()[0];
    ASSERT_TRUE(role_value);
    const auto pair = role_value.value();
    EXPECT_EQ(pair.get_first().get_index(), ygg::Index<tyr::formalism::Object>(0));
    EXPECT_EQ(pair.get_second().get_index(), ygg::Index<tyr::formalism::Object>(1));

    const auto returned_state = [&]()
    {
        auto data = execution_builder.get_builder<kr::ps::ext::ProgramState<tyr::GroundTag>>();
        ygg::set(program, data->program);
        ygg::set(caller_configuration, data->module_state);
        canonicalize(*data);
        return execution_repository->get_or_create(*data).first;
    }();
    EXPECT_EQ(returned_state.get_module_state().get_module().get_name(), "caller");
    EXPECT_EQ(returned_state.get_program().get_index(), program.get_index());
    EXPECT_EQ(returned_state.get_module_state(), caller_configuration);
    EXPECT_EQ(returned_state.get_state().get_index(), state.get_index());
    EXPECT_FALSE(returned_state.get_call_stack());
}

TEST(RunirTests, ExtExecutionRecordsAreCistaCompatible)
{
    auto dl_builder = kr::dl::semantics::Builder();
    {
        auto data = kr::dl::semantics::checkout<kr::dl::semantics::RegisterValues>(dl_builder);
        data->concept_values.resize(6);
        data->role_values.resize(7);
        data->concept_values[0] = ygg::Index<tyr::formalism::Object>(3);
        auto& role = data->role_values[0].emplace();
        role.first = ygg::Index<tyr::formalism::Object>(4);
        role.second = ygg::Index<tyr::formalism::Object>(5);
        data->concept_values[5] = ygg::Index<tyr::formalism::Object>(6);
        data->role_values[6] = role;
        expect_cista_round_trip(*data);
    }
    {
        auto data = kr::dl::semantics::checkout<kr::dl::semantics::CallArguments>(dl_builder);
        data->concept_arguments.push_back(ygg::Index<kr::dl::semantics::Denotation<kr::dl::ConceptTag>>(1));
        data->role_arguments.push_back(ygg::Index<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(2));
        data->boolean_arguments.push_back(ygg::Index<kr::dl::semantics::Denotation<kr::dl::BooleanTag>>(3));
        data->numerical_arguments.push_back(ygg::Index<kr::dl::semantics::Denotation<kr::dl::NumericalTag>>(4));
        expect_cista_round_trip(*data);
    }
    expect_execution_records_round_trip<tyr::GroundTag>();
    expect_execution_records_round_trip<tyr::LiftedTag>();
}

}  // namespace runir::tests
