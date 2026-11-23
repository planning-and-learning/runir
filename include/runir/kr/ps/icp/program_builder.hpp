#ifndef RUNIR_KR_PS_ICP_PROGRAM_BUILDER_HPP_
#define RUNIR_KR_PS_ICP_PROGRAM_BUILDER_HPP_

#include "runir/kr/ps/icp/program_data.hpp"
#include "runir/kr/ps/icp/program_view.hpp"

#include <optional>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of Program: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::ps::icp::Program, C>
{
    urd::key<runir::kr::ps::icp::Program> index;
    urd::built_t<urd::index<runir::kr::ps::icp::Module>, C> entry_mod;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"entry_mod", &builder::entry_mod>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::entry_mod> {}; }
};

}  // namespace urd

#endif
