#include "runir/kr/ps/declarations.hpp"

#include <concepts>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/dl/condition_data.hpp>
#include <runir/kr/ps/dl/condition_view.hpp>
#include <urd/elements/flat.hpp>
#include <boost/mp11.hpp>
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
concept ConditionContract = IndexedDataView<Entity> && requires(urd::data<Entity>& data, const View<Entity>& view) {
    data.index;
    data.feature;
    view.index();
    view.get_feature();
};

template<typename Feature, typename Observation>
using Condition = kr::ps::ConcreteCondition<kr::BaseFamilyTag, kr::DlTag, Feature, Observation>;

using ConcreteTypes = boost::mp11::mp_list<Condition<kr::dl::BooleanTag, kr::ps::dl::Positive>,
                                    Condition<kr::dl::BooleanTag, kr::ps::dl::Negative>,
                                    Condition<kr::dl::NumericalTag, kr::ps::dl::EqualZero>,
                                    Condition<kr::dl::NumericalTag, kr::ps::dl::GreaterZero>>;
static_assert(std::same_as<kr::ps::ConditionVariant<kr::BaseFamilyTag>, urd::index_variant<ConcreteTypes>>);
static_assert(std::same_as<kr::ps::PsConditionTypes<kr::BaseFamilyTag>, ConcreteTypes>);

static_assert(ConditionContract<Condition<kr::dl::BooleanTag, kr::ps::dl::Positive>>);
static_assert(ConditionContract<Condition<kr::dl::BooleanTag, kr::ps::dl::Negative>>);
static_assert(ConditionContract<Condition<kr::dl::NumericalTag, kr::ps::dl::EqualZero>>);
static_assert(ConditionContract<Condition<kr::dl::NumericalTag, kr::ps::dl::GreaterZero>>);

}  // namespace

}
