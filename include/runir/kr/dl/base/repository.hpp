#ifndef RUNIR_KR_DL_BASE_REPOSITORY_HPP_
#define RUNIR_KR_DL_BASE_REPOSITORY_HPP_

#include "runir/kr/dl/base/datas.hpp"
#include "runir/kr/dl/base/declarations.hpp"
#include "runir/kr/dl/repository.hpp"

namespace runir::kr::dl::base
{

using ConstructorRepository = runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>;
using ConstructorRepositoryPtr = runir::kr::dl::ConstructorRepositoryPtrFor<runir::kr::BaseFamilyTag>;
using ConstructorRepositoryFactory = runir::kr::dl::ConstructorRepositoryFactoryFor<runir::kr::BaseFamilyTag>;

template<typename Tag>
    requires runir::kr::dl::BaseConceptConstructorTag<Tag>
using ConceptView = runir::kr::dl::FamilyConceptView<runir::kr::BaseFamilyTag, Tag>;

template<typename Tag>
    requires runir::kr::dl::BaseRoleConstructorTag<Tag>
using RoleView = runir::kr::dl::FamilyRoleView<runir::kr::BaseFamilyTag, Tag>;

template<runir::kr::dl::BaseBooleanConstructorTag Tag>
using BooleanView = runir::kr::dl::FamilyBooleanView<runir::kr::BaseFamilyTag, Tag>;

template<runir::kr::dl::BaseNumericalConstructorTag Tag>
using NumericalView = runir::kr::dl::FamilyNumericalView<runir::kr::BaseFamilyTag, Tag>;

template<runir::kr::dl::CategoryTag Category>
using ConstructorView = runir::kr::dl::FamilyConstructorView<runir::kr::BaseFamilyTag, Category>;

}  // namespace runir::kr::dl::base

#endif
