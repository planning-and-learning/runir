#include "fixtures.hpp"

#include <concepts>
#include <filesystem>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <runir/datasets/task_class.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/builder.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <runir/kr/dl/semantics/uns/evaluation.hpp>
#include <runir/kr/errors.hpp>
#include <runir/kr/uns.hpp>
#include <string>
#include <tyr/formalism/planning/parser.hpp>
#include <tyr/planning/ground/task.hpp>
#include <tyr/planning/lifted/task.hpp>
#include <yggdrasil/execution/onetbb.hpp>

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

}  // namespace

namespace
{
namespace fp = tyr::formalism::planning;
namespace p = tyr::planning;
namespace sem = runir::kr::dl::semantics;
using Uns = runir::kr::UnsFamilyTag;

std::filesystem::path benchmark_prefix() { return std::filesystem::path(BENCHMARKS_DIR); }

struct Fixture
{
    std::shared_ptr<ygg::ExecutionContext> execution_context;
    decltype(std::declval<p::Task<p::LiftedTag>>().instantiate_ground_task(std::declval<ygg::ExecutionContext&>()).task) task;
    std::shared_ptr<runir::datasets::TaskSearchContext<p::GroundTag>> search;
    runir::kr::dl::ConstructorRepositoryPtrFor<runir::kr::UnsFamilyTag> dl_repository;
    runir::kr::uns::RepositoryPtr repository;

    Fixture()
    {
        const auto domain_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
        const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";

        auto parser = fp::Parser(domain_file);
        const auto planning_task = parser.parse_task(task_file);
        execution_context = ygg::ExecutionContext::create(1);
        auto lifted_task = p::Task<p::LiftedTag>(planning_task);
        task = lifted_task.instantiate_ground_task(*execution_context).task;
        search = runir::datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);

        auto dl_repository_factory = runir::kr::dl::ConstructorRepositoryFactoryFor<runir::kr::UnsFamilyTag>();
        auto repository_factory = runir::kr::uns::RepositoryFactory();
        dl_repository = dl_repository_factory.create(task->get_repository());
        repository = repository_factory.create(dl_repository);
    }

    fp::DomainView domain() const { return task->get_domain().get_domain(); }
};

}  // namespace

TEST(RunirTests, UnsClassifierParsesAndClassifies)
{
    Fixture fixture;

    // Two boolean features over the gripper initial state:
    //  - some_ball: there exists at least one object satisfying c_top (always true for a nonempty domain)
    //  - no_object: the domain has no objects (false for gripper)
    const auto description = read_fixture("kr/uns/positive.classifier");

    auto classifier = runir::kr::uns::dl::parse_classifier(description, fixture.domain(), *fixture.repository);

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
    const auto concrete_complexity = ygg::visit([](auto concrete) { return runir::kr::uns::dl::syntactic_complexity(concrete); }, feature.get_variant());
    EXPECT_EQ(runir::kr::uns::syntactic_complexity(feature), concrete_complexity);
    EXPECT_EQ(concrete_complexity, runir::kr::dl::semantics::syntactic_complexity(feature.get_expression()));
    EXPECT_EQ(runir::kr::uns::syntactic_complexity(classifier), 7);
    auto empty_data = ygg::Data<runir::kr::uns::Classifier>(std::string("empty"));
    EXPECT_EQ(runir::kr::uns::syntactic_complexity(fixture.repository->get_or_create(empty_data).first), 0);

    const auto state = fixture.search->state_repository->get_initial_state();
    auto builder = sem::Builder();
    auto denotation_repository_factory = sem::DenotationRepositoryFactory();
    auto denotation_repository = denotation_repository_factory.create(fixture.task->get_repository());
    auto context = sem::EvaluationContext<Uns, p::GroundTag>(state, builder, denotation_repository);

    // some_ball is true and no_object is false, so the first clause (some_ball AND NOT no_object) holds.
    const auto formatted = fmt::format("{}", classifier);
    EXPECT_EQ(formatted.find("(:boolean (:symbol"), std::string::npos) << formatted;
    EXPECT_NE(formatted.find("(:expression (b_nonempty (c_top)))"), std::string::npos) << formatted;
    EXPECT_EQ(formatted.find(std::string(":") + "description"), std::string::npos) << formatted;
    const auto reparsed = runir::kr::uns::dl::parse_classifier(formatted, fixture.domain(), *fixture.repository);
    EXPECT_EQ(fmt::format("{}", reparsed), formatted);

    EXPECT_TRUE(runir::kr::uns::classify(classifier, context));
}

TEST(RunirTests, UnsClassifierInterningIsStructural)
{
    Fixture fixture;

    const auto description = read_fixture("kr/uns/interning.classifier");

    auto first = runir::kr::uns::dl::parse_classifier(description, fixture.domain(), *fixture.repository);
    auto second = runir::kr::uns::dl::parse_classifier(description, fixture.domain(), *fixture.repository);
    EXPECT_EQ(first.get_index(), second.get_index());
}

TEST(RunirTests, UnsClassifierRejectsUnknownFeatureSymbol)
{
    Fixture fixture;

    const auto description = read_fixture("kr/uns/unknown_feature.classifier");

    try
    {
        (void) runir::kr::uns::dl::parse_classifier(description, fixture.domain(), *fixture.repository);
        FAIL() << "Expected unknown feature parsing to fail.";
    }
    catch (const runir::kr::UndefinedSymbolError& error)
    {
        const auto message = std::string(error.what());
        EXPECT_NE(message.find("In line 15:"), std::string::npos) << message;
        EXPECT_NE(message.find("                (and b)"), std::string::npos) << message;
        EXPECT_NE(message.find(std::string(21, '_') + "^_"), std::string::npos) << message;
    }
}

}  // namespace runir::tests
