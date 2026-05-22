#ifndef RUNIR_KR_MPG_DL_CONCEPT_VIEW_HPP_
#define RUNIR_KR_MPG_DL_CONCEPT_VIEW_HPP_

#include "runir/kr/mpg/dl/concept_data.hpp"
#include "runir/kr/mpg/register_view.hpp"

#include <tuple>
#include <tyr/common/types.hpp>

namespace tyr
{

template<typename C>
class View<Index<runir::kr::mpg::dl::Concept<runir::kr::mpg::dl::RegisterTag>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::mpg::dl::Concept<runir::kr::mpg::dl::RegisterTag>> m_handle;

public:
    View(Index<runir::kr::mpg::dl::Concept<runir::kr::mpg::dl::RegisterTag>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_register() const noexcept { return View<Index<runir::kr::mpg::Register>, C>(get_data().reg, *m_context); }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace tyr

#endif
