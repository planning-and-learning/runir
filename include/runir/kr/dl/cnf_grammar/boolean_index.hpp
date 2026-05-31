#ifndef RUNIR_CNF_GRAMMAR_BOOLEAN_INDEX_HPP_
#define RUNIR_CNF_GRAMMAR_BOOLEAN_INDEX_HPP_

#include "runir/kr/dl/cnf_grammar/declarations.hpp"

#include <yggdrasil/ids/index_mixins.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyBooleanConstructorTag<Family, Tag>
struct Index<runir::kr::dl::cnf_grammar::Boolean<Family, Tag>> : IndexMixin<Index<runir::kr::dl::cnf_grammar::Boolean<Family, Tag>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::cnf_grammar::Boolean<Family, Tag>>>;
    using Base::Base;
};

}

#endif
