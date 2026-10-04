#ifndef RUNIR_KR_PS_FEATURE_VIEW_HPP_
#define RUNIR_KR_PS_FEATURE_VIEW_HPP_

#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/feature_index.hpp"

#include <tuple>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/detail/view.hpp>

namespace ygg
{

template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, formalism::SymbolContextFor<runir::kr::ps::Feature<Family, FeatureTag>> C>
class View<Index<runir::kr::ps::Feature<Family, FeatureTag>>, C> : public formalism::detail::View<Index<runir::kr::ps::Feature<Family, FeatureTag>>, C>
{
public:
    View(Index<runir::kr::ps::Feature<Family, FeatureTag>> handle, const C& context) noexcept :
        formalism::detail::View<Index<runir::kr::ps::Feature<Family, FeatureTag>>, C>(handle, context)
    {
    }

    auto get_variant() const noexcept { return make_view(this->get_data().variant, *this->m_context); }
    auto get_symbol() const noexcept
    {
        return ygg::visit([](auto feature) { return feature.get_symbol(); }, get_variant());
    }
    auto get_expression() const noexcept
    {
        return ygg::visit([](auto feature) { return feature.get_expression(); }, get_variant());
    }
    auto get_feature() const noexcept { return get_expression(); }
};

}  // namespace ygg

#endif
