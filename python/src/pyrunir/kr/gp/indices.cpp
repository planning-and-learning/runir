#include "module.hpp"

#include <runir/kr/gp/repository.hpp>
#include <tyr/common/python/bindings.hpp>

namespace runir::kr::gp
{

void bind_indices(nb::module_& m)
{
    tyr::bind_index<tyr::Index<ConditionVariant>>(m, "ConditionVariantIndex");
    tyr::bind_index<tyr::Index<EffectVariant>>(m, "EffectVariantIndex");
    tyr::bind_index<tyr::Index<Rule>>(m, "RuleIndex");
    tyr::bind_index<tyr::Index<Policy>>(m, "PolicyIndex");
}

}  // namespace runir::kr::gp
