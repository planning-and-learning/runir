#ifndef RUNIR_KR_DL_CONCEPT_INDEX_HPP_
#define RUNIR_KR_DL_CONCEPT_INDEX_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::ConceptConstructorTag Tag>
struct Index<runir::kr::dl::Concept<Family, Tag>> : IndexMixin<Index<runir::kr::dl::Concept<Family, Tag>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::Concept<Family, Tag>>>;
    using Base::Base;
};

}  // namespace tyr

#endif
