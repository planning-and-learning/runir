#ifndef RUNIR_KR_GP_DL_EFFECT_VIEW_HPP_
#define RUNIR_KR_GP_DL_EFFECT_VIEW_HPP_

#include "runir/kr/gp/dl/effect_data.hpp"

#include <tuple>
#include <tyr/common/types.hpp>

namespace tyr
{

template<typename FeatureTag, typename ObservationTag, typename C>
class View<Index<runir::kr::gp::ConcreteEffect<runir::kr::DlTag, FeatureTag, ObservationTag>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::gp::ConcreteEffect<runir::kr::DlTag, FeatureTag, ObservationTag>> m_handle;

public:
    View(Index<runir::kr::gp::ConcreteEffect<runir::kr::DlTag, FeatureTag, ObservationTag>> handle, const C& context) noexcept :
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
