#ifndef RUNIR_KR_DL_EXT_CONCEPT_DATA_HPP_
#define RUNIR_KR_DL_EXT_CONCEPT_DATA_HPP_

#include "runir/kr/dl/concept_data.hpp"
#include "runir/kr/dl/ext/declarations.hpp"

namespace tyr
{

template<>
struct Data<runir::kr::dl::Concept<runir::kr::dl::ExtFamilyTag, runir::kr::dl::RegisterTag>> :
    runir::kr::dl::semantics::RegisterData<runir::kr::dl::Concept<runir::kr::dl::ExtFamilyTag, runir::kr::dl::RegisterTag>,
                                           runir::kr::dl::RegisterIdentifier<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::semantics::RegisterData<runir::kr::dl::Concept<runir::kr::dl::ExtFamilyTag, runir::kr::dl::RegisterTag>,
                                                        runir::kr::dl::RegisterIdentifier<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::Concept<runir::kr::dl::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>> :
    runir::kr::dl::semantics::ArgumentData<runir::kr::dl::Concept<runir::kr::dl::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>,
                                           runir::kr::dl::ArgumentIdentifier<runir::kr::dl::ConceptTag>>
{
    using Base =
        runir::kr::dl::semantics::ArgumentData<runir::kr::dl::Concept<runir::kr::dl::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>,
                                               runir::kr::dl::ArgumentIdentifier<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

}  // namespace tyr

#endif
