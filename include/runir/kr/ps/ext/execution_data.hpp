#ifndef RUNIR_KR_PS_EXT_EXECUTION_DATA_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_DATA_HPP_

#include "runir/kr/dl/semantics/denotation_index.hpp"
#include "runir/kr/ps/ext/execution_index.hpp"
#include "runir/kr/ps/ext/memory_state_index.hpp"
#include "runir/kr/ps/ext/module_index.hpp"

#include <array>
#include <cista/containers/optional.h>
#include <tuple>
#include <tyr/formalism/object_index.hpp>
#include <tyr/planning/state_index.hpp>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::ps::ext::RegisterValues>
{
    using ObjectIndex = Index<tyr::formalism::Object>;
    using ConceptValue = ::cista::optional<ObjectIndex>;
    using RoleValue = ::cista::optional<std::pair<ObjectIndex, ObjectIndex>>;

    Index<runir::kr::ps::ext::RegisterValues> index;
    std::array<ConceptValue, runir::kr::dl::num_registers> concept_values;
    std::array<RoleValue, runir::kr::dl::num_registers> role_values;

    Data() = default;
    Data(std::array<ConceptValue, runir::kr::dl::num_registers> concept_values_, std::array<RoleValue, runir::kr::dl::num_registers> role_values_) noexcept :
        concept_values(std::move(concept_values_)),
        role_values(std::move(role_values_))
    {
    }

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(concept_values);
        ygg::clear(role_values);
    }

    auto cista_members() const noexcept { return std::tie(index, concept_values, role_values); }
    auto identifying_members() const noexcept { return std::tie(concept_values, role_values); }
};

template<>
struct Data<runir::kr::ps::ext::CallArguments>
{
    Index<runir::kr::ps::ext::CallArguments> index;
    IndexList<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>> concept_arguments;
    IndexList<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>> role_arguments;
    IndexList<runir::kr::dl::semantics::Denotation<runir::kr::dl::BooleanTag>> boolean_arguments;
    IndexList<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>> numerical_arguments;

    Data() = default;

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(concept_arguments);
        ygg::clear(role_arguments);
        ygg::clear(boolean_arguments);
        ygg::clear(numerical_arguments);
    }

    auto cista_members() const noexcept { return std::tie(index, concept_arguments, role_arguments, boolean_arguments, numerical_arguments); }
    auto identifying_members() const noexcept { return std::tie(concept_arguments, role_arguments, boolean_arguments, numerical_arguments); }
};

template<>
struct Data<runir::kr::ps::ext::CallStack>
{
    Index<runir::kr::ps::ext::CallStack> index;
    Index<runir::kr::ps::ext::Module> module;
    Index<runir::kr::ps::ext::MemoryState> memory_state;
    Index<runir::kr::ps::ext::RegisterValues> registers;
    Index<runir::kr::ps::ext::CallArguments> arguments;
    ::cista::optional<Index<runir::kr::ps::ext::CallStack>> caller;

    Data() = default;
    Data(Index<runir::kr::ps::ext::Module> module_,
         Index<runir::kr::ps::ext::MemoryState> memory_state_,
         Index<runir::kr::ps::ext::RegisterValues> registers_,
         Index<runir::kr::ps::ext::CallArguments> arguments_,
         ::cista::optional<Index<runir::kr::ps::ext::CallStack>> caller_ = {}) noexcept :
        module(module_),
        memory_state(memory_state_),
        registers(registers_),
        arguments(arguments_),
        caller(caller_)
    {
    }

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(module);
        ygg::clear(memory_state);
        ygg::clear(registers);
        ygg::clear(arguments);
        ygg::clear(caller);
    }

    auto cista_members() const noexcept { return std::tie(index, module, memory_state, registers, arguments, caller); }
    auto identifying_members() const noexcept { return std::tie(module, memory_state, registers, arguments, caller); }
};

template<tyr::planning::TaskKind Kind>
struct Data<runir::kr::ps::ext::ExecutionState<Kind>>
{
    Index<runir::kr::ps::ext::ExecutionState<Kind>> index;
    Index<tyr::planning::State<Kind>> state;
    Index<runir::kr::ps::ext::CallStack> call_stack;
    runir::kr::ps::ext::ExecutionPhase phase = runir::kr::ps::ext::ExecutionPhase::EXTERNAL;

    Data() = default;
    Data(Index<tyr::planning::State<Kind>> state_, Index<runir::kr::ps::ext::CallStack> call_stack_, runir::kr::ps::ext::ExecutionPhase phase_) noexcept :
        state(state_),
        call_stack(call_stack_),
        phase(phase_)
    {
    }

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(state);
        ygg::clear(call_stack);
        phase = runir::kr::ps::ext::ExecutionPhase::EXTERNAL;
    }

    auto cista_members() const noexcept { return std::tie(index, state, call_stack, phase); }
    auto identifying_members() const noexcept { return std::tie(state, call_stack, phase); }
};

}  // namespace ygg

#endif
