#ifndef RUNIR_KR_PS_EXT_CONDITION_VIEW_HPP_
#define RUNIR_KR_PS_EXT_CONDITION_VIEW_HPP_

#include "runir/kr/ps/ext/condition_data.hpp"

#include <tuple>
#include <tyr/common/types.hpp>

namespace tyr
{

template<typename C>
class View<Index<runir::kr::ps::ext::ConditionVariant>, C>
{
private:
    const C* m_context;
    Index<runir::kr::ps::ext::ConditionVariant> m_handle;

public:
    View(Index<runir::kr::ps::ext::ConditionVariant> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace tyr

#endif
