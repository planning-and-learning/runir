#ifndef RUNIR_KR_DL_SEMANTICS_CALL_ARGUMENTS_INDEX_HPP_
#define RUNIR_KR_DL_SEMANTICS_CALL_ARGUMENTS_INDEX_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"

#include <yggdrasil/core/types.hpp>
#include <yggdrasil/ids/index_mixins.hpp>

namespace ygg
{

template<>
struct Index<runir::kr::dl::semantics::CallArguments> : IndexMixin<Index<runir::kr::dl::semantics::CallArguments>>
{
    using Base = IndexMixin<Index<runir::kr::dl::semantics::CallArguments>>;
    using Base::Base;
};

}  // namespace ygg

#endif
