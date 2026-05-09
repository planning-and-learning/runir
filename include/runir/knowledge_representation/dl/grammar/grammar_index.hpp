#ifndef RUNIR_GRAMMAR_GRAMMAR_INDEX_HPP_
#define RUNIR_GRAMMAR_GRAMMAR_INDEX_HPP_

#include "runir/knowledge_representation/dl/grammar/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<>
struct Index<runir::kr::dl::grammar::GrammarTag> : IndexMixin<Index<runir::kr::dl::grammar::GrammarTag>>
{
    using Base = IndexMixin<Index<runir::kr::dl::grammar::GrammarTag>>;
    using Base::Base;
};

}

#endif
