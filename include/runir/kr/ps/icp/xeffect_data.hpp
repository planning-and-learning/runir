#ifndef RUNIR_KR_PS_ICP_XEFFECT_DATA_HPP_
#define RUNIR_KR_PS_ICP_XEFFECT_DATA_HPP_

#include "runir/kr/ps/icp/object_reference.hpp"
#include "runir/kr/ps/icp/xeffect_index.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::ps::icp::XEffect>
{
    Index<runir::kr::ps::icp::XEffect> index;
    runir::kr::ps::icp::EffectOperation operation {};
    ::cista::offset::variant<runir::kr::ps::icp::ArgumentPosition, Index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>>> object;
    Index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>> feature;

    Data() = default;
    Data(runir::kr::ps::icp::EffectOperation operation_,
         ::cista::offset::variant<runir::kr::ps::icp::ArgumentPosition, Index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>>> object_,
         Index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>> feature_) :
        index(),
        operation(operation_),
        object(std::move(object_)),
        feature(feature_)
    {
    }

    void clear() noexcept
    {
        ygg::clear(index);
        operation = {};
        object = runir::kr::ps::icp::ArgumentPosition {};
        ygg::clear(feature);
    }

    auto cista_members() const noexcept { return std::tie(index, operation, object, feature); }
    auto identifying_members() const noexcept { return std::tie(operation, object, feature); }
};

}  // namespace ygg

#endif
