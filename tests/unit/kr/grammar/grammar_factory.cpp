#include <filesystem>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <iostream>
#include <runir/kr/dl/cnf_grammar/formatter.hpp>
#include <runir/kr/dl/cnf_grammar/translate.hpp>
#include <runir/kr/dl/grammar/formatter.hpp>
#include <runir/kr/dl/grammar/grammar_factory.hpp>
#include <runir/kr/dl/grammar/parser.hpp>
#include <runir/kr/ps/base/dl/parser.hpp>
#include <runir/kr/ps/base/dl/sketch_factory.hpp>
#include <runir/kr/ps/base/formatter.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/base/syntactic_complexity.hpp>
#include <string>
#include <tyr/formalism/planning/parser.hpp>
#include <yggdrasil/semantics/equal_to.hpp>

namespace runir::tests
{

namespace
{

std::filesystem::path runir_root() { return std::filesystem::path(RUNIR_ROOT_DIR); }

std::filesystem::path benchmark_prefix() { return std::filesystem::path(BENCHMARKS_DIR); }

}  // namespace

TEST(RunirTests, FranceEtAlAaai2021SketchFactoriesParse)
{
    namespace fp = tyr::formalism::planning;

    struct Case
    {
        std::filesystem::path domain;
        kr::ps::base::dl::SketchSpecification specification;
    };

    const auto cases = std::vector<Case> {
        { benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl", kr::ps::base::dl::SketchSpecification::GRIPPER_FRANCE_ET_AL_AAAI2021 },
        { benchmark_prefix() / "classical" / "tests" / "blocks_3" / "domain.pddl", kr::ps::base::dl::SketchSpecification::BLOCKS3OPS_FRANCE_ET_AL_AAAI2021 },
        { benchmark_prefix() / "classical" / "tests" / "spanner" / "domain.pddl", kr::ps::base::dl::SketchSpecification::SPANNER_FRANCE_ET_AL_AAAI2021 },
        { benchmark_prefix() / "classical" / "tests" / "delivery" / "domain.pddl", kr::ps::base::dl::SketchSpecification::DELIVERY_FRANCE_ET_AL_AAAI2021 },
    };

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>();
    auto repository_factory = kr::ps::base::RepositoryFactory();

    for (const auto& test_case : cases)
    {
        const auto planning_domain = fp::Parser(test_case.domain).get_domain();
        auto dl_repository = dl_repository_factory.create(planning_domain.get_repository());
        auto repository = repository_factory.create(dl_repository);

        const auto sketch = kr::ps::base::dl::SketchFactory::create(test_case.specification, planning_domain.get_domain(), *repository);

        EXPECT_EQ(sketch.get_index(), ygg::Index<kr::ps::base::Sketch>(0));
        EXPECT_EQ(repository->template size<kr::ps::base::Sketch>(), 1);

        const auto formatted = fmt::format("{}", sketch);
        const auto reparsed = kr::ps::base::dl::parse_sketch(formatted, planning_domain.get_domain(), *repository);
        EXPECT_EQ(fmt::format("{}", reparsed), formatted);
    }
}

TEST(RunirTests, RejectsUnknownFactorySpecifications)
{
    namespace fp = tyr::formalism::planning;

    const auto domain_filepath = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto domain = fp::Parser(domain_filepath).get_domain().get_domain();

    EXPECT_THROW(
        try {
            static_cast<void>(kr::ps::base::dl::SketchFactory::create_description(static_cast<kr::ps::base::dl::SketchSpecification>(999)));
        } catch (const std::runtime_error& error) {
            EXPECT_STREQ(error.what(), "Unknown sketch specification: 999.");
            throw;
        },
        std::runtime_error);

    EXPECT_THROW(
        try {
            static_cast<void>(kr::dl::grammar::GrammarFactory::create_description(static_cast<kr::dl::grammar::GrammarSpecification>(999), domain));
        } catch (const std::runtime_error& error) {
            EXPECT_STREQ(error.what(), "Unknown grammar specification: 999.");
            throw;
        },
        std::runtime_error);
}

TEST(RunirTests, PolicySketchParserParsesConditionsAndEffects)
{
    namespace fp = tyr::formalism::planning;

    const auto domain_filepath = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto planning_domain = fp::Parser(domain_filepath).get_domain();

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>();
    auto repository_factory = kr::ps::base::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(planning_domain.get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto description = std::string { R"(
(:sketch
    (:features
        (:boolean
            (:symbol r) ; feature comment
            (:expression
                (b_nonempty
                    (c_atomic_state "ball")
                )
            )
        )
        (:numerical
            (:symbol c)
            (:expression
                (n_count
                    (c_atomic_state "ball")
                )
            )
        )
    )
    (:rules
        (:rule
            (:symbol ready-rule) ; rule comment
            (:expression
                (:conditions
                    (positive r)
                    (equal_zero c)
                )
                (:effects
                    (negative r)
                    (increases c)
                )
            )
        )
        (:rule
            (:symbol auto1)
            (:expression
                (:conditions
                    (negative r)
                    (greater_zero c)
                )
                (:effects
                    (positive r)
                    (decreases c)
                    (unchanged c)
                )
            )
        )
    )
)
)" };

    const auto sketch = kr::ps::base::dl::parse_sketch(description, planning_domain.get_domain(), *repository);

    EXPECT_EQ(sketch.get_index(), ygg::Index<kr::ps::base::Sketch>(0));
    EXPECT_EQ(repository->template size<kr::ps::base::Sketch>(), 1);
    EXPECT_EQ(repository->template size<kr::ps::base::Rule>(), 2);
    EXPECT_EQ((repository->template size<kr::ps::Feature<kr::BaseFamilyTag, kr::ps::dl::BooleanFeature>>()), 1);
    EXPECT_EQ((repository->template size<kr::ps::Feature<kr::BaseFamilyTag, kr::ps::dl::NumericalFeature>>()), 1);
    EXPECT_EQ(repository->template size<kr::ps::ConditionVariant<kr::BaseFamilyTag>>(), 4);
    EXPECT_EQ(repository->template size<kr::ps::EffectVariant<kr::BaseFamilyTag>>(), 5);

    const auto first_rule = sketch.get_rules()[0];
    EXPECT_EQ(first_rule.get_symbol(), "ready-rule");

    const auto formatted = fmt::format("{}", sketch);
    EXPECT_EQ(formatted.find("(:boolean (:symbol"), std::string::npos) << formatted;
    EXPECT_EQ(formatted.find("(:numerical (:symbol"), std::string::npos) << formatted;
    EXPECT_NE(formatted.find("(:expression"), std::string::npos);
    const auto reparsed = kr::ps::base::dl::parse_sketch(formatted, planning_domain.get_domain(), *repository);
    EXPECT_EQ(fmt::format("{}", reparsed), formatted);
    EXPECT_EQ(kr::ps::base::syntactic_complexity(sketch), 6);
}

TEST(RunirTests, FranceEtAlAaai2021GrammarFactoryForGripperDomain)
{
    namespace fp = tyr::formalism::planning;

    const auto domain_filepath = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto planning_domain = fp::Parser(domain_filepath).get_domain();
    const auto domain = planning_domain.get_domain();

    const auto expected = std::string { R"EXPECTED((
    (c_3 (c_bot))
    (c_4 (c_top))
    (c_5 (c_nominal "rooma"))
    (c_6 (c_nominal "roomb"))
    (c_7 (c_atomic_state "ball"))
    (c_8 (c_atomic_goal "ball" true))
    (c_9 (c_atomic_goal "ball" false))
    (c_10 (c_atomic_state "gripper"))
    (c_11 (c_atomic_goal "gripper" true))
    (c_12 (c_atomic_goal "gripper" false))
    (c_13 (c_atomic_state "number"))
    (c_14 (c_atomic_goal "number" true))
    (c_15 (c_atomic_goal "number" false))
    (c_16 (c_atomic_state "object"))
    (c_17 (c_atomic_goal "object" true))
    (c_18 (c_atomic_goal "object" false))
    (c_19 (c_atomic_state "room"))
    (c_20 (c_atomic_goal "room" true))
    (c_21 (c_atomic_goal "room" false))
    (c_22 (c_atomic_state "at-robby"))
    (c_23 (c_atomic_goal "at-robby" true))
    (c_24 (c_atomic_goal "at-robby" false))
    (c_25 (c_atomic_state "free"))
    (c_26 (c_atomic_goal "free" true))
    (c_27 (c_atomic_goal "free" false))
    (c_28 (c_and c_1 c_1))
    (c_29 (c_not c_1))
    (c_30 (c_some r_1 c_1))
    (c_31 (c_all r_1 c_1))
    (c_32 (c_same_as r_1 r_1))
    (c_2 (c_3 or c_4 or c_5 or c_6 or c_7 or c_8 or c_9 or c_10 or c_11 or c_12 or c_13 or c_14 or c_15 or c_16 or c_17 or c_18 or c_19 or c_20 or c_21 or c_22 or c_23 or c_24 or c_25 or c_26 or c_27))
    (c_0 (c_1))
    (c_1 (c_2 or c_28 or c_29 or c_30 or c_31 or c_32))
    (r_3 (r_atomic_state "at"))
    (r_4 (r_atomic_goal "at" true))
    (r_5 (r_atomic_goal "at" false))
    (r_6 (r_atomic_state "carry"))
    (r_7 (r_atomic_goal "carry" true))
    (r_8 (r_atomic_goal "carry" false))
    (r_9 (r_transitive_closure r_2))
    (r_10 (r_inverse r_2))
    (r_2 (r_3 or r_4 or r_5 or r_6 or r_7 or r_8))
    (r_0 (r_1))
    (r_1 (r_2 or r_9 or r_10))
    (b_2 (b_nonempty c_1))
    (b_3 (b_nonempty r_1))
    (b_0 (b_1))
    (b_1 (b_2 or b_3))
    (n_2 (n_count c_1))
    (n_3 (n_count r_1))
    (n_4 (n_distance c_1 (r_restriction r_2 c_2) c_1))
    (n_4 (n_distance c_1 r_2 c_1))
    (n_0 (n_1))
    (n_1 (n_2 or n_3 or n_4))
)
)EXPECTED" };
    const auto expected_cnf = std::string { R"EXPECTED((
    (c_3 (c_bot))
    (c_4 (c_top))
    (c_5 (c_nominal "rooma"))
    (c_6 (c_nominal "roomb"))
    (c_7 (c_atomic_state "ball"))
    (c_8 (c_atomic_goal "ball" true))
    (c_9 (c_atomic_goal "ball" false))
    (c_10 (c_atomic_state "gripper"))
    (c_11 (c_atomic_goal "gripper" true))
    (c_12 (c_atomic_goal "gripper" false))
    (c_13 (c_atomic_state "number"))
    (c_14 (c_atomic_goal "number" true))
    (c_15 (c_atomic_goal "number" false))
    (c_16 (c_atomic_state "object"))
    (c_17 (c_atomic_goal "object" true))
    (c_18 (c_atomic_goal "object" false))
    (c_19 (c_atomic_state "room"))
    (c_20 (c_atomic_goal "room" true))
    (c_21 (c_atomic_goal "room" false))
    (c_22 (c_atomic_state "at-robby"))
    (c_23 (c_atomic_goal "at-robby" true))
    (c_24 (c_atomic_goal "at-robby" false))
    (c_25 (c_atomic_state "free"))
    (c_26 (c_atomic_goal "free" true))
    (c_27 (c_atomic_goal "free" false))
    (c_28 (c_and c_1 c_1))
    (c_29 (c_not c_1))
    (c_30 (c_some r_1 c_1))
    (c_31 (c_all r_1 c_1))
    (c_32 (c_same_as r_1 r_1))
    (r_3 (r_atomic_state "at"))
    (r_4 (r_atomic_goal "at" true))
    (r_5 (r_atomic_goal "at" false))
    (r_6 (r_atomic_state "carry"))
    (r_7 (r_atomic_goal "carry" true))
    (r_8 (r_atomic_goal "carry" false))
    (r_9 (r_transitive_closure r_2))
    (r_10 (r_inverse r_2))
    (r_11 (r_restriction r_2 c_2))
    (b_2 (b_nonempty c_1))
    (b_3 (b_nonempty r_1))
    (n_2 (n_count c_1))
    (n_3 (n_count r_1))
    (n_4 (n_distance c_1 r_11 c_1))
    (n_4 (n_distance c_1 r_2 c_1))
    (c_2 (c_3))
    (c_2 (c_4))
    (c_2 (c_5))
    (c_2 (c_6))
    (c_2 (c_7))
    (c_2 (c_8))
    (c_2 (c_9))
    (c_2 (c_10))
    (c_2 (c_11))
    (c_2 (c_12))
    (c_2 (c_13))
    (c_2 (c_14))
    (c_2 (c_15))
    (c_2 (c_16))
    (c_2 (c_17))
    (c_2 (c_18))
    (c_2 (c_19))
    (c_2 (c_20))
    (c_2 (c_21))
    (c_2 (c_22))
    (c_2 (c_23))
    (c_2 (c_24))
    (c_2 (c_25))
    (c_2 (c_26))
    (c_2 (c_27))
    (c_0 (c_1))
    (c_1 (c_2))
    (c_1 (c_28))
    (c_1 (c_29))
    (c_1 (c_30))
    (c_1 (c_31))
    (c_1 (c_32))
    (r_2 (r_3))
    (r_2 (r_4))
    (r_2 (r_5))
    (r_2 (r_6))
    (r_2 (r_7))
    (r_2 (r_8))
    (r_0 (r_1))
    (r_1 (r_2))
    (r_1 (r_9))
    (r_1 (r_10))
    (b_0 (b_1))
    (b_1 (b_2))
    (b_1 (b_3))
    (n_0 (n_1))
    (n_1 (n_2))
    (n_1 (n_3))
    (n_1 (n_4))
)
)EXPECTED" };

    auto repository_factory = kr::dl::grammar::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>();
    auto repository = repository_factory.create(planning_domain.get_repository());
    auto other_repository = repository_factory.create(planning_domain.get_repository());
    EXPECT_EQ(repository->get_index(), 0);
    EXPECT_EQ(other_repository->get_index(), 1);

    const auto grammar_view = kr::dl::grammar::GrammarFactory::create(kr::dl::grammar::GrammarSpecification::FRANCE_ET_AL_AAAI2021, domain, *repository);
    const auto formatted = fmt::format("{}", grammar_view);
    std::cout << formatted << std::endl;
    EXPECT_EQ(formatted, expected);

    const auto reparsed_grammar_view = kr::dl::grammar::parse_grammar(formatted, domain, *repository);
    EXPECT_TRUE(ygg::EqualTo<decltype(grammar_view)> {}(grammar_view, reparsed_grammar_view));
    EXPECT_EQ(grammar_view.get_context().template size<kr::dl::grammar::GrammarTag<runir::kr::BaseFamilyTag>>(), 1);
    EXPECT_EQ(grammar_view.get_index(), ygg::Index<kr::dl::grammar::GrammarTag<runir::kr::BaseFamilyTag>>(0));

    auto cnf_repository_factory = kr::dl::cnf_grammar::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>();
    auto cnf_repository = cnf_repository_factory.create(planning_domain.get_repository());
    const auto cnf_grammar_view = kr::dl::cnf_grammar::translate(grammar_view, *cnf_repository);
    const auto cnf_formatted = fmt::format("{}", cnf_grammar_view);
    std::cout << cnf_formatted << std::endl;
    EXPECT_EQ(cnf_formatted, expected_cnf);

    const auto reparsed_cnf_source_grammar_view = kr::dl::grammar::parse_grammar(cnf_formatted, domain, *repository);
    const auto reparsed_cnf_grammar_view = kr::dl::cnf_grammar::translate(reparsed_cnf_source_grammar_view, *cnf_repository);
    EXPECT_TRUE(ygg::EqualTo<decltype(cnf_grammar_view)> {}(cnf_grammar_view, reparsed_cnf_grammar_view));
    EXPECT_EQ(grammar_view.get_context().template size<kr::dl::grammar::GrammarTag<runir::kr::BaseFamilyTag>>(), 2);
}

TEST(RunirTests, FranceEtAlAaai2021GrammarFactoryParsesDomainsWithBooleanPrimitivePredicates)
{
    namespace fp = tyr::formalism::planning;

    const auto domain_filepath = benchmark_prefix() / "classical" / "tests" / "ferry" / "domain.pddl";
    const auto planning_domain = fp::Parser(domain_filepath).get_domain();
    const auto domain = planning_domain.get_domain();

    const auto expected = std::string { R"EXPECTED((
    (c_3 (c_bot))
    (c_4 (c_top))
    (c_5 (c_atomic_state "car"))
    (c_6 (c_atomic_goal "car" true))
    (c_7 (c_atomic_goal "car" false))
    (c_8 (c_atomic_state "location"))
    (c_9 (c_atomic_goal "location" true))
    (c_10 (c_atomic_goal "location" false))
    (c_11 (c_atomic_state "number"))
    (c_12 (c_atomic_goal "number" true))
    (c_13 (c_atomic_goal "number" false))
    (c_14 (c_atomic_state "object"))
    (c_15 (c_atomic_goal "object" true))
    (c_16 (c_atomic_goal "object" false))
    (c_17 (c_atomic_state "at-ferry"))
    (c_18 (c_atomic_goal "at-ferry" true))
    (c_19 (c_atomic_goal "at-ferry" false))
    (c_20 (c_atomic_state "on"))
    (c_21 (c_atomic_goal "on" true))
    (c_22 (c_atomic_goal "on" false))
    (c_23 (c_and c_1 c_1))
    (c_24 (c_not c_1))
    (c_25 (c_some r_1 c_1))
    (c_26 (c_all r_1 c_1))
    (c_27 (c_same_as r_1 r_1))
    (c_2 (c_3 or c_4 or c_5 or c_6 or c_7 or c_8 or c_9 or c_10 or c_11 or c_12 or c_13 or c_14 or c_15 or c_16 or c_17 or c_18 or c_19 or c_20 or c_21 or c_22))
    (c_0 (c_1))
    (c_1 (c_2 or c_23 or c_24 or c_25 or c_26 or c_27))
    (r_3 (r_atomic_state "at"))
    (r_4 (r_atomic_goal "at" true))
    (r_5 (r_atomic_goal "at" false))
    (r_6 (r_transitive_closure r_2))
    (r_7 (r_inverse r_2))
    (r_2 (r_3 or r_4 or r_5))
    (r_0 (r_1))
    (r_1 (r_2 or r_6 or r_7))
    (b_2 (b_atomic_state "empty-ferry" true))
    (b_3 (b_atomic_state "empty-ferry" false))
    (b_4 (b_nonempty c_1))
    (b_5 (b_nonempty r_1))
    (b_0 (b_1))
    (b_1 (b_4 or b_5 or b_2 or b_3))
    (n_2 (n_count c_1))
    (n_3 (n_count r_1))
    (n_4 (n_distance c_1 (r_restriction r_2 c_2) c_1))
    (n_4 (n_distance c_1 r_2 c_1))
    (n_0 (n_1))
    (n_1 (n_2 or n_3 or n_4))
)
)EXPECTED" };
    const auto expected_cnf = std::string { R"EXPECTED((
    (c_3 (c_bot))
    (c_4 (c_top))
    (c_5 (c_atomic_state "car"))
    (c_6 (c_atomic_goal "car" true))
    (c_7 (c_atomic_goal "car" false))
    (c_8 (c_atomic_state "location"))
    (c_9 (c_atomic_goal "location" true))
    (c_10 (c_atomic_goal "location" false))
    (c_11 (c_atomic_state "number"))
    (c_12 (c_atomic_goal "number" true))
    (c_13 (c_atomic_goal "number" false))
    (c_14 (c_atomic_state "object"))
    (c_15 (c_atomic_goal "object" true))
    (c_16 (c_atomic_goal "object" false))
    (c_17 (c_atomic_state "at-ferry"))
    (c_18 (c_atomic_goal "at-ferry" true))
    (c_19 (c_atomic_goal "at-ferry" false))
    (c_20 (c_atomic_state "on"))
    (c_21 (c_atomic_goal "on" true))
    (c_22 (c_atomic_goal "on" false))
    (c_23 (c_and c_1 c_1))
    (c_24 (c_not c_1))
    (c_25 (c_some r_1 c_1))
    (c_26 (c_all r_1 c_1))
    (c_27 (c_same_as r_1 r_1))
    (r_3 (r_atomic_state "at"))
    (r_4 (r_atomic_goal "at" true))
    (r_5 (r_atomic_goal "at" false))
    (r_6 (r_transitive_closure r_2))
    (r_7 (r_inverse r_2))
    (r_8 (r_restriction r_2 c_2))
    (b_2 (b_atomic_state "empty-ferry" true))
    (b_3 (b_atomic_state "empty-ferry" false))
    (b_4 (b_nonempty c_1))
    (b_5 (b_nonempty r_1))
    (n_2 (n_count c_1))
    (n_3 (n_count r_1))
    (n_4 (n_distance c_1 r_8 c_1))
    (n_4 (n_distance c_1 r_2 c_1))
    (c_2 (c_3))
    (c_2 (c_4))
    (c_2 (c_5))
    (c_2 (c_6))
    (c_2 (c_7))
    (c_2 (c_8))
    (c_2 (c_9))
    (c_2 (c_10))
    (c_2 (c_11))
    (c_2 (c_12))
    (c_2 (c_13))
    (c_2 (c_14))
    (c_2 (c_15))
    (c_2 (c_16))
    (c_2 (c_17))
    (c_2 (c_18))
    (c_2 (c_19))
    (c_2 (c_20))
    (c_2 (c_21))
    (c_2 (c_22))
    (c_0 (c_1))
    (c_1 (c_2))
    (c_1 (c_23))
    (c_1 (c_24))
    (c_1 (c_25))
    (c_1 (c_26))
    (c_1 (c_27))
    (r_2 (r_3))
    (r_2 (r_4))
    (r_2 (r_5))
    (r_0 (r_1))
    (r_1 (r_2))
    (r_1 (r_6))
    (r_1 (r_7))
    (b_0 (b_1))
    (b_1 (b_4))
    (b_1 (b_5))
    (b_1 (b_2))
    (b_1 (b_3))
    (n_0 (n_1))
    (n_1 (n_2))
    (n_1 (n_3))
    (n_1 (n_4))
)
)EXPECTED" };

    auto repository_factory = kr::dl::grammar::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>();
    auto repository = repository_factory.create(planning_domain.get_repository());

    const auto grammar_view = kr::dl::grammar::GrammarFactory::create(kr::dl::grammar::GrammarSpecification::FRANCE_ET_AL_AAAI2021, domain, *repository);
    const auto formatted = fmt::format("{}", grammar_view);
    EXPECT_EQ(formatted, expected);

    const auto reparsed_grammar_view = kr::dl::grammar::parse_grammar(formatted, domain, *repository);
    EXPECT_TRUE(ygg::EqualTo<decltype(grammar_view)> {}(grammar_view, reparsed_grammar_view));
    EXPECT_EQ(grammar_view.get_context().template size<kr::dl::grammar::GrammarTag<runir::kr::BaseFamilyTag>>(), 1);
    EXPECT_EQ(grammar_view.get_index(), ygg::Index<kr::dl::grammar::GrammarTag<runir::kr::BaseFamilyTag>>(0));

    auto cnf_repository_factory = kr::dl::cnf_grammar::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>();
    auto cnf_repository = cnf_repository_factory.create(planning_domain.get_repository());
    const auto cnf_grammar_view = kr::dl::cnf_grammar::translate(grammar_view, *cnf_repository);
    const auto cnf_formatted = fmt::format("{}", cnf_grammar_view);
    EXPECT_EQ(cnf_formatted, expected_cnf);

    const auto reparsed_cnf_source_grammar_view = kr::dl::grammar::parse_grammar(cnf_formatted, domain, *repository);
    const auto reparsed_cnf_grammar_view = kr::dl::cnf_grammar::translate(reparsed_cnf_source_grammar_view, *cnf_repository);
    EXPECT_TRUE(ygg::EqualTo<decltype(cnf_grammar_view)> {}(cnf_grammar_view, reparsed_cnf_grammar_view));
    EXPECT_EQ(grammar_view.get_context().template size<kr::dl::grammar::GrammarTag<runir::kr::BaseFamilyTag>>(), 2);
}

}  // namespace runir::tests
