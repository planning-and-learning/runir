#ifndef RUNIR_KR_PS_EXT_MEMORY_STATE_INDEX_HPP_
#define RUNIR_KR_PS_EXT_MEMORY_STATE_INDEX_HPP_

#include "runir/kr/ps/ext/declarations.hpp"

#include <yggdrasil/ids/index_mixins.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<>
struct Index<runir::kr::ps::ext::MemoryState> : IndexMixin<Index<runir::kr::ps::ext::MemoryState>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ext::MemoryState>>;
    using Base::Base;
};

}  // namespace ygg

#endif
