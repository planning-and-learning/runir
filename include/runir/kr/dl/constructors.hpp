#ifndef RUNIR_CONSTRUCTORS_HPP_
#define RUNIR_CONSTRUCTORS_HPP_

#include "runir/kr/dl/declarations.hpp"

namespace runir::kr::dl
{

template<ConceptConstructorTag Tag>
struct Concept;

template<RoleConstructorTag Tag>
struct Role;

template<BooleanConstructorTag Tag>
struct Boolean;

template<NumericalConstructorTag Tag>
struct Numerical;

template<CategoryTag Category>
struct Constructor;

}

#endif
