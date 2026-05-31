#ifndef RUNIR_GRAMMAR_CONSTRUCTOR_VIEW_HPP_
#define RUNIR_GRAMMAR_CONSTRUCTOR_VIEW_HPP_

#include "runir/kr/dl/grammar/boolean_view.hpp"
#include "runir/kr/dl/grammar/concept_view.hpp"
#include "runir/kr/dl/grammar/constructor_index.hpp"
#include "runir/kr/dl/grammar/numerical_view.hpp"
#include "runir/kr/dl/grammar/role_view.hpp"

#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/containers/variant.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
class View<Index<runir::kr::dl::grammar::Constructor<Family, Category>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::dl::grammar::Constructor<Family, Category>> m_handle;

public:
    View(Index<runir::kr::dl::grammar::Constructor<Family, Category>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_variant() const noexcept { return make_view(get_data().value, *m_context); }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}

#endif
