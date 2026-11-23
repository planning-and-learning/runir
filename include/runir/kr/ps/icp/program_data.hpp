#ifndef RUNIR_KR_PS_ICP_PROGRAM_DATA_HPP_
#define RUNIR_KR_PS_ICP_PROGRAM_DATA_HPP_

#include "runir/kr/ps/icp/declarations.hpp"

#include <tuple>
#include <type_traits>
#include <urd/elements/flat.hpp>
#include <utility>

namespace urd
{

template<>
struct data<runir::kr::ps::icp::Program>
{
    urd::key<runir::kr::ps::icp::Program> index;
    urd::index<runir::kr::ps::icp::Module> entry_mod;

    data() = default;

    data(urd::index<runir::kr::ps::icp::Module> entry_mod_) : index(), entry_mod(entry_mod_) {}

    static constexpr bool copyable() noexcept
    {
        static_assert(std::is_trivially_copyable_v<data>, "a copyable record is trivially copyable");
        return true;
    }

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"entry_mod", &data::entry_mod>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&data::entry_mod> {}; }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::entry_mod> {}; }

    [[nodiscard]] urd::key<runir::kr::ps::icp::Program> key() const noexcept { return urd::key<runir::kr::ps::icp::Program>(index); }
};

}  // namespace urd

#endif
