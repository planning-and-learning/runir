#ifndef RUNIR_KR_PS_CONDITION_VIEW_HPP_
#define RUNIR_KR_PS_CONDITION_VIEW_HPP_

#include "runir/kr/ps/condition_index.hpp"

#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/containers/variant.hpp>

namespace ygg
{

template<runir::kr::FamilyTag Family, typename C>
class View<Index<runir::kr::ps::ConditionVariant<Family>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::ps::ConditionVariant<Family>> m_handle;

public:
    View(Index<runir::kr::ps::ConditionVariant<Family>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_variant() const noexcept { return make_view(get_data().value, *m_context); }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

template<runir::kr::FamilyTag Family, typename LanguageTag, typename C>
class View<Index<runir::kr::ps::ConcreteConditionVariant<Family, LanguageTag>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::ps::ConcreteConditionVariant<Family, LanguageTag>> m_handle;

public:
    View(Index<runir::kr::ps::ConcreteConditionVariant<Family, LanguageTag>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_variant() const noexcept { return make_view(get_data().value, *m_context); }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace ygg

#endif
