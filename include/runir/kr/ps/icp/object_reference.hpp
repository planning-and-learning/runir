#ifndef RUNIR_KR_PS_ICP_OBJECT_REFERENCE_HPP_
#define RUNIR_KR_PS_ICP_OBJECT_REFERENCE_HPP_

#include "runir/kr/dl/register_index.hpp"
#include "runir/kr/ps/feature_index.hpp"

#include <cista/containers/variant.h>
#include <compare>
#include <tuple>

namespace runir::kr::ps::icp
{

struct ArgumentPosition
{
    ygg::uint_t value = 0;

    auto cista_members() const noexcept { return std::tie(value); }
    auto identifying_members() const noexcept { return std::tie(value); }
    auto operator<=>(const ArgumentPosition&) const = default;
};

using ObjectReference = ::cista::offset::variant<ArgumentPosition, ygg::Index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>>>;
using ConceptFeatureIndex = ygg::Index<ps::Feature<IcpFamilyTag, runir::kr::dl::ConceptTag>>;

struct ResetPair
{
    ConceptFeatureIndex before;
    ConceptFeatureIndex after;

    auto cista_members() const noexcept { return std::tie(before, after); }
    auto identifying_members() const noexcept { return std::tie(before, after); }
    bool operator==(const ResetPair&) const = default;
    bool operator<(const ResetPair& other) const noexcept { return std::tie(before, after) < std::tie(other.before, other.after); }
};

}

#endif
