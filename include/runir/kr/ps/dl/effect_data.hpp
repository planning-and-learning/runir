#ifndef RUNIR_KR_PS_DL_EFFECT_DATA_HPP_
#define RUNIR_KR_PS_DL_EFFECT_DATA_HPP_

#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/family_traits.hpp"
#include <yggdrasil/containers/variant.hpp>

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <variant>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<runir::kr::FamilyTag Family>
struct Data<runir::kr::ps::ConcreteEffectVariant<Family, runir::kr::DlTag>>
{
    using Variant = ygg::IndexVariant<runir::kr::ps::detail::PsConcreteEffectTypes<Family>>;

    Index<runir::kr::ps::ConcreteEffectVariant<Family, runir::kr::DlTag>> index;
    Variant variant;

    Data() = default;
    Data(Variant variant_) : index(), variant(std::move(variant_)) {}
    template<typename C>
    using ViewVariant = ::ygg::ViewVariant<Variant, C>;
    template<typename C>
    explicit Data(const ViewVariant<C>& variant_) : index(), variant(std::visit([](const auto& view) -> Variant { return Variant(view.get_index()); }, variant_))
    {
    }

    auto cista_members() noexcept { return std::tie(index, variant); }
    auto cista_members() const noexcept { return std::tie(index, variant); }
    auto identifying_members() const noexcept { return std::tie(variant); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, runir::kr::ps::dl::EffectObservationTag<FeatureTag> ObservationTag>
struct Data<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, FeatureTag, ObservationTag>>
{
    Index<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, FeatureTag, ObservationTag>> index;
    Index<runir::kr::ps::Feature<Family, FeatureTag>> feature;

    Data() = default;
    Data(Index<runir::kr::ps::Feature<Family, FeatureTag>> feature_) : index(), feature(std::move(feature_)) {}
    template<typename C>
    Data(::ygg::View<Index<runir::kr::ps::Feature<Family, FeatureTag>>, C> feature_) : index(), feature()
    {
        set(feature_, feature);
    }

    auto cista_members() noexcept { return std::tie(index, feature); }
    auto cista_members() const noexcept { return std::tie(index, feature); }
    auto identifying_members() const noexcept { return std::tie(feature); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}  // namespace ygg

#endif
