#include <concepts>
#include <runir/kr/ps/base/dl/effect_data.hpp>
#include <runir/kr/ps/base/dl/effect_view.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/effect_index.hpp>

namespace runir::tests
{

namespace
{

template<typename Entity>
using View = ygg::View<ygg::Index<Entity>, kr::ps::base::Repository>;

template<typename Entity>
concept IndexedDataView = std::constructible_from<ygg::Index<Entity>, ygg::uint_t> && std::totally_ordered<ygg::Index<Entity>>
                          && std::totally_ordered<ygg::Data<Entity>> && std::totally_ordered<View<Entity>>;

using EffectVariant = kr::ps::ConcreteEffectVariant<kr::BaseFamilyTag, kr::DlTag>;

static_assert(IndexedDataView<EffectVariant> && requires(ygg::Data<EffectVariant>& data, const View<EffectVariant>& view) {
    data.index;
    data.value;
    data.clear();
    view.get_index();
    view.get_variant();
});

template<typename Entity>
concept EffectContract = IndexedDataView<Entity> && requires(ygg::Data<Entity>& data, const View<Entity>& view) {
    data.index;
    data.feature;
    data.clear();
    view.get_index();
    view.get_feature();
};

template<typename Feature, typename Observation>
using Effect = kr::ps::ConcreteEffect<kr::BaseFamilyTag, kr::DlTag, Feature, Observation>;

static_assert(EffectContract<Effect<kr::ps::dl::BooleanFeature, kr::ps::dl::Positive>>);
static_assert(EffectContract<Effect<kr::ps::dl::BooleanFeature, kr::ps::dl::Negative>>);
static_assert(EffectContract<Effect<kr::ps::dl::BooleanFeature, kr::ps::dl::Unchanged>>);
static_assert(EffectContract<Effect<kr::ps::dl::NumericalFeature, kr::ps::dl::Increases>>);
static_assert(EffectContract<Effect<kr::ps::dl::NumericalFeature, kr::ps::dl::Decreases>>);
static_assert(EffectContract<Effect<kr::ps::dl::NumericalFeature, kr::ps::dl::Unchanged>>);

}  // namespace

}
