#ifndef RUNIR_KR_PS_DL_FEATURE_VIEW_HPP_
#define RUNIR_KR_PS_DL_FEATURE_VIEW_HPP_

#include "runir/kr/dl/query_view.hpp"
#include "runir/kr/dl/semantics/constructor_view.hpp"
#include "runir/kr/ps/dl/feature_data.hpp"

#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/detail/view.hpp>

namespace ygg
{

template<runir::kr::FamilyTag Family,
         runir::kr::ps::dl::FeatureTag FeatureTag,
         formalism::SymbolContextFor<runir::kr::ps::ConcreteFeature<Family, runir::kr::DlTag, FeatureTag>> C>
class View<Index<runir::kr::ps::ConcreteFeature<Family, runir::kr::DlTag, FeatureTag>>, C> :
    public formalism::detail::View<Index<runir::kr::ps::ConcreteFeature<Family, runir::kr::DlTag, FeatureTag>>, C>
{
public:
    View(Index<runir::kr::ps::ConcreteFeature<Family, runir::kr::DlTag, FeatureTag>> handle, const C& context) noexcept :
        formalism::detail::View<Index<runir::kr::ps::ConcreteFeature<Family, runir::kr::DlTag, FeatureTag>>, C>(handle, context)
    {
    }

    auto get_expression() const noexcept { return make_view(this->get_data().feature, this->m_context->get_dl_repository()); }
    auto get_feature() const noexcept { return get_expression(); }
    const auto& get_symbol() const noexcept { return this->get_data().symbol; }
};

}  // namespace ygg

#endif
