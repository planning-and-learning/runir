#ifndef RUNIR_KR_DL_UNS_REPOSITORY_HPP_
#define RUNIR_KR_DL_UNS_REPOSITORY_HPP_

#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/uns/datas.hpp"

namespace runir::kr::dl::uns
{

using ConstructorRepository = runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>;
using ConstructorRepositoryPtr = runir::kr::dl::ConstructorRepositoryPtrFor<runir::kr::UnsFamilyTag>;
using ConstructorRepositoryFactory = runir::kr::dl::ConstructorRepositoryFactoryFor<runir::kr::UnsFamilyTag>;

template<typename Tag>
    requires runir::kr::dl::UnsConceptConstructorTag<Tag>
using ConceptView = runir::kr::dl::FamilyConceptView<runir::kr::UnsFamilyTag, Tag>;

template<typename Tag>
    requires runir::kr::dl::UnsRoleConstructorTag<Tag>
using RoleView = runir::kr::dl::FamilyRoleView<runir::kr::UnsFamilyTag, Tag>;

template<runir::kr::dl::UnsBooleanConstructorTag Tag>
using BooleanView = runir::kr::dl::FamilyBooleanView<runir::kr::UnsFamilyTag, Tag>;

template<runir::kr::dl::UnsNumericalConstructorTag Tag>
using NumericalView = runir::kr::dl::FamilyNumericalView<runir::kr::UnsFamilyTag, Tag>;

template<runir::kr::dl::CategoryTag Category>
using ConstructorView = runir::kr::dl::FamilyConstructorView<runir::kr::UnsFamilyTag, Category>;

}  // namespace runir::kr::dl::uns

#endif
