#include <concepts>
#include <runir/kr/ps/ext/module_symbol_data.hpp>
#include <runir/kr/ps/ext/module_symbol_index.hpp>
#include <runir/kr/ps/ext/module_symbol_view.hpp>
#include <runir/kr/ps/ext/repository.hpp>

namespace runir::tests
{

using Entity = kr::ps::ext::ModuleSymbol;
using Index = ygg::Index<Entity>;
using Data = ygg::Data<Entity>;
using View = ygg::View<Index, kr::ps::ext::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, kr::ps::ext::ModuleSymbolView>);
static_assert(requires(Data& data) {
    data.index;
    data.name;
    data.clear();
});
static_assert(requires(const View& view) {
    view.get_index();
    view.get_name();
});

}
