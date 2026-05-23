#ifndef RUNIR_KR_PS_EXT_RULE_DATA_HPP_
#define RUNIR_KR_PS_EXT_RULE_DATA_HPP_

#include "runir/kr/ps/ext/memory_state_index.hpp"
#include "runir/kr/ps/ext/condition_index.hpp"
#include "runir/kr/ps/ext/effect_index.hpp"
#include "runir/kr/ps/ext/rule_index.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<runir::kr::ps::ext::RuleKind Kind>
struct Data<runir::kr::ps::ext::Rule<Kind>>
{
    Index<runir::kr::ps::ext::Rule<Kind>> index;
    Index<runir::kr::ps::ext::MemoryState> source;
    Index<runir::kr::ps::ext::MemoryState> target;
    IndexList<runir::kr::ps::ext::ConditionVariant> conditions;
    IndexList<runir::kr::ps::ext::EffectVariant> effects;

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
