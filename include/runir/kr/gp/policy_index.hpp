#ifndef RUNIR_KR_GP_POLICY_INDEX_HPP_
#define RUNIR_KR_GP_POLICY_INDEX_HPP_

#include "runir/kr/gp/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<>
struct Index<runir::kr::gp::Policy> : IndexMixin<Index<runir::kr::gp::Policy>>
{
    using Base = IndexMixin<Index<runir::kr::gp::Policy>>;
    using Base::Base;
};

}  // namespace tyr

#endif
