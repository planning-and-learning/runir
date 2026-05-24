#ifndef RUNIR_KR_PS_BASE_DL_CONDITION_VIEW_HPP_
#define RUNIR_KR_PS_BASE_DL_CONDITION_VIEW_HPP_

#include "runir/kr/ps/base/dl/condition_data.hpp"
#include "runir/kr/ps/base/dl/evaluation_context.hpp"
#include "runir/kr/ps/base/dl/feature_view.hpp"
#include "runir/kr/ps/feature_view.hpp"

#include <concepts>
#include <tuple>
#include <tyr/common/types.hpp>

namespace tyr
{

template<typename FeatureTag, typename ObservationTag, typename C>
class View<Index<runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>> m_handle;

public:
    View(Index<runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>> handle, const C& context) noexcept :
        m_context(&context),
        m_handle(handle)
    {
    }

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_feature() const noexcept { return make_view(get_data().feature, *m_context); }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace tyr

#endif
