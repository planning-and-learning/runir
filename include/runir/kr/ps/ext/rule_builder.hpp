#ifndef RUNIR_KR_PS_EXT_RULE_BUILDER_HPP_
#define RUNIR_KR_PS_EXT_RULE_BUILDER_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/ext/rule_data.hpp"
#include "runir/kr/ps/ext/rule_view.hpp"

#include <optional>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of Rule: its record's members, each in its built form (urd::built_t).
template<runir::kr::ps::ext::BindingRuleKind Kind, typename C>
    requires std::is_same_v<Kind, runir::kr::ps::ext::LoadTag<runir::kr::ps::ext::RuleCategoryFor<Kind>>>
struct builder<runir::kr::ps::ext::Rule<Kind>, C>
{
    urd::index<runir::kr::ps::ext::Rule<Kind>> index;
    urd::built_t<urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>>, C> source;
    urd::built_t<urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>>, C> target;
    urd::built_t<runir::kr::ps::ConditionList<runir::kr::ExtFamilyTag>, C> conditions;
    urd::built_t<urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::ext::RuleCategoryFor<Kind>>>, C> feature;
    urd::built_t<urd::index<runir::kr::dl::Register<runir::kr::ps::ext::RuleCategoryFor<Kind>>>, ::runir::kr::dl::ConstructorRepository> slot;
    urd::built_t<runir::kr::ps::EffectList<runir::kr::ExtFamilyTag>, C> effects;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"source", &builder::source>,
                           urd::field<"target", &builder::target>,
                           urd::field<"conditions", &builder::conditions>,
                           urd::field<"feature", &builder::feature>,
                           urd::field<"slot", &builder::slot>,
                           urd::field<"effects", &builder::effects>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Rule: its record's members, each in its built form (urd::built_t).
template<runir::kr::ps::ext::BindingRuleKind Kind, typename C>
    requires std::is_same_v<Kind, runir::kr::ps::ext::ChooseTag<runir::kr::ps::ext::RuleCategoryFor<Kind>>>
struct builder<runir::kr::ps::ext::Rule<Kind>, C>
{
    urd::index<runir::kr::ps::ext::Rule<Kind>> index;
    urd::built_t<urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>>, C> source;
    urd::built_t<urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>>, C> target;
    urd::built_t<runir::kr::ps::ConditionList<runir::kr::ExtFamilyTag>, C> conditions;
    urd::built_t<urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::ext::RuleCategoryFor<Kind>>>, C> feature;
    urd::built_t<urd::index<runir::kr::dl::Register<runir::kr::ps::ext::RuleCategoryFor<Kind>>>, ::runir::kr::dl::ConstructorRepository> slot;
    urd::built_t<runir::kr::ps::EffectList<runir::kr::ExtFamilyTag>, C> effects;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::ps::ext::OrderTerm>>, C> order;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"source", &builder::source>,
                           urd::field<"target", &builder::target>,
                           urd::field<"conditions", &builder::conditions>,
                           urd::field<"feature", &builder::feature>,
                           urd::field<"slot", &builder::slot>,
                           urd::field<"effects", &builder::effects>,
                           urd::field<"order", &builder::order>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Rule: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::ps::ext::Rule<runir::kr::ps::ext::SketchTag>, C>
{
    urd::index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::SketchTag>> index;
    urd::built_t<urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>>, C> source;
    urd::built_t<urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>>, C> target;
    urd::built_t<runir::kr::ps::ConditionList<runir::kr::ExtFamilyTag>, C> conditions;
    urd::built_t<runir::kr::ps::EffectList<runir::kr::ExtFamilyTag>, C> effects;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"source", &builder::source>,
                           urd::field<"target", &builder::target>,
                           urd::field<"conditions", &builder::conditions>,
                           urd::field<"effects", &builder::effects>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Rule: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::ps::ext::Rule<runir::kr::ps::ext::DoTag>, C>
{
    urd::index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::DoTag>> index;
    urd::built_t<urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>>, C> source;
    urd::built_t<urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>>, C> target;
    urd::built_t<runir::kr::ps::ConditionList<runir::kr::ExtFamilyTag>, C> conditions;
    urd::built_t<runir::kr::ps::EffectList<runir::kr::ExtFamilyTag>, C> effects;
    urd::built_t<urd::flat_string, C> action_name;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>>, C> arguments;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"source", &builder::source>,
                           urd::field<"target", &builder::target>,
                           urd::field<"conditions", &builder::conditions>,
                           urd::field<"effects", &builder::effects>,
                           urd::field<"action_name", &builder::action_name>,
                           urd::field<"arguments", &builder::arguments>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Rule: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::ps::ext::Rule<runir::kr::ps::ext::ActionTag>, C>
{
    urd::index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::ActionTag>> index;
    urd::built_t<urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>>, C> source;
    urd::built_t<urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>>, C> target;
    urd::built_t<runir::kr::ps::ConditionList<runir::kr::ExtFamilyTag>, C> conditions;
    urd::built_t<runir::kr::ps::EffectList<runir::kr::ExtFamilyTag>, C> effects;
    urd::built_t<urd::flat_string, C> action_name;
    urd::built_t<urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::QueryFeature>>, C> query_feature;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"source", &builder::source>,
                           urd::field<"target", &builder::target>,
                           urd::field<"conditions", &builder::conditions>,
                           urd::field<"effects", &builder::effects>,
                           urd::field<"action_name", &builder::action_name>,
                           urd::field<"query_feature", &builder::query_feature>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Rule: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::ps::ext::Rule<runir::kr::ps::ext::CallTag>, C>
{
    urd::index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::CallTag>> index;
    urd::built_t<urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>>, C> source;
    urd::built_t<urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>>, C> target;
    urd::built_t<runir::kr::ps::ConditionList<runir::kr::ExtFamilyTag>, C> conditions;
    urd::built_t<urd::index<runir::kr::ps::ModuleSymbol<runir::kr::ExtFamilyTag>>, C> callee;
    urd::built_t<urd::flat_vector<runir::kr::ps::ext::CallArgument>, C> arguments;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"source", &builder::source>,
                           urd::field<"target", &builder::target>,
                           urd::field<"conditions", &builder::conditions>,
                           urd::field<"callee", &builder::callee>,
                           urd::field<"arguments", &builder::arguments>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Rule: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::ps::ext::Rule<runir::kr::ps::ext::BacktrackTag>, C>
{
    urd::index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::BacktrackTag>> index;
    urd::built_t<urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>>, C> source;
    urd::built_t<runir::kr::ps::ConditionList<runir::kr::ExtFamilyTag>, C> conditions;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"source", &builder::source>, urd::field<"conditions", &builder::conditions>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

}  // namespace urd

#endif
