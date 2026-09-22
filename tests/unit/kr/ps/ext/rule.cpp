#include <concepts>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/ext/rule_data.hpp>
#include <runir/kr/ps/ext/rule_index.hpp>
#include <runir/kr/ps/ext/rule_view.hpp>

namespace runir::tests
{

namespace
{

template<kr::ps::ext::RuleKind Kind>
using Entity = kr::ps::ext::Rule<Kind>;

template<kr::ps::ext::RuleKind Kind>
using View = ygg::View<ygg::Index<Entity<Kind>>, kr::ps::ext::Repository>;

template<typename Kind>
concept RuleContract = kr::ps::ext::RuleKind<Kind> && std::constructible_from<ygg::Index<Entity<Kind>>, ygg::uint_t>
                       && std::totally_ordered<ygg::Index<Entity<Kind>>> && std::totally_ordered<ygg::Data<Entity<Kind>>> && std::totally_ordered<View<Kind>>
                       && std::same_as<View<Kind>, kr::ps::ext::RuleView<Kind>> && requires(ygg::Data<Entity<Kind>>& data, const View<Kind>& view) {
                              data.index;
                              data.source;
                              data.target;
                              data.conditions;
                              data.clear();
                              view.get_index();
                              view.get_source();
                              view.get_target();
                              view.get_conditions();
                          };

template<kr::dl::CategoryTag Category>
using Load = kr::ps::ext::LoadTag<Category>;

template<typename Kind>
concept BindingRuleContract = kr::ps::ext::BindingRuleKind<Kind> && RuleContract<Kind>
                             && requires(ygg::Data<Entity<Kind>>& data, const View<Kind>& view) {
        data.feature;
        data.reg;
        data.effects;
        view.get_feature();
        view.get_register();
        view.get_effects();
    };

using Sketch = kr::ps::ext::SketchTag;
using Do = kr::ps::ext::DoTag;
using Action = kr::ps::ext::ActionTag;
using Call = kr::ps::ext::CallTag;

static_assert(BindingRuleContract<Load<kr::dl::ConceptTag>>);
static_assert(BindingRuleContract<Load<kr::dl::RoleTag>>);
static_assert(BindingRuleContract<kr::ps::ext::ChooseTag<kr::dl::ConceptTag>>);
static_assert(BindingRuleContract<kr::ps::ext::ChooseTag<kr::dl::RoleTag>>);
static_assert(kr::ps::ext::ChooseRuleView<View<kr::ps::ext::ChooseTag<kr::dl::ConceptTag>>>);
static_assert(!kr::ps::ext::LoadRuleView<View<kr::ps::ext::ChooseTag<kr::dl::RoleTag>>>);
static_assert(RuleContract<Sketch> && requires(ygg::Data<Entity<Sketch>>& data, const View<Sketch>& view) {
    data.effects;
    view.get_effects();
});
static_assert(RuleContract<Do> && requires(ygg::Data<Entity<Do>>& data, const View<Do>& view) {
    data.effects;
    data.action_name;
    data.arguments;
    view.get_effects();
    view.get_action_name();
    view.get_action_arguments();
});
static_assert(RuleContract<Action> && requires(ygg::Data<Entity<Action>>& data, const View<Action>& view) {
    data.effects;
    data.action_name;
    data.query_feature;
    view.get_effects();
    view.get_action_name();
    view.get_query_feature();
});
static_assert(RuleContract<Call> && requires(ygg::Data<Entity<Call>>& data, const View<Call>& view) {
    data.callee;
    data.arguments;
    view.get_callee();
    view.for_each_call_argument([](auto) {});
});

}  // namespace

}
