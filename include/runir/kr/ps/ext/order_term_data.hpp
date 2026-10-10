#ifndef RUNIR_KR_PS_EXT_ORDER_TERM_DATA_HPP_
#define RUNIR_KR_PS_EXT_ORDER_TERM_DATA_HPP_

#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/ext/declarations.hpp"
#include <yggdrasil/containers/variant.hpp>

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <variant>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::ps::ext::OrderTerm>
{
    using Feature = ::ygg::IndexVariant<::ygg::MapTypeListSecondT<runir::kr::ps::Feature, runir::kr::ExtFamilyTag, runir::kr::dl::BooleanOrNumericalTags>>;

    Index<runir::kr::ps::ext::OrderTerm> index;
    runir::kr::ps::ext::OrderDirection direction = runir::kr::ps::ext::OrderDirection::MIN;
    Feature feature;

    Data() = default;
    Data(runir::kr::ps::ext::OrderDirection direction_, Feature feature_) : index(), direction(direction_), feature(std::move(feature_)) {}
    template<typename C>
    using ViewVariant = ::ygg::ViewVariant<Feature, C>;
    template<typename C>
    Data(runir::kr::ps::ext::OrderDirection direction_, const ViewVariant<C>& feature_) :
        index(),
        direction(direction_),
        feature(std::visit([](const auto& view) -> Feature { return Feature(view.get_index()); }, feature_))
    {
    }

    auto cista_members() noexcept { return std::tie(index, direction, feature); }
    auto cista_members() const noexcept { return std::tie(index, direction, feature); }
    auto identifying_members() const noexcept { return std::tie(direction, feature); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}  // namespace ygg

#endif
