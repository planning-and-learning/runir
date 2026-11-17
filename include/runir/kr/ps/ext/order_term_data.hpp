#ifndef RUNIR_KR_PS_EXT_ORDER_TERM_DATA_HPP_
#define RUNIR_KR_PS_EXT_ORDER_TERM_DATA_HPP_

#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/ext/declarations.hpp"

#include <boost/mp11.hpp>
#include <cista/containers/variant.h>
#include <tuple>
#include <urd/elements/flat.hpp>
#include <utility>
#include <variant>

namespace runir::kr::ps::ext
{

/// The feature an order term orders by: a boolean or numerical feature.
using OrderTermFeature = urd::index_variant<
    boost::mp11::mp_transform_q<boost::mp11::mp_bind_front<runir::kr::ps::Feature, runir::kr::ExtFamilyTag>, runir::kr::dl::BooleanOrNumericalTags>>;

}  // namespace runir::kr::ps::ext

namespace urd
{

template<>
struct data<runir::kr::ps::ext::OrderTerm>
{
    urd::index<runir::kr::ps::ext::OrderTerm> index;
    runir::kr::ps::ext::OrderDirection direction = runir::kr::ps::ext::OrderDirection::MIN;
    runir::kr::ps::ext::OrderTermFeature feature;

    data() = default;

    data(runir::kr::ps::ext::OrderDirection direction_, runir::kr::ps::ext::OrderTermFeature feature_) :
        index(),
        direction(direction_),
        feature(std::move(feature_))
    {
    }

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"direction", &data::direction>, urd::field<"feature", &data::feature>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&data::direction, &data::feature> {}; }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::direction, &data::feature> {}; }
};

}  // namespace urd

#endif
