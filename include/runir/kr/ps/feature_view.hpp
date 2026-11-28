#ifndef RUNIR_KR_PS_FEATURE_VIEW_HPP_
#define RUNIR_KR_PS_FEATURE_VIEW_HPP_

#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/dl/declarations.hpp"

#include <cstddef>
#include <tuple>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace runir::kr::ps
{
/// The syntactic complexity of a feature (defined in syntactic_complexity.hpp), declared before its view, whose
/// functions() list it.
template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, typename C>
std::size_t syntactic_complexity(const urd::index_view<Feature<Family, FeatureTag>, C>& view);
}  // namespace runir::kr::ps

namespace urd
{
template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, typename V>
struct view_mixin<runir::kr::ps::Feature<Family, FeatureTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto variant() const noexcept { return self().template member<&urd::record_of<V>::variant>(); }

    static constexpr auto functions() noexcept
    { return urd::Functions<urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::ps::syntactic_complexity)>> {}; }
};
}  // namespace urd

#endif
