#ifndef RUNIR_GRAMMAR_GRAMMAR_INDEX_HPP_
#define RUNIR_GRAMMAR_GRAMMAR_INDEX_HPP_

#include "runir/kr/dl/grammar/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::dl::FamilyTag Family>
struct Index<runir::kr::dl::grammar::GrammarTag<Family>> : IndexMixin<Index<runir::kr::dl::grammar::GrammarTag<Family>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::grammar::GrammarTag<Family>>>;
    using Base::Base;
};

}  // namespace tyr

#endif
