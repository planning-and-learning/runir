#ifndef RUNIR_CNF_GRAMMAR_NUMERICAL_INDEX_HPP_
#define RUNIR_CNF_GRAMMAR_NUMERICAL_INDEX_HPP_

#include "runir/cnf_grammar/constructors.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::NumericalConstructorTag Tag>
struct Index<runir::cnf_grammar::Numerical<Tag>> : IndexMixin<Index<runir::cnf_grammar::Numerical<Tag>>>
{
    using Base = IndexMixin<Index<runir::cnf_grammar::Numerical<Tag>>>;
    using Base::Base;
};

}

#endif
