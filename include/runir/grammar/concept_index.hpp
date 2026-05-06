#ifndef RUNIR_GRAMMAR_CONCEPT_INDEX_HPP_
#define RUNIR_GRAMMAR_CONCEPT_INDEX_HPP_

#include "runir/grammar/constructors.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::ConceptConstructorTag Tag>
struct Index<runir::grammar::Concept<Tag>> : IndexMixin<Index<runir::grammar::Concept<Tag>>>
{
    using Base = IndexMixin<Index<runir::grammar::Concept<Tag>>>;
    using Base::Base;
};

}

#endif
