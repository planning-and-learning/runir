#ifndef RUNIR_KR_DL_EXT_ROLE_DATA_HPP_
#define RUNIR_KR_DL_EXT_ROLE_DATA_HPP_

#include "runir/kr/dl/ext/declarations.hpp"
#include "runir/kr/dl/role_data.hpp"

namespace ygg
{

template<>
struct Data<runir::kr::dl::Role<runir::kr::ExtFamilyTag, runir::kr::dl::RegisterTag>> :
    runir::kr::dl::semantics::RegisterData<runir::kr::dl::Role<runir::kr::ExtFamilyTag, runir::kr::dl::RegisterTag>,
                                           runir::kr::dl::RegisterIdentifier<runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::semantics::RegisterData<runir::kr::dl::Role<runir::kr::ExtFamilyTag, runir::kr::dl::RegisterTag>,
                                                        runir::kr::dl::RegisterIdentifier<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::Role<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>> :
    runir::kr::dl::semantics::ArgumentData<runir::kr::dl::Role<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>,
                                           runir::kr::dl::ArgumentIdentifier<runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::semantics::ArgumentData<runir::kr::dl::Role<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>,
                                                        runir::kr::dl::ArgumentIdentifier<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

}  // namespace ygg

#endif
