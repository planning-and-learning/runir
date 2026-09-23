#ifndef RUNIR_KR_PS_EXT_EXECUTION_INDEX_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_INDEX_HPP_

#include "runir/kr/dl/semantics/call_arguments_index.hpp"
#include "runir/kr/dl/semantics/register_values_index.hpp"
#include "runir/kr/ps/ext/execution_declarations.hpp"

#include <yggdrasil/core/types.hpp>
#include <yggdrasil/ids/index_mixins.hpp>

namespace ygg
{

template<tyr::TaskKind Kind>
struct Index<runir::kr::ps::ext::ModuleState<Kind>> : IndexMixin<Index<runir::kr::ps::ext::ModuleState<Kind>>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ext::ModuleState<Kind>>>;
    using Base::Base;
};

template<>
struct Index<runir::kr::ps::ext::CallStack> : IndexMixin<Index<runir::kr::ps::ext::CallStack>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ext::CallStack>>;
    using Base::Base;
};

template<tyr::TaskKind Kind>
struct Index<runir::kr::ps::ext::ProgramState<Kind>> : IndexMixin<Index<runir::kr::ps::ext::ProgramState<Kind>>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ext::ProgramState<Kind>>>;
    using Base::Base;
};

}  // namespace ygg

#endif
