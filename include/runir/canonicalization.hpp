#ifndef RUNIR_CANONICALIZATION_HPP_
#define RUNIR_CANONICALIZATION_HPP_

#include "runir/semantics/datas.hpp"

#include <tyr/common/types.hpp>

namespace runir
{

template<ConceptConstructorTag Tag>
bool is_canonical(const tyr::Data<Concept<Tag>>&)
{
    return true;
}

template<RoleConstructorTag Tag>
bool is_canonical(const tyr::Data<Role<Tag>>&)
{
    return true;
}

template<BooleanConstructorTag Tag>
bool is_canonical(const tyr::Data<Boolean<Tag>>&)
{
    return true;
}

template<NumericalConstructorTag Tag>
bool is_canonical(const tyr::Data<Numerical<Tag>>&)
{
    return true;
}

template<CategoryTag Category>
bool is_canonical(const tyr::Data<Constructor<Category>>&)
{
    return true;
}

template<ConceptConstructorTag Tag>
void canonicalize(tyr::Data<Concept<Tag>>&)
{
}

template<RoleConstructorTag Tag>
void canonicalize(tyr::Data<Role<Tag>>&)
{
}

template<BooleanConstructorTag Tag>
void canonicalize(tyr::Data<Boolean<Tag>>&)
{
}

template<NumericalConstructorTag Tag>
void canonicalize(tyr::Data<Numerical<Tag>>&)
{
}

template<CategoryTag Category>
void canonicalize(tyr::Data<Constructor<Category>>&)
{
}

}

#endif
