#ifndef RUNIR_KR_PS_ICP_RULE_INDEX_HPP_
#define RUNIR_KR_PS_ICP_RULE_INDEX_HPP_

#include "runir/kr/ps/icp/declarations.hpp"

#include <yggdrasil/core/types.hpp>
#include <yggdrasil/ids/index_mixins.hpp>

namespace ygg
{

template<runir::kr::ps::icp::RuleKind Kind>
struct Index<runir::kr::ps::icp::Rule<Kind>> : IndexMixin<Index<runir::kr::ps::icp::Rule<Kind>>>
{
    using Base = IndexMixin<Index<runir::kr::ps::icp::Rule<Kind>>>;
    using Base::Base;
};

}  // namespace ygg

#endif
