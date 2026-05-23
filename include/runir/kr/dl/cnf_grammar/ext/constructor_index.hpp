#ifndef RUNIR_KR_DL_CNF_GRAMMAR_EXT_CONSTRUCTOR_INDEX_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_EXT_CONSTRUCTOR_INDEX_HPP_

#include "runir/kr/dl/cnf_grammar/ext/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::dl::CategoryTag Category>
struct Index<runir::kr::dl::cnf_grammar::ext::Constructor<Category>> : IndexMixin<Index<runir::kr::dl::cnf_grammar::ext::Constructor<Category>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::cnf_grammar::ext::Constructor<Category>>>;
    using Base::Base;
};

}  // namespace tyr

#endif
