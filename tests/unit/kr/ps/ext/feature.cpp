#include "runir/kr/ps/declarations.hpp"

#include <cista/reflection/to_tuple.h>
#include <concepts>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/feature_data.hpp>
#include <runir/kr/ps/feature_view.hpp>
#include <utility>
#include <urd/elements/flat.hpp>
#include <yggdrasil/core/config.hpp>

namespace runir::tests
{

namespace
{

template<typename Entity>
using View = urd::index_view<Entity, kr::ps::ext::Repository>;

template<typename Entity, typename Concrete>
concept FeatureContract =
    std::constructible_from<urd::index<Entity>, ygg::uint_t> && std::totally_ordered<urd::index<Entity>> && std::totally_ordered<urd::data<Entity>>
    && std::totally_ordered<View<Entity>> && std::same_as<typename urd::data<Entity>::Variant, ::cista::offset::variant<urd::index<Concrete>>>
    && requires(urd::data<Entity>& data, const View<Entity>& view) {
           data.index;
           data.variant;
           { ::cista::to_tuple(std::as_const(data)) } -> std::same_as<std::tuple<const urd::index<Entity>&, const typename urd::data<Entity>::Variant&>>;
           requires std::same_as<std::remove_cvref_t<std::tuple_element_t<0, urd::detail::identity_t<urd::data<Entity>>>>, typename urd::data<Entity>::Variant>;
           view.index();
           view.get_variant();
           view.get_symbol();
           view.get_expression();
           view.get_feature();
       };

using ConceptFeature = kr::ps::Feature<kr::ExtFamilyTag, kr::dl::ConceptTag>;
using RoleFeature = kr::ps::Feature<kr::ExtFamilyTag, kr::dl::RoleTag>;
using BooleanFeature = kr::ps::Feature<kr::ExtFamilyTag, kr::dl::BooleanTag>;
using NumericalFeature = kr::ps::Feature<kr::ExtFamilyTag, kr::dl::NumericalTag>;
using QueryFeature = kr::ps::Feature<kr::ExtFamilyTag, kr::ps::dl::QueryFeature>;

template<typename FeatureTag>
using ConcreteFeature = kr::ps::ConcreteFeature<kr::ExtFamilyTag, kr::DlTag, FeatureTag>;

static_assert(FeatureContract<ConceptFeature, ConcreteFeature<kr::dl::ConceptTag>>);
static_assert(FeatureContract<RoleFeature, ConcreteFeature<kr::dl::RoleTag>>);
static_assert(FeatureContract<BooleanFeature, ConcreteFeature<kr::dl::BooleanTag>>);
static_assert(FeatureContract<NumericalFeature, ConcreteFeature<kr::dl::NumericalTag>>);
static_assert(FeatureContract<QueryFeature, ConcreteFeature<kr::ps::dl::QueryFeature>>);
static_assert(std::same_as<View<QueryFeature>, kr::ps::ext::dl::QueryFeatureView>);
static_assert(std::same_as<View<BooleanFeature>, kr::ps::ext::dl::BooleanFeatureView>);
static_assert(std::same_as<View<NumericalFeature>, kr::ps::ext::dl::NumericalFeatureView>);

}  // namespace

}
