#ifndef RUNIR_GRAMMAR_GRAMMAR_INDEX_HPP_
#define RUNIR_GRAMMAR_GRAMMAR_INDEX_HPP_

#include "runir/grammar/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<>
struct Index<runir::grammar::GrammarTag> : IndexMixin<Index<runir::grammar::GrammarTag>>
{
    using Base = IndexMixin<Index<runir::grammar::GrammarTag>>;
    using Base::Base;
};

}

#endif
