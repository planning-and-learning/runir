#ifndef RUNIR_KR_DL_EXT_CONCEPT_DATA_HPP_
#define RUNIR_KR_DL_EXT_CONCEPT_DATA_HPP_

#include "runir/kr/dl/concept_index.hpp"
#include "runir/kr/dl/ext/declarations.hpp"
#include "runir/kr/dl/ext/register_index.hpp"
#include "runir/kr/dl/semantics/data_helpers.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<>
struct Data<runir::kr::dl::Concept<runir::kr::dl::ExtFamilyTag, runir::kr::dl::RegisterTag>> :
    runir::kr::dl::semantics::RegisterData<runir::kr::dl::Concept<runir::kr::dl::ExtFamilyTag, runir::kr::dl::RegisterTag>,
                                           runir::kr::dl::Concept<runir::kr::dl::ExtFamilyTag, runir::kr::dl::RegisterIdentifierTag>>
{
    using Base = runir::kr::dl::semantics::RegisterData<runir::kr::dl::Concept<runir::kr::dl::ExtFamilyTag, runir::kr::dl::RegisterTag>,
                                                        runir::kr::dl::Concept<runir::kr::dl::ExtFamilyTag, runir::kr::dl::RegisterIdentifierTag>>;
    using Base::Base;
};

}  // namespace tyr

#endif
