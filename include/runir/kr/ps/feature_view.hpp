#ifndef RUNIR_KR_PS_FEATURE_VIEW_HPP_
#define RUNIR_KR_PS_FEATURE_VIEW_HPP_

#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/dl/declarations.hpp"

#include <tuple>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, formalism::SymbolContextFor<runir::kr::ps::Feature<Family, FeatureTag>> C>
class View<Index<runir::kr::ps::Feature<Family, FeatureTag>>, C> : public ygg::IndexViewBase<runir::kr::ps::Feature<Family, FeatureTag>, C>
{
public:
    using ygg::IndexViewBase<runir::kr::ps::Feature<Family, FeatureTag>, C>::IndexViewBase;

    auto get_variant() const noexcept { return make_view(this->get_data().variant, this->get_context()); }
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
