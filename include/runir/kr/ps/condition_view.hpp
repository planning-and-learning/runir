#ifndef RUNIR_KR_PS_CONDITION_VIEW_HPP_
#define RUNIR_KR_PS_CONDITION_VIEW_HPP_

#include "runir/kr/ps/condition_data.hpp"

#include <tuple>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

template<runir::kr::FamilyTag Family, formalism::SymbolContextFor<runir::kr::ps::ConditionVariant<Family>> C>
class View<Index<runir::kr::ps::ConditionVariant<Family>>, C> : public ygg::IndexViewBase<runir::kr::ps::ConditionVariant<Family>, C>
{
public:
    using ygg::IndexViewBase<runir::kr::ps::ConditionVariant<Family>, C>::IndexViewBase;

    auto get_variant() const noexcept { return make_view(this->get_data().variant, this->get_context()); }
};

template<runir::kr::FamilyTag Family, typename LanguageTag, formalism::SymbolContextFor<runir::kr::ps::ConcreteConditionVariant<Family, LanguageTag>> C>
class View<Index<runir::kr::ps::ConcreteConditionVariant<Family, LanguageTag>>, C> :
    public ygg::IndexViewBase<runir::kr::ps::ConcreteConditionVariant<Family, LanguageTag>, C>
{
public:
    using ygg::IndexViewBase<runir::kr::ps::ConcreteConditionVariant<Family, LanguageTag>, C>::IndexViewBase;

    auto get_variant() const noexcept { return make_view(this->get_data().variant, this->get_context()); }
};

}  // namespace ygg

#endif
