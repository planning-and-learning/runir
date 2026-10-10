#ifndef RUNIR_KR_PS_ICP_XCONDITION_VIEW_HPP_
#define RUNIR_KR_PS_ICP_XCONDITION_VIEW_HPP_

#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/ps/icp/xcondition_data.hpp"

namespace ygg
{

template<typename C>
class View<Index<runir::kr::ps::icp::XCondition>, C> : public ygg::IndexViewBase<runir::kr::ps::icp::XCondition, C>
{
public:
    using ygg::IndexViewBase<runir::kr::ps::icp::XCondition, C>::IndexViewBase;

    auto get_operation() const noexcept { return this->get_data().operation; }
    const auto& get_object_reference() const noexcept { return this->get_data().object; }
    auto get_concept_feature() const noexcept { return make_view(this->get_data().feature, this->get_context()); }
    auto get_feature() const noexcept { return get_concept_feature(); }
};

}

#endif
