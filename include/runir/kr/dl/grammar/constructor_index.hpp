#ifndef RUNIR_GRAMMAR_CONSTRUCTOR_INDEX_HPP_
#define RUNIR_GRAMMAR_CONSTRUCTOR_INDEX_HPP_

#include "runir/kr/dl/grammar/declarations.hpp"

#include <yggdrasil/ids/index_mixins.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct Index<runir::kr::dl::grammar::Constructor<Family, Category>> : IndexMixin<Index<runir::kr::dl::grammar::Constructor<Family, Category>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::grammar::Constructor<Family, Category>>>;
    using Base::Base;
};

}  // namespace ygg

#endif
