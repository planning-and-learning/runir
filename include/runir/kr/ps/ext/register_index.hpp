#ifndef RUNIR_KR_PS_EXT_REGISTER_INDEX_HPP_
#define RUNIR_KR_PS_EXT_REGISTER_INDEX_HPP_

#include "runir/kr/ps/ext/declarations.hpp"

#include <yggdrasil/ids/index_mixins.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<>
struct Index<runir::kr::ps::ext::Register> : IndexMixin<Index<runir::kr::ps::ext::Register>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ext::Register>>;
    using Base::Base;
};

}  // namespace ygg

#endif
