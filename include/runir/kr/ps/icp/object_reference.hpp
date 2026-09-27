#ifndef RUNIR_KR_PS_ICP_OBJECT_REFERENCE_HPP_
#define RUNIR_KR_PS_ICP_OBJECT_REFERENCE_HPP_

#include "runir/kr/dl/register_index.hpp"
#include "runir/kr/ps/feature_index.hpp"

#include <tuple>
#include <yggdrasil/ids/uint_mixins.hpp>

namespace runir::kr::ps::icp
{

struct ArgumentPosition : ygg::FixedUintMixin<ArgumentPosition>
{
    using Base = ygg::FixedUintMixin<ArgumentPosition>;
    using Base::Base;
};

struct ResetPair
{
    ygg::Index<ps::Feature<IcpFamilyTag, runir::kr::dl::ConceptTag>> before;
    ygg::Index<ps::Feature<IcpFamilyTag, runir::kr::dl::ConceptTag>> after;

    auto cista_members() const noexcept { return std::tie(before, after); }
    auto identifying_members() const noexcept { return std::tie(before, after); }
    bool operator==(const ResetPair&) const = default;
    bool operator<(const ResetPair& other) const noexcept { return std::tie(before, after) < std::tie(other.before, other.after); }
};

}

#endif
