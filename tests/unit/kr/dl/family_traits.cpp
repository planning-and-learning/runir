#include <concepts>
#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
#include <runir/kr/dl/cnf_grammar/generate.hpp>
#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/execution_repository.hpp>
#include <runir/kr/ps/ext/execution_data.hpp>
#include <runir/kr/ps/ext/execution_view.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/icp/execution_data.hpp>
#include <runir/kr/ps/icp/execution_view.hpp>
#include <runir/kr/ps/icp/repository.hpp>
#include <runir/kr/uns/repository.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

namespace runir::tests::dl_family_traits
{
namespace
{

consteval bool distinct_repository_contexts()
{
    return !std::same_as<kr::dl::ConstructorRepository, kr::dl::grammar::ConstructorRepository>
           && !std::same_as<kr::dl::ConstructorRepository, kr::dl::cnf_grammar::ConstructorRepository>
           && !std::same_as<kr::dl::grammar::ConstructorRepository, kr::dl::cnf_grammar::ConstructorRepository>;
}

static_assert(distinct_repository_contexts());

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
    requires(kr::dl::cnf_grammar::GrammarView grammar,
             const urd::Store<urd::sequence<tyr::planning::PackedStateView<Kind>>>& states,
             kr::dl::ConstructorRepository& constructors,
             kr::dl::semantics::DenotationRepository& denotations,
             const kr::dl::cnf_grammar::GenerateOptions& options) { kr::dl::cnf_grammar::generate<Kind>(grammar, states, constructors, denotations, options); };

static_assert(GeneratesConstructors<tyr::GroundTag>);
static_assert(GeneratesConstructors<tyr::LiftedTag>);

static_assert(kr::dl::NumericalConstructorTag<kr::dl::AddTag>);
static_assert(kr::dl::ConceptConstructorTag<kr::dl::RegisterTag>);

consteval bool can_checkout_constructors()
{
    return requires(kr::dl::Builder& builder, kr::dl::grammar::Builder& grammar_builder, kr::dl::cnf_grammar::Builder& cnf_builder) {
        kr::dl::checkout<kr::dl::Concept<kr::dl::SemanticFamily, kr::dl::TopTag>>(builder);
        kr::dl::grammar::checkout<kr::dl::Concept<kr::dl::grammar::GrammarFamily, kr::dl::TopTag>>(grammar_builder);
        kr::dl::cnf_grammar::checkout<kr::dl::Concept<kr::dl::cnf_grammar::CnfGrammarFamily, kr::dl::TopTag>>(cnf_builder);
    };
}

static_assert(can_checkout_constructors());

template<typename B>
concept SemanticCheckout = requires(B& builder) { kr::dl::checkout<kr::dl::Concept<kr::dl::SemanticFamily, kr::dl::TopTag>>(builder); };

static_assert(!SemanticCheckout<kr::dl::grammar::Builder>);
static_assert(!SemanticCheckout<kr::dl::cnf_grammar::Builder>);

}  // namespace
}  // namespace runir::tests::dl_family_traits
