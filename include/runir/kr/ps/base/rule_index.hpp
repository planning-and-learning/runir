#ifndef RUNIR_KR_PS_BASE_RULE_INDEX_HPP_
#define RUNIR_KR_PS_BASE_RULE_INDEX_HPP_

#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"

#include <yggdrasil/ids/index_mixins.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<>
struct Index<runir::kr::ps::base::Rule> : IndexMixin<Index<runir::kr::ps::base::Rule>>
{
    using Base = IndexMixin<Index<runir::kr::ps::base::Rule>>;
    using Base::Base;
};

}  // namespace ygg

#endif
