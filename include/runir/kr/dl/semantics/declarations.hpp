#ifndef RUNIR_SEMANTICS_DECLARATIONS_HPP_
#define RUNIR_SEMANTICS_DECLARATIONS_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <concepts>
#include <memory>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::dl::semantics
{

struct RegisterValues
{
};

struct CallArguments
{
};

template<CategoryTag Category>
struct Denotation;

template<typename Index, std::unsigned_integral Block>
struct IndexCoder;

class DenotationRepository;
class DenotationRepositoryFactory;

using RegisterValuesView = ygg::View<ygg::Index<RegisterValues>, DenotationRepository>;
using CallArgumentsView = ygg::View<ygg::Index<CallArguments>, DenotationRepository>;

template<CategoryTag Category>
using DenotationView = ygg::View<ygg::Index<Denotation<Category>>, DenotationRepository>;

using ConceptDenotationView = DenotationView<ConceptTag>;
using DenotationRepositoryPtr = std::shared_ptr<DenotationRepository>;

}

#endif
