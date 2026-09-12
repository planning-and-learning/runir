#include <concepts>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/feature_data.hpp>
#include <runir/kr/ps/feature_index.hpp>
#include <runir/kr/ps/feature_view.hpp>

namespace runir::tests
{

namespace
{

template<typename Entity>
using View = ygg::View<ygg::Index<Entity>, kr::ps::ext::Repository>;

template<typename Entity, typename Concrete>
concept FeatureContract =
    std::constructible_from<ygg::Index<Entity>, ygg::uint_t> && std::totally_ordered<ygg::Index<Entity>> && std::totally_ordered<ygg::Data<Entity>>
    && std::totally_ordered<View<Entity>> && std::same_as<typename ygg::Data<Entity>::Variant, ::cista::offset::variant<ygg::Index<Concrete>>>
    && requires(ygg::Data<Entity>& data, const View<Entity>& view) {
           data.index;
           data.variant;
           data.clear();
           { data.cista_members() } -> std::same_as<std::tuple<const ygg::Index<Entity>&, const typename ygg::Data<Entity>::Variant&>>;
           { data.identifying_members() } -> std::same_as<std::tuple<const typename ygg::Data<Entity>::Variant&>>;
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

template<typename FeatureTag>
using ConcreteFeature = kr::ps::ConcreteFeature<kr::ExtFamilyTag, kr::DlTag, FeatureTag>;

static_assert(FeatureContract<ConceptFeature, ConcreteFeature<kr::dl::ConceptTag>>);
static_assert(FeatureContract<RoleFeature, ConcreteFeature<kr::dl::RoleTag>>);
static_assert(FeatureContract<BooleanFeature, ConcreteFeature<kr::ps::dl::BooleanFeature>>);
static_assert(FeatureContract<NumericalFeature, ConcreteFeature<kr::ps::dl::NumericalFeature>>);
static_assert(std::same_as<View<BooleanFeature>, kr::ps::ext::dl::BooleanFeatureView>);
static_assert(std::same_as<View<NumericalFeature>, kr::ps::ext::dl::NumericalFeatureView>);

}  // namespace

}
