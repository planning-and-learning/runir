#ifndef RUNIR_KR_PS_ICP_RESET_PAIR_DATA_HPP_
#define RUNIR_KR_PS_ICP_RESET_PAIR_DATA_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/icp/declarations.hpp"

#include <type_traits>
#include <urd/elements/flat.hpp>
#include <urd/elements/index.hpp>

namespace urd
{

/// A reset pair: the concept feature whose change resets, and the one it resets.
template<>
struct data<runir::kr::ps::icp::ResetPair>
{
    urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>> before;
    urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>> after;

    data() = default;

    data(urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>> before_,
         urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>> after_) :
        before(before_),
        after(after_)
    {
    }

    static constexpr bool copyable() noexcept
    {
        static_assert(std::is_trivially_copyable_v<data>, "a copyable record is trivially copyable");
        return true;
    }

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"before", &data::before>, urd::field<"after", &data::after>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::before, &data::after> {}; }
};

}  // namespace urd

#endif
