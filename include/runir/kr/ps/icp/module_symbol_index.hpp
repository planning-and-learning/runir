#ifndef RUNIR_KR_PS_ICP_MODULE_SYMBOL_INDEX_HPP_
#define RUNIR_KR_PS_ICP_MODULE_SYMBOL_INDEX_HPP_

#include "runir/kr/ps/icp/declarations.hpp"

#include <yggdrasil/core/types.hpp>
#include <yggdrasil/ids/index_mixins.hpp>

namespace ygg
{

template<>
struct Index<runir::kr::ps::icp::ModuleSymbol> : IndexMixin<Index<runir::kr::ps::icp::ModuleSymbol>>
{
    using Base = IndexMixin<Index<runir::kr::ps::icp::ModuleSymbol>>;
    using Base::Base;
};

}  // namespace ygg

#endif
