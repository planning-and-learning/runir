#ifndef RUNIR_KR_PS_SKETCH_INDEX_HPP_
#define RUNIR_KR_PS_SKETCH_INDEX_HPP_

#include "runir/kr/ps/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<>
struct Index<runir::kr::ps::Sketch> : IndexMixin<Index<runir::kr::ps::Sketch>>
{
    using Base = IndexMixin<Index<runir::kr::ps::Sketch>>;
    using Base::Base;
};

}  // namespace tyr

#endif
