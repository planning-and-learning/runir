#ifndef RUNIR_SEMANTICS_DENOTATION_INDEX_HPP_
#define RUNIR_SEMANTICS_DENOTATION_INDEX_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"

#include <yggdrasil/core/config.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/ids/index_mixins.hpp>

namespace ygg
{

template<runir::kr::dl::CategoryTag Category>
struct Index<runir::kr::dl::semantics::Denotation<Category>> : IndexMixin<Index<runir::kr::dl::semantics::Denotation<Category>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::semantics::Denotation<Category>>>;
    using Base::Base;
};

}

#endif
