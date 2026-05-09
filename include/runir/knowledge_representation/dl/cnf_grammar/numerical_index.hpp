#ifndef RUNIR_CNF_GRAMMAR_NUMERICAL_INDEX_HPP_
#define RUNIR_CNF_GRAMMAR_NUMERICAL_INDEX_HPP_

#include "runir/knowledge_representation/dl/cnf_grammar/constructors.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::dl::NumericalConstructorTag Tag>
struct Index<runir::kr::dl::cnf_grammar::Numerical<Tag>> : IndexMixin<Index<runir::kr::dl::cnf_grammar::Numerical<Tag>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::cnf_grammar::Numerical<Tag>>>;
    using Base::Base;
};

}

#endif
