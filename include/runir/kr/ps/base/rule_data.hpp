#ifndef RUNIR_KR_PS_BASE_RULE_DATA_HPP_
#define RUNIR_KR_PS_BASE_RULE_DATA_HPP_

#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/base/rule_index.hpp"
#include "runir/kr/ps/condition_index.hpp"
#include "runir/kr/ps/effect_index.hpp"

#include <cista/containers/string.h>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>>
{
    Index<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>> index;
    ::cista::offset::string symbol;
    IndexList<runir::kr::ps::ConditionVariant<runir::kr::BaseFamilyTag>> conditions;
    IndexList<runir::kr::ps::EffectVariant<runir::kr::BaseFamilyTag>> effects;

    Data() = default;
    Data(::cista::offset::string symbol_,
         IndexList<runir::kr::ps::ConditionVariant<runir::kr::BaseFamilyTag>> conditions_,
         IndexList<runir::kr::ps::EffectVariant<runir::kr::BaseFamilyTag>> effects_) :
        index(),
        symbol(std::move(symbol_)),
        conditions(std::move(conditions_)),
        effects(std::move(effects_))
    {
    }
    template<typename C>
    Data(::cista::offset::string symbol_,
         const std::vector<::ygg::View<Index<runir::kr::ps::ConditionVariant<runir::kr::BaseFamilyTag>>, C>>& conditions_,
         const std::vector<::ygg::View<Index<runir::kr::ps::EffectVariant<runir::kr::BaseFamilyTag>>, C>>& effects_) :
        index(),
        symbol(std::move(symbol_)),
        conditions(),
        effects()
    {
        set(conditions_, conditions);
        set(effects_, effects);
    }

    auto cista_members() noexcept { return std::tie(index, symbol, conditions, effects); }
    auto cista_members() const noexcept { return std::tie(index, symbol, conditions, effects); }
    auto identifying_members() const noexcept { return std::tie(symbol, conditions, effects); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}  // namespace ygg

#endif
