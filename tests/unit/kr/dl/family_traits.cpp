#include <concepts>
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

static_assert(kr::dl::FamilyTag<kr::BaseFamilyTag>);
static_assert(kr::dl::FamilyTag<kr::ExtFamilyTag>);
static_assert(kr::dl::FamilyTag<kr::UnsFamilyTag>);
static_assert(!kr::dl::FamilyTag<kr::IcpFamilyTag>);

template<kr::dl::FamilyTag Family>
consteval bool distinct_repository_contexts()
{
    return !std::same_as<kr::dl::ConstructorRepositoryFor<Family>, kr::dl::grammar::ConstructorRepositoryFor<Family>>
           && !std::same_as<kr::dl::ConstructorRepositoryFor<Family>, kr::dl::cnf_grammar::ConstructorRepositoryFor<Family>>
           && !std::same_as<kr::dl::grammar::ConstructorRepositoryFor<Family>, kr::dl::cnf_grammar::ConstructorRepositoryFor<Family>>;
}

static_assert(distinct_repository_contexts<kr::BaseFamilyTag>());
static_assert(distinct_repository_contexts<kr::ExtFamilyTag>());
static_assert(distinct_repository_contexts<kr::UnsFamilyTag>());

template<typename Repository, typename T>
concept InternsData = requires(Repository& repository, ygg::Data<T>& data) {
    { ygg::formalism::insert(repository, data) } -> std::same_as<std::pair<ygg::View<ygg::Index<T>, Repository>, bool>>;
};

template<typename Repository, typename T>
consteval bool rejects_symbol()
{
    return !ygg::formalism::SupportsSymbol<Repository, T> && !InternsData<Repository, T>
           && !requires(Repository& repository, ygg::Data<T>& data) { repository.insert(data); }
           && !requires(const Repository& repository, const ygg::Data<T>& data) { repository.find(data); }
           && !requires(const Repository& repository, ygg::Index<T> index) { repository[index]; }
           && !requires(const Repository& repository) { repository.template size<T>(); };
}

template<kr::dl::FamilyTag Family>
consteval bool constructor_interning_contracts()
{
    using Semantic = kr::dl::Concept<Family, kr::dl::TopTag>;
    using Grammar = kr::dl::grammar::Concept<Family, kr::dl::TopTag>;
    using Cnf = kr::dl::cnf_grammar::Concept<Family, kr::dl::TopTag>;
    using SemanticRepository = kr::dl::ConstructorRepositoryFor<Family>;
    using GrammarRepository = kr::dl::grammar::ConstructorRepositoryFor<Family>;
    using CnfRepository = kr::dl::cnf_grammar::ConstructorRepositoryFor<Family>;
    return InternsData<SemanticRepository, Semantic> && InternsData<GrammarRepository, Grammar> && InternsData<CnfRepository, Cnf>
           && rejects_symbol<SemanticRepository, Grammar>() && rejects_symbol<GrammarRepository, Cnf>() && rejects_symbol<CnfRepository, Semantic>();
}

static_assert(constructor_interning_contracts<kr::BaseFamilyTag>());
static_assert(constructor_interning_contracts<kr::ExtFamilyTag>());
static_assert(constructor_interning_contracts<kr::UnsFamilyTag>());
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
    return requires(kr::dl::Builder<Family>& builder, kr::dl::grammar::Builder<Family>& grammar_builder, kr::dl::cnf_grammar::Builder<Family>& cnf_builder) {
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
