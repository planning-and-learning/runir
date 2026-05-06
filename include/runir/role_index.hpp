#ifndef RUNIR_ROLE_INDEX_HPP_
#define RUNIR_ROLE_INDEX_HPP_

#include "runir/constructors.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::RoleConstructorTag Tag>
struct Index<runir::Role<Tag>> : IndexMixin<Index<runir::Role<Tag>>>
{
    using Base = IndexMixin<Index<runir::Role<Tag>>>;
    using Base::Base;
};

}

#endif
