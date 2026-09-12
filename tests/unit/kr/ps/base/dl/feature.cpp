#include <concepts>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/dl/feature_data.hpp>
#include <runir/kr/ps/dl/feature_view.hpp>
#include <runir/kr/ps/feature_index.hpp>

namespace runir::tests
{

namespace
{

template<typename Entity>
using View = ygg::View<ygg::Index<Entity>, kr::ps::base::Repository>;

template<typename Entity>
concept IndexedDataView = std::constructible_from<ygg::Index<Entity>, ygg::uint_t> && std::totally_ordered<ygg::Index<Entity>>
                          && std::totally_ordered<ygg::Data<Entity>> && std::totally_ordered<View<Entity>>;

template<typename Entity, typename Concrete>
concept FeatureContract = IndexedDataView<Entity> && std::same_as<typename ygg::Data<Entity>::Variant, ::cista::offset::variant<ygg::Index<Concrete>>>
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

template<typename Entity, typename Expression>
concept ConcreteFeatureContract =
    IndexedDataView<Entity> && std::constructible_from<ygg::Data<Entity>, ygg::Index<Expression>, ::cista::offset::string>
    && std::constructible_from<ygg::Data<Entity>, ygg::Index<Expression>, const std::string&> && requires(ygg::Data<Entity>& data, const View<Entity>& view) {
           data.index;
           data.feature;
           data.symbol;
           data.clear();
           { data.feature } -> std::same_as<ygg::Index<Expression>&>;
           { data.cista_members() } -> std::same_as<std::tuple<const ygg::Index<Entity>&, const ygg::Index<Expression>&, const ::cista::offset::string&>>;
           { data.identifying_members() } -> std::same_as<std::tuple<const ygg::Index<Expression>&, const ::cista::offset::string&>>;
           view.get_index();
           view.get_expression();
           view.get_feature();
           view.get_symbol();
       };

using BooleanFeature = kr::ps::Feature<kr::BaseFamilyTag, kr::ps::dl::BooleanFeature>;
using NumericalFeature = kr::ps::Feature<kr::BaseFamilyTag, kr::ps::dl::NumericalFeature>;
using ConcreteBooleanFeature = kr::ps::ConcreteFeature<kr::BaseFamilyTag, kr::DlTag, kr::ps::dl::BooleanFeature>;
using ConcreteNumericalFeature = kr::ps::ConcreteFeature<kr::BaseFamilyTag, kr::DlTag, kr::ps::dl::NumericalFeature>;

static_assert(FeatureContract<BooleanFeature, ConcreteBooleanFeature>);
static_assert(FeatureContract<NumericalFeature, ConcreteNumericalFeature>);
static_assert(ConcreteFeatureContract<ConcreteBooleanFeature, kr::dl::Constructor<kr::BaseFamilyTag, kr::dl::BooleanTag>>);
static_assert(ConcreteFeatureContract<ConcreteNumericalFeature, kr::dl::Constructor<kr::BaseFamilyTag, kr::dl::NumericalTag>>);
static_assert(std::same_as<View<BooleanFeature>, kr::ps::base::dl::BooleanFeatureView>);
static_assert(std::same_as<View<NumericalFeature>, kr::ps::base::dl::NumericalFeatureView>);

}  // namespace

}
