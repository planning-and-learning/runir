#include "runir/kr/ps/ext/declarations.hpp"

#include <concepts>
#include <runir/kr/ps/ext/program_data.hpp>
#include <runir/kr/ps/ext/program_view.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <urd/elements/flat.hpp>
#include <yggdrasil/core/config.hpp>

namespace runir::tests
{

using Entity = kr::ps::ext::Program;
using Index = urd::index<Entity>;
using Data = urd::data<Entity>;
using View = urd::view<Entity, kr::ps::ext::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, kr::ps::ext::ProgramView>);
static_assert(requires(Data& data) {
    data.index;
    data.entry_module;
    data.modules;
});
static_assert(requires(const View& view, kr::ps::ext::ModuleSymbolView symbol) {
    view.index();
    view.get_entry_module();
    view.get_modules();
    view.find_module(symbol);
});

}
