#ifndef RUNIR_GRAMMAR_BOOLEAN_INDEX_HPP_
#define RUNIR_GRAMMAR_BOOLEAN_INDEX_HPP_

#include "runir/kr/dl/grammar/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::BooleanConstructorTag<Tag>
struct Index<runir::kr::dl::grammar::Boolean<Family, Tag>> : IndexMixin<Index<runir::kr::dl::grammar::Boolean<Family, Tag>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::grammar::Boolean<Family, Tag>>>;
    using Base::Base;
};

}  // namespace tyr

#endif
