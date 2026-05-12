#ifndef RUNIR_KR_GP_RULE_DATA_HPP_
#define RUNIR_KR_GP_RULE_DATA_HPP_

#include "runir/kr/gp/condition_index.hpp"
#include "runir/kr/gp/effect_index.hpp"
#include "runir/kr/gp/rule_index.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>
#include <vector>

namespace runir::kr::gp
{

using ConditionIndexVariant = cista::variant<tyr::Index<ConditionVariantTag>>;
using EffectIndexVariant = cista::variant<tyr::Index<EffectVariantTag>>;

}  // namespace runir::kr::gp

namespace tyr
{

template<>
struct Data<runir::kr::gp::RuleTag>
{
    Index<runir::kr::gp::RuleTag> index;
    std::vector<runir::kr::gp::ConditionIndexVariant> conditions;
    std::vector<runir::kr::gp::EffectIndexVariant> effects;

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
