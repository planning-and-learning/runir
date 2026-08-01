#include "module_fixtures.hpp"
#include "planning_fixtures.hpp"

#include <cista/serialization.h>
#include <gtest/gtest.h>
#include <optional>
#include <runir/kr/ps/ext/execution_builder.hpp>
#include <runir/kr/ps/ext/execution_repository.hpp>
#include <runir/kr/ps/ext/execution_view.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/task_context.hpp>
#include <string>

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

}  // namespace

TEST(RunirTests, ExtExecutionRepositoryPersistsRecordsAndSharesCallers)
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

}  // namespace runir::tests
