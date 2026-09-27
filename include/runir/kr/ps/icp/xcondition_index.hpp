#ifndef RUNIR_KR_PS_ICP_XCONDITION_INDEX_HPP_
#define RUNIR_KR_PS_ICP_XCONDITION_INDEX_HPP_

#include "runir/kr/ps/icp/declarations.hpp"

#include <yggdrasil/core/types.hpp>
#include <yggdrasil/ids/index_mixins.hpp>

namespace ygg
{

template<>
struct Index<runir::kr::ps::icp::XCondition> : IndexMixin<Index<runir::kr::ps::icp::XCondition>>
{
    using Base = IndexMixin<Index<runir::kr::ps::icp::XCondition>>;
    using Base::Base;
};

}  // namespace ygg

#endif
