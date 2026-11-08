#ifndef RUNIR_KR_PS_BASE_SKETCH_VIEW_HPP_
#define RUNIR_KR_PS_BASE_SKETCH_VIEW_HPP_

#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/base/rule_view.hpp"
#include "runir/kr/ps/base/sketch_data.hpp"

#include <concepts>
#include <tuple>
#include <urd/repr/core/dependent_false.hpp>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<typename V>
struct view_mixin<runir::kr::ps::base::Sketch, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    template<typename FeatureTag>
    auto get_features() const noexcept
    {
        if constexpr (std::same_as<FeatureTag, runir::kr::dl::BooleanTag>)
            return self().template member<&urd::record_of<V>::boolean_features>();
        else if constexpr (std::same_as<FeatureTag, runir::kr::dl::NumericalTag>)
            return self().template member<&urd::record_of<V>::numerical_features>();
        else
        {
            static_assert(urd::repr::dependent_false<FeatureTag>::value, "unhandled feature tag in Sketch::get_features");
        }
    }

    auto get_rules() const noexcept { return self().template member<&urd::record_of<V>::rules>(); }
};
}  // namespace urd

#endif
