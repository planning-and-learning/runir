#ifndef RUNIR_KR_PS_ICP_RULE_DATA_HPP_
#define RUNIR_KR_PS_ICP_RULE_DATA_HPP_

#include "runir/kr/dl/declarations.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/icp/declarations.hpp"

#include <cista/containers/string.h>
#include <cista/containers/vector.h>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>
#include <urd/elements/flat.hpp>

namespace urd
{

template<runir::kr::dl::CategoryTag Category>
struct data<runir::kr::ps::icp::Rule<runir::kr::ps::icp::LoadTag<Category>>>
{
    urd::index<runir::kr::ps::icp::Rule<runir::kr::ps::icp::LoadTag<Category>>> index;
    urd::index<runir::kr::ps::icp::MemoryState> source;
    urd::index<runir::kr::ps::icp::MemoryState> target;
    runir::kr::ps::ConditionList<runir::kr::IcpFamilyTag> conditions;
    urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, Category>> feature;
    urd::index<runir::kr::dl::Register<Category>> reg;
    runir::kr::ps::EffectList<runir::kr::IcpFamilyTag> effects;

    data() = default;
    // Registers live in the description-logic repository, hence the separate context parameter.
    data(urd::index<runir::kr::ps::icp::MemoryState> source_,
         urd::index<runir::kr::ps::icp::MemoryState> target_,
         runir::kr::ps::ConditionList<runir::kr::IcpFamilyTag> conditions_,
         urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, Category>> feature_,
         urd::index<runir::kr::dl::Register<Category>> reg_,
         runir::kr::ps::EffectList<runir::kr::IcpFamilyTag> effects_) :
        index(),
        source(source_),
        target(target_),
        conditions(std::move(conditions_)),
        feature(std::move(feature_)),
        reg(std::move(reg_)),
        effects(std::move(effects_))
    {
    }

    static constexpr auto cista_members() noexcept
    { return urd::Members<&data::index, &data::source, &data::target, &data::conditions, &data::feature, &data::reg, &data::effects> {}; }
    static constexpr auto identifying_members() noexcept
    { return urd::Members<&data::source, &data::target, &data::conditions, &data::feature, &data::reg, &data::effects> {}; }
};

template<>
struct data<runir::kr::ps::icp::Rule<runir::kr::ps::icp::CruleTag>>
{
    urd::index<runir::kr::ps::icp::Rule<runir::kr::ps::icp::CruleTag>> index;
    urd::index<runir::kr::ps::icp::MemoryState> source;
    urd::index<runir::kr::ps::icp::MemoryState> target;
    runir::kr::ps::ConditionList<runir::kr::IcpFamilyTag> conditions;
    runir::kr::ps::EffectList<runir::kr::IcpFamilyTag> effects;
    ::cista::offset::string action_name;
    ::cista::offset::vector<::cista::offset::string> argument_names;
    urd::flat_vector<urd::index<runir::kr::ps::icp::XCondition>> xconditions;
    urd::flat_vector<urd::index<runir::kr::ps::icp::XEffect>> xeffects;

    data() = default;
    data(urd::index<runir::kr::ps::icp::MemoryState> source_,
         urd::index<runir::kr::ps::icp::MemoryState> target_,
         runir::kr::ps::ConditionList<runir::kr::IcpFamilyTag> conditions_,
         runir::kr::ps::EffectList<runir::kr::IcpFamilyTag> effects_,
         ::cista::offset::string action_name_,
         ::cista::offset::vector<::cista::offset::string> argument_names_,
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
    static constexpr auto identifying_members() noexcept
    {
        return urd::Members<&data::source,
                            &data::target,
                            &data::conditions,
                            &data::effects,
                            &data::action_name,
                            &data::argument_names,
                            &data::xconditions,
                            &data::xeffects> {};
    }
};

}  // namespace urd

#endif
