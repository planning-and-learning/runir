#ifndef RUNIR_KR_PS_BASE_SKETCH_INDEX_HPP_
#define RUNIR_KR_PS_BASE_SKETCH_INDEX_HPP_

#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<>
struct Index<runir::kr::ps::base::Sketch> : IndexMixin<Index<runir::kr::ps::base::Sketch>>
{
    using Base = IndexMixin<Index<runir::kr::ps::base::Sketch>>;
    using Base::Base;
};

}  // namespace tyr

#endif
