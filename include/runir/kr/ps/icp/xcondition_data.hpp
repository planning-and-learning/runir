#ifndef RUNIR_KR_PS_ICP_XCONDITION_DATA_HPP_
#define RUNIR_KR_PS_ICP_XCONDITION_DATA_HPP_

#include "runir/kr/ps/icp/declarations.hpp"
#include "runir/kr/ps/icp/object_reference.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::ps::icp::XCondition>
{
    Index<runir::kr::ps::icp::XCondition> index;
    runir::kr::ps::icp::ConditionOperation operation {};
    ::cista::offset::variant<runir::kr::ps::icp::ArgumentPosition, Index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>>> object;
    Index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>> feature;

    Data() = default;
    Data(runir::kr::ps::icp::ConditionOperation operation_,
         ::cista::offset::variant<runir::kr::ps::icp::ArgumentPosition, Index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>>> object_,
         Index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>> feature_) :
        index(),
        operation(std::move(operation_)),
        object(std::move(object_)),
        feature(std::move(feature_))
    {
    }
    template<typename C>
    Data(runir::kr::ps::icp::ConditionOperation operation_,
         ::cista::offset::variant<runir::kr::ps::icp::ArgumentPosition, Index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>>> object_,
         ::ygg::View<Index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>>, C> feature_) :
        index(),
        operation(std::move(operation_)),
        object(std::move(object_)),
        feature()
    {
        set(feature_, feature);
    }

    auto cista_members() noexcept { return std::tie(index, operation, object, feature); }
    auto cista_members() const noexcept { return std::tie(index, operation, object, feature); }
    auto identifying_members() const noexcept { return std::tie(operation, object, feature); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}  // namespace ygg

#endif
