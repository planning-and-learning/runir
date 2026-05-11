#ifndef RUNIR_GRAMMAR_CONSTRUCTOR_VIEW_HPP_
#define RUNIR_GRAMMAR_CONSTRUCTOR_VIEW_HPP_

#include "runir/kr/dl/grammar/boolean_view.hpp"
#include "runir/kr/dl/grammar/concept_view.hpp"
#include "runir/kr/dl/grammar/constructor_data.hpp"
#include "runir/kr/dl/grammar/numerical_view.hpp"
#include "runir/kr/dl/grammar/role_view.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/variant.hpp>

namespace tyr
{

template<runir::kr::dl::CategoryTag Category, typename C>
class View<Index<runir::kr::dl::grammar::Constructor<Category>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::dl::grammar::Constructor<Category>> m_handle;

public:
    View(Index<runir::kr::dl::grammar::Constructor<Category>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_variant() const noexcept { return make_view(get_data().value, *m_context); }

    auto identifying_members() const noexcept { return std::tie(get_data()); }
};

}

#endif
