#ifndef RUNIR_GRAMMAR_CONCEPT_INDEX_HPP_
#define RUNIR_GRAMMAR_CONCEPT_INDEX_HPP_

#include "runir/kr/dl/grammar/constructors.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::dl::ConceptConstructorTag Tag>
struct Index<runir::kr::dl::grammar::Concept<Tag>> : IndexMixin<Index<runir::kr::dl::grammar::Concept<Tag>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::grammar::Concept<Tag>>>;
    using Base::Base;
};

}

#endif
