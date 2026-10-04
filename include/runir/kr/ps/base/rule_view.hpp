#ifndef RUNIR_KR_PS_BASE_RULE_VIEW_HPP_
#define RUNIR_KR_PS_BASE_RULE_VIEW_HPP_

#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/base/rule_data.hpp"
#include "runir/kr/ps/condition_view.hpp"
#include "runir/kr/ps/effect_view.hpp"

#include <tuple>
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/detail/view.hpp>

namespace ygg
{

template<formalism::SymbolContextFor<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>> C>
class View<Index<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>>, C> : public formalism::detail::View<Index<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>>, C>
{
public:
    View(Index<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>> handle, const C& context) noexcept :
        formalism::detail::View<Index<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>>, C>(handle, context)
    {
    }

    const auto& get_symbol() const noexcept { return this->get_data().symbol; }
    auto get_conditions() const noexcept { return make_view(this->get_data().conditions, *this->m_context); }
    auto get_effects() const noexcept { return make_view(this->get_data().effects, *this->m_context); }
};

}  // namespace ygg

#endif
