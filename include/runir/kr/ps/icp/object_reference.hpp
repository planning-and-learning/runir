#ifndef RUNIR_KR_PS_ICP_OBJECT_REFERENCE_HPP_
#define RUNIR_KR_PS_ICP_OBJECT_REFERENCE_HPP_

#include <yggdrasil/core/unsigned_integer.hpp>
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"

#include <tuple>
#include <urd/elements/flat.hpp>
#include <urd/elements/index.hpp>

namespace runir::kr::ps::icp
{

/// An argument's position: a nonnegative integer of its own type (no index: nothing resolves it).
struct ArgumentPosition : ygg::UnsignedInteger<ArgumentPosition>
{
    using UnsignedInteger::UnsignedInteger;
};

struct ResetPair
{
    urd::index<ps::Feature<IcpFamilyTag, runir::kr::dl::ConceptTag>> before;
    urd::index<ps::Feature<IcpFamilyTag, runir::kr::dl::ConceptTag>> after;

    static constexpr auto cista_members() noexcept { return urd::Members<&ResetPair::before, &ResetPair::after> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&ResetPair::before, &ResetPair::after> {}; }
    bool operator==(const ResetPair&) const = default;
    bool operator<(const ResetPair& other) const noexcept { return std::tie(before, after) < std::tie(other.before, other.after); }
};

}

#endif
