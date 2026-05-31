#ifndef RUNIR_CNF_GRAMMAR_NUMERICAL_INDEX_HPP_
#define RUNIR_CNF_GRAMMAR_NUMERICAL_INDEX_HPP_

#include "runir/kr/dl/cnf_grammar/declarations.hpp"

#include <yggdrasil/ids/index_mixins.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Family, Tag>
struct Index<runir::kr::dl::cnf_grammar::Numerical<Family, Tag>> : IndexMixin<Index<runir::kr::dl::cnf_grammar::Numerical<Family, Tag>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::cnf_grammar::Numerical<Family, Tag>>>;
    using Base::Base;
};

}

#endif
