#include <concepts>
#include <runir/kr/ps/ext/module_program_data.hpp>
#include <runir/kr/ps/ext/module_program_index.hpp>
#include <runir/kr/ps/ext/module_program_view.hpp>
#include <runir/kr/ps/ext/repository.hpp>

namespace runir::tests
{

using Entity = kr::ps::ext::ModuleProgram;
using Index = ygg::Index<Entity>;
using Data = ygg::Data<Entity>;
using View = ygg::View<Index, kr::ps::ext::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, kr::ps::ext::ModuleProgramView>);
static_assert(requires(Data& data) {
    data.index;
    data.entry_module;
    data.modules;
    data.clear();
});
static_assert(requires(const View& view, ygg::Index<kr::ps::ext::ModuleSymbol> symbol) {
    view.get_index();
    view.get_entry_module();
    view.get_modules();
    view.find_module(symbol);
});

}
