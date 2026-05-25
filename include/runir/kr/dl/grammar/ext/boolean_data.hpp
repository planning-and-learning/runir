#ifndef RUNIR_KR_DL_GRAMMAR_EXT_BOOLEAN_DATA_HPP_
#define RUNIR_KR_DL_GRAMMAR_EXT_BOOLEAN_DATA_HPP_

#include "runir/kr/dl/ext/declarations.hpp"
#include "runir/kr/dl/grammar/boolean_data.hpp"

namespace tyr
{

template<>
struct Data<runir::kr::dl::grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>> :
    runir::kr::dl::semantics::ArgumentData<runir::kr::dl::grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>,
                                           runir::kr::dl::ArgumentIdentifier<runir::kr::dl::BooleanTag>>
{
    using Base =
        runir::kr::dl::semantics::ArgumentData<runir::kr::dl::grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>,
                                               runir::kr::dl::ArgumentIdentifier<runir::kr::dl::BooleanTag>>;
    using Base::Base;
};

}  // namespace tyr

#endif
