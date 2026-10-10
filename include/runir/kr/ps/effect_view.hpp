#ifndef RUNIR_KR_PS_EFFECT_VIEW_HPP_
#define RUNIR_KR_PS_EFFECT_VIEW_HPP_

#include "runir/kr/ps/effect_data.hpp"

#include <tuple>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

template<runir::kr::FamilyTag Family, formalism::SymbolContextFor<runir::kr::ps::EffectVariant<Family>> C>
class View<Index<runir::kr::ps::EffectVariant<Family>>, C> : public ygg::IndexViewBase<runir::kr::ps::EffectVariant<Family>, C>
{
public:
    using ygg::IndexViewBase<runir::kr::ps::EffectVariant<Family>, C>::IndexViewBase;

    auto get_variant() const noexcept { return make_view(this->get_data().variant, this->get_context()); }
};

template<runir::kr::FamilyTag Family, typename LanguageTag, formalism::SymbolContextFor<runir::kr::ps::ConcreteEffectVariant<Family, LanguageTag>> C>
class View<Index<runir::kr::ps::ConcreteEffectVariant<Family, LanguageTag>>, C> :
    public ygg::IndexViewBase<runir::kr::ps::ConcreteEffectVariant<Family, LanguageTag>, C>
{
public:
    using ygg::IndexViewBase<runir::kr::ps::ConcreteEffectVariant<Family, LanguageTag>, C>::IndexViewBase;

    auto get_variant() const noexcept { return make_view(this->get_data().variant, this->get_context()); }
};

}  // namespace ygg

#endif
