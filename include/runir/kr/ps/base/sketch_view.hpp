#ifndef RUNIR_KR_PS_BASE_SKETCH_VIEW_HPP_
#define RUNIR_KR_PS_BASE_SKETCH_VIEW_HPP_

#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/base/rule_view.hpp"
#include "runir/kr/ps/base/sketch_data.hpp"

#include <concepts>
#include <cstddef>
#include <tuple>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace runir::kr::ps::base
{
/// The syntactic complexity of a sketch (defined in syntactic_complexity.hpp), declared before its view, whose
/// functions() list it.
template<typename C>
std::size_t syntactic_complexity(const urd::index_view<Sketch, C>& view);
}  // namespace runir::kr::ps::base

namespace urd
{
template<typename V>
struct view_mixin<runir::kr::ps::base::Sketch, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_boolean_features() const noexcept { return self().template member<&urd::record_of<V>::boolean_features>(); }

    auto get_numerical_features() const noexcept { return self().template member<&urd::record_of<V>::numerical_features>(); }

    auto get_rules() const noexcept { return self().template member<&urd::record_of<V>::rules>(); }

    template<typename FeatureTag>
    auto get_features() const noexcept
    {
        if constexpr (std::same_as<FeatureTag, runir::kr::dl::BooleanTag>)
            return get_boolean_features();
        else if constexpr (std::same_as<FeatureTag, runir::kr::dl::NumericalTag>)
            return get_numerical_features();
        else
        {
            static_assert(false, "unhandled feature tag in Sketch::get_features");
        }
    }

    static constexpr auto functions() noexcept
    { return urd::Functions<urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::ps::base::syntactic_complexity)>> {}; }
};
}  // namespace urd

#endif
