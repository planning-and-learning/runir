#ifndef RUNIR_KR_PS_ICP_RESET_PAIR_BUILDER_HPP_
#define RUNIR_KR_PS_ICP_RESET_PAIR_BUILDER_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/icp/reset_pair_data.hpp"
#include "runir/kr/ps/icp/reset_pair_view.hpp"

#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>

namespace urd
{

/// The builder of a reset pair: its features, each a child.
template<typename C>
struct builder<runir::kr::ps::icp::ResetPair, C>
{
    urd::built_t<urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>>, C> before;
    urd::built_t<urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>>, C> after;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"before", &builder::before>, urd::field<"after", &builder::after>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

}  // namespace urd

#endif
