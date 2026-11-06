#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"

#include <concepts>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/base/rule_data.hpp>
#include <runir/kr/ps/base/rule_view.hpp>
#include <urd/elements/flat.hpp>
#include <yggdrasil/core/config.hpp>

namespace runir::tests
{

using Entity = kr::ps::Rule<kr::BaseFamilyTag>;
using Index = urd::index<Entity>;
using Data = urd::data<Entity>;
using View = urd::view<Entity, kr::ps::base::Repository>;

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
});
static_assert(requires(const View& view) {
    view.index();
    view.get_symbol();
    view.get_conditions();
    view.get_effects();
});

}
