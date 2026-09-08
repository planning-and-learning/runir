#include "fixtures.hpp"

#include <boost/variant/get.hpp>
#include <gtest/gtest.h>
#include <optional>
#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <runir/kr/dl/grammar/parser.hpp>
#include <runir/kr/dl/grammar/parser/parser.hpp>
#include <runir/kr/domain_context.hpp>
#include <runir/kr/errors.hpp>
#include <runir/kr/ps/base/dl/parser.hpp>
#include <runir/kr/ps/base/dl/parser/parser.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <runir/kr/ps/ext/dl/parser/parser.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/uns/dl/parser.hpp>
#include <runir/kr/uns/dl/parser/parser.hpp>
#include <runir/kr/uns/repository.hpp>
#include <string>
#include <tuple>
#include <typeinfo>
#include <tyr/formalism/planning/parser.hpp>

namespace runir::tests
{
namespace
{

template<typename Parse>
void expect_diagnostic(Parse parse,
                       const std::string& source,
                       const std::string& context,
                       std::size_t position,
                       std::size_t start,
                       const std::string& expected = "")
{
    std::optional<kr::ParseError> error;
    try
    {
        static_cast<void>(parse(source));
    }
    catch (const kr::ParseError& caught)
    {
        error = caught;
    }
    ASSERT_TRUE(error.has_value()) << source;
    const auto& diagnostic = error->diagnostic();
    EXPECT_NE(diagnostic.message.find("while parsing " + context), std::string::npos) << diagnostic.message;
    if (!expected.empty())
    {
        EXPECT_EQ(diagnostic.message, "Expected " + expected + " while parsing " + context);
    }
    ASSERT_TRUE(diagnostic.location.has_value());
    EXPECT_EQ(diagnostic.location->begin(), position);
    EXPECT_EQ(diagnostic.location->end(), position);
    EXPECT_EQ(diagnostic.location->source()->text(), source);
    ASSERT_EQ(diagnostic.notes.size(), 1);
    EXPECT_EQ(diagnostic.notes[0].message, context + " starts here");
    ASSERT_TRUE(diagnostic.notes[0].location.has_value());
    EXPECT_EQ(diagnostic.notes[0].location->begin(), start);
    EXPECT_EQ(diagnostic.notes[0].location->source(), diagnostic.location->source());
    EXPECT_EQ(error->what(), ygg::diagnostics::format_diagnostic(diagnostic));
}

template<kr::dl::FamilyTag Family>
void check_dl_expression_diagnostics()
{
    const auto parse = [](const std::string& source) { return kr::dl::grammar::parser::parse_concept_ast<Family>(source); };
    EXPECT_THROW(parse("c_top"), kr::ParseError);
    EXPECT_NO_THROW(parse("(c_top)"));
    EXPECT_NO_THROW(parse("(c_and (c_top) (c_bot))"));
    EXPECT_NO_THROW(parse("(c_and c_0 c_1)"));
    const auto one_of = parse("(c_one_of \"a\" \"b\")");
    using OneOf = boost::spirit::x3::forward_ast<kr::dl::grammar::ast::ConceptOneOf<Family>>;
    const auto& object_names = boost::get<OneOf>(one_of.get()).get().object_names;
    ASSERT_EQ(object_names.size(), 2);
    EXPECT_EQ(object_names[0].text, "a");
    EXPECT_EQ(object_names[1].text, "b");
    EXPECT_NO_THROW(parse("(c_fillers (r_universal) \"a\" \"b\")"));
    EXPECT_THROW(kr::dl::grammar::parser::parse_role_ast<Family>("r_universal"), kr::ParseError);
    EXPECT_THROW(kr::dl::grammar::parser::parse_boolean_ast<Family>("b_nonempty (c_top)"), kr::ParseError);
    EXPECT_THROW(kr::dl::grammar::parser::parse_numerical_ast<Family>("n_count (c_top)"), kr::ParseError);
    for (const auto* keyword : { "c_at_least", "c_at_most", "c_exactly" })
    {
        SCOPED_TRACE(keyword);
        const auto head = std::string("(") + keyword + " 1 (r_universal)";
        for (const auto* tail : { "", " (c_top)" })
        {
            const auto incomplete = head + tail;
            EXPECT_NO_THROW(parse(incomplete + ")"));
            EXPECT_NO_THROW(parse("(c_and " + incomplete + ") (c_top))"));
            expect_diagnostic(parse, incomplete, keyword, incomplete.size(), 0, "\")\"");
        }
        EXPECT_NO_THROW(parse(head + " c_0)"));
    }
    EXPECT_NO_THROW(parse("; a comment with ) and (\n(c_top) ; )"));
    expect_diagnostic(parse, "(c_top", "c_top", 6, 0, "\")\"");
    expect_diagnostic(parse, "(c_not", "c_not", 6, 0);
    expect_diagnostic(parse, "(c_top))", "concept expression", 7, 0);

    const auto unopened = std::string("(c_and c_at_least 1 r_universal c_top)");
    expect_diagnostic(parse, unopened, "c_and", unopened.find("c_at_least"), 0);
    const auto unknown = std::string("(c_and (c_unknown) (c_top))");
    expect_diagnostic(parse, unknown, "c_and", unknown.find("(c_unknown)"), 0);
    const auto keyword_prefix = std::string("(c_not (c_andrew))");
    expect_diagnostic(parse, keyword_prefix, "c_not", keyword_prefix.find("(c_andrew)"), 0);
    const auto entered = std::string("(c_and (c_at_least 1 r_universal) (c_top))");
    expect_diagnostic(parse, entered, "c_at_least", entered.find("r_universal"), entered.find("(c_at_least"));
    const auto nested_close = std::string("(c_and (c_top) (c_at_least 1 (r_universal)");
    expect_diagnostic(parse, nested_close, "c_at_least", nested_close.size(), nested_close.find("(c_at_least"), "\")\"");

    const auto grammar = kr::dl::grammar::parser::parse_grammar_ast<Family>("((c_0 (c_top))(c_1 ((c_top) or (c_bot)))(c_2 (c_0 or c_1))(c_3 (c_and c_0 c_1))"
                                                                            "(r_0 (r_universal))(b_0 (b_nonempty c_0))(n_0 (n_count c_0)))");
    ASSERT_EQ(grammar.rules.size(), 7);
    using ConceptDerivation = kr::dl::grammar::ast::DerivationRule<Family, kr::dl::ConceptTag>;
    EXPECT_EQ(boost::get<ConceptDerivation>(grammar.rules[0].get()).rhs.size(), 1);
    EXPECT_EQ(boost::get<ConceptDerivation>(grammar.rules[1].get()).rhs.size(), 2);
    EXPECT_EQ(boost::get<ConceptDerivation>(grammar.rules[2].get()).rhs.size(), 2);

    for (const auto& [source, owner, expected, unexpected] : {
             std::tuple { std::string("(c_atomic_state room)"), "c_atomic_state", "quoted predicate name", "room" },
             std::tuple { std::string("(c_nominal rooma)"), "c_nominal", "quoted object name", "rooma" },
             std::tuple { std::string("(c_atomic_state \"room)"), "quoted predicate name", "'\"'", ")" },
             std::tuple { std::string("(c_nominal \"rooma)"), "quoted object name", "'\"'", ")" },
             std::tuple { std::string("(c_atomic_state \" room\")"), "quoted predicate name", "predicate name", " room" },
             std::tuple { std::string("(c_atomic_state \"room \" )"), "quoted predicate name", "'\"'", " \"" },
             std::tuple { std::string("(c_nominal \" rooma\")"), "quoted object name", "object name", " rooma" },
             std::tuple { std::string("(c_nominal \"rooma \" )"), "quoted object name", "'\"'", " \"" },
             std::tuple { std::string("(c_atomic_state \"; ignored\nroom\")"), "quoted predicate name", "predicate name", ";" },
             std::tuple { std::string("(c_atomic_state \"room; ignored\n\")"), "quoted predicate name", "'\"'", ";" },
             std::tuple { std::string("(c_nominal \"; ignored\nrooma\")"), "quoted object name", "object name", ";" },
             std::tuple { std::string("(c_nominal \"rooma; ignored\n\")"), "quoted object name", "'\"'", ";" },
             std::tuple { std::string("(c_atomic_goal \"room\" maybe)"), "c_atomic_goal", "true or false", "maybe" },
             std::tuple { std::string("(c_atomic_goal \"room\" true_extra)"), "c_atomic_goal", "true or false", "true_extra" },
             std::tuple { std::string("(c_at_least bad (r_universal))"), "c_at_least", "unsigned integer", "bad" },
             std::tuple { std::string("(c_at_least -1 (r_universal))"), "c_at_least", "unsigned integer", "-1" },
         })
    {
        SCOPED_TRACE(source);
        const auto start = std::string(owner).starts_with("quoted") ? source.find('"') : 0;
        expect_diagnostic(parse, source, owner, source.find(unexpected, start), start, expected);
    }
    EXPECT_NO_THROW(parse("(c_atomic_state \"room\")"));
    EXPECT_NO_THROW(parse("(c_atomic_goal \"room\" true)"));
    EXPECT_NO_THROW(parse("(c_nominal \"rooma\")"));
    EXPECT_NO_THROW(parse("(c_atomic_state ; outside quote\n \"room\")"));

    const auto parse_grammar = [](const std::string& source) { return kr::dl::grammar::parser::parse_grammar_ast<Family>(source); };
    const auto empty_derivation = std::string("((c_0 ))");
    expect_diagnostic(parse_grammar, empty_derivation, "concept derivation", empty_derivation.find(')'), 1, "constructor or alternatives");
    const auto empty_alternatives = std::string("((c_0 ()))");
    expect_diagnostic(parse_grammar, empty_alternatives, "concept derivation", empty_alternatives.find(')'), 1, "constructor alternatives");
}

}  // namespace

TEST(RunirTests, ParserDiagnosticsCoverSketchModuleAndClassifier)
{
    const auto sketch = std::string("(:sketch (:features) (:rules))");
    const auto module = std::string("(:module (:symbol m) (:arguments) (:registers) (:entry s) (:memory s) (:features) (:rules))");
    const auto classifier = std::string("(:classifier (:symbol c) (:features) (:expression (or)))");
    const auto parse_sketch = kr::ps::base::dl::parser::parse_sketch_ast;
    const auto parse_module = [](const std::string& source) { return kr::ps::ext::dl::parser::parse_module_ast(source); };
    const auto parse_classifier = kr::uns::dl::parser::parse_classifier_ast;

    EXPECT_NO_THROW(parse_sketch("; )\n" + sketch + " ; ("));
    EXPECT_NO_THROW(parse_module("; )\n" + module + " ; ("));
    EXPECT_NO_THROW(parse_classifier("; )\n" + classifier + " ; ("));
    expect_diagnostic(parse_sketch, sketch.substr(0, sketch.size() - 1), "sketch", sketch.size() - 1, 0);
    expect_diagnostic(parse_sketch, sketch + ")", "sketch", sketch.size(), 0);
    expect_diagnostic(parse_module, module.substr(0, module.size() - 1), "module", module.size() - 1, 0);
    expect_diagnostic(parse_module, module + ")", "module", module.size(), 0);
    expect_diagnostic(parse_classifier, classifier.substr(0, classifier.size() - 1), "classifier", classifier.size() - 1, 0);
    expect_diagnostic(parse_classifier, classifier + ")", "classifier", classifier.size(), 0);
}

TEST(RunirTests, LookupErrorsPreserveIdentifierSpansAcrossLanguages)
{
    const auto planning_domain = tyr::formalism::planning::Parser(benchmark_path("classical/tests/gripper/domain.pddl")).get_domain();
    const auto domain = planning_domain.get_domain();
    const auto context = kr::DomainContext::create(planning_domain);
    const auto grammar_repository = kr::dl::grammar::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>().create(planning_domain.get_repository());
    const auto prefix = std::string("; UTF-8 é\n");
    for (const auto& [expression, name, message] : {
             std::tuple { "(c_atomic_state \"missing\")", "missing", "Undefined predicate: missing" },
             std::tuple { "(c_atomic_state \"at\")", "at", "Arity mismatch for ConceptAtomicState: expected 1, got 2" },
             std::tuple { "(c_nominal \"missing\")", "missing", "Undefined constant: missing" },
         })
    {
        const auto feature = std::string("(:boolean (:symbol b) (:expression (b_nonempty ") + expression + ")))";
        const auto check = [&](const std::string& source, auto&& parse)
        {
            SCOPED_TRACE(source);
            try
            {
                parse(source);
                FAIL() << "Expected lookup to fail";
            }
            catch (const kr::SemanticError& error)
            {
                EXPECT_EQ(typeid(error), std::string(name) == "at" ? typeid(kr::ArityMismatchError) : typeid(kr::UndefinedSymbolError));
                const auto& diagnostic = error.diagnostic();
                EXPECT_EQ(diagnostic.message, message);
                ASSERT_TRUE(diagnostic.location.has_value());
                const auto begin = source.find(std::string("\"") + name + "\"") + 1;
                EXPECT_EQ(diagnostic.location->begin(), begin);
                EXPECT_EQ(diagnostic.location->end(), begin + std::string(name).size());
                EXPECT_EQ(diagnostic.location->source()->text(), source);
            }
        };
        check(prefix + "((c_0 " + expression + "))",
              [&](const auto& source) { static_cast<void>(kr::dl::grammar::parse_grammar(source, domain, *grammar_repository)); });
        check(prefix + "(:sketch (:features " + feature + ") (:rules))",
              [&](const auto& source) { static_cast<void>(kr::ps::base::dl::parse_sketch(source, domain, *context->base_repository)); });
        check(prefix + "(:module (:symbol m) (:arguments) (:registers) (:entry s) (:memory s) (:features " + feature + ") (:rules))",
              [&](const auto& source) { static_cast<void>(kr::ps::ext::dl::parse_module(source, domain, *context->ext_repository)); });
        check(prefix + "(:classifier (:symbol c) (:features " + feature + ") (:expression (or)))",
              [&](const auto& source) { static_cast<void>(kr::uns::dl::parse_classifier(source, domain, *context->uns_repository)); });
    }
}

TEST(RunirTests, ParserDiagnosticsPreserveTheInnermostContext)
{
    const auto source = std::string("; UTF-8: é )\n(:sketch (:features (:boolean (:symbol b) (:expression (b_nonempty (c_top))");
    expect_diagnostic(kr::ps::base::dl::parser::parse_sketch_ast, source, ":expression", source.size(), source.find("(:expression"));

    const auto program = std::string("(:program (:entry root)");
    expect_diagnostic([](const std::string& source) { return kr::ps::ext::dl::parser::parse_module_program_ast(source); },
                      program,
                      "program",
                      program.size(),
                      0,
                      "one or more modules");
}

TEST(RunirTests, ParserDiagnosticsKeepUnrecognizedSectionsAtTheirParent)
{
    const auto sketch = std::string("(:sketch (:unknown))");
    expect_diagnostic(kr::ps::base::dl::parser::parse_sketch_ast, sketch, "sketch", sketch.find("(:unknown"), 0, ":features");
    const auto module = std::string("(:module (:unknown))");
    expect_diagnostic([](const std::string& source) { return kr::ps::ext::dl::parser::parse_module_ast(source); },
                      module,
                      "module",
                      module.find("(:unknown"),
                      0,
                      ":symbol");
    const auto classifier = std::string("(:classifier (:unknown))");
    expect_diagnostic(kr::uns::dl::parser::parse_classifier_ast, classifier, "classifier", classifier.find("(:unknown"), 0, ":symbol");

    const auto unopened = std::string("(:sketch (:features (:boolean (:symbol b) (:unknown))))");
    expect_diagnostic(kr::ps::base::dl::parser::parse_sketch_ast,
                      unopened,
                      "boolean feature",
                      unopened.find("(:unknown"),
                      unopened.find("(:boolean"),
                      ":expression");
    const auto entered = std::string("(:sketch (:features (:boolean (:symbol))))");
    expect_diagnostic(kr::ps::base::dl::parser::parse_sketch_ast, entered, ":symbol", entered.find(")"), entered.find("(:symbol"), "identifier");
    const auto keyword_prefix = std::string("(:sketch (:features (:boolean (:symbolic b))))");
    expect_diagnostic(kr::ps::base::dl::parser::parse_sketch_ast,
                      keyword_prefix,
                      "boolean feature",
                      keyword_prefix.find("(:symbolic"),
                      keyword_prefix.find("(:boolean"),
                      ":symbol");
}

TEST(RunirTests, ParserDiagnosticsNameRequiredModuleListsAndDelimiters)
{
    const auto parse_module = [](const std::string& source) { return kr::ps::ext::dl::parser::parse_module_ast(source); };
    const auto prefix = std::string("(:module (:symbol m) (:arguments) (:registers) (:entry s) ");
    const auto memory = prefix + "(:memory) (:features) (:rules))";
    expect_diagnostic(parse_module, memory, ":memory", memory.find(')', memory.find("(:memory")), memory.find("(:memory"), "one or more memory states");
    const auto rules = prefix + "(:memory s) (:features) (:rules (:rule (:symbol r) (:expression (:source-memory s) (:target-memory s)))))";
    expect_diagnostic(parse_module, rules, "rule expression", rules.find("s)))") + 2, rules.find("(:expression"), "one or more rules");
    const auto argument = std::string("(:module (:symbol m) (:arguments (:concept c");
    expect_diagnostic(parse_module, argument, "concept argument", argument.size(), argument.find("(:concept"), "\")\"");
    const auto action = prefix
                        + "(:memory s) (:features) (:rules (:rule (:symbol r) (:expression (:source-memory s) (:target-memory s) "
                          "(:do (:conditions) (:action \"move";
    expect_diagnostic(parse_module, action, "quoted string", action.size(), action.find('"'), "'\"'");
}

TEST(RunirTests, ParserDiagnosticsNameConstantScalars)
{
    const auto boolean = std::string("(b_const invalid)");
    expect_diagnostic([](const std::string& source) { return kr::dl::grammar::parser::parse_boolean_ast<kr::UnsFamilyTag>(source); },
                      boolean,
                      "b_const",
                      boolean.find("invalid"),
                      0,
                      "true or false");
    const auto numerical = std::string("(n_const invalid)");
    expect_diagnostic([](const std::string& source) { return kr::dl::grammar::parser::parse_numerical_ast<kr::UnsFamilyTag>(source); },
                      numerical,
                      "n_const",
                      numerical.find("invalid"),
                      0,
                      "unsigned integer");
}

TEST(RunirTests, ParserDiagnosticsRequireDelimitedConstructorsInEveryDlFamily)
{
    check_dl_expression_diagnostics<kr::BaseFamilyTag>();
    check_dl_expression_diagnostics<kr::ExtFamilyTag>();
    check_dl_expression_diagnostics<kr::UnsFamilyTag>();
}

TEST(RunirTests, ParserDiagnosticOutlivesInputAndErrorHandler)
{
    const auto diagnostic = []
    {
        const auto source = std::string("(:sketch (:features) (:rules)");
        try
        {
            static_cast<void>(kr::ps::base::dl::parser::parse_sketch_ast(source));
        }
        catch (const kr::ParseError& error)
        {
            return error.diagnostic();
        }
        throw std::logic_error("expected parse error");
    }();
    ASSERT_TRUE(diagnostic.location.has_value());
    EXPECT_EQ(diagnostic.location->source()->text(), "(:sketch (:features) (:rules)");
    EXPECT_NE(ygg::diagnostics::format_diagnostic(diagnostic).find("sketch starts here"), std::string::npos);
}

}  // namespace runir::tests
