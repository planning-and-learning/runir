#ifndef RUNIR_KR_MPG_RULE_VARIANT_INDEX_HPP_
#define RUNIR_KR_MPG_RULE_VARIANT_INDEX_HPP_

#include "runir/kr/mpg/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<>
struct Index<runir::kr::mpg::RuleVariant> : IndexMixin<Index<runir::kr::mpg::RuleVariant>>
{
    using Base = IndexMixin<Index<runir::kr::mpg::RuleVariant>>;
    using Base::Base;
};

}  // namespace tyr

#endif
