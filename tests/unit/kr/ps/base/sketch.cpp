#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"

#include <concepts>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/base/sketch_data.hpp>
#include <runir/kr/ps/base/sketch_view.hpp>
#include <urd/elements/flat.hpp>
#include <yggdrasil/core/config.hpp>

namespace runir::tests::ps_base_sketch
{

using Entity = kr::ps::base::Sketch;
using Index = urd::index<Entity>;
using Data = urd::data<Entity>;
using View = urd::index_view<Entity, kr::ps::base::Repository>;

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
});
static_assert(requires(const View& view) {
    view.index();
    view.template get_features<kr::dl::BooleanTag>();
    view.template get_features<kr::dl::NumericalTag>();
    view.get_rules();
});

}
