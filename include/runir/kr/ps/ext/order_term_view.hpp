#ifndef RUNIR_KR_PS_EXT_ORDER_TERM_VIEW_HPP_
#define RUNIR_KR_PS_EXT_ORDER_TERM_VIEW_HPP_

#include "runir/kr/ps/ext/order_term_data.hpp"
#include "runir/kr/ps/feature_view.hpp"

#include <yggdrasil/containers/variant.hpp>

namespace ygg
{

template<typename C>
class View<Index<runir::kr::ps::ext::OrderTerm>, C>
{
private:
    const C* m_context;
    Index<runir::kr::ps::ext::OrderTerm> m_handle;

public:
    View(Index<runir::kr::ps::ext::OrderTerm> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_direction() const noexcept { return get_data().direction; }
    auto get_feature() const noexcept { return make_view(get_data().feature, *m_context); }

    auto identifying_members() const noexcept { return std::make_tuple(m_handle, m_context->get_index()); }
};

}  // namespace ygg

#endif
