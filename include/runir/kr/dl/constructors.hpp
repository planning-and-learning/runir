#ifndef RUNIR_CONSTRUCTORS_HPP_
#define RUNIR_CONSTRUCTORS_HPP_

#include "runir/kr/dl/declarations.hpp"

namespace runir::kr::dl
{

template<FamilyTag Family, ConceptConstructorTag Tag>
using FamilyConcept = Concept<Family, Tag>;

template<FamilyTag Family, RoleConstructorTag Tag>
using FamilyRole = Role<Family, Tag>;

template<FamilyTag Family, BooleanConstructorTag Tag>
using FamilyBoolean = Boolean<Family, Tag>;

template<FamilyTag Family, NumericalConstructorTag Tag>
using FamilyNumerical = Numerical<Family, Tag>;

template<FamilyTag Family, CategoryTag Category>
using FamilyConstructor = Constructor<Family, Category>;

template<ConceptConstructorTag Tag>
using BaseConcept = FamilyConcept<BaseFamilyTag, Tag>;

template<CategoryTag Category>
using BaseConstructor = FamilyConstructor<BaseFamilyTag, Category>;

template<ConceptConstructorTag Tag>
using ExtConcept = FamilyConcept<ExtFamilyTag, Tag>;

template<CategoryTag Category>
using ExtConstructor = FamilyConstructor<ExtFamilyTag, Category>;

}

#endif
