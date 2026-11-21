#ifndef RUNIR_SEMANTICS_DECLARATIONS_HPP_
#define RUNIR_SEMANTICS_DECLARATIONS_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <boost/mp11.hpp>
#include <concepts>
#include <cstddef>
#include <memory>
#include <ranges>
#include <runir/kr/database/declarations.hpp>
#include <span>
#include <tyr/formalism/declarations.hpp>
#include <urd/elements/concepts.hpp>
#include <urd/elements/flat.hpp>
#include <variant>
#include <vector>
#include <yggdrasil/formalism/relation_view.hpp>

namespace runir::kr::dl::semantics
{

struct RegisterValues
{
    static constexpr auto name = "RegisterValues";
};

struct CallArguments
{
    static constexpr auto name = "CallArguments";
};

/// The words of a concept's or role's denotation, a bitset: a record of their own, which the denotation refers to by
/// key (equal bitsets are interned once).
struct DenotationBlocks
{
    static constexpr auto name = "DenotationBlocks";
};

template<CategoryTag Category>
struct Denotation
{
    static constexpr std::string_view name = urd::joined_name<"Denotation", Category::name>;
};

using DenotationTypes = boost::mp11::mp_list<Denotation<BooleanTag>, Denotation<NumericalTag>, Denotation<ConceptTag>, Denotation<RoleTag>>;
using DenotationRecordTypes = boost::mp11::mp_append<DenotationTypes, boost::mp11::mp_list<RegisterValues, CallArguments, DenotationBlocks>>;

class Builder;
class DenotationRepository;
/// The repository owning what is viewed through a DenotationRepository: itself (denotation_repository.hpp). Declared
/// with the class, so that a view's constraints find it whenever they are first checked.
const DenotationRepository& get_repository(const DenotationRepository& repository) noexcept;
class EvaluationWorkspace;

class DenotationCaches;
class EvaluationStorage;

using RegisterValuesView = urd::index_view<RegisterValues, DenotationRepository>;
using CallArgumentsView = urd::index_view<CallArguments, DenotationRepository>;
/// Registers being built, borrowed: read through the denotation repository that interns them (each object through the
/// planning repository, as its child says).
using RegisterValuesBuilderView = urd::borrowed_view<RegisterValues, DenotationRepository, urd::builder<RegisterValues, DenotationRepository>>;
/// A register record before it is interned, borrowed: read through the denotation repository.
using RegisterValuesDataView = urd::borrowed_view<RegisterValues, DenotationRepository>;
/// Registers as a child holds them, read: the view of its form (interned, built, or a record not interned yet).
using RegisterValuesChild = std::variant<RegisterValuesView, RegisterValuesBuilderView, RegisterValuesDataView>;

template<CategoryTag Category>
using DenotationView = urd::index_view<Denotation<Category>, DenotationRepository>;

/// A denotation being computed: read through the denotation repository that interns it.
template<CategoryTag Category>
using DenotationBuilder = urd::builder<Denotation<Category>, DenotationRepository>;

template<CategoryTag Category>
using DenotationBuilderView = urd::borrowed_view<Denotation<Category>, DenotationRepository, DenotationBuilder<Category>>;

using ConceptDenotationView = DenotationView<ConceptTag>;
using QueryDenotationView = ygg::FreeRelationView;
using DenotationRepositoryPtr = std::shared_ptr<DenotationRepository>;

}

#endif
