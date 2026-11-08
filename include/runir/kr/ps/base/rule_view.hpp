#ifndef RUNIR_KR_PS_BASE_RULE_VIEW_HPP_
#define RUNIR_KR_PS_BASE_RULE_VIEW_HPP_

#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/base/rule_data.hpp"

#include <tuple>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<typename V>
struct view_mixin<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    const auto& get_symbol() const noexcept { return self().data().symbol; }
    auto get_conditions() const noexcept { return self().template member<&urd::record_of<V>::conditions>(); }
    auto get_effects() const noexcept { return self().template member<&urd::record_of<V>::effects>(); }
};
}  // namespace urd

#endif
