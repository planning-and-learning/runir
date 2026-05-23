#ifndef RUNIR_KR_PS_EXT_REGISTER_INDEX_HPP_
#define RUNIR_KR_PS_EXT_REGISTER_INDEX_HPP_

#include "runir/kr/ps/ext/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<>
struct Index<runir::kr::ps::ext::Register> : IndexMixin<Index<runir::kr::ps::ext::Register>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ext::Register>>;
    using Base::Base;
};

}  // namespace tyr

#endif
