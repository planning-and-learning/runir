#ifndef RUNIR_KR_PS_EXT_ORDER_TERM_DATA_HPP_
#define RUNIR_KR_PS_EXT_ORDER_TERM_DATA_HPP_

#include "runir/kr/ps/ext/order_term_index.hpp"
#include "runir/kr/ps/feature_index.hpp"
#include "runir/kr/ps/dl/declarations.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::ps::ext::OrderTerm>
{
    using Feature = ::cista::offset::variant<Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::BooleanFeature>>,
                                             Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::NumericalFeature>>>;

    Index<runir::kr::ps::ext::OrderTerm> index;
    runir::kr::ps::ext::OrderDirection direction = runir::kr::ps::ext::OrderDirection::MIN;
    Feature feature;

    Data() = default;
    Data(runir::kr::ps::ext::OrderDirection direction_, Feature feature_) : index(), direction(direction_), feature(std::move(feature_)) {}

    void clear() noexcept
    {
        ygg::clear(index);
        direction = runir::kr::ps::ext::OrderDirection::MIN;
        ygg::clear(feature);
    }

    auto cista_members() const noexcept { return std::tie(index, direction, feature); }
    auto identifying_members() const noexcept { return std::tie(direction, feature); }
};

}  // namespace ygg

#endif
