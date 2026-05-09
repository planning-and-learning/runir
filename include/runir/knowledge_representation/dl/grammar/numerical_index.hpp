#ifndef RUNIR_GRAMMAR_NUMERICAL_INDEX_HPP_
#define RUNIR_GRAMMAR_NUMERICAL_INDEX_HPP_

#include "runir/knowledge_representation/dl/grammar/constructors.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::dl::NumericalConstructorTag Tag>
struct Index<runir::kr::dl::grammar::Numerical<Tag>> : IndexMixin<Index<runir::kr::dl::grammar::Numerical<Tag>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::grammar::Numerical<Tag>>>;
    using Base::Base;
};

}

#endif
