#include <concepts>
#include <runir/kr/ps/base/dl/feature_data.hpp>
#include <runir/kr/ps/base/dl/feature_view.hpp>
#include <runir/kr/ps/base/repository.hpp>
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

template<typename Entity>
concept FeatureContract = IndexedDataView<Entity> && requires(ygg::Data<Entity>& data, const View<Entity>& view) {
    data.index;
    data.value;
    data.clear();
    view.get_index();
    view.get_variant();
    view.get_symbol();
    view.get_expression();
    view.get_feature();
};

template<typename Entity>
concept ConcreteFeatureContract = IndexedDataView<Entity> && requires(ygg::Data<Entity>& data, const View<Entity>& view) {
    data.index;
    data.feature;
    data.symbol;
    data.clear();
    view.get_index();
    view.get_expression();
    view.get_feature();
    view.get_symbol();
};

using BooleanFeature = kr::ps::Feature<kr::BaseFamilyTag, kr::ps::dl::BooleanFeature>;
using NumericalFeature = kr::ps::Feature<kr::BaseFamilyTag, kr::ps::dl::NumericalFeature>;
using ConcreteBooleanFeature = kr::ps::ConcreteFeature<kr::BaseFamilyTag, kr::DlTag, kr::ps::dl::BooleanFeature>;
using ConcreteNumericalFeature = kr::ps::ConcreteFeature<kr::BaseFamilyTag, kr::DlTag, kr::ps::dl::NumericalFeature>;

static_assert(FeatureContract<BooleanFeature>);
static_assert(FeatureContract<NumericalFeature>);
static_assert(ConcreteFeatureContract<ConcreteBooleanFeature>);
static_assert(ConcreteFeatureContract<ConcreteNumericalFeature>);
static_assert(std::same_as<View<BooleanFeature>, kr::ps::base::dl::BooleanFeatureView>);
static_assert(std::same_as<View<NumericalFeature>, kr::ps::base::dl::NumericalFeatureView>);

}  // namespace

}
