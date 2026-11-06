#ifndef RUNIR_KR_PS_EXT_ORDER_TERM_DATA_HPP_
#define RUNIR_KR_PS_EXT_ORDER_TERM_DATA_HPP_

#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/ext/declarations.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <variant>
#include <urd/elements/flat.hpp>
#include <boost/mp11.hpp>

namespace urd
{

template<>
struct data<runir::kr::ps::ext::OrderTerm>
{
    using Feature = urd::index_variant<boost::mp11::mp_transform_q<boost::mp11::mp_bind_front<runir::kr::ps::Feature, runir::kr::ExtFamilyTag>, runir::kr::dl::BooleanOrNumericalTags>>;

    urd::index<runir::kr::ps::ext::OrderTerm> index;
    runir::kr::ps::ext::OrderDirection direction = runir::kr::ps::ext::OrderDirection::MIN;
    Feature feature;

    data() = default;
    data(runir::kr::ps::ext::OrderDirection direction_, Feature feature_) : index(), direction(direction_), feature(std::move(feature_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::direction, &data::feature> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::direction, &data::feature> {}; }
};

}  // namespace urd

#endif
