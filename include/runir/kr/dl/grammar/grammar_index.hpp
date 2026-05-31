#ifndef RUNIR_GRAMMAR_GRAMMAR_INDEX_HPP_
#define RUNIR_GRAMMAR_GRAMMAR_INDEX_HPP_

#include "runir/kr/dl/grammar/declarations.hpp"

#include <yggdrasil/ids/index_mixins.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family>
struct Index<runir::kr::dl::grammar::GrammarTag<Family>> : IndexMixin<Index<runir::kr::dl::grammar::GrammarTag<Family>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::grammar::GrammarTag<Family>>>;
    using Base::Base;
};

}  // namespace ygg

#endif
