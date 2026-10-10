#ifndef RUNIR_KR_PS_ICP_RULE_DATA_HPP_
#define RUNIR_KR_PS_ICP_RULE_DATA_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/icp/declarations.hpp"

#include <cista/containers/string.h>
#include <cista/containers/vector.h>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <yggdrasil/core/types.hpp>
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

    Data() = default;
    // Registers live in the description-logic repository, hence the separate context parameter.
    Data(Index<runir::kr::ps::icp::MemoryState> source_,
         Index<runir::kr::ps::icp::MemoryState> target_,
         IndexList<runir::kr::ps::ConditionVariant<runir::kr::IcpFamilyTag>> conditions_,
         Index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, Category>> feature_,
         Index<runir::kr::dl::Register<Category>> reg_,
         IndexList<runir::kr::ps::EffectVariant<runir::kr::IcpFamilyTag>> effects_) :
        index(),
        source(std::move(source_)),
        target(std::move(target_)),
        conditions(std::move(conditions_)),
        feature(std::move(feature_)),
        reg(std::move(reg_)),
        effects(std::move(effects_))
    {
    }
    template<typename C, typename D>
    Data(::ygg::View<Index<runir::kr::ps::icp::MemoryState>, C> source_,
         ::ygg::View<Index<runir::kr::ps::icp::MemoryState>, C> target_,
         const std::vector<::ygg::View<Index<runir::kr::ps::ConditionVariant<runir::kr::IcpFamilyTag>>, C>>& conditions_,
         ::ygg::View<Index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, Category>>, C> feature_,
         ::ygg::View<Index<runir::kr::dl::Register<Category>>, D> reg_,
         const std::vector<::ygg::View<Index<runir::kr::ps::EffectVariant<runir::kr::IcpFamilyTag>>, C>>& effects_) :
        index(),
        source(),
        target(),
        conditions(),
        feature(),
        reg(),
        effects()
    {
        set(source_, source);
        set(target_, target);
        set(conditions_, conditions);
        set(feature_, feature);
        set(reg_, reg);
        set(effects_, effects);
    }

    auto cista_members() noexcept { return std::tie(index, source, target, conditions, feature, reg, effects); }
    auto cista_members() const noexcept { return std::tie(index, source, target, conditions, feature, reg, effects); }
    auto identifying_members() const noexcept { return std::tie(source, target, conditions, feature, reg, effects); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
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
    Data(Index<runir::kr::ps::icp::MemoryState> source_,
         Index<runir::kr::ps::icp::MemoryState> target_,
         IndexList<runir::kr::ps::ConditionVariant<runir::kr::IcpFamilyTag>> conditions_,
         IndexList<runir::kr::ps::EffectVariant<runir::kr::IcpFamilyTag>> effects_,
         ::cista::offset::string action_name_,
         ::cista::offset::vector<::cista::offset::string> argument_names_,
         IndexList<runir::kr::ps::icp::XCondition> xconditions_,
         IndexList<runir::kr::ps::icp::XEffect> xeffects_) :
        index(),
        source(std::move(source_)),
        target(std::move(target_)),
        conditions(std::move(conditions_)),
        effects(std::move(effects_)),
        action_name(std::move(action_name_)),
        argument_names(std::move(argument_names_)),
        xconditions(std::move(xconditions_)),
        xeffects(std::move(xeffects_))
    {
    }
    template<typename C>
    Data(::ygg::View<Index<runir::kr::ps::icp::MemoryState>, C> source_,
         ::ygg::View<Index<runir::kr::ps::icp::MemoryState>, C> target_,
         const std::vector<::ygg::View<Index<runir::kr::ps::ConditionVariant<runir::kr::IcpFamilyTag>>, C>>& conditions_,
         const std::vector<::ygg::View<Index<runir::kr::ps::EffectVariant<runir::kr::IcpFamilyTag>>, C>>& effects_,
         ::cista::offset::string action_name_,
         ::cista::offset::vector<::cista::offset::string> argument_names_,
         const std::vector<::ygg::View<Index<runir::kr::ps::icp::XCondition>, C>>& xconditions_,
         const std::vector<::ygg::View<Index<runir::kr::ps::icp::XEffect>, C>>& xeffects_) :
        index(),
        source(),
        target(),
        conditions(),
        effects(),
        action_name(std::move(action_name_)),
        argument_names(std::move(argument_names_)),
        xconditions(),
        xeffects()
    {
        set(source_, source);
        set(target_, target);
        set(conditions_, conditions);
        set(effects_, effects);
        set(xconditions_, xconditions);
        set(xeffects_, xeffects);
    }

    auto cista_members() noexcept { return std::tie(index, source, target, conditions, effects, action_name, argument_names, xconditions, xeffects); }
    auto cista_members() const noexcept { return std::tie(index, source, target, conditions, effects, action_name, argument_names, xconditions, xeffects); }
    auto identifying_members() const noexcept { return std::tie(source, target, conditions, effects, action_name, argument_names, xconditions, xeffects); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}  // namespace ygg

#endif
