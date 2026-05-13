#ifndef RUNIR_KR_GP_RULE_DATA_HPP_
#define RUNIR_KR_GP_RULE_DATA_HPP_

#include "runir/kr/gp/condition_index.hpp"
#include "runir/kr/gp/effect_index.hpp"
#include "runir/kr/gp/rule_index.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>
#include <vector>

namespace tyr
{

template<>
struct Data<runir::kr::gp::Rule>
{
    Index<runir::kr::gp::Rule> index;
    IndexList<runir::kr::gp::ConditionVariant> conditions;
    IndexList<runir::kr::gp::EffectVariant> effects;

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
