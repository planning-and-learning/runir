#ifndef RUNIR_KR_DL_CONSTRUCTOR_INDEX_HPP_
#define RUNIR_KR_DL_CONSTRUCTOR_INDEX_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct Index<runir::kr::dl::Constructor<Family, Category>> : IndexMixin<Index<runir::kr::dl::Constructor<Family, Category>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::Constructor<Family, Category>>>;
    using Base::Base;
};

}  // namespace tyr

#endif
