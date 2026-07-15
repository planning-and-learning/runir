#ifndef RUNIR_KR_PS_EXT_DL_FEATURE_VIEW_HPP_
#define RUNIR_KR_PS_EXT_DL_FEATURE_VIEW_HPP_

#include "runir/kr/dl/semantics/constructor_view.hpp"
#include "runir/kr/ps/ext/dl/feature_data.hpp"

#include <concepts>
#include <tuple>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<typename FeatureTag, typename C>
class View<Index<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag>> m_handle;

public:
    View(Index<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag>> handle, const C& context) noexcept :
        m_context(&context),
        m_handle(handle)
    {
    }

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_expression() const noexcept { return make_view(get_data().feature, m_context->get_dl_repository()); }
    auto get_feature() const noexcept { return get_expression(); }
    const auto& get_symbol() const noexcept { return get_data().symbol; }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace ygg

#endif
