#ifndef RUNIR_KR_PS_ICP_EXECUTION_DATA_HPP_
#define RUNIR_KR_PS_ICP_EXECUTION_DATA_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/icp/declarations.hpp"
#include "runir/kr/ps/icp/execution_declarations.hpp"

#include <tuple>
#include <tyr/planning/declarations.hpp>
#include <urd/elements/flat.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/core/config.hpp>

namespace urd
{

template<>
struct data<runir::kr::ps::icp::Histories>
{
    urd::index<runir::kr::ps::icp::Histories> index;
    urd::flat_vector<urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>>> concepts;

    data() = default;
    data(urd::flat_vector<urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>>> concepts_) : index(), concepts(std::move(concepts_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::concepts> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::concepts> {}; }
};

template<tyr::TaskKind Kind>
struct data<runir::kr::ps::icp::ProgramState<Kind>>
{
    urd::index<runir::kr::ps::icp::ProgramState<Kind>> index;
    urd::index<runir::kr::ps::icp::Program> program;
    urd::index<runir::kr::ps::MemoryState<runir::kr::IcpFamilyTag>> memory_state;
    urd::index<runir::kr::dl::semantics::RegisterValues> registers;
    urd::index<runir::kr::ps::icp::Histories> histories;
    urd::index<tyr::planning::State<Kind>> state;

    data() = default;
    // The referenced indices live in different repositories, hence the separate context parameters.
    data(urd::index<runir::kr::ps::icp::Program> program_,
         urd::index<runir::kr::ps::MemoryState<runir::kr::IcpFamilyTag>> memory_state_,
         urd::index<runir::kr::dl::semantics::RegisterValues> registers_,
         urd::index<runir::kr::ps::icp::Histories> histories_,
         urd::index<tyr::planning::State<Kind>> state_) :
        index(),
        program(program_),
        memory_state(memory_state_),
        registers(registers_),
        histories(histories_),
        state(std::move(state_))
    {
    }

    static constexpr auto cista_members() noexcept
    { return urd::Members<&data::index, &data::program, &data::memory_state, &data::registers, &data::histories, &data::state> {}; }
    static constexpr auto identifying_members() noexcept
    { return urd::Members<&data::program, &data::memory_state, &data::registers, &data::histories, &data::state> {}; }
};

}  // namespace urd

#endif
