#ifndef RUNIR_KR_PS_SKETCH_INDEX_HPP_
#define RUNIR_KR_PS_SKETCH_INDEX_HPP_

#include "runir/kr/ps/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::FamilyTag Family>
struct Index<runir::kr::ps::Sketch<Family>> : IndexMixin<Index<runir::kr::ps::Sketch<Family>>>
{
    using Base = IndexMixin<Index<runir::kr::ps::Sketch<Family>>>;
    using Base::Base;
};

}  // namespace tyr

#endif
