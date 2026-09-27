#ifndef RUNIR_KR_PS_ICP_RULE_VARIANT_VIEW_HPP_
#define RUNIR_KR_PS_ICP_RULE_VARIANT_VIEW_HPP_

#include "runir/kr/ps/icp/rule_variant_data.hpp"

#include <tuple>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<typename C>
class View<Index<runir::kr::ps::Rule<runir::kr::IcpFamilyTag>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::ps::Rule<runir::kr::IcpFamilyTag>> m_handle;

public:
    View(Index<runir::kr::ps::Rule<runir::kr::IcpFamilyTag>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    const auto& get_symbol() const noexcept { return get_data().symbol; }
    auto get_variant() const noexcept { return make_view(get_data().variant, *m_context); }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace ygg

#endif
