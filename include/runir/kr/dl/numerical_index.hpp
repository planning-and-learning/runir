#ifndef RUNIR_KR_DL_NUMERICAL_INDEX_HPP_
#define RUNIR_KR_DL_NUMERICAL_INDEX_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::NumericalConstructorTag<Tag>
struct Index<runir::kr::dl::Numerical<Family, Tag>> : IndexMixin<Index<runir::kr::dl::Numerical<Family, Tag>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::Numerical<Family, Tag>>>;
    using Base::Base;
};

}  // namespace tyr

#endif
