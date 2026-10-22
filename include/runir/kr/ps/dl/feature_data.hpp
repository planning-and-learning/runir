#ifndef RUNIR_KR_PS_DL_FEATURE_DATA_HPP_
#define RUNIR_KR_PS_DL_FEATURE_DATA_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/family_traits.hpp"
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

template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag>
struct FeatureExpression
{
    using Type = runir::kr::dl::Constructor<FeatureTag>;
};

template<runir::kr::FamilyTag Family>
struct FeatureExpression<Family, QueryFeature>
{
    using Type = runir::kr::dl::Query<>;
};

}  // namespace runir::kr::ps::dl

namespace ygg
{

template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag>
struct Data<runir::kr::ps::ConcreteFeature<Family, runir::kr::DlTag, FeatureTag>>
{
    using Expression = typename runir::kr::ps::dl::FeatureExpression<Family, FeatureTag>::Type;

    Index<runir::kr::ps::ConcreteFeature<Family, runir::kr::DlTag, FeatureTag>> index;
    Index<Expression> feature;
    ::cista::offset::string symbol;

    Data() = default;
    Data(Index<Expression> feature_, ::cista::offset::string symbol_) : index(), feature(std::move(feature_)), symbol(std::move(symbol_)) {}
    template<typename C>
    Data(::ygg::View<Index<Expression>, C> feature_, ::cista::offset::string symbol_) : index(), feature(), symbol(std::move(symbol_))
    {
        set(feature_, feature);
    }

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::feature, &Data::symbol> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::feature, &Data::symbol> {}; }
    void clear() noexcept
    {
        ygg::clear_members(*this);
    }
};

}  // namespace ygg

#endif
