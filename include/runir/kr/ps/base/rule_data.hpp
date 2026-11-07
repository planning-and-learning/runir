#ifndef RUNIR_KR_PS_BASE_RULE_DATA_HPP_
#define RUNIR_KR_PS_BASE_RULE_DATA_HPP_

#include "runir/kr/ps/base/declarations.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include "runir/kr/ps/declarations.hpp"

#include <cista/containers/string.h>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>
#include <urd/elements/flat.hpp>

namespace urd
{

template<>
struct data<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>>
{
    urd::index<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>> index;
    urd::flat_string symbol;
    runir::kr::ps::ConditionList<runir::kr::BaseFamilyTag> conditions;
    runir::kr::ps::EffectList<runir::kr::BaseFamilyTag> effects;

    data() = default;
    data(urd::flat_string symbol_,
         runir::kr::ps::ConditionList<runir::kr::BaseFamilyTag> conditions_,
         runir::kr::ps::EffectList<runir::kr::BaseFamilyTag> effects_) :
        index(),
        symbol(std::move(symbol_)),
        conditions(std::move(conditions_)),
        effects(std::move(effects_))
    {
    }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::symbol, &data::conditions, &data::effects> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::symbol, &data::conditions, &data::effects> {}; }
};

}  // namespace urd

#endif
