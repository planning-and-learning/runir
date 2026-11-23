#ifndef RUNIR_KR_PS_ICP_RULE_BUILDER_HPP_
#define RUNIR_KR_PS_ICP_RULE_BUILDER_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/icp/rule_data.hpp"
#include "runir/kr/ps/icp/rule_view.hpp"

#include <optional>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of Rule: its record's members, each in its built form (urd::built_t).
template<runir::kr::ps::icp::BindingRuleKind Kind, typename C>
    requires std::is_same_v<Kind, runir::kr::ps::icp::LoadTag<runir::kr::ps::icp::RuleCategoryFor<Kind>>>
struct builder<runir::kr::ps::icp::Rule<Kind>, C>
{
    urd::key<runir::kr::ps::icp::Rule<Kind>> index;
    urd::built_t<urd::index<runir::kr::ps::MemoryState<runir::kr::IcpFamilyTag>>, C> source;
    urd::built_t<urd::index<runir::kr::ps::MemoryState<runir::kr::IcpFamilyTag>>, C> target;
    urd::built_t<runir::kr::ps::ConditionList<runir::kr::IcpFamilyTag>, C> conditions;
    urd::built_t<urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::ps::icp::RuleCategoryFor<Kind>>>, C> feature;
    urd::built_t<urd::index<runir::kr::dl::Register<runir::kr::ps::icp::RuleCategoryFor<Kind>>>, ::runir::kr::dl::ConstructorRepository> slot;
    urd::built_t<runir::kr::ps::EffectList<runir::kr::IcpFamilyTag>, C> effects;

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
template<typename C>
struct builder<runir::kr::ps::icp::Rule<runir::kr::ps::icp::CruleTag>, C>
{
    urd::key<runir::kr::ps::icp::Rule<runir::kr::ps::icp::CruleTag>> index;
    urd::built_t<urd::index<runir::kr::ps::MemoryState<runir::kr::IcpFamilyTag>>, C> source;
    urd::built_t<urd::index<runir::kr::ps::MemoryState<runir::kr::IcpFamilyTag>>, C> target;
    urd::built_t<runir::kr::ps::ConditionList<runir::kr::IcpFamilyTag>, C> conditions;
    urd::built_t<runir::kr::ps::EffectList<runir::kr::IcpFamilyTag>, C> effects;
    urd::built_t<urd::flat_string, C> action_name;
    urd::built_t<urd::flat_vector<urd::flat_string>, C> argument_names;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::ps::icp::XCondition>>, C> xconditions;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::ps::icp::XEffect>>, C> xeffects;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"source", &builder::source>,
                           urd::field<"target", &builder::target>,
                           urd::field<"conditions", &builder::conditions>,
                           urd::field<"effects", &builder::effects>,
                           urd::field<"action_name", &builder::action_name>,
                           urd::field<"argument_names", &builder::argument_names>,
                           urd::field<"xconditions", &builder::xconditions>,
                           urd::field<"xeffects", &builder::xeffects>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

}  // namespace urd

#endif
