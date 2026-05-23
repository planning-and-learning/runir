#ifndef RUNIR_KR_MPG_RULE_DATA_HPP_
#define RUNIR_KR_MPG_RULE_DATA_HPP_

#include "runir/kr/mpg/memory_state_index.hpp"
#include "runir/kr/mpg/condition_index.hpp"
#include "runir/kr/mpg/effect_index.hpp"
#include "runir/kr/mpg/rule_index.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<runir::kr::mpg::RuleKind Kind>
struct Data<runir::kr::mpg::Rule<Kind>>
{
    Index<runir::kr::mpg::Rule<Kind>> index;
    Index<runir::kr::mpg::MemoryState> source;
    Index<runir::kr::mpg::MemoryState> target;
    IndexList<runir::kr::mpg::ConditionVariant> conditions;
    IndexList<runir::kr::mpg::EffectVariant> effects;

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(source);
        tyr::clear(target);
        tyr::clear(conditions);
        tyr::clear(effects);
    }

    auto cista_members() const noexcept { return std::tie(index, source, target, conditions, effects); }
    auto identifying_members() const noexcept { return std::tie(source, target, conditions, effects); }
};

}  // namespace tyr

#endif
