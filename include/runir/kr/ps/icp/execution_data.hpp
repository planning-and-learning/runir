#ifndef RUNIR_KR_PS_ICP_EXECUTION_DATA_HPP_
#define RUNIR_KR_PS_ICP_EXECUTION_DATA_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/icp/declarations.hpp"
#include "runir/kr/ps/icp/execution_declarations.hpp"

#include <tuple>
#include <type_traits>
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
    urd::key<runir::kr::ps::icp::Histories> index;
    urd::flat_vector<urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>>> concepts;

    data() = default;

    data(urd::flat_vector<urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>>> concepts_) : index(), concepts(std::move(concepts_)) {}

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"concepts", &data::concepts>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&data::concepts> {}; }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::concepts> {}; }

    [[nodiscard]] urd::key<runir::kr::ps::icp::Histories> key() const noexcept { return urd::key<runir::kr::ps::icp::Histories>(index); }
};

template<tyr::TaskKind Kind>
struct data<runir::kr::ps::icp::ProgramState<Kind>>
{
    urd::key<runir::kr::ps::icp::ProgramState<Kind>> index;
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

    static constexpr bool copyable() noexcept
    {
        static_assert(std::is_trivially_copyable_v<data>, "a copyable record is trivially copyable");
        return true;
    }

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"program", &data::program>,
                           urd::field<"memory_state", &data::memory_state>,
                           urd::field<"registers", &data::registers>,
                           urd::field<"histories", &data::histories>,
                           urd::field<"state", &data::state>> {};
    }

    static constexpr auto identifying_members() noexcept
    { return urd::Members<&data::program, &data::memory_state, &data::registers, &data::histories, &data::state> {}; }

    static constexpr auto cista_members() noexcept
    { return urd::Members<&data::index, &data::program, &data::memory_state, &data::registers, &data::histories, &data::state> {}; }

    [[nodiscard]] urd::key<runir::kr::ps::icp::ProgramState<Kind>> key() const noexcept { return urd::key<runir::kr::ps::icp::ProgramState<Kind>>(index); }
};

}  // namespace urd

#endif
