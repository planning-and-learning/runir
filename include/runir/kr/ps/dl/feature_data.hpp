#ifndef RUNIR_KR_PS_DL_FEATURE_DATA_HPP_
#define RUNIR_KR_PS_DL_FEATURE_DATA_HPP_

#include "runir/kr/dl/constructor_index.hpp"
#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/feature_data.hpp"

#include <cista/containers/string.h>
#include <concepts>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace runir::kr::ps::dl
{

template<typename FeatureTag>
using FeatureCategory = std::conditional_t<std::same_as<FeatureTag, BooleanFeature>,
                                           runir::kr::dl::BooleanTag,
                                           std::conditional_t<std::same_as<FeatureTag, NumericalFeature>, runir::kr::dl::NumericalTag, FeatureTag>>;

}  // namespace runir::kr::ps::dl

namespace ygg
{

template<runir::kr::FamilyTag Family, typename FeatureTag>
struct Data<runir::kr::ps::ConcreteFeature<Family, runir::kr::DlTag, FeatureTag>>
{
    using Expression = runir::kr::dl::Constructor<Family, runir::kr::ps::dl::FeatureCategory<FeatureTag>>;

    Index<runir::kr::ps::ConcreteFeature<Family, runir::kr::DlTag, FeatureTag>> index;
    Index<Expression> feature;
    ::cista::offset::string symbol;

    Data() = default;
    Data(Index<Expression> feature_, ::cista::offset::string symbol_) : index(), feature(feature_), symbol(std::move(symbol_)) {}
    Data(Index<Expression> feature_, const std::string& symbol_) : index(), feature(feature_), symbol(symbol_) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(feature);
        ygg::clear(symbol);
    }

    auto cista_members() const noexcept { return std::tie(index, feature, symbol); }
    auto identifying_members() const noexcept { return std::tie(feature, symbol); }
};

}  // namespace ygg

#endif
