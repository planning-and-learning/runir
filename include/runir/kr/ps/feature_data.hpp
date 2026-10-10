#ifndef RUNIR_KR_PS_FEATURE_DATA_HPP_
#define RUNIR_KR_PS_FEATURE_DATA_HPP_

#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/dl/declarations.hpp"
#include <yggdrasil/containers/variant.hpp>

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <variant>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag>
struct Data<runir::kr::ps::Feature<Family, FeatureTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::ps::ConcreteFeature<Family, runir::kr::DlTag, FeatureTag>>>;

    Index<runir::kr::ps::Feature<Family, FeatureTag>> index;
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

}  // namespace ygg

#endif
