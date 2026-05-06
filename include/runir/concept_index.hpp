#ifndef RUNIR_CONCEPT_INDEX_HPP_
#define RUNIR_CONCEPT_INDEX_HPP_

#include "runir/constructors.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::ConceptConstructorTag Tag>
struct Index<runir::Concept<Tag>> : IndexMixin<Index<runir::Concept<Tag>>>
{
    using Base = IndexMixin<Index<runir::Concept<Tag>>>;
    using Base::Base;
};

}

#endif
