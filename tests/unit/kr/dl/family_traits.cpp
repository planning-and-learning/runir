#include <concepts>
#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
#include <runir/kr/dl/cnf_grammar/generate.hpp>
#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <runir/kr/dl/repository.hpp>

namespace runir::tests
{
namespace
{

static_assert(kr::dl::FamilyTag<kr::BaseFamilyTag>);
static_assert(kr::dl::FamilyTag<kr::ExtFamilyTag>);
static_assert(kr::dl::FamilyTag<kr::UnsFamilyTag>);
static_assert(!kr::dl::FamilyTag<kr::IcpFamilyTag>);

template<typename Category>
concept RegisterCategory = requires {
    typename kr::dl::Register<Category>;
    typename kr::dl::RegisterIdentifier<Category>;
    typename kr::dl::RegisterView<Category>;
};

template<typename Category>
concept QueryProjectionCategory = requires {
    typename kr::dl::QueryProjection<kr::BaseFamilyTag, Category>;
    typename kr::dl::QueryProjection<kr::ExtFamilyTag, Category>;
    typename kr::dl::QueryProjection<kr::UnsFamilyTag, Category>;
};

static_assert(RegisterCategory<kr::dl::ConceptTag> && RegisterCategory<kr::dl::RoleTag>);
static_assert(!RegisterCategory<kr::dl::BooleanTag> && !RegisterCategory<kr::dl::NumericalTag>);
static_assert(QueryProjectionCategory<kr::dl::ConceptTag> && QueryProjectionCategory<kr::dl::RoleTag>);
static_assert(!QueryProjectionCategory<kr::dl::BooleanTag> && !QueryProjectionCategory<kr::dl::NumericalTag>);

template<typename Family, typename Kind>
concept GeneratesConstructors = requires(kr::dl::cnf_grammar::FamilyGrammarView<Family> grammar,
                                        const std::vector<tyr::planning::PackedStateView<Kind>>& states,
                                        kr::dl::ConstructorRepositoryFor<Family>& constructors,
                                        kr::dl::semantics::DenotationRepository& denotations,
                                        const kr::dl::cnf_grammar::GenerateOptions& options) {
    kr::dl::cnf_grammar::generate<Family, Kind>(grammar, states, constructors, denotations, options);
};

static_assert(GeneratesConstructors<kr::BaseFamilyTag, tyr::GroundTag>);
static_assert(GeneratesConstructors<kr::BaseFamilyTag, tyr::LiftedTag>);
static_assert(!GeneratesConstructors<kr::ExtFamilyTag, tyr::GroundTag>);
static_assert(!GeneratesConstructors<kr::ExtFamilyTag, tyr::LiftedTag>);
static_assert(!GeneratesConstructors<kr::UnsFamilyTag, tyr::GroundTag>);
static_assert(!GeneratesConstructors<kr::UnsFamilyTag, tyr::LiftedTag>);

static_assert(kr::dl::FamilyNumericalConstructorTag<kr::ExtFamilyTag, kr::dl::AddTag>);
static_assert(kr::dl::FamilyNumericalConstructorTag<kr::UnsFamilyTag, kr::dl::AddTag>);
static_assert(!kr::dl::FamilyNumericalConstructorTag<kr::BaseFamilyTag, kr::dl::AddTag>);
static_assert(kr::dl::FamilyConceptConstructorTag<kr::ExtFamilyTag, kr::dl::RegisterTag>);
static_assert(!kr::dl::FamilyConceptConstructorTag<kr::BaseFamilyTag, kr::dl::RegisterTag>);
static_assert(!kr::dl::FamilyConceptConstructorTag<kr::UnsFamilyTag, kr::dl::RegisterTag>);

template<kr::dl::FamilyTag Family>
consteval bool can_checkout_constructors()
{
    return requires(kr::dl::Builder<Family>& builder,
                    kr::dl::grammar::Builder<Family>& grammar_builder,
                    kr::dl::cnf_grammar::Builder<Family>& cnf_builder) {
        kr::dl::checkout<kr::dl::Concept<Family, kr::dl::TopTag>>(builder);
        kr::dl::grammar::checkout<kr::dl::grammar::Concept<Family, kr::dl::TopTag>>(grammar_builder);
        kr::dl::cnf_grammar::checkout<kr::dl::cnf_grammar::Concept<Family, kr::dl::TopTag>>(cnf_builder);
    };
}

static_assert(can_checkout_constructors<kr::BaseFamilyTag>());
static_assert(can_checkout_constructors<kr::ExtFamilyTag>());
static_assert(can_checkout_constructors<kr::UnsFamilyTag>());

template<typename B>
concept SemanticCheckout = requires(B& builder) { kr::dl::checkout<kr::dl::Concept<kr::BaseFamilyTag, kr::dl::TopTag>>(builder); };

static_assert(!SemanticCheckout<kr::dl::grammar::BaseBuilder>);
static_assert(!SemanticCheckout<kr::dl::cnf_grammar::BaseBuilder>);

}  // namespace
}  // namespace runir::tests
