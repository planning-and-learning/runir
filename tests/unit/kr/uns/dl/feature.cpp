#include <concepts>
#include <runir/kr/ps/dl/feature_data.hpp>
#include <runir/kr/ps/dl/feature_view.hpp>
#include <runir/kr/ps/feature_index.hpp>
#include <runir/kr/ps/feature_view.hpp>
#include <runir/kr/uns/dl/evaluation.hpp>
#include <runir/kr/uns/repository.hpp>

namespace runir::tests
{

using Entity = kr::ps::ConcreteFeature<kr::UnsFamilyTag, kr::DlTag, kr::ps::dl::BooleanFeature>;
using Index = ygg::Index<Entity>;
using Data = ygg::Data<Entity>;
using View = ygg::View<Index, kr::uns::Repository>;
using ExpressionIndex = ygg::Index<kr::dl::Constructor<kr::UnsFamilyTag, kr::dl::BooleanTag>>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, kr::uns::dl::ConcreteBooleanFeatureView>);
static_assert(std::constructible_from<Data, ExpressionIndex, ::cista::offset::string>);
static_assert(std::constructible_from<Data, ExpressionIndex, const std::string&>);
static_assert(requires(Data& data) {
    data.index;
    data.feature;
    data.symbol;
    data.clear();
    { data.feature } -> std::same_as<ExpressionIndex&>;
    { data.cista_members() } -> std::same_as<std::tuple<const Index&, const ExpressionIndex&, const ::cista::offset::string&>>;
    { data.identifying_members() } -> std::same_as<std::tuple<const ExpressionIndex&, const ::cista::offset::string&>>;
    { canonicalize(data) } -> std::same_as<void>;
    { is_canonical(data) } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.get_index();
    view.get_expression();
    view.get_feature();
    view.get_symbol();
});

static_assert(requires(View feature, kr::dl::semantics::EvaluationContext<kr::UnsFamilyTag, tyr::GroundTag>& context) {
    { kr::ps::evaluate(feature, context) } -> std::same_as<bool>;
});
static_assert(requires(View feature, kr::dl::semantics::EvaluationContext<kr::UnsFamilyTag, tyr::LiftedTag>& context) {
    { kr::ps::evaluate(feature, context) } -> std::same_as<bool>;
});

}
