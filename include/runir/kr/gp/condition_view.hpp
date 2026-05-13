#ifndef RUNIR_KR_GP_CONDITION_VIEW_HPP_
#define RUNIR_KR_GP_CONDITION_VIEW_HPP_

#include "runir/kr/gp/condition_data.hpp"
#include "runir/kr/gp/dl/condition_view.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/variant.hpp>

namespace tyr
{

template<typename C>
class View<Index<runir::kr::gp::ConditionVariant>, C>
{
private:
    const C* m_context;
    Index<runir::kr::gp::ConditionVariant> m_handle;

public:
    View(Index<runir::kr::gp::ConditionVariant> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

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
class View<Index<runir::kr::gp::ConcreteConditionVariant<LanguageTag>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::gp::ConcreteConditionVariant<LanguageTag>> m_handle;

public:
    View(Index<runir::kr::gp::ConcreteConditionVariant<LanguageTag>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_variant() const noexcept { return make_view(get_data().value, *m_context); }

    template<typename EvaluationContext>
    bool is_compatible_with(EvaluationContext& context) const
        requires runir::kr::gp::IsEvaluationContext<LanguageTag, EvaluationContext>
    {
        return get_variant().apply([&](auto condition) { return condition.is_compatible_with(context); });
    }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace tyr

#endif
