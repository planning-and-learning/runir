#ifndef RUNIR_KR_PS_EXT_RULE_VARIANT_INDEX_HPP_
#define RUNIR_KR_PS_EXT_RULE_VARIANT_INDEX_HPP_

#include "runir/kr/ps/ext/declarations.hpp"

#include <yggdrasil/core/types.hpp>
#include <yggdrasil/ids/index_mixins.hpp>

namespace ygg
{

template<>
struct Index<runir::kr::ps::Rule<runir::kr::ExtFamilyTag>> : IndexMixin<Index<runir::kr::ps::Rule<runir::kr::ExtFamilyTag>>>
{
    using Base = IndexMixin<Index<runir::kr::ps::Rule<runir::kr::ExtFamilyTag>>>;
    using Base::Base;
};

}  // namespace ygg

#endif
