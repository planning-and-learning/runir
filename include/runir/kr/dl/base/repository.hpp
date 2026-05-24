#ifndef RUNIR_KR_DL_BASE_REPOSITORY_HPP_
#define RUNIR_KR_DL_BASE_REPOSITORY_HPP_

#include "runir/kr/dl/base/datas.hpp"
#include "runir/kr/dl/declarations.hpp"

#include <tyr/common/type_list.hpp>

namespace runir::kr::dl::base
{

template<runir::kr::dl::FamilyTag Family>
using ConceptTypes = tyr::MapTypeListSecondT<runir::kr::dl::Concept, Family, runir::kr::dl::FamilyConceptConstructorTags<Family>>;

template<runir::kr::dl::FamilyTag Family>
using RoleTypes = tyr::MapTypeListSecondT<runir::kr::dl::Role, Family, runir::kr::dl::FamilyRoleConstructorTags<Family>>;

template<runir::kr::dl::FamilyTag Family>
using BooleanTypes = tyr::MapTypeListSecondT<runir::kr::dl::Boolean, Family, runir::kr::dl::FamilyBooleanConstructorTags<Family>>;

template<runir::kr::dl::FamilyTag Family>
using NumericalTypes = tyr::MapTypeListSecondT<runir::kr::dl::Numerical, Family, runir::kr::dl::FamilyNumericalConstructorTags<Family>>;

template<runir::kr::dl::FamilyTag Family>
using ConstructorTypes = tyr::MapTypeListSecondT<runir::kr::dl::Constructor, Family, runir::kr::dl::CategoryTags>;

template<runir::kr::dl::FamilyTag Family>
using ConstructorRepositoryTypes =
    tyr::ConcatTypeListsT<ConceptTypes<Family>, RoleTypes<Family>, BooleanTypes<Family>, NumericalTypes<Family>, ConstructorTypes<Family>>;

}  // namespace runir::kr::dl::base

#endif
