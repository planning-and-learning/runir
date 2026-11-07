#include "module_fixtures.hpp"
#include "planning_fixtures.hpp"

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
        auto data = builder.template checkout<kr::ps::ext::ModuleState<Kind>>();
        data->state = urd::index<tyr::planning::State<Kind>>(5);
        data->module_ = urd::index<kr::ps::ext::Module>(1);
        data->memory_state = urd::index<kr::ps::MemoryState<kr::ExtFamilyTag>>(2);
        data->registers = urd::index<kr::dl::semantics::RegisterValues>(3);
        data->arguments = urd::index<kr::dl::semantics::CallArguments>(4);
        expect_cista_round_trip(*data);
    }
    {
        auto data = builder.template checkout<kr::ps::ext::CallStack>();
        data->module_ = urd::index<kr::ps::ext::Module>(1);
        data->return_memory_state = urd::index<kr::ps::MemoryState<kr::ExtFamilyTag>>(2);
        data->registers = urd::index<kr::dl::semantics::RegisterValues>(3);
        data->arguments = urd::index<kr::dl::semantics::CallArguments>(4);
        expect_cista_round_trip(*data);
        data->caller = urd::index<kr::ps::ext::CallStack>(7);
        expect_cista_round_trip(*data);
    }
    {
        auto data = builder.template checkout<kr::ps::ext::ProgramState<Kind>>();
        data->program = urd::index<kr::ps::ext::Program>(8);
        data->module_state = urd::index<kr::ps::ext::ModuleState<Kind>>(6);
        expect_cista_round_trip(*data);
        data->call_stack = urd::index<kr::ps::ext::CallStack>(9);
        expect_cista_round_trip(*data);
    }
    {
        auto data = builder.template get_builder<kr::ps::ext::ProgramState<Kind>>();
        EXPECT_EQ(data->program, urd::index<kr::ps::ext::Program>(8));
        ASSERT_TRUE(data->call_stack);
        EXPECT_EQ(*data->call_stack, urd::index<kr::ps::ext::CallStack>(9));
    }
    {
        auto data = kr::ps::ext::checkout<kr::ps::ext::ProgramState<Kind>>(builder);
        EXPECT_EQ(data->program, urd::index<kr::ps::ext::Program>());
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
        scratch->concept_values[0] = urd::index<tyr::formalism::Object>(0);
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
        auto data = kr::dl::semantics::checkout<kr::dl::semantics::RegisterValues>(dl_builder);
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
        urd::clear_members(*data);
        EXPECT_EQ(changed.template values<kr::dl::ConceptTag>()[0].value().index(), urd::index<tyr::formalism::Object>(1));
        EXPECT_EQ(registers.template values<kr::dl::ConceptTag>()[0].value().index(), urd::index<tyr::formalism::Object>(0));
        EXPECT_EQ(denotations.size<kr::dl::semantics::RegisterValues>(), 2);
    }

    const auto arguments = [&]()
    {
        auto data = kr::dl::semantics::checkout<kr::dl::semantics::CallArguments>(dl_builder);
        auto boolean = urd::data<kr::dl::semantics::Denotation<kr::dl::BooleanTag>>();
        boolean.value = true;
        data->boolean_arguments.push_back(denotations.insert(boolean).first.index());
        canonicalize(*data);
        return denotations.insert(*data).first;
    }();
    {
        auto data = kr::dl::semantics::checkout<kr::dl::semantics::CallArguments>(dl_builder);
        data->boolean_arguments = arguments.data().boolean_arguments;
        const auto found = denotations.find(*data);
        ASSERT_TRUE(found);
        EXPECT_EQ(found.value().index(), arguments.index());
        const auto [duplicate, created] = denotations.insert(*data);
        EXPECT_FALSE(created);
        EXPECT_EQ(duplicate.index(), arguments.index());
        EXPECT_EQ(&duplicate.context(), &denotations);
        urd::clear_members(*data);
        const auto empty = denotations.insert(*data).first;
        EXPECT_TRUE(empty.arguments<kr::dl::BooleanTag>().empty());
        EXPECT_TRUE(arguments.arguments<kr::dl::BooleanTag>()[0].value());
        EXPECT_EQ(denotations.size<kr::dl::semantics::CallArguments>(), 2);
    }

    const auto initial = search_context->successor_generator->get_initial_node(*search_context->state_repository, *search_context->axiom_evaluator);
    const auto& state = initial.get_state();
    const auto caller_configuration = [&]()
    {
        auto data = execution_builder.checkout<kr::ps::ext::ModuleState<tyr::GroundTag>>();
        data->state = urd::to_stored(state);
        data->module_ = urd::to_stored(caller);
        data->memory_state = urd::to_stored(caller_return);
        data->registers = urd::to_stored(registers);
        data->arguments = urd::to_stored(arguments);
        return execution_repository->insert(*data).first;
    }();

    const auto callee_configuration = [&]()
    {
        auto data = execution_builder.checkout<kr::ps::ext::ModuleState<tyr::GroundTag>>();
        data->state = urd::to_stored(state);
        data->module_ = urd::to_stored(callee);
        data->memory_state = urd::to_stored(callee_entry);
        data->registers = urd::to_stored(registers);
        data->arguments = urd::to_stored(arguments);
        return execution_repository->insert(*data).first;
    }();

    {
        auto data = callee_configuration.data();
        data.index = urd::index<kr::ps::ext::ModuleState<tyr::GroundTag>>();
        const auto [duplicate, created] = execution_repository->insert(data);
        EXPECT_FALSE(created);
        EXPECT_EQ(duplicate, callee_configuration);
        EXPECT_EQ(data.index, duplicate.index());
        EXPECT_EQ(&duplicate.context(), execution_repository.get());
        EXPECT_EQ(&duplicate.get_module().context(), repository.get());
        EXPECT_EQ(&duplicate.get_registers().context(), &denotations);
        const auto found = execution_repository->find(data);
        ASSERT_TRUE(found);
        EXPECT_EQ(found.value(), callee_configuration);

        const auto successors =
            search_context->successor_generator->get_successor_nodes(initial, *search_context->state_repository, *search_context->axiom_evaluator);
        const auto changed = std::ranges::find_if(successors, [&](const auto& node) { return node.get_state().index() != state.index(); });
        ASSERT_NE(changed, successors.end());
        data.state = urd::to_stored(changed->get_state());
        const auto [changed_configuration, changed_created] = execution_repository->insert(data);
        EXPECT_TRUE(changed_created);
        EXPECT_EQ(data.index, changed_configuration.index());
        EXPECT_EQ(&changed_configuration.context(), execution_repository.get());
        EXPECT_NE(changed_configuration, callee_configuration);
        EXPECT_EQ(changed_configuration.get_state().index(), changed->get_state().index());
        EXPECT_EQ(callee_configuration.get_state().index(), state.index());
        EXPECT_EQ(execution_repository->size<kr::ps::ext::ModuleState<tyr::GroundTag>>(), 3);
    }

    const auto caller_frame = [&]()
    {
        auto data = execution_builder.checkout<kr::ps::ext::CallStack>();
        data->module_ = urd::to_stored(caller);
        data->return_memory_state = urd::to_stored(caller_return);
        data->registers = urd::to_stored(registers);
        data->arguments = urd::to_stored(arguments);
        return execution_repository->insert(*data).first;
    }();
    const auto callee_frame = [&]()
    {
        auto data = execution_builder.checkout<kr::ps::ext::CallStack>();
        data->module_ = urd::to_stored(callee);
        data->return_memory_state = urd::to_stored(callee_entry);
        data->registers = urd::to_stored(registers);
        data->arguments = urd::to_stored(arguments);
        data->caller = urd::to_stored(std::optional { caller_frame });
        return execution_repository->insert(*data).first;
    }();
    {
        auto nested_caller_data = caller_frame.data();
        nested_caller_data.caller = caller_frame.index();
        const auto nested_caller = execution_repository->insert(nested_caller_data).first;
        auto data = execution_builder.checkout<kr::ps::ext::ProgramState<tyr::GroundTag>>();
        data->program = urd::to_stored(program);
        data->module_state = urd::to_stored(callee_configuration);
        data->call_stack = caller_frame.index();
        const auto first = execution_repository->insert(*data).first;
        data->call_stack = nested_caller.index();
        const auto nested = execution_repository->insert(*data).first;
        EXPECT_NE(nested, first);
        EXPECT_EQ(nested.get_module_state(), first.get_module_state());
        EXPECT_EQ(nested.get_call_stack().value(), nested_caller);
        const auto [duplicate, created] = execution_repository->insert(*data);
        EXPECT_FALSE(created);
        EXPECT_EQ(duplicate, nested);
        EXPECT_EQ(execution_repository->size<kr::ps::ext::ModuleState<tyr::GroundTag>>(), 3);
    }

    ASSERT_TRUE(callee_frame.get_caller());
    EXPECT_EQ(callee_frame.get_caller().value().index(), caller_frame.index());
    EXPECT_EQ(callee_frame.get_caller().value().get_registers().template values<kr::dl::ConceptTag>()[0].value().index(),
              urd::index<tyr::formalism::Object>(0));
    const auto role_value = callee_frame.get_caller().value().get_registers().template values<kr::dl::RoleTag>()[0];
    ASSERT_TRUE(role_value);
    const auto pair = role_value.value();
    EXPECT_EQ(pair.first.index(), urd::index<tyr::formalism::Object>(0));
    EXPECT_EQ(pair.second.index(), urd::index<tyr::formalism::Object>(1));

    const auto returned_state = [&]()
    {
        auto data = execution_builder.checkout<kr::ps::ext::ProgramState<tyr::GroundTag>>();
        data->program = urd::to_stored(program);
        data->module_state = urd::to_stored(caller_configuration);
        return execution_repository->insert(*data).first;
    }();
    EXPECT_EQ(returned_state.get_module_state().get_module().get_name(), "caller");
    EXPECT_EQ(returned_state.get_program().index(), program.index());
    EXPECT_EQ(returned_state.get_module_state(), caller_configuration);
    EXPECT_EQ(returned_state.get_state().index(), state.index());
    EXPECT_FALSE(returned_state.get_call_stack());
}

TEST(RunirTests, ExtExecutionRecordsAreCistaCompatible)
{
    auto dl_builder = kr::dl::semantics::Builder();
    {
        auto data = kr::dl::semantics::checkout<kr::dl::semantics::RegisterValues>(dl_builder);
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
        auto data = kr::dl::semantics::checkout<kr::dl::semantics::CallArguments>(dl_builder);
        data->concept_arguments.push_back(urd::index<kr::dl::semantics::Denotation<kr::dl::ConceptTag>>(1));
        data->role_arguments.push_back(urd::index<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(2));
        data->boolean_arguments.push_back(urd::index<kr::dl::semantics::Denotation<kr::dl::BooleanTag>>(3));
        data->numerical_arguments.push_back(urd::index<kr::dl::semantics::Denotation<kr::dl::NumericalTag>>(4));
        expect_cista_round_trip(*data);
    }
    expect_execution_records_round_trip<tyr::GroundTag>();
    expect_execution_records_round_trip<tyr::LiftedTag>();
}

}  // namespace runir::tests
