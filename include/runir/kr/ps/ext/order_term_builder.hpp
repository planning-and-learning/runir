#ifndef RUNIR_KR_PS_EXT_ORDER_TERM_BUILDER_HPP_
#define RUNIR_KR_PS_EXT_ORDER_TERM_BUILDER_HPP_

#include "runir/kr/ps/ext/order_term_data.hpp"
#include "runir/kr/ps/ext/order_term_view.hpp"

#include <optional>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of OrderTerm: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::ps::ext::OrderTerm, C>
{
    urd::index<runir::kr::ps::ext::OrderTerm> index;
    urd::built_t<runir::kr::ps::ext::OrderDirection, C> direction = runir::kr::ps::ext::OrderDirection::MIN;
    urd::built_t<runir::kr::ps::ext::OrderTermFeature, C> feature;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"direction", &builder::direction>, urd::field<"feature", &builder::feature>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::direction, &builder::feature> {}; }
};

}  // namespace urd

#endif
