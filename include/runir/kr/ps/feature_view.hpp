#ifndef RUNIR_KR_PS_FEATURE_VIEW_HPP_
#define RUNIR_KR_PS_FEATURE_VIEW_HPP_

#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/dl/declarations.hpp"

#include <tuple>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, typename V>
struct view_mixin<runir::kr::ps::Feature<Family, FeatureTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_variant() const noexcept { return self().template member<&urd::record_of<V>::variant>(); }
    auto get_symbol() const noexcept
    {
        return std::visit([](auto feature) { return feature.get_symbol(); }, get_variant());
    }
    auto get_expression() const noexcept
    {
        return std::visit([](auto feature) { return feature.get_expression(); }, get_variant());
    }
    auto get_feature() const noexcept { return get_expression(); }
};
}  // namespace urd

#endif
