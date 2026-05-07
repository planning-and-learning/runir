#ifndef RUNIR_SEMANTICS_DECLARATIONS_HPP_
#define RUNIR_SEMANTICS_DECLARATIONS_HPP_

#include "runir/declarations.hpp"

#include <concepts>

namespace runir::semantics
{

template<CategoryTag Category>
struct Denotation;

template<typename Index, std::unsigned_integral Block>
struct IndexCoder;

}

#endif
