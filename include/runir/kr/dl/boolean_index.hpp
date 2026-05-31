#ifndef RUNIR_KR_DL_BOOLEAN_INDEX_HPP_
#define RUNIR_KR_DL_BOOLEAN_INDEX_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <yggdrasil/ids/index_mixins.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyBooleanConstructorTag<Family, Tag>
struct Index<runir::kr::dl::Boolean<Family, Tag>> : IndexMixin<Index<runir::kr::dl::Boolean<Family, Tag>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::Boolean<Family, Tag>>>;
    using Base::Base;
};

}  // namespace ygg

#endif
