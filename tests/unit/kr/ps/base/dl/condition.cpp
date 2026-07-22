#include <concepts>
#include <runir/kr/ps/base/dl/condition_data.hpp>
#include <runir/kr/ps/base/dl/condition_view.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/condition_index.hpp>

namespace runir::tests
{

namespace
{

template<typename Entity>
using View = ygg::View<ygg::Index<Entity>, kr::ps::base::Repository>;

template<typename Entity>
concept IndexedDataView = std::constructible_from<ygg::Index<Entity>, ygg::uint_t> && std::totally_ordered<ygg::Index<Entity>>
                          && std::totally_ordered<ygg::Data<Entity>> && std::totally_ordered<View<Entity>>;

using ConditionVariant = kr::ps::ConcreteConditionVariant<kr::BaseFamilyTag, kr::DlTag>;

static_assert(IndexedDataView<ConditionVariant> && requires(ygg::Data<ConditionVariant>& data, const View<ConditionVariant>& view) {
    data.index;
    data.value;
    data.clear();
    view.get_index();
    view.get_variant();
});

template<typename Entity>
concept ConditionContract = IndexedDataView<Entity> && requires(ygg::Data<Entity>& data, const View<Entity>& view) {
    data.index;
    data.feature;
    data.clear();
    view.get_index();
    view.get_feature();
};

template<typename Feature, typename Observation>
using Condition = kr::ps::ConcreteCondition<kr::BaseFamilyTag, kr::DlTag, Feature, Observation>;

static_assert(ConditionContract<Condition<kr::ps::dl::BooleanFeature, kr::ps::dl::Positive>>);
static_assert(ConditionContract<Condition<kr::ps::dl::BooleanFeature, kr::ps::dl::Negative>>);
static_assert(ConditionContract<Condition<kr::ps::dl::NumericalFeature, kr::ps::dl::EqualZero>>);
static_assert(ConditionContract<Condition<kr::ps::dl::NumericalFeature, kr::ps::dl::GreaterZero>>);

}  // namespace

}
