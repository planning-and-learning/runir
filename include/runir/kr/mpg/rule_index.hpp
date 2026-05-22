#ifndef RUNIR_KR_MPG_RULE_INDEX_HPP_
#define RUNIR_KR_MPG_RULE_INDEX_HPP_

#include "runir/kr/mpg/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::mpg::RuleKind Kind>
struct Index<runir::kr::mpg::Rule<Kind>> : IndexMixin<Index<runir::kr::mpg::Rule<Kind>>>
{
    using Base = IndexMixin<Index<runir::kr::mpg::Rule<Kind>>>;
    using Base::Base;
};

}  // namespace tyr

#endif
