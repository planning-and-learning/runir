#ifndef RUNIR_KR_PS_ICP_RULE_DATA_HPP_
#define RUNIR_KR_PS_ICP_RULE_DATA_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/icp/declarations.hpp"

#include <cista/containers/string.h>
#include <cista/containers/vector.h>
#include <string>
#include <tuple>
#include <type_traits>
#include <urd/elements/flat.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

template<runir::kr::ps::icp::BindingRuleKind Kind>
    requires std::is_same_v<Kind, runir::kr::ps::icp::LoadTag<runir::kr::ps::icp::RuleCategoryFor<Kind>>>
struct data<runir::kr::ps::icp::Rule<Kind>>
{
    urd::key<runir::kr::ps::icp::Rule<Kind>> index;
    urd::index<runir::kr::ps::MemoryState<runir::kr::IcpFamilyTag>> source;
    urd::index<runir::kr::ps::MemoryState<runir::kr::IcpFamilyTag>> target;
    runir::kr::ps::ConditionList<runir::kr::IcpFamilyTag> conditions;
    urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::ps::icp::RuleCategoryFor<Kind>>> feature;
    urd::index<runir::kr::dl::Register<runir::kr::ps::icp::RuleCategoryFor<Kind>>> slot;
    runir::kr::ps::EffectList<runir::kr::IcpFamilyTag> effects;

    data() = default;

    // Registers live in the description-logic repository, hence the separate context parameter.
    data(urd::index<runir::kr::ps::MemoryState<runir::kr::IcpFamilyTag>> source_,
         urd::index<runir::kr::ps::MemoryState<runir::kr::IcpFamilyTag>> target_,
         runir::kr::ps::ConditionList<runir::kr::IcpFamilyTag> conditions_,
         urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::ps::icp::RuleCategoryFor<Kind>>> feature_,
         urd::index<runir::kr::dl::Register<runir::kr::ps::icp::RuleCategoryFor<Kind>>> slot_,
         runir::kr::ps::EffectList<runir::kr::IcpFamilyTag> effects_) :
        index(),
        source(source_),
        target(target_),
        conditions(std::move(conditions_)),
        feature(std::move(feature_)),
        slot(std::move(slot_)),
        effects(std::move(effects_))
    {
    }

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) noexcept = default;
    data& operator=(data&& other) noexcept = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"source", &data::source>,
                           urd::field<"target", &data::target>,
                           urd::field<"conditions", &data::conditions>,
                           urd::field<"feature", &data::feature>,
                           urd::field<"slot", &data::slot>,
                           urd::field<"effects", &data::effects>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept
    { return urd::Members<&data::index, &data::source, &data::target, &data::conditions, &data::feature, &data::slot, &data::effects> {}; }

    [[nodiscard]] urd::key<runir::kr::ps::icp::Rule<Kind>> key() const noexcept { return urd::key<runir::kr::ps::icp::Rule<Kind>>(index); }
};

template<>
struct data<runir::kr::ps::icp::Rule<runir::kr::ps::icp::CruleTag>>
{
    urd::key<runir::kr::ps::icp::Rule<runir::kr::ps::icp::CruleTag>> index;
    urd::index<runir::kr::ps::MemoryState<runir::kr::IcpFamilyTag>> source;
    urd::index<runir::kr::ps::MemoryState<runir::kr::IcpFamilyTag>> target;
    runir::kr::ps::ConditionList<runir::kr::IcpFamilyTag> conditions;
    runir::kr::ps::EffectList<runir::kr::IcpFamilyTag> effects;
    urd::flat_string action_name;
    urd::flat_vector<urd::flat_string> argument_names;
    urd::flat_vector<urd::index<runir::kr::ps::icp::XCondition>> xconditions;
    urd::flat_vector<urd::index<runir::kr::ps::icp::XEffect>> xeffects;

    data() = default;

    data(urd::index<runir::kr::ps::MemoryState<runir::kr::IcpFamilyTag>> source_,
         urd::index<runir::kr::ps::MemoryState<runir::kr::IcpFamilyTag>> target_,
         runir::kr::ps::ConditionList<runir::kr::IcpFamilyTag> conditions_,
         runir::kr::ps::EffectList<runir::kr::IcpFamilyTag> effects_,
         urd::flat_string action_name_,
         urd::flat_vector<urd::flat_string> argument_names_,
         urd::flat_vector<urd::index<runir::kr::ps::icp::XCondition>> xconditions_,
         urd::flat_vector<urd::index<runir::kr::ps::icp::XEffect>> xeffects_) :
        index(),
        source(source_),
        target(target_),
        conditions(std::move(conditions_)),
        effects(std::move(effects_)),
        action_name(std::move(action_name_)),
        argument_names(std::move(argument_names_)),
        xconditions(std::move(xconditions_)),
        xeffects(std::move(xeffects_))
    {
    }

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) noexcept = default;
    data& operator=(data&& other) noexcept = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"source", &data::source>,
                           urd::field<"target", &data::target>,
                           urd::field<"conditions", &data::conditions>,
                           urd::field<"effects", &data::effects>,
                           urd::field<"action_name", &data::action_name>,
                           urd::field<"argument_names", &data::argument_names>,
                           urd::field<"xconditions", &data::xconditions>,
                           urd::field<"xeffects", &data::xeffects>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept
    {
        return urd::Members<&data::index,
                            &data::source,
                            &data::target,
                            &data::conditions,
                            &data::effects,
                            &data::action_name,
                            &data::argument_names,
                            &data::xconditions,
                            &data::xeffects> {};
    }

    [[nodiscard]] urd::key<runir::kr::ps::icp::Rule<runir::kr::ps::icp::CruleTag>> key() const noexcept
    { return urd::key<runir::kr::ps::icp::Rule<runir::kr::ps::icp::CruleTag>>(index); }
};

}  // namespace urd

#endif
