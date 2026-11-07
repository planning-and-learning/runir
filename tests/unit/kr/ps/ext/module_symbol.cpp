#include "runir/kr/ps/ext/declarations.hpp"

#include <concepts>
#include <runir/kr/ps/module_symbol_data.hpp>
#include <runir/kr/ps/module_symbol_view.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <urd/elements/flat.hpp>
#include <yggdrasil/core/config.hpp>

namespace runir::tests
{

using Entity = kr::ps::ModuleSymbol<kr::ExtFamilyTag>;
using Index = urd::index<Entity>;
using Data = urd::data<Entity>;
using View = urd::index_view<Entity, kr::ps::ext::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, kr::ps::ext::ModuleSymbolView>);
static_assert(requires(Data& data) {
    data.index;
    data.name;
});
static_assert(requires(const View& view) {
    view.index();
    view.get_name();
});

}
