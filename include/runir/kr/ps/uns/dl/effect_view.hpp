#ifndef RUNIR_KR_PS_UNS_DL_EFFECT_VIEW_HPP_
#define RUNIR_KR_PS_UNS_DL_EFFECT_VIEW_HPP_

#include "runir/kr/ps/uns/dl/effect_data.hpp"
#include "runir/kr/ps/uns/dl/evaluation_context.hpp"
#include "runir/kr/ps/feature_view.hpp"

#include <concepts>
#include <tuple>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<typename FeatureTag, typename ObservationTag, typename C>
class View<Index<runir::kr::ps::ConcreteEffect<runir::kr::UnsFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::ps::ConcreteEffect<runir::kr::UnsFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>> m_handle;

public:
    View(Index<runir::kr::ps::ConcreteEffect<runir::kr::UnsFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>> handle, const C& context) noexcept :
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

}  // namespace ygg

#endif
