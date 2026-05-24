#ifndef RUNIR_KR_DL_CNF_GRAMMAR_EXT_BOOLEAN_DATA_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_EXT_BOOLEAN_DATA_HPP_

#include "runir/kr/dl/cnf_grammar/boolean_data.hpp"
#include "runir/kr/dl/ext/declarations.hpp"

namespace tyr
{

template<>
struct Data<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>> :
    runir::kr::dl::semantics::ArgumentData<
        runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>,
        runir::kr::dl::ArgumentIdentifier<runir::kr::dl::BooleanTag>>
{
    using Base = runir::kr::dl::semantics::ArgumentData<
        runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>,
        runir::kr::dl::ArgumentIdentifier<runir::kr::dl::BooleanTag>>;
    using Base::Base;
};

}  // namespace tyr

#endif
