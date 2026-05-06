#ifndef RUNIR_GRAMMAR_ROLE_INDEX_HPP_
#define RUNIR_GRAMMAR_ROLE_INDEX_HPP_

#include "runir/grammar/constructors.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::RoleConstructorTag Tag>
struct Index<runir::grammar::Role<Tag>> : IndexMixin<Index<runir::grammar::Role<Tag>>>
{
    using Base = IndexMixin<Index<runir::grammar::Role<Tag>>>;
    using Base::Base;
};

}

#endif
