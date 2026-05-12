#ifndef RUNIR_KR_GP_DL_FEATURE_VIEW_HPP_
#define RUNIR_KR_GP_DL_FEATURE_VIEW_HPP_

#include "runir/kr/gp/dl/feature_data.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/variant.hpp>

namespace tyr
{

template<typename FeatureTypeTag, typename C>
class View<Index<runir::kr::gp::Feature<runir::kr::DlTag, FeatureTypeTag>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::gp::Feature<runir::kr::DlTag, FeatureTypeTag>> m_handle;

public:
    View(Index<runir::kr::gp::Feature<runir::kr::DlTag, FeatureTypeTag>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_feature() const noexcept { return make_view(get_data().feature, m_context->get_dl_repository()); }
    const auto& get_symbol() const noexcept { return get_data().symbol; }
    const auto& get_description() const noexcept { return get_data().description; }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace tyr

#endif
