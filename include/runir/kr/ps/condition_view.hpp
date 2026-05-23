#ifndef RUNIR_KR_PS_CONDITION_VIEW_HPP_
#define RUNIR_KR_PS_CONDITION_VIEW_HPP_

#include "runir/kr/ps/condition_data.hpp"
#include "runir/kr/ps/base/dl/condition_view.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/variant.hpp>

namespace tyr
{

template<typename C>
class View<Index<runir::kr::ps::ConditionVariant>, C>
{
private:
    const C* m_context;
    Index<runir::kr::ps::ConditionVariant> m_handle;

public:
    View(Index<runir::kr::ps::ConditionVariant> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_variant() const noexcept { return make_view(get_data().value, *m_context); }

    template<typename EvaluationContext>
    bool is_compatible_with(EvaluationContext& context) const
    {
        return get_variant().apply([&](auto condition) { return condition.is_compatible_with(context); });
    }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

template<typename LanguageTag, typename C>
class View<Index<runir::kr::ps::ConcreteConditionVariant<LanguageTag>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::ps::ConcreteConditionVariant<LanguageTag>> m_handle;

public:
    View(Index<runir::kr::ps::ConcreteConditionVariant<LanguageTag>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_variant() const noexcept { return make_view(get_data().value, *m_context); }

    template<typename EvaluationContext>
    bool is_compatible_with(EvaluationContext& context) const
        requires runir::kr::ps::IsEvaluationContext<typename EvaluationContext::Family, LanguageTag, EvaluationContext>
    {
        return get_variant().apply([&](auto condition) { return condition.is_compatible_with(context); });
    }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace tyr

#endif
