#ifndef RUNIR_CNF_GRAMMAR_BOOLEAN_INDEX_HPP_
#define RUNIR_CNF_GRAMMAR_BOOLEAN_INDEX_HPP_

#include "runir/kr/dl/cnf_grammar/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::BooleanConstructorTag Tag>
struct Index<runir::kr::dl::cnf_grammar::Boolean<Family, Tag>> : IndexMixin<Index<runir::kr::dl::cnf_grammar::Boolean<Family, Tag>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::cnf_grammar::Boolean<Family, Tag>>>;
    using Base::Base;
};

}

#endif
