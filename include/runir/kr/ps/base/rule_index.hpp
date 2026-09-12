#ifndef RUNIR_KR_PS_BASE_RULE_INDEX_HPP_
#define RUNIR_KR_PS_BASE_RULE_INDEX_HPP_

#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"

#include <yggdrasil/core/types.hpp>
#include <yggdrasil/ids/index_mixins.hpp>

namespace ygg
{

template<>
struct Index<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>> : IndexMixin<Index<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>>>
{
    using Base = IndexMixin<Index<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>>>;
    using Base::Base;
};

}  // namespace ygg

#endif
