#include "fixtures.hpp"

#include <concepts>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/errors.hpp>
#include <runir/kr/uns/classifier.hpp>
#include <runir/kr/uns/dl/parser.hpp>
#include <runir/kr/uns/formatter.hpp>
#include <runir/kr/uns/repository.hpp>
#include <runir/kr/uns/syntactic_complexity.hpp>
#include <string>
#include <tyr/formalism/planning/parser.hpp>

namespace runir::tests
{
namespace
{

template<typename Entity>
using View = ygg::View<ygg::Index<Entity>, kr::uns::Repository>;

template<typename Entity>
concept IndexedDataView = std::constructible_from<ygg::Index<Entity>, ygg::uint_t> && std::totally_ordered<ygg::Index<Entity>>
                          && std::totally_ordered<ygg::Data<Entity>> && std::totally_ordered<View<Entity>>;

using Literal = kr::uns::ClassifierLiteral;
using Clause = kr::uns::ClassifierClause;
using Classifier = kr::uns::Classifier;

static_assert(IndexedDataView<Literal> && std::same_as<View<Literal>, kr::uns::ClassifierLiteralView>
              && requires(ygg::Data<Literal>& data, const View<Literal>& view) {
                     data.index;
                     data.value;
                     data.polarity;
                     data.clear();
                     view.get_index();
                     view.get_feature();
                     view.get_polarity();
                 });
static_assert(IndexedDataView<Clause> && std::same_as<View<Clause>, kr::uns::ClassifierClauseView>
              && requires(ygg::Data<Clause>& data, const View<Clause>& view) {
                     data.index;
                     data.literals;
                     data.clear();
                     view.get_index();
                     view.get_literals();
                 });
static_assert(IndexedDataView<Classifier> && std::same_as<View<Classifier>, kr::uns::ClassifierView>
              && requires(ygg::Data<Classifier>& data, const View<Classifier>& view) {
                     data.index;
                     data.symbol;
                     data.features;
                     data.clauses;
                     data.clear();
                     view.get_index();
                     view.get_symbol();
                     view.get_features();
                     view.get_clauses();
                 });

namespace fp = tyr::formalism::planning;

struct Fixture
{
    fp::PlanningDomain planning_domain;
    kr::dl::ConstructorRepositoryPtrFor<kr::UnsFamilyTag> dl_repository;
    kr::uns::RepositoryPtr repository;

    Fixture() :
        planning_domain(fp::Parser(benchmark_path("classical/tests/gripper/domain.pddl")).get_domain()),
        dl_repository(kr::dl::ConstructorRepositoryFactoryFor<kr::UnsFamilyTag>().create(planning_domain.get_repository())),
        repository(kr::uns::RepositoryFactory().create(dl_repository))
    {
    }

    fp::DomainView domain() const { return planning_domain.get_domain(); }
};

}  // namespace

TEST(RunirTests, UnsClassifierParsesAndFormats)
{
    Fixture fixture;

    const auto description = read_fixture("kr/uns/positive.classifier");
    auto classifier = kr::uns::dl::parse_classifier(description, fixture.domain(), *fixture.repository);

    EXPECT_EQ(classifier.get_symbol(), "c0");

    std::size_t num_features = 0;
    for (auto feature : classifier.get_features())
    {
        (void) feature;
        ++num_features;
    }
    EXPECT_EQ(num_features, 3u);
    EXPECT_EQ(fmt::format("{}", classifier.get_features().front()),
              "(:boolean\n"
              "    (:symbol some_ball)\n"
              "    (:expression (b_nonempty (c_top)))\n"
              ")");
    EXPECT_EQ(fmt::format("{}", classifier.get_clauses().front().get_literals()[0]), "some_ball");
    EXPECT_EQ(fmt::format("{}", classifier.get_clauses().front().get_literals()[1]), "(not no_object)");

    const auto feature = classifier.get_features().front();
    const auto concrete_complexity = ygg::visit([](auto concrete) { return kr::uns::dl::syntactic_complexity(concrete); }, feature.get_variant());
    EXPECT_EQ(kr::uns::syntactic_complexity(feature), concrete_complexity);
    EXPECT_EQ(concrete_complexity, kr::dl::semantics::syntactic_complexity(feature.get_expression()));
    EXPECT_EQ(kr::uns::syntactic_complexity(classifier), 7);
    auto empty_data = ygg::Data<kr::uns::Classifier>(std::string("empty"));
    EXPECT_EQ(kr::uns::syntactic_complexity(fixture.repository->get_or_create(empty_data).first), 0);

    const auto formatted = fmt::format("{}", classifier);
    EXPECT_EQ(formatted.find("(:boolean (:symbol"), std::string::npos) << formatted;
    EXPECT_NE(formatted.find("(:expression (b_nonempty (c_top)))"), std::string::npos) << formatted;
    EXPECT_EQ(formatted.find(std::string(":") + "description"), std::string::npos) << formatted;
    const auto reparsed = kr::uns::dl::parse_classifier(formatted, fixture.domain(), *fixture.repository);
    EXPECT_EQ(fmt::format("{}", reparsed), formatted);
}

TEST(RunirTests, UnsClassifierInterningIsStructural)
{
    Fixture fixture;

    const auto description = read_fixture("kr/uns/interning.classifier");

    auto first = kr::uns::dl::parse_classifier(description, fixture.domain(), *fixture.repository);
    auto second = kr::uns::dl::parse_classifier(description, fixture.domain(), *fixture.repository);
    EXPECT_EQ(first.get_index(), second.get_index());
}

TEST(RunirTests, UnsClassifierRejectsUnknownFeatureSymbol)
{
    Fixture fixture;

    const auto description = read_fixture("kr/uns/unknown_feature.classifier");

    try
    {
        (void) kr::uns::dl::parse_classifier(description, fixture.domain(), *fixture.repository);
        FAIL() << "Expected unknown feature parsing to fail.";
    }
    catch (const kr::UndefinedSymbolError& error)
    {
        const auto message = std::string(error.what());
        EXPECT_NE(message.find("In line 15:"), std::string::npos) << message;
        EXPECT_NE(message.find("                (and b)"), std::string::npos) << message;
        EXPECT_NE(message.find(std::string(21, '_') + "^_"), std::string::npos) << message;
    }
}

}  // namespace runir::tests
