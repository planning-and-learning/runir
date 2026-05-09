#ifndef RUNIR_SEMANTICS_DECLARATIONS_HPP_
#define RUNIR_SEMANTICS_DECLARATIONS_HPP_

#include "runir/knowledge_representation/dl/declarations.hpp"

#include <concepts>

namespace runir::kr::dl::semantics
{

template<CategoryTag Category>
struct Denotation;

template<typename Index, std::unsigned_integral Block>
struct IndexCoder;

}

#endif
