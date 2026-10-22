#include <concepts>
#include <runir/kr/detail/interning_contracts.hpp>
#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
#include <runir/kr/dl/cnf_grammar/generate.hpp>
#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/ext/execution_repository.hpp>
#include <runir/kr/ps/ext/execution_view.hpp>
#include <runir/kr/ps/icp/execution_repository.hpp>
#include <runir/kr/ps/icp/execution_view.hpp>
#include <runir/kr/uns/repository.hpp>

namespace runir::tests
{
namespace
{


consteval bool distinct_repository_contexts()
{
    return !std::same_as<kr::dl::ConstructorRepositoryFor, kr::dl::grammar::ConstructorRepositoryFor>
           && !std::same_as<kr::dl::ConstructorRepositoryFor, kr::dl::cnf_grammar::ConstructorRepositoryFor>
           && !std::same_as<kr::dl::grammar::ConstructorRepositoryFor, kr::dl::cnf_grammar::ConstructorRepositoryFor>;
}

static_assert(distinct_repository_contexts());

// The interning contracts of the semantic, grammar and CNF constructor repositories are checked in the library
// sources that instantiate them (src/kr/dl/{,grammar/,cnf_grammar/}repository_*.cpp).
using kr::detail::InternsData;
using kr::detail::rejects_symbol;
static_assert(InternsData<kr::dl::semantics::DenotationRepository, kr::dl::semantics::RegisterValues>);
static_assert(rejects_symbol<kr::dl::semantics::DenotationRepository, kr::dl::QueryColumn>());
static_assert(InternsData<kr::ps::base::Repository, kr::ps::base::Sketch>);
static_assert(InternsData<kr::ps::ext::Repository, kr::ps::ext::Module>);
static_assert(InternsData<kr::ps::icp::Repository, kr::ps::icp::Module>);
static_assert(InternsData<kr::uns::Repository, kr::uns::Classifier>);
static_assert(rejects_symbol<kr::ps::base::Repository, kr::ps::ext::Module>());
static_assert(rejects_symbol<kr::ps::ext::Repository, kr::ps::icp::Module>());
static_assert(rejects_symbol<kr::ps::icp::Repository, kr::ps::ext::Module>());
static_assert(rejects_symbol<kr::uns::Repository, kr::ps::base::Sketch>());

template<tyr::TaskKind Kind>
consteval bool execution_interning_contracts()
{
    using ExtRepository = kr::ps::ext::ExecutionRepository<Kind>;
    using IcpRepository = kr::ps::icp::ExecutionRepository<Kind>;
    return InternsData<ExtRepository, kr::ps::ext::ProgramState<Kind>> && InternsData<IcpRepository, kr::ps::icp::Histories>
           && rejects_symbol<ExtRepository, kr::ps::ext::Module>() && rejects_symbol<IcpRepository, kr::ps::icp::Module>();
}

static_assert(execution_interning_contracts<tyr::GroundTag>());
static_assert(execution_interning_contracts<tyr::LiftedTag>());

template<typename Category>
concept RegisterCategory = requires {
    typename kr::dl::Register<Category>;
    typename kr::dl::RegisterIdentifier<Category>;
    typename kr::dl::RegisterView<Category>;
};

template<typename Category>
concept QueryProjectionCategory = requires {
    typename kr::dl::QueryProjection<Category>;
    typename kr::dl::QueryProjection<Category>;
    typename kr::dl::QueryProjection<Category>;
};

static_assert(RegisterCategory<kr::dl::ConceptTag> && RegisterCategory<kr::dl::RoleTag>);
static_assert(!RegisterCategory<kr::dl::BooleanTag> && !RegisterCategory<kr::dl::NumericalTag>);
static_assert(QueryProjectionCategory<kr::dl::ConceptTag> && QueryProjectionCategory<kr::dl::RoleTag>);
static_assert(!QueryProjectionCategory<kr::dl::BooleanTag> && !QueryProjectionCategory<kr::dl::NumericalTag>);

template<typename Kind>
concept GeneratesConstructors =
    requires(kr::dl::cnf_grammar::FamilyGrammarView grammar,
             const std::vector<tyr::planning::PackedStateView<Kind>>& states,
             kr::dl::ConstructorRepositoryFor& constructors,
             kr::dl::semantics::DenotationRepository& denotations,
             const kr::dl::cnf_grammar::GenerateOptions& options) { kr::dl::cnf_grammar::generate<Kind>(grammar, states, constructors, denotations, options); };

static_assert(GeneratesConstructors<tyr::GroundTag>);
static_assert(GeneratesConstructors<tyr::LiftedTag>);

static_assert(kr::dl::FamilyNumericalConstructorTag<kr::dl::AddTag>);
static_assert(kr::dl::FamilyConceptConstructorTag<kr::dl::RegisterTag>);

consteval bool can_checkout_constructors()
{
    return requires(kr::dl::Builder& builder, kr::dl::grammar::Builder& grammar_builder, kr::dl::cnf_grammar::Builder& cnf_builder) {
        kr::dl::checkout<kr::dl::Concept<kr::dl::TopTag>>(builder);
        kr::dl::grammar::checkout<kr::dl::grammar::Concept<kr::dl::TopTag>>(grammar_builder);
        kr::dl::cnf_grammar::checkout<kr::dl::cnf_grammar::Concept<kr::dl::TopTag>>(cnf_builder);
    };
}

static_assert(can_checkout_constructors());

template<typename B>
concept SemanticCheckout = requires(B& builder) { kr::dl::checkout<kr::dl::Concept<kr::dl::TopTag>>(builder); };

static_assert(!SemanticCheckout<kr::dl::grammar::BaseBuilder>);
static_assert(!SemanticCheckout<kr::dl::cnf_grammar::BaseBuilder>);

}  // namespace
}  // namespace runir::tests
