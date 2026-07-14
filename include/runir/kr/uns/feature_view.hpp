#ifndef RUNIR_KR_UNS_FEATURE_VIEW_HPP_
#define RUNIR_KR_UNS_FEATURE_VIEW_HPP_

#include "runir/kr/uns/dl/feature_view.hpp"
#include "runir/kr/uns/feature_data.hpp"

#include <tuple>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<typename C>
class View<Index<runir::kr::uns::Feature>, C>
{
private:
    const C* m_context;
    Index<runir::kr::uns::Feature> m_handle;

public:
    View(Index<runir::kr::uns::Feature> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_variant() const noexcept { return make_view(get_data().value, *m_context); }
    auto get_symbol() const noexcept
    {
        return ygg::visit([](auto feature) { return feature.get_symbol(); }, get_variant());
    }
    auto get_expression() const noexcept
    {
        return ygg::visit([](auto feature) { return feature.get_expression(); }, get_variant());
    }
    auto get_feature() const noexcept { return get_expression(); }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace ygg

#endif
