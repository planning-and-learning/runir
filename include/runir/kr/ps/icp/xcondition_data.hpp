#ifndef RUNIR_KR_PS_ICP_XCONDITION_DATA_HPP_
#define RUNIR_KR_PS_ICP_XCONDITION_DATA_HPP_

#include "runir/kr/ps/icp/declarations.hpp"
#include "runir/kr/ps/icp/object_reference.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <urd/elements/flat.hpp>

namespace urd
{

template<>
struct data<runir::kr::ps::icp::XCondition>
{
    urd::index<runir::kr::ps::icp::XCondition> index;
    runir::kr::ps::icp::ConditionOperation operation {};
    ::cista::offset::variant<runir::kr::ps::icp::ArgumentPosition, urd::index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>>> object;
    urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>> feature;

    data() = default;
    data(runir::kr::ps::icp::ConditionOperation operation_,
         ::cista::offset::variant<runir::kr::ps::icp::ArgumentPosition, urd::index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>>> object_,
         urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>> feature_) :
        index(),
        operation(operation_),
        object(std::move(object_)),
        feature(feature_)
    {
    }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::operation, &data::object, &data::feature> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::operation, &data::object, &data::feature> {}; }
};

}  // namespace urd

#endif
