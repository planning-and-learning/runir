#include <array>
#include <concepts>
#include <gtest/gtest.h>
#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <runir/kr/dl/repository.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <urd/elements/flat.hpp>
#include <boost/mp11.hpp>
#include <yggdrasil/core/config.hpp>

namespace runir::tests
{

namespace
{

// The constructor records of the three families: the semantic constructors, grammars and CNF grammars.

using SemanticFamily = kr::dl::SemanticFamily;
using GrammarFamily = kr::dl::grammar::GrammarFamily;
using CnfGrammarFamily = kr::dl::cnf_grammar::CnfGrammarFamily;

template<typename T, typename Repository>
concept IndexedDataView =
    std::constructible_from<urd::index<T>, ygg::uint_t> && urd::Identifiable<urd::index<T>> && urd::Identifiable<urd::data<T>>
    && urd::Identifiable<urd::index_view<T, Repository>> && std::totally_ordered<urd::index<T>> && std::totally_ordered<urd::data<T>>
    && std::totally_ordered<urd::index_view<T, Repository>> && requires(urd::data<T>& data, const urd::index_view<T, Repository>& view) {
           data.index;
           view.index();
       };

/// Which members a record holds, in the order of record_view.hpp.
template<typename Data>
constexpr auto members = std::array {
    requires(Data& data) { data.predicate; }, requires(Data& data) { data.polarity; }, requires(Data& data) { data.object; },
    requires(Data& data) { data.objects; },   requires(Data& data) { data.n; },        requires(Data& data) { data.role; },
    requires(Data& data) { data.concept_; },  requires(Data& data) { data.arg; },      requires(Data& data) { data.lhs; },
    requires(Data& data) { data.mid; },       requires(Data& data) { data.rhs; },      requires(Data& data) { data.reference; },
    requires(Data& data) { data.identifier; },
};

/// Which accessors a view has, in the order of members.
template<typename View>
constexpr auto accessors = std::array {
    requires(const View& view) { view.get_predicate(); },
    requires(const View& view) { view.get_polarity(); },
    requires(const View& view) { view.get_object(); },
    requires(const View& view) { view.get_objects(); },
    requires(const View& view) { view.get_n(); },
    requires(const View& view) { view.get_role(); },
    requires(const View& view) { view.get_concept(); },
    requires(const View& view) { view.get_arg(); },
    requires(const View& view) { view.get_lhs(); },
    requires(const View& view) { view.get_mid(); },
    requires(const View& view) { view.get_rhs(); },
    requires(const View& view) { view.get_register(); } || requires(const View& view) { view.get_argument(); },
    requires(const View& view) { view.get_value(); },
};

/// A record and its view through Repository: indexed, and the view has an accessor for each member, and no other.
template<typename Repository, typename... Ts>
consteval bool records_and_views(boost::mp11::mp_list<Ts...>)
{
    return ((IndexedDataView<Ts, Repository> && members<urd::data<Ts>> == accessors<urd::index_view<Ts, Repository>>
             && !requires(const urd::index_view<Ts, Repository>& view) { view.get_columns(); }
             && !requires(const urd::index_view<Ts, Repository>& view) { view.get_plan(); })
            && ...);
}

template<typename F, typename Repository>
consteval bool family_records_and_views()
{
    return records_and_views<Repository>(kr::dl::RecordTypes<F, kr::dl::ConceptTag> {}) && records_and_views<Repository>(kr::dl::RecordTypes<F, kr::dl::RoleTag> {})
           && records_and_views<Repository>(kr::dl::RecordTypes<F, kr::dl::BooleanTag> {})
           && records_and_views<Repository>(kr::dl::RecordTypes<F, kr::dl::NumericalTag> {});
}

static_assert(family_records_and_views<SemanticFamily, kr::dl::ConstructorRepository>());
static_assert(family_records_and_views<GrammarFamily, kr::dl::grammar::ConstructorRepository>());
static_assert(family_records_and_views<CnfGrammarFamily, kr::dl::cnf_grammar::ConstructorRepository>());

/// The records of the base constructors have the same members in every family; only their operand types differ.
template<template<typename, typename> typename Record, typename... Tags>
consteval bool same_shapes(boost::mp11::mp_list<Tags...>)
{
    return ((members<urd::data<Record<SemanticFamily, Tags>>> == members<urd::data<Record<GrammarFamily, Tags>>>
             && members<urd::data<Record<GrammarFamily, Tags>>> == members<urd::data<Record<CnfGrammarFamily, Tags>>>)
            && ...);
}

static_assert(same_shapes<kr::dl::Concept>(kr::dl::BaseConceptConstructorTags {}));
static_assert(same_shapes<kr::dl::Role>(kr::dl::BaseRoleConstructorTags {}));
static_assert(same_shapes<kr::dl::Boolean>(kr::dl::BaseBooleanConstructorTags {}));
static_assert(same_shapes<kr::dl::Numerical>(kr::dl::BaseNumericalConstructorTags {}));

// Registers, arguments, comparisons, constants and arithmetic are semantic constructors only.
static_assert(kr::dl::ConstructorOf<kr::dl::RegisterTag, SemanticFamily, kr::dl::ConceptTag>);
static_assert(!kr::dl::ConstructorOf<kr::dl::RegisterTag, GrammarFamily, kr::dl::ConceptTag>);
static_assert(!kr::dl::ConstructorOf<kr::dl::ArgumentTag<kr::dl::RoleTag>, CnfGrammarFamily, kr::dl::RoleTag>);
static_assert(!kr::dl::ConstructorOf<kr::dl::AddTag, GrammarFamily, kr::dl::NumericalTag>);

// Each tree names its views of the records.
static_assert(std::same_as<kr::dl::ConceptView<kr::dl::TopTag>, urd::index_view<kr::dl::Concept<SemanticFamily, kr::dl::TopTag>, kr::dl::ConstructorRepository>>);
static_assert(std::same_as<kr::dl::grammar::RoleView<kr::dl::InverseTag>,
                           urd::index_view<kr::dl::Role<GrammarFamily, kr::dl::InverseTag>, kr::dl::grammar::ConstructorRepository>>);
static_assert(std::same_as<kr::dl::cnf_grammar::NumericalView<kr::dl::CountTag>,
                           urd::index_view<kr::dl::Numerical<CnfGrammarFamily, kr::dl::CountTag>, kr::dl::cnf_grammar::ConstructorRepository>>);

}  // namespace

TEST(RunirKrDlConcept, RoutesUnaryBinaryAndNumberRestrictionData)
{
    using Concept = kr::dl::Constructor<kr::dl::ConceptTag>;
    using Role = kr::dl::Constructor<kr::dl::RoleTag>;
    auto planning_repository = std::make_shared<tyr::formalism::planning::Repository>();
    auto repository = kr::dl::ConstructorRepositoryFactory().create(planning_repository);

    auto unary_data = urd::data<kr::dl::Concept<SemanticFamily, kr::dl::NegationTag>>(urd::index<Concept>(3));
    const auto unary = repository->insert(unary_data).first;
    EXPECT_EQ(unary.get_arg().index(), urd::index<Concept>(3));

    auto binary_data = urd::data<kr::dl::Concept<SemanticFamily, kr::dl::ValueRestrictionTag>>(urd::index<Role>(1), urd::index<Concept>(2));
    const auto binary = repository->insert(binary_data).first;
    EXPECT_EQ(binary.get_lhs().index(), urd::index<Role>(1));
    EXPECT_EQ(binary.get_rhs().index(), urd::index<Concept>(2));

    auto restriction_data = urd::data<kr::dl::Concept<SemanticFamily, kr::dl::AtLeastNumberRestrictionTag>>(5, urd::index<Role>(4));
    const auto restriction = repository->insert(restriction_data).first;
    EXPECT_EQ(restriction.get_n(), 5);
    EXPECT_EQ(restriction.get_role().index(), urd::index<Role>(4));
}

TEST(RunirKrDlRole, RoutesUnaryAndMixedBinaryData)
{
    using Concept = kr::dl::Constructor<kr::dl::ConceptTag>;
    using Role = kr::dl::Constructor<kr::dl::RoleTag>;
    auto planning_repository = std::make_shared<tyr::formalism::planning::Repository>();
    auto repository = kr::dl::ConstructorRepositoryFactory().create(planning_repository);

    auto unary_data = urd::data<kr::dl::Role<SemanticFamily, kr::dl::InverseTag>>(urd::index<Role>(3));
    const auto unary = repository->insert(unary_data).first;
    EXPECT_EQ(unary.get_arg().index(), urd::index<Role>(3));

    auto binary_data = urd::data<kr::dl::Role<SemanticFamily, kr::dl::RestrictionTag>>(urd::index<Role>(1), urd::index<Concept>(2));
    const auto binary = repository->insert(binary_data).first;
    EXPECT_EQ(binary.get_lhs().index(), urd::index<Role>(1));
    EXPECT_EQ(binary.get_rhs().index(), urd::index<Concept>(2));
}

TEST(RunirKrDlBoolean, RoutesConstantUnaryAndBinaryData)
{
    using Constructor = kr::dl::Constructor<kr::dl::BooleanTag>;
    auto planning_repository = std::make_shared<tyr::formalism::planning::Repository>();
    auto repository = kr::dl::ConstructorRepositoryFactory().create(planning_repository);

    auto constant_data = urd::data<kr::dl::Boolean<SemanticFamily, kr::dl::BooleanConstantTag>>(true);
    const auto constant = repository->insert(constant_data).first;
    EXPECT_TRUE(constant.get_value());

    auto unary_data = urd::data<kr::dl::Boolean<SemanticFamily, kr::dl::NotTag>>(urd::index<Constructor>(4));
    const auto unary = repository->insert(unary_data).first;
    EXPECT_EQ(unary.get_arg().index(), urd::index<Constructor>(4));

    auto binary_data = urd::data<kr::dl::Boolean<SemanticFamily, kr::dl::AndTag>>(urd::index<Constructor>(1), urd::index<Constructor>(2));
    const auto binary = repository->insert(binary_data).first;
    EXPECT_EQ(binary.get_lhs().index(), urd::index<Constructor>(1));
    EXPECT_EQ(binary.get_rhs().index(), urd::index<Constructor>(2));
}

TEST(RunirKrDlNumerical, RoutesValueBinaryAndDistanceData)
{
    using Concept = kr::dl::Constructor<kr::dl::ConceptTag>;
    using Role = kr::dl::Constructor<kr::dl::RoleTag>;
    using Numerical = kr::dl::Constructor<kr::dl::NumericalTag>;
    auto planning_repository = std::make_shared<tyr::formalism::planning::Repository>();
    auto repository = kr::dl::ConstructorRepositoryFactory().create(planning_repository);

    auto constant_data = urd::data<kr::dl::Numerical<SemanticFamily, kr::dl::NumericalConstantTag>>(7);
    EXPECT_EQ(repository->insert(constant_data).first.get_value(), 7);

    auto binary_data = urd::data<kr::dl::Numerical<SemanticFamily, kr::dl::AddTag>>(urd::index<Numerical>(1), urd::index<Numerical>(2));
    const auto binary = repository->insert(binary_data).first;
    EXPECT_EQ(binary.get_lhs().index(), urd::index<Numerical>(1));
    EXPECT_EQ(binary.get_rhs().index(), urd::index<Numerical>(2));

    // Vertices are concept-or-query and edges role-or-query variants.
    using Query = kr::dl::Query<>;
    using DistanceData = urd::data<kr::dl::Numerical<SemanticFamily, kr::dl::DistanceTag>>;
    static_assert(std::same_as<DistanceData::Vertex, urd::index_variant<boost::mp11::mp_list<Concept, Query>>>);
    static_assert(std::same_as<DistanceData::Edge, urd::index_variant<boost::mp11::mp_list<Role, Query>>>);
    auto distance_data = DistanceData(urd::index<Concept>(3), urd::index<Query>(4), urd::index<Query>(5));
    const auto distance = repository->insert(distance_data).first;
    EXPECT_EQ((std::get<urd::index_view<Concept, runir::kr::dl::ConstructorRepository>>(distance.get_lhs()).index()), urd::index<Concept>(3));
    EXPECT_EQ((std::get<urd::index_view<Query, runir::kr::dl::ConstructorRepository>>(distance.get_mid()).index()), urd::index<Query>(4));
    EXPECT_EQ((std::get<urd::index_view<Query, runir::kr::dl::ConstructorRepository>>(distance.get_rhs()).index()), urd::index<Query>(5));
    EXPECT_TRUE((std::holds_alternative<urd::index_view<Query, runir::kr::dl::ConstructorRepository>>(distance.get_mid())));
}

/// A grammar tree: its family, non-terminals and repository.
template<typename F, template<typename> typename NonTerminalTemplate, typename Factory>
struct GrammarTree
{
    using Family = F;
    template<typename Category>
    using NonTerminal = NonTerminalTemplate<Category>;
    using RepositoryFactory = Factory;
};

template<typename Tree>
class RunirKrDlGrammarNumerical : public testing::Test
{
};

using GrammarTrees = testing::Types<GrammarTree<GrammarFamily, kr::dl::grammar::NonTerminal, kr::dl::grammar::ConstructorRepositoryFactory>,
                                    GrammarTree<CnfGrammarFamily, kr::dl::cnf_grammar::NonTerminal, kr::dl::cnf_grammar::ConstructorRepositoryFactory>>;
TYPED_TEST_SUITE(RunirKrDlGrammarNumerical, GrammarTrees);

TYPED_TEST(RunirKrDlGrammarNumerical, ExposesDistanceOperands)
{
    using F = typename TypeParam::Family;
    using ConceptOperand = kr::dl::operand_t<F, kr::dl::ConceptTag>;
    using RoleOperand = kr::dl::operand_t<F, kr::dl::RoleTag>;
    auto planning_repository = std::make_shared<tyr::formalism::planning::Repository>();
    auto repository = typename TypeParam::RepositoryFactory().create(planning_repository);

    auto source_data = urd::data<typename TypeParam::template NonTerminal<kr::dl::ConceptTag>>(std::string("source"));
    auto role_data = urd::data<typename TypeParam::template NonTerminal<kr::dl::RoleTag>>(std::string("role"));
    auto target_data = urd::data<typename TypeParam::template NonTerminal<kr::dl::ConceptTag>>(std::string("target"));
    const auto source = repository->insert(source_data).first;
    const auto role = repository->insert(role_data).first;
    const auto target = repository->insert(target_data).first;
    auto distance_data =
        urd::data<kr::dl::Numerical<F, kr::dl::DistanceTag>>(ConceptOperand(source.index()), RoleOperand(role.index()), ConceptOperand(target.index()));
    const auto distance = repository->insert(distance_data).first;

    // A grammar operand resolves to its non-terminal's view, or (grammars) to the variant holding it.
    EXPECT_EQ(distance.get_lhs(), urd::resolve(ConceptOperand(source.index()), *repository));
    EXPECT_EQ(distance.get_mid(), urd::resolve(RoleOperand(role.index()), *repository));
    EXPECT_EQ(distance.get_rhs(), urd::resolve(ConceptOperand(target.index()), *repository));
}

}  // namespace runir::tests
