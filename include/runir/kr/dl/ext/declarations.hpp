#ifndef RUNIR_KR_DL_EXT_DECLARATIONS_HPP_
#define RUNIR_KR_DL_EXT_DECLARATIONS_HPP_

#include "runir/kr/dl/declarations.hpp"

namespace runir::kr::dl::ext
{

using FamilyTag = runir::kr::dl::ExtFamilyTag;
using RegisterTag = runir::kr::dl::RegisterTag;

template<typename Tag>
    requires runir::kr::dl::FamilyConceptConstructorTag<FamilyTag, Tag>
using Concept = runir::kr::dl::Concept<FamilyTag, Tag>;

template<runir::kr::dl::RoleConstructorTag Tag>
using Role = runir::kr::dl::Role<FamilyTag, Tag>;

template<runir::kr::dl::BooleanConstructorTag Tag>
using Boolean = runir::kr::dl::Boolean<FamilyTag, Tag>;

template<runir::kr::dl::NumericalConstructorTag Tag>
using Numerical = runir::kr::dl::Numerical<FamilyTag, Tag>;

template<runir::kr::dl::CategoryTag Category>
using Constructor = runir::kr::dl::Constructor<FamilyTag, Category>;

}  // namespace runir::kr::dl::ext

#endif
