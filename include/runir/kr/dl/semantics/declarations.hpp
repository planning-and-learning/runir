#ifndef RUNIR_SEMANTICS_DECLARATIONS_HPP_
#define RUNIR_SEMANTICS_DECLARATIONS_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <concepts>
#include <cstddef>
#include <memory>
#include <ranges>
#include <span>
#include <tyr/formalism/declarations.hpp>
#include <tyr/formalism/planning/declarations.hpp>
#include <vector>
#include <yggdrasil/containers/span.hpp>
#include <yggdrasil/core/type_list.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/database/declarations.hpp>
#include <yggdrasil/database/syntax/row.hpp>

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

using DenotationTypes = ygg::TypeList<Denotation<BooleanTag>, Denotation<NumericalTag>, Denotation<ConceptTag>, Denotation<RoleTag>>;
using DenotationRecordTypes = ygg::ConcatTypeListsT<DenotationTypes, ygg::TypeList<RegisterValues, CallArguments>>;

class Builder;
class DenotationRepository;
class DenotationRepositoryFactory;

using RegisterValuesView = ygg::View<ygg::Index<RegisterValues>, DenotationRepository>;
using CallArgumentsView = ygg::View<ygg::Index<CallArguments>, DenotationRepository>;
using BorrowedRegisterValuesView = ygg::View<ygg::Data<RegisterValues>, tyr::formalism::planning::Repository>;

template<CategoryTag Category>
using DenotationView = ygg::View<ygg::Index<Denotation<Category>>, DenotationRepository>;

template<CategoryTag Category>
using BorrowedDenotationView = ygg::View<ygg::Builder<Denotation<Category>>, tyr::formalism::planning::Repository>;

using ConceptDenotationView = DenotationView<ConceptTag>;
using QueryDenotationView = ygg::database::RelationView<QueryValues>;
using DenotationRepositoryPtr = std::shared_ptr<DenotationRepository>;

}

#endif
