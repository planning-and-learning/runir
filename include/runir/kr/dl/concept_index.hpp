#ifndef RUNIR_CONCEPT_INDEX_HPP_
#define RUNIR_CONCEPT_INDEX_HPP_

#include "runir/kr/dl/constructors.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::dl::ConceptConstructorTag Tag>
struct Index<runir::kr::dl::Concept<Tag>> : IndexMixin<Index<runir::kr::dl::Concept<Tag>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::Concept<Tag>>>;
    using Base::Base;
};

}

#endif
