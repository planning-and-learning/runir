#ifndef RUNIR_KR_DL_SEMANTICS_REGISTER_VALUES_INDEX_HPP_
#define RUNIR_KR_DL_SEMANTICS_REGISTER_VALUES_INDEX_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"

#include <yggdrasil/core/types.hpp>
#include <yggdrasil/ids/index_mixins.hpp>

namespace ygg
{

template<>
struct Index<runir::kr::dl::semantics::RegisterValues> : IndexMixin<Index<runir::kr::dl::semantics::RegisterValues>>
{
    using Base = IndexMixin<Index<runir::kr::dl::semantics::RegisterValues>>;
    using Base::Base;
};

}  // namespace ygg

#endif
