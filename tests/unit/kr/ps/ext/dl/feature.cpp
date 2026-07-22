#include <concepts>
#include <runir/kr/ps/ext/dl/feature_data.hpp>
#include <runir/kr/ps/ext/dl/feature_view.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/feature_index.hpp>

namespace runir::tests
{

namespace
{

template<typename Entity>
using View = ygg::View<ygg::Index<Entity>, kr::ps::ext::Repository>;

template<typename Entity>
concept ConcreteFeatureContract =
    std::constructible_from<ygg::Index<Entity>, ygg::uint_t> && std::totally_ordered<ygg::Index<Entity>> && std::totally_ordered<ygg::Data<Entity>>
    && std::totally_ordered<View<Entity>> && requires(ygg::Data<Entity>& data, const View<Entity>& view) {
           data.index;
           data.feature;
           data.symbol;
           data.clear();
           view.get_index();
           view.get_expression();
           view.get_feature();
           view.get_symbol();
       };

template<typename Feature>
using ConcreteFeature = kr::ps::ConcreteFeature<kr::ExtFamilyTag, kr::DlTag, Feature>;

static_assert(ConcreteFeatureContract<ConcreteFeature<kr::dl::ConceptTag>>);
static_assert(ConcreteFeatureContract<ConcreteFeature<kr::dl::RoleTag>>);
static_assert(ConcreteFeatureContract<ConcreteFeature<kr::ps::dl::BooleanFeature>>);
static_assert(ConcreteFeatureContract<ConcreteFeature<kr::ps::dl::NumericalFeature>>);

}  // namespace

}
