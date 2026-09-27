#ifndef RUNIR_KR_PS_ICP_XEFFECT_VIEW_HPP_
#define RUNIR_KR_PS_ICP_XEFFECT_VIEW_HPP_

#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/ps/icp/xeffect_data.hpp"

namespace ygg
{

template<typename C>
class View<Index<runir::kr::ps::icp::XEffect>, C>
{
    const C* m_context;
    Index<runir::kr::ps::icp::XEffect> m_handle;

public:
    View(Index<runir::kr::ps::icp::XEffect> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    auto get_index() const noexcept { return m_handle; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_operation() const noexcept { return get_data().operation; }
    const auto& get_object_reference() const noexcept { return get_data().object; }
    auto get_concept_feature() const noexcept { return make_view(get_data().feature, *m_context); }
    auto get_feature() const noexcept { return get_concept_feature(); }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}

#endif
