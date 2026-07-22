#include <concepts>
#include <runir/kr/uns/dl/feature_data.hpp>
#include <runir/kr/uns/dl/feature_index.hpp>
#include <runir/kr/uns/dl/feature_view.hpp>
#include <runir/kr/uns/repository.hpp>

namespace runir::tests
{

using Entity = kr::uns::dl::Feature;
using Index = ygg::Index<Entity>;
using Data = ygg::Data<Entity>;
using View = ygg::View<Index, kr::uns::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, kr::uns::ConcreteFeatureView>);
static_assert(requires(Data& data) {
    data.index;
    data.feature;
    data.symbol;
    data.clear();
});
static_assert(requires(const View& view) {
    view.get_index();
    view.get_expression();
    view.get_feature();
    view.get_symbol();
});

}
