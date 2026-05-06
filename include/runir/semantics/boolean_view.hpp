#ifndef RUNIR_SEMANTICS_BOOLEAN_VIEW_HPP_
#define RUNIR_SEMANTICS_BOOLEAN_VIEW_HPP_

#include "runir/semantics/boolean_data.hpp"

#include <concepts>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/variant.hpp>
#include <tyr/formalism/predicate_view.hpp>

namespace tyr
{

template<runir::BooleanConstructorTag Tag, typename C>
class View<Index<runir::Boolean<Tag>>, C>
{
private:
    const C* m_context;
    Index<runir::Boolean<Tag>> m_handle;

public:
    View(Index<runir::Boolean<Tag>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }

    auto get_predicate() const noexcept
        requires runir::is_atomic_state_tag_v<Tag>
    {
        return make_view(get_data().predicate, *m_context);
    }

    auto get_polarity() const noexcept
        requires runir::is_atomic_state_tag_v<Tag>
    {
        return get_data().polarity;
    }

    auto get_arg() const noexcept
        requires std::same_as<Tag, runir::NonemptyTag>
    {
        return make_view(get_data().arg, *m_context);
    }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}

#endif
