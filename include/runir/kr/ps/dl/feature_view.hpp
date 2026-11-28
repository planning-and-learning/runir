#ifndef RUNIR_KR_PS_DL_FEATURE_VIEW_HPP_
#define RUNIR_KR_PS_DL_FEATURE_VIEW_HPP_

#include "runir/kr/dl/query_view.hpp"
#include "runir/kr/dl/semantics/constructor_view.hpp"
#include "runir/kr/ps/dl/feature_data.hpp"

#include <cstddef>
#include <tuple>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace runir::kr::ps
{
/// The syntactic complexity of a concrete (DL) feature, its DL feature's (defined in syntactic_complexity.hpp),
/// declared before its view, whose functions() list it.
template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, typename C>
std::size_t syntactic_complexity(const urd::index_view<ConcreteFeature<Family, FeatureTag>, C>& view);
}  // namespace runir::kr::ps

namespace urd
{
template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, typename V>
struct view_mixin<runir::kr::ps::ConcreteFeature<Family, FeatureTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto feature() const noexcept { return self().template member<&urd::record_of<V>::feature>(); }

    const auto& symbol() const noexcept { return self().template member<&urd::record_of<V>::symbol>(); }

    static constexpr auto functions() noexcept
    { return urd::Functions<urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::ps::syntactic_complexity)>> {}; }
};
}  // namespace urd

#endif
