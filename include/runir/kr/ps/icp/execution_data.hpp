#ifndef RUNIR_KR_PS_ICP_EXECUTION_DATA_HPP_
#define RUNIR_KR_PS_ICP_EXECUTION_DATA_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/icp/declarations.hpp"
#include "runir/kr/ps/icp/execution_declarations.hpp"

#include <tuple>
#include <tyr/planning/declarations.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/core/types.hpp>
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

    Data() = default;
    Data(IndexList<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>> concepts_) : index(), concepts(std::move(concepts_)) {}
    template<typename C>
    Data(const std::vector<::ygg::View<Index<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>>, C>>& concepts_) : index(), concepts()
    {
        set(concepts_, concepts);
    }

    auto cista_members() noexcept { return std::tie(index, concepts); }
    auto cista_members() const noexcept { return std::tie(index, concepts); }
    auto identifying_members() const noexcept { return std::tie(concepts); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
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
    // The referenced indices live in different repositories, hence the separate context parameters.
    Data(Index<runir::kr::ps::icp::Program> program_,
         Index<runir::kr::ps::icp::MemoryState> memory_state_,
         Index<runir::kr::dl::semantics::RegisterValues> registers_,
         Index<runir::kr::ps::icp::Histories> histories_,
         Index<tyr::planning::State<Kind>> state_) :
        index(),
        program(std::move(program_)),
        memory_state(std::move(memory_state_)),
        registers(std::move(registers_)),
        histories(std::move(histories_)),
        state(std::move(state_))
    {
    }
    template<typename P, typename D, typename S>
    Data(::ygg::View<Index<runir::kr::ps::icp::Program>, P> program_,
         ::ygg::View<Index<runir::kr::ps::icp::MemoryState>, P> memory_state_,
         ::ygg::View<Index<runir::kr::dl::semantics::RegisterValues>, D> registers_,
         ::ygg::View<Index<runir::kr::ps::icp::Histories>, D> histories_,
         ::ygg::View<Index<tyr::planning::State<Kind>>, S> state_) :
        index(),
        program(),
        memory_state(),
        registers(),
        histories(),
        state()
    {
        set(program_, program);
        set(memory_state_, memory_state);
        set(registers_, registers);
        set(histories_, histories);
        set(state_, state);
    }

    auto cista_members() noexcept { return std::tie(index, program, memory_state, registers, histories, state); }
    auto cista_members() const noexcept { return std::tie(index, program, memory_state, registers, histories, state); }
    auto identifying_members() const noexcept { return std::tie(program, memory_state, registers, histories, state); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}  // namespace ygg

#endif
