#include <concepts>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/base/sketch_data.hpp>
#include <runir/kr/ps/base/sketch_index.hpp>
#include <runir/kr/ps/base/sketch_view.hpp>

namespace runir::tests
{

using Entity = kr::ps::base::Sketch;
using Index = ygg::Index<Entity>;
using Data = ygg::Data<Entity>;
using View = ygg::View<Index, kr::ps::base::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, kr::ps::base::SketchView>);
static_assert(requires(Data& data) {
    data.index;
    data.boolean_features;
    data.numerical_features;
    data.rules;
    data.clear();
});
static_assert(requires(const View& view) {
    view.get_index();
    view.template get_features<kr::ps::dl::BooleanFeature>();
    view.template get_features<kr::ps::dl::NumericalFeature>();
    view.get_rules();
});

}
