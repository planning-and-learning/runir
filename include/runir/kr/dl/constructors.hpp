#ifndef RUNIR_CONSTRUCTORS_HPP_
#define RUNIR_CONSTRUCTORS_HPP_

#include "runir/kr/dl/declarations.hpp"

namespace runir::kr::dl
{

template<typename Tag>
    requires FamilyConceptConstructorTag<Tag>
using FamilyConcept = Concept<Tag>;

template<typename Tag>
    requires FamilyRoleConstructorTag<Tag>
using FamilyRole = Role<Tag>;

template<typename Tag>
    requires FamilyBooleanConstructorTag<Tag>
using FamilyBoolean = Boolean<Tag>;

template<typename Tag>
    requires FamilyNumericalConstructorTag<Tag>
using FamilyNumerical = Numerical<Tag>;

template<CategoryTag Category>
using FamilyConstructor = Constructor<Category>;
}

#endif
