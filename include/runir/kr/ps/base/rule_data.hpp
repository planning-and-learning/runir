#ifndef RUNIR_KR_PS_BASE_RULE_DATA_HPP_
#define RUNIR_KR_PS_BASE_RULE_DATA_HPP_

#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/base/rule_index.hpp"
#include "runir/kr/ps/condition_index.hpp"
#include "runir/kr/ps/effect_index.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>
#include <vector>

namespace tyr
{

template<>
struct Data<runir::kr::ps::base::Rule>
{
    Index<runir::kr::ps::base::Rule> index;
    IndexList<runir::kr::ps::ConditionVariant<runir::kr::BaseFamilyTag>> conditions;
    IndexList<runir::kr::ps::EffectVariant<runir::kr::BaseFamilyTag>> effects;

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(conditions);
        tyr::clear(effects);
    }

    auto cista_members() const noexcept { return std::tie(index, conditions, effects); }
    auto identifying_members() const noexcept { return std::tie(conditions, effects); }
};

}  // namespace tyr

#endif
