#include <concepts>
#include <runir/kr/ps/dl/feature_data.hpp>
#include <runir/kr/ps/dl/feature_view.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/feature_index.hpp>

namespace runir::tests
{

namespace
{

template<typename Entity>
using View = ygg::View<ygg::Index<Entity>, kr::ps::ext::Repository>;

template<typename Entity, typename Expression>
concept ConcreteFeatureContract =
    std::constructible_from<ygg::Index<Entity>, ygg::uint_t> && std::totally_ordered<ygg::Index<Entity>> && std::totally_ordered<ygg::Data<Entity>>
    && std::totally_ordered<View<Entity>> && std::constructible_from<ygg::Data<Entity>, ygg::Index<Expression>, ::cista::offset::string>
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

template<typename Feature>
using ConcreteFeature = kr::ps::ConcreteFeature<kr::ExtFamilyTag, kr::DlTag, Feature>;

static_assert(ConcreteFeatureContract<ConcreteFeature<kr::dl::ConceptTag>, kr::dl::Constructor<kr::ExtFamilyTag, kr::dl::ConceptTag>>);
static_assert(ConcreteFeatureContract<ConcreteFeature<kr::dl::RoleTag>, kr::dl::Constructor<kr::ExtFamilyTag, kr::dl::RoleTag>>);
static_assert(ConcreteFeatureContract<ConcreteFeature<kr::ps::dl::BooleanFeature>, kr::dl::Constructor<kr::ExtFamilyTag, kr::dl::BooleanTag>>);
static_assert(ConcreteFeatureContract<ConcreteFeature<kr::ps::dl::NumericalFeature>, kr::dl::Constructor<kr::ExtFamilyTag, kr::dl::NumericalTag>>);

}  // namespace

}
