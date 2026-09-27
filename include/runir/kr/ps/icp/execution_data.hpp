#ifndef RUNIR_KR_PS_ICP_EXECUTION_DATA_HPP_
#define RUNIR_KR_PS_ICP_EXECUTION_DATA_HPP_

#include "runir/kr/dl/semantics/denotation_index.hpp"
#include "runir/kr/dl/semantics/register_values_index.hpp"
#include "runir/kr/ps/icp/execution_index.hpp"
#include "runir/kr/ps/icp/indices.hpp"

#include <tuple>
#include <tyr/planning/state_index.hpp>
#include <yggdrasil/core/types_utils.hpp>
#include <yggdrasil/serialization/cista_equal_to.hpp>
#include <yggdrasil/serialization/cista_hash.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::ps::icp::Histories>
{
    Index<runir::kr::ps::icp::Histories> index;
    IndexList<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>> concepts;

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(concepts);
    }

    auto cista_members() const noexcept { return std::tie(index, concepts); }
    auto identifying_members() const noexcept { return std::tie(concepts); }
};

template<tyr::TaskKind Kind>
struct Data<runir::kr::ps::icp::ProgramState<Kind>>
{
    Index<runir::kr::ps::icp::ProgramState<Kind>> index;
    Index<runir::kr::ps::icp::Program> program;
    Index<runir::kr::ps::icp::MemoryState> memory_state;
    Index<runir::kr::dl::semantics::RegisterValues> registers;
    Index<runir::kr::ps::icp::Histories> histories;
    Index<tyr::planning::State<Kind>> state;

    Data() = default;
    Data(Index<runir::kr::ps::icp::Program> program_,
         Index<runir::kr::ps::icp::MemoryState> memory_state_,
         Index<runir::kr::dl::semantics::RegisterValues> registers_,
         Index<runir::kr::ps::icp::Histories> histories_,
         Index<tyr::planning::State<Kind>> state_) noexcept :
        program(program_),
        memory_state(memory_state_),
        registers(registers_),
        histories(histories_),
        state(state_)
    {
    }

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(program);
        ygg::clear(memory_state);
        ygg::clear(registers);
        ygg::clear(histories);
        ygg::clear(state);
    }

    auto cista_members() const noexcept { return std::tie(index, program, memory_state, registers, histories, state); }
    auto identifying_members() const noexcept { return std::tie(program, memory_state, registers, histories, state); }
};

}  // namespace ygg

#endif
