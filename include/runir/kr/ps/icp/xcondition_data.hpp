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

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::operation, &Data::object, &Data::feature> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::operation, &Data::object, &Data::feature> {}; }
    void clear() noexcept
    {
        ygg::clear_members(*this);
    }
};

}  // namespace ygg

#endif
