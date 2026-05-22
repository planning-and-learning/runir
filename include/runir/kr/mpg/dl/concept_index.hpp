#ifndef RUNIR_KR_MPG_DL_CONCEPT_INDEX_HPP_
#define RUNIR_KR_MPG_DL_CONCEPT_INDEX_HPP_

#include "runir/kr/mpg/dl/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::mpg::dl::ConceptConstructorTag Tag>
struct Index<runir::kr::mpg::dl::Concept<Tag>> : IndexMixin<Index<runir::kr::mpg::dl::Concept<Tag>>>
{
    using Base = IndexMixin<Index<runir::kr::mpg::dl::Concept<Tag>>>;
    using Base::Base;
};

}  // namespace tyr

#endif
