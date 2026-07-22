#include <concepts>
#include <runir/kr/ps/ext/feature_data.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/feature_index.hpp>
#include <runir/kr/ps/feature_view.hpp>

namespace runir::tests
{

namespace
{

template<typename Entity>
using View = ygg::View<ygg::Index<Entity>, kr::ps::ext::Repository>;

template<typename Entity>
concept FeatureContract =
    std::constructible_from<ygg::Index<Entity>, ygg::uint_t> && std::totally_ordered<ygg::Index<Entity>> && std::totally_ordered<ygg::Data<Entity>>
    && std::totally_ordered<View<Entity>> && requires(ygg::Data<Entity>& data, const View<Entity>& view) {
           data.index;
           data.value;
           data.clear();
           view.get_index();
           view.get_variant();
           view.get_symbol();
           view.get_expression();
           view.get_feature();
       };

using ConceptFeature = kr::ps::Feature<kr::ExtFamilyTag, kr::dl::ConceptTag>;
using RoleFeature = kr::ps::Feature<kr::ExtFamilyTag, kr::dl::RoleTag>;
using BooleanFeature = kr::ps::Feature<kr::ExtFamilyTag, kr::ps::dl::BooleanFeature>;
using NumericalFeature = kr::ps::Feature<kr::ExtFamilyTag, kr::ps::dl::NumericalFeature>;

static_assert(FeatureContract<ConceptFeature>);
static_assert(FeatureContract<RoleFeature>);
static_assert(FeatureContract<BooleanFeature>);
static_assert(FeatureContract<NumericalFeature>);
static_assert(std::same_as<View<BooleanFeature>, kr::ps::ext::dl::BooleanFeatureView>);
static_assert(std::same_as<View<NumericalFeature>, kr::ps::ext::dl::NumericalFeatureView>);

}  // namespace

}
