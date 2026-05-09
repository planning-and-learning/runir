#ifndef RUNIR_ROLE_INDEX_HPP_
#define RUNIR_ROLE_INDEX_HPP_

#include "runir/knowledge_representation/dl/constructors.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::dl::RoleConstructorTag Tag>
struct Index<runir::kr::dl::Role<Tag>> : IndexMixin<Index<runir::kr::dl::Role<Tag>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::Role<Tag>>>;
    using Base::Base;
};

}

#endif
