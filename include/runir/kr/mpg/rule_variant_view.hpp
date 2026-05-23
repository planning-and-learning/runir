#ifndef RUNIR_KR_MPG_RULE_VARIANT_VIEW_HPP_
#define RUNIR_KR_MPG_RULE_VARIANT_VIEW_HPP_

#include "runir/kr/mpg/rule_variant_data.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/variant.hpp>

namespace tyr
{

template<typename C>
class View<Index<runir::kr::mpg::RuleVariant>, C>
{
private:
    const C* m_context;
    Index<runir::kr::mpg::RuleVariant> m_handle;

public:
    View(Index<runir::kr::mpg::RuleVariant> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_variant() const noexcept { return make_view(get_data().value, *m_context); }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace tyr

#endif
