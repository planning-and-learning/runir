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
#include <boost/mp11.hpp>
#include <urd/elements/flat.hpp>
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

using DenotationTypes = boost::mp11::mp_list<Denotation<BooleanTag>, Denotation<NumericalTag>, Denotation<ConceptTag>, Denotation<RoleTag>>;
using DenotationRecordTypes = boost::mp11::mp_append<DenotationTypes, boost::mp11::mp_list<RegisterValues, CallArguments>>;

class Builder;
class DenotationRepository;
/// The repository owning what is viewed through a DenotationRepository: itself (denotation_repository.hpp). Declared
/// with the class, so that a view's constraints find it whenever they are first checked.
const DenotationRepository& get_repository(const DenotationRepository& repository) noexcept;
class DenotationRepositoryFactory;
class EvaluationWorkspace;

class DenotationCaches;
class EvaluationStorage;

using RegisterValuesView = urd::view<RegisterValues, DenotationRepository>;
using CallArgumentsView = urd::view<CallArguments, DenotationRepository>;
using BorrowedRegisterValuesView = urd::borrowed_view<RegisterValues, tyr::formalism::planning::Repository>;

template<CategoryTag Category>
using DenotationView = urd::view<Denotation<Category>, DenotationRepository>;

template<CategoryTag Category>
using BorrowedDenotationView = urd::borrowed_view<Denotation<Category>, tyr::formalism::planning::Repository, urd::builder<Denotation<Category>>>;

using ConceptDenotationView = DenotationView<ConceptTag>;
using QueryDenotationView = ygg::database::RelationView;
using DenotationRepositoryPtr = std::shared_ptr<DenotationRepository>;

}

#endif
