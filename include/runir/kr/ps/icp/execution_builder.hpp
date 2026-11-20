#ifndef RUNIR_KR_PS_ICP_EXECUTION_BUILDER_HPP_
#define RUNIR_KR_PS_ICP_EXECUTION_BUILDER_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/ps/icp/control_view.hpp"
#include "runir/kr/ps/icp/execution_data.hpp"
#include "runir/kr/ps/icp/execution_view.hpp"

#include <optional>
#include <tyr/planning/declarations.hpp>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of Histories: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::ps::icp::Histories, C>
{
    urd::index<runir::kr::ps::icp::Histories> index;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>>>,
                 ::runir::kr::dl::semantics::DenotationRepository>
        concepts;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"concepts", &builder::concepts>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::concepts> {}; }
};

/// The builder of ProgramState: its record's members, each in its built form (urd::built_t).
template<tyr::TaskKind Kind, typename C>
struct builder<runir::kr::ps::icp::ProgramState<Kind>, C>
{
    urd::index<runir::kr::ps::icp::ProgramState<Kind>> index;
    urd::built_t<urd::index<runir::kr::ps::icp::Program>, ::runir::kr::ps::icp::Repository> program;
    urd::built_t<urd::index<runir::kr::ps::MemoryState<runir::kr::IcpFamilyTag>>, ::runir::kr::ps::icp::Repository> memory_state;
    urd::built_t<urd::index<runir::kr::dl::semantics::RegisterValues>, ::runir::kr::dl::semantics::DenotationRepository> registers;
    urd::built_t<urd::index<runir::kr::ps::icp::Histories>, ::runir::kr::ps::icp::ControlRepository> histories;
    urd::built_t<urd::index<tyr::planning::State<Kind>>, ::tyr::planning::StateRepository<Kind>> state;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"program", &builder::program>,
                           urd::field<"memory_state", &builder::memory_state>,
                           urd::field<"registers", &builder::registers>,
                           urd::field<"histories", &builder::histories>,
                           urd::field<"state", &builder::state>> {};
    }

    static constexpr auto identifying_members() noexcept
    { return urd::Members<&builder::program, &builder::memory_state, &builder::registers, &builder::histories, &builder::state> {}; }
};

}  // namespace urd

#endif
