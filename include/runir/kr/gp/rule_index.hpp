#ifndef RUNIR_KR_GP_RULE_INDEX_HPP_
#define RUNIR_KR_GP_RULE_INDEX_HPP_

#include "runir/kr/gp/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<>
struct Index<runir::kr::gp::Rule> : IndexMixin<Index<runir::kr::gp::Rule>>
{
    using Base = IndexMixin<Index<runir::kr::gp::Rule>>;
    using Base::Base;
};

}  // namespace tyr

#endif
