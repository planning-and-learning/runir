#ifndef RUNIR_KR_DL_QUERY_INDEX_HPP_
#define RUNIR_KR_DL_QUERY_INDEX_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <yggdrasil/ids/index_mixins.hpp>

namespace ygg
{

template<>
struct Index<runir::kr::dl::QueryColumn> : IndexMixin<Index<runir::kr::dl::QueryColumn>>
{
    using Base = IndexMixin<Index<runir::kr::dl::QueryColumn>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, typename Tag>
struct Index<runir::kr::dl::Query<Family, Tag>> : IndexMixin<Index<runir::kr::dl::Query<Family, Tag>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::Query<Family, Tag>>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct Index<runir::kr::dl::QueryProjection<Family, Category>> : IndexMixin<Index<runir::kr::dl::QueryProjection<Family, Category>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::QueryProjection<Family, Category>>>;
    using Base::Base;
};

}  // namespace ygg

#endif
