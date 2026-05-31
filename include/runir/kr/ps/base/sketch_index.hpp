#ifndef RUNIR_KR_PS_BASE_SKETCH_INDEX_HPP_
#define RUNIR_KR_PS_BASE_SKETCH_INDEX_HPP_

#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"

#include <yggdrasil/ids/index_mixins.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<>
struct Index<runir::kr::ps::base::Sketch> : IndexMixin<Index<runir::kr::ps::base::Sketch>>
{
    using Base = IndexMixin<Index<runir::kr::ps::base::Sketch>>;
    using Base::Base;
};

}  // namespace ygg

#endif
