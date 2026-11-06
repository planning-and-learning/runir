#include "runir/kr/ps/declarations.hpp"

#include <cista/reflection/to_tuple.h>
#include <concepts>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/dl/feature_data.hpp>
#include <runir/kr/ps/dl/feature_view.hpp>
#include <utility>
#include <urd/elements/flat.hpp>
#include <yggdrasil/core/config.hpp>

namespace runir::tests
{

namespace
{

template<typename Entity>
using View = urd::view<Entity, kr::ps::base::Repository>;

template<typename Entity>
concept IndexedDataView = std::constructible_from<urd::index<Entity>, ygg::uint_t> && std::totally_ordered<urd::index<Entity>>
                          && std::totally_ordered<urd::data<Entity>> && std::totally_ordered<View<Entity>>;

template<typename Entity, typename Concrete>
concept FeatureContract = IndexedDataView<Entity> && std::same_as<typename urd::data<Entity>::Variant, ::cista::offset::variant<urd::index<Concrete>>>
                          && requires(urd::data<Entity>& data, const View<Entity>& view) {
                                 data.index;
                                 data.variant;
                                 {
                                     ::cista::to_tuple(std::as_const(data))
                                 } -> std::same_as<std::tuple<const urd::index<Entity>&, const typename urd::data<Entity>::Variant&>>;
                                 requires std::same_as<std::remove_cvref_t<std::tuple_element_t<0, urd::detail::identity_t<urd::data<Entity>>>>, typename urd::data<Entity>::Variant>;
                                 view.index();
                                 view.get_variant();
                                 view.get_symbol();
                                 view.get_expression();
                                 view.get_feature();
                             };

template<typename Entity, typename Expression>
concept ConcreteFeatureContract =
    IndexedDataView<Entity> && std::constructible_from<urd::data<Entity>, urd::index<Expression>, ::cista::offset::string>
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

using BooleanFeature = kr::ps::Feature<kr::BaseFamilyTag, kr::dl::BooleanTag>;
using NumericalFeature = kr::ps::Feature<kr::BaseFamilyTag, kr::dl::NumericalTag>;
using ConcreteBooleanFeature = kr::ps::ConcreteFeature<kr::BaseFamilyTag, kr::DlTag, kr::dl::BooleanTag>;
using ConcreteNumericalFeature = kr::ps::ConcreteFeature<kr::BaseFamilyTag, kr::DlTag, kr::dl::NumericalTag>;

static_assert(FeatureContract<BooleanFeature, ConcreteBooleanFeature>);
static_assert(FeatureContract<NumericalFeature, ConcreteNumericalFeature>);
static_assert(ConcreteFeatureContract<ConcreteBooleanFeature, kr::dl::Constructor<kr::dl::BooleanTag>>);
static_assert(ConcreteFeatureContract<ConcreteNumericalFeature, kr::dl::Constructor<kr::dl::NumericalTag>>);
static_assert(std::same_as<View<BooleanFeature>, kr::ps::base::dl::BooleanFeatureView>);
static_assert(std::same_as<View<NumericalFeature>, kr::ps::base::dl::NumericalFeatureView>);

}  // namespace

}
