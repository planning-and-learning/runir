#ifndef RUNIR_KR_PS_EXT_RULE_INDEX_HPP_
#define RUNIR_KR_PS_EXT_RULE_INDEX_HPP_

#include "runir/kr/ps/ext/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::ps::ext::RuleKind Kind>
struct Index<runir::kr::ps::ext::Rule<Kind>> : IndexMixin<Index<runir::kr::ps::ext::Rule<Kind>>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ext::Rule<Kind>>>;
    using Base::Base;
};

}  // namespace tyr

#endif
