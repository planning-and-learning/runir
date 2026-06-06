#ifndef RUNIR_KR_PS_BASE_RULE_DATA_HPP_
#define RUNIR_KR_PS_BASE_RULE_DATA_HPP_

#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/base/rule_index.hpp"
#include "runir/kr/ps/condition_index.hpp"
#include "runir/kr/ps/effect_index.hpp"

#include <cista/containers/string.h>
#include <string>
#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>
#include <vector>

namespace ygg
{

template<>
struct Data<runir::kr::ps::base::Rule>
{
    Index<runir::kr::ps::base::Rule> index;
    ::cista::offset::string symbol;
    ::cista::offset::string description;
    IndexList<runir::kr::ps::ConditionVariant<runir::kr::BaseFamilyTag>> conditions;
    IndexList<runir::kr::ps::EffectVariant<runir::kr::BaseFamilyTag>> effects;

    Data() = default;
    Data(const std::string& symbol_, const std::string& description_) : index(), symbol(symbol_), description(description_)
    {
    }

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(symbol);
        ygg::clear(description);
        ygg::clear(conditions);
        ygg::clear(effects);
    }

    auto cista_members() const noexcept { return std::tie(index, symbol, description, conditions, effects); }
    auto identifying_members() const noexcept { return std::tie(symbol, description, conditions, effects); }
};

}  // namespace ygg

#endif
