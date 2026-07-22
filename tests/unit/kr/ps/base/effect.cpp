#include <concepts>
#include <runir/kr/ps/base/effect_data.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/effect_index.hpp>
#include <runir/kr/ps/effect_view.hpp>

namespace runir::tests
{

using Entity = kr::ps::EffectVariant<kr::BaseFamilyTag>;
using Index = ygg::Index<Entity>;
using Data = ygg::Data<Entity>;
using View = ygg::View<Index, kr::ps::base::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(requires(Data& data) {
    data.index;
    data.value;
    data.clear();
});
static_assert(requires(const View& view) {
    view.get_index();
    view.get_variant();
});

}
