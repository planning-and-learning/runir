#ifndef RUNIR_KR_PS_CONDITION_VIEW_HPP_
#define RUNIR_KR_PS_CONDITION_VIEW_HPP_

#include "runir/kr/ps/condition_data.hpp"

#include <tuple>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/detail/view.hpp>

namespace ygg
{

template<runir::kr::FamilyTag Family, formalism::SymbolContextFor<runir::kr::ps::ConditionVariant<Family>> C>
class View<Index<runir::kr::ps::ConditionVariant<Family>>, C> : public formalism::detail::View<Index<runir::kr::ps::ConditionVariant<Family>>, C>
{
public:
    View(Index<runir::kr::ps::ConditionVariant<Family>> handle, const C& context) noexcept :
        formalism::detail::View<Index<runir::kr::ps::ConditionVariant<Family>>, C>(handle, context)
    {
    }

    auto get_variant() const noexcept { return make_view(this->get_data().variant, *this->m_context); }
};

template<runir::kr::FamilyTag Family, typename LanguageTag, formalism::SymbolContextFor<runir::kr::ps::ConcreteConditionVariant<Family, LanguageTag>> C>
class View<Index<runir::kr::ps::ConcreteConditionVariant<Family, LanguageTag>>, C> :
    public formalism::detail::View<Index<runir::kr::ps::ConcreteConditionVariant<Family, LanguageTag>>, C>
{
public:
    View(Index<runir::kr::ps::ConcreteConditionVariant<Family, LanguageTag>> handle, const C& context) noexcept :
        formalism::detail::View<Index<runir::kr::ps::ConcreteConditionVariant<Family, LanguageTag>>, C>(handle, context)
    {
    }

    auto get_variant() const noexcept { return make_view(this->get_data().variant, *this->m_context); }
};

}  // namespace ygg

#endif
