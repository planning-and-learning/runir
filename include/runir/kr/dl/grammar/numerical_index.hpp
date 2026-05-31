#ifndef RUNIR_GRAMMAR_NUMERICAL_INDEX_HPP_
#define RUNIR_GRAMMAR_NUMERICAL_INDEX_HPP_

#include "runir/kr/dl/grammar/declarations.hpp"

#include <yggdrasil/ids/index_mixins.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Family, Tag>
struct Index<runir::kr::dl::grammar::Numerical<Family, Tag>> : IndexMixin<Index<runir::kr::dl::grammar::Numerical<Family, Tag>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::grammar::Numerical<Family, Tag>>>;
    using Base::Base;
};

}  // namespace ygg

#endif
