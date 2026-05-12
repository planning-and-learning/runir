#ifndef RUNIR_KR_GP_POLICY_DATA_HPP_
#define RUNIR_KR_GP_POLICY_DATA_HPP_

#include "runir/kr/gp/policy_index.hpp"
#include "runir/kr/gp/rule_index.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<>
struct Data<runir::kr::gp::PolicyTag>
{
    Index<runir::kr::gp::PolicyTag> index;
    IndexList<runir::kr::gp::RuleTag> rules;

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(rules);
    }

    auto cista_members() const noexcept { return std::tie(index, rules); }
    auto identifying_members() const noexcept { return std::tie(rules); }
};

}  // namespace tyr

#endif
