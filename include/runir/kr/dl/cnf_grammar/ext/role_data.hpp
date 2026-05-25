#ifndef RUNIR_KR_DL_CNF_GRAMMAR_EXT_ROLE_DATA_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_EXT_ROLE_DATA_HPP_

#include "runir/kr/dl/cnf_grammar/role_data.hpp"
#include "runir/kr/dl/ext/declarations.hpp"

namespace tyr
{

template<>
struct Data<runir::kr::dl::cnf_grammar::Role<runir::kr::ExtFamilyTag, runir::kr::dl::RegisterTag>> :
    runir::kr::dl::cnf_grammar::RegisterData<runir::kr::dl::cnf_grammar::Role<runir::kr::ExtFamilyTag, runir::kr::dl::RegisterTag>,
                                             runir::kr::dl::RegisterIdentifier<runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::cnf_grammar::RegisterData<runir::kr::dl::cnf_grammar::Role<runir::kr::ExtFamilyTag, runir::kr::dl::RegisterTag>,
                                                          runir::kr::dl::RegisterIdentifier<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::cnf_grammar::Role<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>> :
    runir::kr::dl::semantics::ArgumentData<runir::kr::dl::cnf_grammar::Role<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>,
                                           runir::kr::dl::ArgumentIdentifier<runir::kr::dl::RoleTag>>
{
    using Base =
        runir::kr::dl::semantics::ArgumentData<runir::kr::dl::cnf_grammar::Role<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>,
                                               runir::kr::dl::ArgumentIdentifier<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

}  // namespace tyr

#endif
