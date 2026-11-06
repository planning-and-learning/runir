#ifndef RUNIR_KR_PS_ICP_RULE_VARIANT_VIEW_HPP_
#define RUNIR_KR_PS_ICP_RULE_VARIANT_VIEW_HPP_

#include "runir/kr/ps/icp/rule_variant_data.hpp"

#include <tuple>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<typename V>
struct view_mixin<runir::kr::ps::Rule<runir::kr::IcpFamilyTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    const auto& get_symbol() const noexcept { return self().data().symbol; }
    auto get_variant() const noexcept { return self().template get<&urd::record_of<V>::variant>(); }
};
}  // namespace urd

#endif
