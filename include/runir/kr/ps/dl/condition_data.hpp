#ifndef RUNIR_KR_PS_DL_CONDITION_DATA_HPP_
#define RUNIR_KR_PS_DL_CONDITION_DATA_HPP_

#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/family_traits.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <variant>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<runir::kr::FamilyTag Family>
struct Data<runir::kr::ps::ConcreteConditionVariant<Family, runir::kr::DlTag>>
{
    using Variant = ygg::IndexVariant<runir::kr::ps::detail::PsConcreteConditionTypes<Family>>;

    Index<runir::kr::ps::ConcreteConditionVariant<Family, runir::kr::DlTag>> index;
    Variant variant;

    Data() = default;
    Data(Variant variant_) : index(), variant(std::move(variant_)) {}
    template<typename C>
    using ViewVariant = ::ygg::ViewVariant<Variant, C>;
    template<typename C>
    explicit Data(const ViewVariant<C>& variant_) :
        index(),
        variant(std::visit([](const auto& view) -> Variant { return Variant(view.get_index()); }, variant_))
    {
    }

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::variant> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::variant> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, runir::kr::ps::dl::ConditionObservationTag<FeatureTag> ObservationTag>
struct Data<runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, FeatureTag, ObservationTag>>
{
    Index<runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, FeatureTag, ObservationTag>> index;
    Index<runir::kr::ps::Feature<Family, FeatureTag>> feature;

    Data() = default;
    Data(Index<runir::kr::ps::Feature<Family, FeatureTag>> feature_) : index(), feature(std::move(feature_)) {}
    template<typename C>
    Data(::ygg::View<Index<runir::kr::ps::Feature<Family, FeatureTag>>, C> feature_) : index(), feature()
    { set(feature_, feature); }

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::feature> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::feature> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

}  // namespace ygg

#endif
