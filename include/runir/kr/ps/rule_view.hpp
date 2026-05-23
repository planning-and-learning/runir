#ifndef RUNIR_KR_PS_RULE_VIEW_HPP_
#define RUNIR_KR_PS_RULE_VIEW_HPP_

#include "runir/kr/ps/rule_data.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/vector.hpp>

namespace tyr
{

template<runir::kr::FamilyTag Family, typename C>
class View<Index<runir::kr::ps::Rule<Family>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::ps::Rule<Family>> m_handle;

public:
    View(Index<runir::kr::ps::Rule<Family>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_conditions() const noexcept { return make_view(get_data().conditions, *m_context); }
    auto get_effects() const noexcept { return make_view(get_data().effects, *m_context); }

    template<typename EvaluationContext>
    bool is_compatible_with(EvaluationContext& context) const
    {
        for (auto condition : get_conditions())
            if (!condition.is_compatible_with(context))
                return false;

        for (auto effect : get_effects())
            if (!effect.is_compatible_with(context))
                return false;

        return true;
    }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace tyr

#endif
