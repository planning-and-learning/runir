#include "runir/kr/ps/declarations.hpp"

#include <cista/reflection/to_tuple.h>
#include <concepts>
#include <runir/kr/ps/dl/feature_data.hpp>
#include <runir/kr/ps/dl/feature_view.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <utility>
#include <urd/elements/flat.hpp>
#include <yggdrasil/core/config.hpp>

namespace runir::tests
{

namespace
{

template<typename Entity>
using View = urd::index_view<Entity, kr::ps::ext::Repository>;

template<typename Entity, typename Expression>
concept ConcreteFeatureContract =
    std::constructible_from<urd::index<Entity>, ygg::uint_t> && std::totally_ordered<urd::index<Entity>> && std::totally_ordered<urd::data<Entity>>
    && std::totally_ordered<View<Entity>> && std::constructible_from<urd::data<Entity>, urd::index<Expression>, ::cista::offset::string>
    && std::constructible_from<urd::data<Entity>, urd::index<Expression>, const std::string&> && requires(urd::data<Entity>& data, const View<Entity>& view) {
           data.index;
           data.feature;
           data.symbol;
           { data.feature } -> std::same_as<urd::index<Expression>&>;
           {
               ::cista::to_tuple(std::as_const(data))
           } -> std::same_as<std::tuple<const urd::index<Entity>&, const urd::index<Expression>&, const ::cista::offset::string&>>;
           requires std::same_as<std::remove_cvref_t<std::tuple_element_t<0, urd::detail::identity_t<urd::data<Entity>>>>, urd::index<Expression>>;
           requires std::same_as<std::remove_cvref_t<std::tuple_element_t<1, urd::detail::identity_t<urd::data<Entity>>>>, ::cista::offset::string>;
           view.index();
           view.get_expression();
           view.get_feature();
           view.get_symbol();
       };

template<typename Feature>
using ConcreteFeature = kr::ps::ConcreteFeature<kr::ExtFamilyTag, kr::DlTag, Feature>;

static_assert(ConcreteFeatureContract<ConcreteFeature<kr::dl::ConceptTag>, kr::dl::Constructor<kr::dl::ConceptTag>>);
static_assert(ConcreteFeatureContract<ConcreteFeature<kr::dl::RoleTag>, kr::dl::Constructor<kr::dl::RoleTag>>);
static_assert(ConcreteFeatureContract<ConcreteFeature<kr::dl::BooleanTag>, kr::dl::Constructor<kr::dl::BooleanTag>>);
static_assert(ConcreteFeatureContract<ConcreteFeature<kr::dl::NumericalTag>, kr::dl::Constructor<kr::dl::NumericalTag>>);

static_assert(ConcreteFeatureContract<ConcreteFeature<kr::ps::dl::QueryFeature>, kr::dl::Query<>>);

}  // namespace

}
