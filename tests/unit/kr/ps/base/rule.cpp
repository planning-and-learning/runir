#include <concepts>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/base/rule_data.hpp>
#include <runir/kr/ps/base/rule_index.hpp>
#include <runir/kr/ps/base/rule_view.hpp>

namespace runir::tests
{

using Entity = kr::ps::base::Rule;
using Index = ygg::Index<Entity>;
using Data = ygg::Data<Entity>;
using View = ygg::View<Index, kr::ps::base::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, kr::ps::base::RuleView>);
static_assert(requires(Data& data) {
    data.index;
    data.symbol;
    data.conditions;
    data.effects;
    data.clear();
});
static_assert(requires(const View& view) {
    view.get_index();
    view.get_symbol();
    view.get_conditions();
    view.get_effects();
});

}
