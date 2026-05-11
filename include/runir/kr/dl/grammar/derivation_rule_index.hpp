#ifndef RUNIR_GRAMMAR_DERIVATION_RULE_INDEX_HPP_
#define RUNIR_GRAMMAR_DERIVATION_RULE_INDEX_HPP_

#include "runir/kr/dl/grammar/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::dl::CategoryTag Category>
struct Index<runir::kr::dl::grammar::DerivationRule<Category>> : IndexMixin<Index<runir::kr::dl::grammar::DerivationRule<Category>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::grammar::DerivationRule<Category>>>;
    using Base::Base;
};

}

#endif
