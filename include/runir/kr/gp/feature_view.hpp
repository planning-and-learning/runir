#ifndef RUNIR_KR_GP_FEATURE_VIEW_HPP_
#define RUNIR_KR_GP_FEATURE_VIEW_HPP_

#include "runir/kr/gp/feature_data.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/variant.hpp>

namespace tyr
{

template<typename FeatureTypeTag, typename C>
class View<Index<runir::kr::gp::FeatureTag<FeatureTypeTag>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::gp::FeatureTag<FeatureTypeTag>> m_handle;

public:
    View(Index<runir::kr::gp::FeatureTag<FeatureTypeTag>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_variant() const noexcept { return make_view(get_data().value, *m_context); }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace tyr

#endif
