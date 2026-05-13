#ifndef RUNIR_KR_GP_POLICY_VIEW_HPP_
#define RUNIR_KR_GP_POLICY_VIEW_HPP_

#include "runir/kr/gp/policy_data.hpp"
#include "runir/kr/gp/rule_view.hpp"

#include <tuple>
#include <tyr/common/types.hpp>

namespace tyr
{

template<typename C>
class View<Index<runir::kr::gp::Policy>, C>
{
private:
    const C* m_context;
    Index<runir::kr::gp::Policy> m_handle;

public:
    View(Index<runir::kr::gp::Policy> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_rules() const noexcept { return make_view(get_data().rules, *m_context); }

    template<typename EvaluationContext>
    bool is_compatible_with(EvaluationContext& context) const
    {
        for (auto rule : get_rules())
            if (rule.is_compatible_with(context))
                return true;

        return false;
    }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace tyr

#endif
