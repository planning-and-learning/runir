#ifndef RUNIR_KR_PS_DL_FEATURE_VIEW_HPP_
#define RUNIR_KR_PS_DL_FEATURE_VIEW_HPP_

#include "runir/kr/dl/query_view.hpp"
#include "runir/kr/dl/semantics/constructor_view.hpp"
#include "runir/kr/ps/dl/feature_data.hpp"

#include <tuple>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, typename V>
struct view_mixin<runir::kr::ps::ConcreteFeature<Family, runir::kr::DlTag, FeatureTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_expression() const noexcept { return urd::resolve(self().data().feature, self().context().get_dl_repository()); }
    auto get_feature() const noexcept { return get_expression(); }
    const auto& get_symbol() const noexcept { return self().data().symbol; }
};
}  // namespace urd

#endif
