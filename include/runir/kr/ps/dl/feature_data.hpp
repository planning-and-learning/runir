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
#include <urd/elements/flat.hpp>

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

namespace urd
{

template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag>
struct data<runir::kr::ps::ConcreteFeature<Family, runir::kr::DlTag, FeatureTag>>
{
    using Expression = typename runir::kr::ps::dl::FeatureExpression<Family, FeatureTag>::Type;

    urd::index<runir::kr::ps::ConcreteFeature<Family, runir::kr::DlTag, FeatureTag>> index;
    urd::index<Expression> feature;
    ::cista::offset::string symbol;

    data() = default;
    data(urd::index<Expression> feature_, ::cista::offset::string symbol_) : index(), feature(std::move(feature_)), symbol(std::move(symbol_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::feature, &data::symbol> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::feature, &data::symbol> {}; }
};

}  // namespace urd

#endif
