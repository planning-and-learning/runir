#ifndef RUNIR_KR_PS_EXT_EXECUTION_INDEX_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_INDEX_HPP_

#include "runir/kr/ps/ext/execution_declarations.hpp"

#include <yggdrasil/core/types.hpp>
#include <yggdrasil/ids/index_mixins.hpp>

namespace ygg
{

template<>
struct Index<runir::kr::ps::ext::RegisterValues> : IndexMixin<Index<runir::kr::ps::ext::RegisterValues>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ext::RegisterValues>>;
    using Base::Base;
};

template<>
struct Index<runir::kr::ps::ext::CallArguments> : IndexMixin<Index<runir::kr::ps::ext::CallArguments>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ext::CallArguments>>;
    using Base::Base;
};

template<>
struct Index<runir::kr::ps::ext::CallStack> : IndexMixin<Index<runir::kr::ps::ext::CallStack>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ext::CallStack>>;
    using Base::Base;
};

template<tyr::planning::TaskKind Kind>
struct Index<runir::kr::ps::ext::ExecutionState<Kind>> : IndexMixin<Index<runir::kr::ps::ext::ExecutionState<Kind>>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ext::ExecutionState<Kind>>>;
    using Base::Base;
};

}  // namespace ygg

#endif
