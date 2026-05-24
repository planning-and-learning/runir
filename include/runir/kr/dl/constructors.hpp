#ifndef RUNIR_CONSTRUCTORS_HPP_
#define RUNIR_CONSTRUCTORS_HPP_

#include "runir/kr/dl/declarations.hpp"

namespace runir::kr::dl
{

template<FamilyTag Family, typename Tag>
    requires FamilyConceptConstructorTag<Family, Tag>
using FamilyConcept = Concept<Family, Tag>;

template<FamilyTag Family, typename Tag>
    requires FamilyRoleConstructorTag<Family, Tag>
using FamilyRole = Role<Family, Tag>;

template<FamilyTag Family, typename Tag>
    requires FamilyBooleanConstructorTag<Family, Tag>
using FamilyBoolean = Boolean<Family, Tag>;

template<FamilyTag Family, typename Tag>
    requires FamilyNumericalConstructorTag<Family, Tag>
using FamilyNumerical = Numerical<Family, Tag>;

template<FamilyTag Family, CategoryTag Category>
using FamilyConstructor = Constructor<Family, Category>;

}

#endif
