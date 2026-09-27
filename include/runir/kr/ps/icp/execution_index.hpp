#ifndef RUNIR_KR_PS_ICP_EXECUTION_INDEX_HPP_
#define RUNIR_KR_PS_ICP_EXECUTION_INDEX_HPP_

#include "runir/kr/ps/icp/execution_declarations.hpp"

#include <yggdrasil/ids/index_mixins.hpp>

namespace ygg
{

template<>
struct Index<runir::kr::ps::icp::Histories> : IndexMixin<Index<runir::kr::ps::icp::Histories>>
{
    using Base = IndexMixin<Index<runir::kr::ps::icp::Histories>>;
    using Base::Base;
};

template<tyr::TaskKind Kind>
struct Index<runir::kr::ps::icp::ProgramState<Kind>> : IndexMixin<Index<runir::kr::ps::icp::ProgramState<Kind>>>
{
    using Base = IndexMixin<Index<runir::kr::ps::icp::ProgramState<Kind>>>;
    using Base::Base;
};

}

#endif
