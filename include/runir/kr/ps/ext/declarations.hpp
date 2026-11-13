#ifndef RUNIR_KR_PS_EXT_DECLARATIONS_HPP_
#define RUNIR_KR_PS_EXT_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/ext/execution_declarations.hpp"
#include "runir/kr/ps/family_traits.hpp"

#include <boost/mp11.hpp>
#include <concepts>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <urd/elements/flat.hpp>

namespace runir::kr::ps::ext
{

struct OrderTerm
{
};

enum class OrderDirection : std::uint8_t
{
    MIN,
    MAX,
};

// Rules

struct SketchTag
{
    static constexpr auto keyword = "sketch";
};

template<runir::kr::dl::CategoryTag CategoryT>
struct LoadTag
{
    using Category = CategoryT;
    static constexpr auto keyword = "load";
};

template<runir::kr::dl::CategoryTag CategoryT>
struct ChooseTag
{
    using Category = CategoryT;
    static constexpr auto keyword = "choose";
};

struct DoTag
{
    static constexpr auto keyword = "do";
};

struct ActionTag
{
    static constexpr auto keyword = "action";
};

struct CallTag
{
    static constexpr auto keyword = "call";
};

struct BacktrackTag
{
    static constexpr auto keyword = "backtrack";
};

template<typename T>
concept BindingRuleKind = std::same_as<T, LoadTag<runir::kr::dl::ConceptTag>> || std::same_as<T, LoadTag<runir::kr::dl::RoleTag>>
                          || std::same_as<T, ChooseTag<runir::kr::dl::ConceptTag>> || std::same_as<T, ChooseTag<runir::kr::dl::RoleTag>>;

template<typename T>
concept RuleKind = BindingRuleKind<T> || std::same_as<T, SketchTag> || std::same_as<T, DoTag> || std::same_as<T, CallTag> || std::same_as<T, ActionTag>
                   || std::same_as<T, BacktrackTag>;

/// A rule that leads to a target memory state: every rule but a Backtrack guard, which has none (docs/proof-search.rst).
template<typename T>
concept TransitionRuleKind = RuleKind<T> && !std::same_as<T, BacktrackTag>;

/// A rule with effects: the binding rules, Sketch, Do and Action.
template<typename T>
concept EffectRuleKind = BindingRuleKind<T> || std::same_as<T, SketchTag> || std::same_as<T, DoTag> || std::same_as<T, ActionTag>;

/// DL category bound by a rule tag.
template<BindingRuleKind Tag>
using RuleCategoryFor = typename Tag::Category;

template<RuleKind Kind>
struct Rule
{
};

struct Module
{
};

struct Program
{
};

using LoadRuleTypes = boost::mp11::mp_list<Rule<LoadTag<runir::kr::dl::ConceptTag>>, Rule<LoadTag<runir::kr::dl::RoleTag>>>;
using ChooseRuleTypes = boost::mp11::mp_list<Rule<ChooseTag<runir::kr::dl::ConceptTag>>, Rule<ChooseTag<runir::kr::dl::RoleTag>>>;
using ConcreteRuleTypes = boost::mp11::mp_append<LoadRuleTypes,
                                                 boost::mp11::mp_transform<Rule, boost::mp11::mp_list<SketchTag, DoTag, CallTag>>,
                                                 ChooseRuleTypes,
                                                 boost::mp11::mp_transform<Rule, boost::mp11::mp_list<ActionTag, BacktrackTag>>>;
using RuleTypes = boost::mp11::mp_append<boost::mp11::mp_list<ps::Rule<ExtFamilyTag>>, ConcreteRuleTypes>;
using FeatureTypes = runir::kr::ps::PsFeatureTypes<runir::kr::ExtFamilyTag>;
using ConditionTypes = runir::kr::ps::PsConditionTypes<runir::kr::ExtFamilyTag>;
using EffectTypes = runir::kr::ps::PsEffectTypes<runir::kr::ExtFamilyTag>;
/// A call rule's argument: a feature of any category.
using CallArgument =
    urd::index_variant<boost::mp11::mp_transform_q<boost::mp11::mp_bind_front<runir::kr::ps::Feature, runir::kr::ExtFamilyTag>, runir::kr::dl::CategoryTags>>;
using ProgramTypes = boost::mp11::mp_list<MemoryState<ExtFamilyTag>, ModuleSymbol<ExtFamilyTag>, Module, Program, OrderTerm>;
using RepositoryTypes =
    boost::mp11::mp_append<runir::kr::ps::base::RepositoryTypes, runir::kr::ps::PsCoreTypes<runir::kr::ExtFamilyTag>, RuleTypes, ProgramTypes>;
using Repository = runir::kr::ps::BasicRepository<runir::kr::ExtFamilyTag, RepositoryTypes>;
using RepositoryPtr = std::shared_ptr<Repository>;
using RepositoryFactory = runir::kr::ps::BasicRepositoryFactory<runir::kr::ExtFamilyTag, RepositoryTypes>;

using ConceptRegisterView = urd::index_view<runir::kr::dl::Register<runir::kr::dl::ConceptTag>, runir::kr::dl::ConstructorRepository>;
using RoleRegisterView = urd::index_view<runir::kr::dl::Register<runir::kr::dl::RoleTag>, runir::kr::dl::ConstructorRepository>;
using OrderTermView = urd::index_view<OrderTerm, Repository>;
using MemoryStateView = urd::index_view<MemoryState<ExtFamilyTag>, Repository>;
using ModuleSymbolView = urd::index_view<ModuleSymbol<ExtFamilyTag>, Repository>;
using ModuleView = urd::index_view<Module, Repository>;
using ProgramView = urd::index_view<Program, Repository>;
using RuleVariantView = urd::index_view<ps::Rule<ExtFamilyTag>, Repository>;

template<RuleKind Kind>
using RuleView = urd::index_view<Rule<Kind>, Repository>;

/// The kind of a rule, or of its view.
template<typename T>
struct rule_kind;
template<RuleKind Kind>
struct rule_kind<Rule<Kind>>
{
    using type = Kind;
};
template<RuleKind Kind, typename C>
struct rule_kind<urd::index_view<Rule<Kind>, C>>
{
    using type = Kind;
};
template<typename T>
using rule_kind_t = typename rule_kind<std::remove_cvref_t<T>>::type;

template<typename R>
concept LoadRuleView = std::same_as<R, RuleView<LoadTag<runir::kr::dl::ConceptTag>>> || std::same_as<R, RuleView<LoadTag<runir::kr::dl::RoleTag>>>;

template<typename R>
concept ChooseRuleView = std::same_as<R, RuleView<ChooseTag<runir::kr::dl::ConceptTag>>> || std::same_as<R, RuleView<ChooseTag<runir::kr::dl::RoleTag>>>;

template<typename R>
concept BindingRuleView = LoadRuleView<R> || ChooseRuleView<R>;

}  // namespace runir::kr::ps::ext

namespace runir::kr::ps
{
/// ext's guards: a Backtrack rule leads to no target memory state.
template<ext::RuleKind Kind, typename C>
inline constexpr bool transition_rule<urd::index_view<ext::Rule<Kind>, C>> = ext::TransitionRuleKind<Kind>;
}  // namespace runir::kr::ps

template<>
struct runir::kr::ps::FamilyTypes<runir::kr::ExtFamilyTag>
{
    using Repository = runir::kr::ps::ext::Repository;
    using ConcreteRuleTypes = runir::kr::ps::ext::ConcreteRuleTypes;
    template<tyr::TaskKind Kind>
    using ExecutionRepositoryTypes = runir::kr::ps::ext::ExecutionRepositoryTypes<Kind>;
    template<tyr::TaskKind Kind>
    using ProgramState = runir::kr::ps::ext::ProgramState<Kind>;
};

namespace runir::kr::ps::ext::dl
{

using BooleanFeatureView = urd::index_view<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanTag>, Repository>;
using NumericalFeatureView = urd::index_view<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalTag>, Repository>;
using QueryFeatureView = urd::index_view<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::QueryFeature>, Repository>;

}  // namespace runir::kr::ps::ext::dl

#endif
