#ifndef RUNIR_KR_DL_BASE_DECLARATIONS_HPP_
#define RUNIR_KR_DL_BASE_DECLARATIONS_HPP_

#include "runir/kr/dl/declarations.hpp"

namespace runir::kr::dl
{

template<typename T>
struct IsFamilyConceptConstructorTag<runir::kr::BaseFamilyTag, T> : std::bool_constant<BaseConceptConstructorTag<T>>
{
};

template<typename T>
struct IsFamilyRoleConstructorTag<runir::kr::BaseFamilyTag, T> : std::bool_constant<BaseRoleConstructorTag<T>>
{
};

template<typename T>
struct IsFamilyBooleanConstructorTag<runir::kr::BaseFamilyTag, T> : std::bool_constant<BaseBooleanConstructorTag<T>>
{
};

template<typename T>
struct IsFamilyNumericalConstructorTag<runir::kr::BaseFamilyTag, T> : std::bool_constant<BaseNumericalConstructorTag<T>>
{
};

template<>
struct ConstructorTagLists<runir::kr::BaseFamilyTag>
{
    using Concept = BaseConceptConstructorTags;
    using Role = BaseRoleConstructorTags;
    using Boolean = BaseBooleanConstructorTags;
    using Numerical = BaseNumericalConstructorTags;
};

}  // namespace runir::kr::dl

#endif
