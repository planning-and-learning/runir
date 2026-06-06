#ifndef RUNIR_KR_PS_BASE_RULE_VIEW_HPP_
#define RUNIR_KR_PS_BASE_RULE_VIEW_HPP_

#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/base/rule_data.hpp"
#include "runir/kr/ps/condition_view.hpp"
#include "runir/kr/ps/effect_view.hpp"

#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/containers/vector.hpp>

namespace ygg
{

template<typename C>
class View<Index<runir::kr::ps::base::Rule>, C>
{
private:
    const C* m_context;
    Index<runir::kr::ps::base::Rule> m_handle;

public:
    View(Index<runir::kr::ps::base::Rule> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    const auto& get_symbol() const noexcept { return get_data().symbol; }
    const auto& get_description() const noexcept { return get_data().description; }
    auto get_conditions() const noexcept { return make_view(get_data().conditions, *m_context); }
    auto get_effects() const noexcept { return make_view(get_data().effects, *m_context); }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace ygg

#endif
