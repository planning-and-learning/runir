#ifndef RUNIR_KR_PS_EXT_RULE_INDEX_HPP_
#define RUNIR_KR_PS_EXT_RULE_INDEX_HPP_

#include "runir/kr/ps/ext/declarations.hpp"

#include <yggdrasil/ids/index_mixins.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<runir::kr::ps::ext::RuleKind Kind, typename Category>
struct Index<runir::kr::ps::ext::Rule<Kind, Category>> : IndexMixin<Index<runir::kr::ps::ext::Rule<Kind, Category>>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ext::Rule<Kind, Category>>>;
    using Base::Base;
};

}  // namespace ygg

#endif
