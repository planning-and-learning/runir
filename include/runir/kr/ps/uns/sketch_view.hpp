#ifndef RUNIR_KR_PS_UNS_SKETCH_VIEW_HPP_
#define RUNIR_KR_PS_UNS_SKETCH_VIEW_HPP_

#include "runir/kr/ps/uns/declarations.hpp"
#include "runir/kr/ps/uns/rule_view.hpp"
#include "runir/kr/ps/uns/sketch_data.hpp"

#include <tuple>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<typename C>
class View<Index<runir::kr::ps::uns::Sketch>, C>
{
private:
    const C* m_context;
    Index<runir::kr::ps::uns::Sketch> m_handle;

public:
    View(Index<runir::kr::ps::uns::Sketch> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_rules() const noexcept { return make_view(get_data().rules, *m_context); }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace ygg

#endif
