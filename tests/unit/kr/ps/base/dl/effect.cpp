#include "runir/kr/ps/declarations.hpp"

#include <boost/mp11.hpp>
#include <concepts>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/dl/effect_data.hpp>
#include <runir/kr/ps/dl/effect_view.hpp>
#include <urd/elements/flat.hpp>
#include <yggdrasil/core/config.hpp>

namespace runir::tests
{

namespace
{

template<typename Entity>
using View = urd::index_view<Entity, kr::ps::base::Repository>;

template<typename Entity>
concept IndexedDataView = std::constructible_from<urd::index<Entity>, ygg::uint_t> && std::totally_ordered<urd::index<Entity>>
                          && std::totally_ordered<urd::data<Entity>> && std::totally_ordered<View<Entity>>;

template<typename Entity>
concept EffectContract = IndexedDataView<Entity> && requires(urd::data<Entity>& data, const View<Entity>& view) {
    data.index;
    data.feature;
    view.index();
    view.get_feature();
};

template<typename Feature, typename Observation>
using Effect = kr::ps::ConcreteEffect<kr::BaseFamilyTag, kr::DlTag, Feature, Observation>;

using ConcreteTypes = boost::mp11::mp_list<Effect<kr::dl::BooleanTag, kr::ps::dl::Positive>,
                                           Effect<kr::dl::BooleanTag, kr::ps::dl::Negative>,
                                           Effect<kr::dl::BooleanTag, kr::ps::dl::Unchanged>,
                                           Effect<kr::dl::NumericalTag, kr::ps::dl::Increases>,
                                           Effect<kr::dl::NumericalTag, kr::ps::dl::Decreases>,
                                           Effect<kr::dl::NumericalTag, kr::ps::dl::Unchanged>>;
static_assert(std::same_as<kr::ps::EffectVariant<kr::BaseFamilyTag>, urd::index_variant<ConcreteTypes>>);
static_assert(std::same_as<kr::ps::PsEffectTypes<kr::BaseFamilyTag>, ConcreteTypes>);

static_assert(EffectContract<Effect<kr::dl::BooleanTag, kr::ps::dl::Positive>>);
static_assert(EffectContract<Effect<kr::dl::BooleanTag, kr::ps::dl::Negative>>);
static_assert(EffectContract<Effect<kr::dl::BooleanTag, kr::ps::dl::Unchanged>>);
static_assert(EffectContract<Effect<kr::dl::NumericalTag, kr::ps::dl::Increases>>);
static_assert(EffectContract<Effect<kr::dl::NumericalTag, kr::ps::dl::Decreases>>);
static_assert(EffectContract<Effect<kr::dl::NumericalTag, kr::ps::dl::Unchanged>>);

}  // namespace

}
