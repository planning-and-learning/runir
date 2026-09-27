#ifndef RUNIR_KR_PS_ICP_XCONDITION_DATA_HPP_
#define RUNIR_KR_PS_ICP_XCONDITION_DATA_HPP_

#include "runir/kr/ps/icp/object_reference.hpp"
#include "runir/kr/ps/icp/xcondition_index.hpp"

#include <tuple>
#include <utility>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::ps::icp::XCondition>
{
    Index<runir::kr::ps::icp::XCondition> index;
    runir::kr::ps::icp::ConditionOperation operation {};
    runir::kr::ps::icp::ObjectReference object;
    runir::kr::ps::icp::ConceptFeatureIndex feature;

    Data() = default;
    Data(runir::kr::ps::icp::ConditionOperation operation_, runir::kr::ps::icp::ObjectReference object_, runir::kr::ps::icp::ConceptFeatureIndex feature_) :
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
