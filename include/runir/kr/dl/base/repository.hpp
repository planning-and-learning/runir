#ifndef RUNIR_KR_DL_BASE_REPOSITORY_HPP_
#define RUNIR_KR_DL_BASE_REPOSITORY_HPP_

#include "runir/kr/dl/base/datas.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/ext/datas.hpp"

#include <tyr/common/type_list.hpp>

namespace runir::kr::dl::base
{

template<runir::kr::dl::FamilyTag Family>
using ConceptTypes = tyr::MapTypeListSecondT<runir::kr::dl::Concept, Family, runir::kr::dl::ConceptConstructorTags>;

template<runir::kr::dl::FamilyTag Family>
using RoleTypes = tyr::MapTypeListSecondT<runir::kr::dl::Role, Family, runir::kr::dl::RoleConstructorTags>;

template<runir::kr::dl::FamilyTag Family>
using BooleanTypes = tyr::MapTypeListSecondT<runir::kr::dl::Boolean, Family, runir::kr::dl::BooleanConstructorTags>;

template<runir::kr::dl::FamilyTag Family>
using NumericalTypes = tyr::MapTypeListSecondT<runir::kr::dl::Numerical, Family, runir::kr::dl::NumericalConstructorTags>;

template<runir::kr::dl::FamilyTag Family>
using ConstructorTypes = tyr::MapTypeListSecondT<runir::kr::dl::Constructor, Family, runir::kr::dl::CategoryTags>;

template<runir::kr::dl::FamilyTag Family>
using ConstructorRepositoryTypes =
    tyr::ConcatTypeListsT<ConceptTypes<Family>, RoleTypes<Family>, BooleanTypes<Family>, NumericalTypes<Family>, ConstructorTypes<Family>>;

}  // namespace runir::kr::dl::base

#endif
