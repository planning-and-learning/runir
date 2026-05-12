#ifndef RUNIR_KR_GP_RULE_VIEW_HPP_
#define RUNIR_KR_GP_RULE_VIEW_HPP_

#include "runir/kr/gp/rule_data.hpp"

#include <tuple>
#include <tyr/common/types.hpp>

namespace tyr
{

template<typename C>
class View<Index<runir::kr::gp::Rule>, C>
{
private:
    const C* m_context;
    Index<runir::kr::gp::Rule> m_handle;

public:
    View(Index<runir::kr::gp::Rule> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    const auto& get_conditions() const noexcept { return get_data().conditions; }
    const auto& get_effects() const noexcept { return get_data().effects; }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace tyr

#endif
