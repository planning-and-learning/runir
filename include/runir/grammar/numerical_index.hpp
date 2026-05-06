#ifndef RUNIR_GRAMMAR_NUMERICAL_INDEX_HPP_
#define RUNIR_GRAMMAR_NUMERICAL_INDEX_HPP_

#include "runir/grammar/constructors.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::NumericalConstructorTag Tag>
struct Index<runir::grammar::Numerical<Tag>> : IndexMixin<Index<runir::grammar::Numerical<Tag>>>
{
    using Base = IndexMixin<Index<runir::grammar::Numerical<Tag>>>;
    using Base::Base;
};

}

#endif
