#ifndef RUNIR_KR_PS_ICP_RULE_DATA_HPP_
#define RUNIR_KR_PS_ICP_RULE_DATA_HPP_

#include "runir/kr/dl/register_index.hpp"
#include "runir/kr/ps/condition_index.hpp"
#include "runir/kr/ps/effect_index.hpp"
#include "runir/kr/ps/feature_index.hpp"
#include "runir/kr/ps/icp/memory_state_index.hpp"
#include "runir/kr/ps/icp/rule_index.hpp"
#include "runir/kr/ps/icp/xcondition_index.hpp"
#include "runir/kr/ps/icp/xeffect_index.hpp"

#include <cista/containers/string.h>
#include <string>
#include <tuple>
#include <utility>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<runir::kr::dl::CategoryTag Category>
struct Data<runir::kr::ps::icp::Rule<runir::kr::ps::icp::LoadTag<Category>>>
{
    Index<runir::kr::ps::icp::Rule<runir::kr::ps::icp::LoadTag<Category>>> index;
    Index<runir::kr::ps::icp::MemoryState> source;
    Index<runir::kr::ps::icp::MemoryState> target;
    IndexList<runir::kr::ps::ConditionVariant<runir::kr::IcpFamilyTag>> conditions;
    Index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, Category>> feature;
    Index<runir::kr::dl::Register<Category>> reg;
    IndexList<runir::kr::ps::EffectVariant<runir::kr::IcpFamilyTag>> effects;

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(source);
        ygg::clear(target);
        ygg::clear(conditions);
        ygg::clear(feature);
        ygg::clear(reg);
        ygg::clear(effects);
    }

    auto cista_members() const noexcept { return std::tie(index, source, target, conditions, feature, reg, effects); }
    auto identifying_members() const noexcept { return std::tie(source, target, conditions, feature, reg, effects); }
};

template<>
struct Data<runir::kr::ps::icp::Rule<runir::kr::ps::icp::CruleTag>>
{
    Index<runir::kr::ps::icp::Rule<runir::kr::ps::icp::CruleTag>> index;
    Index<runir::kr::ps::icp::MemoryState> source;
    Index<runir::kr::ps::icp::MemoryState> target;
    IndexList<runir::kr::ps::ConditionVariant<runir::kr::IcpFamilyTag>> conditions;
    IndexList<runir::kr::ps::EffectVariant<runir::kr::IcpFamilyTag>> effects;
    ::cista::offset::string action_name;
    ::cista::offset::vector<::cista::offset::string> argument_names;
    IndexList<runir::kr::ps::icp::XCondition> xconditions;
    IndexList<runir::kr::ps::icp::XEffect> xeffects;

    Data() = default;
    Data(::cista::offset::string action_name_) : index(), action_name(std::move(action_name_)) {}
    Data(const std::string& action_name_) : index(), action_name(action_name_) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(source);
        ygg::clear(target);
        ygg::clear(conditions);
        ygg::clear(effects);
        ygg::clear(action_name);
        ygg::clear(argument_names);
        ygg::clear(xconditions);
        ygg::clear(xeffects);
    }

    auto cista_members() const noexcept { return std::tie(index, source, target, conditions, effects, action_name, argument_names, xconditions, xeffects); }
    auto identifying_members() const noexcept { return std::tie(source, target, conditions, effects, action_name, argument_names, xconditions, xeffects); }
};

}  // namespace ygg

#endif
