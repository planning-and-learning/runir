#include "runir/kr/uns/dl/parser.hpp"

#include "kr/parser/ext_constructors.hpp"
#include "kr/parser/resolution.hpp"
#include "runir/kr/dl/detail/repository_instantiations.hpp"
#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/dl/repository.hpp"
#include "runir/kr/parser/config.hpp"
#include "runir/kr/parser/diagnostics.hpp"
#include "runir/kr/ps/detail/repository_instantiations.hpp"
#include "runir/kr/uns/dl/parser/parser.hpp"
#include "runir/kr/uns/dl/parser/parsers.hpp"
#include "runir/kr/uns/repository.hpp"

#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <sstream>
#include <string>
#include <type_traits>
#include <tyr/formalism/planning/repository.hpp>
#include <unordered_map>
#include <urd/repr/maps/std_map.hpp>
#include <urd/store.hpp>
#include <utility>

namespace runir::kr::uns::dl
{
namespace
{

using runir::kr::parser::constructors::parse_constructor;
using runir::kr::parser::ext_constructors::ConstructorContext;
using runir::kr::parser::ext_constructors::LanguageCapabilities;

/// A builder interned through the repository, with scratch records from `builder`.
template<typename T>
auto intern(runir::kr::uns::Repository& repository, runir::kr::uns::Builder& builder, const urd::builder<T, runir::kr::uns::Repository>& built)
{ return repository.insert(built, builder).first; }

auto parse_feature(const runir::kr::uns::dl::ast::BooleanFeature& node,
                   tyr::formalism::DomainView domain,
                   Repository& repository,
                   runir::kr::dl::Builder& dl_builder,
                   runir::kr::uns::Builder& builder,
                   const runir::kr::parser::DiagnosticContext& diagnostics)
{
    const auto context = ConstructorContext { repository.get<runir::kr::dl::ConstructorRepository>(),
                                              dl_builder,
                                              diagnostics,
                                              nullptr,
                                              LanguageCapabilities { "classifiers", false, false } };
    const auto feature = parse_constructor(node.feature, domain, context);
    auto concrete_data =
        runir::kr::uns::checkout<runir::kr::ps::ConcreteFeature<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>>(builder);
    urd::store(concrete_data->feature, feature);
    concrete_data->symbol = node.symbol.text;
    const auto concrete = intern(repository, builder, *concrete_data);

    auto data = runir::kr::uns::checkout<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>>(builder);
    urd::store(data->variant, concrete);
    return intern(repository, builder, *data);
}

}  // namespace

ClassifierView parse_classifier(const std::string& description, tyr::formalism::DomainView domain, Repository& repository)
{
    auto dl_builder = runir::kr::dl::Builder {};
    auto builder = runir::kr::uns::Builder {};
    auto first = description.cbegin();
    const auto last = description.cend();
    auto ast = runir::kr::uns::dl::ast::Classifier {};
    auto errors = std::ostringstream {};
    auto error_handler = runir::kr::parser::ErrorHandlerType(first, last, errors);
    if (!ygg::diagnostics::parse_full(first, last, parser::classifier_root_parser(), ast, error_handler))
        throw runir::kr::parser::DiagnosticContext::parse_error(error_handler, "Failed to parse DL classifier description.", first);
    auto diagnostics = runir::kr::parser::DiagnosticContext {};
    const auto scope = runir::kr::parser::DiagnosticContext::Scope(diagnostics, error_handler);

    auto data = runir::kr::uns::checkout<runir::kr::uns::Classifier>(builder);
    data->symbol = ast.symbol.text;
    data->features.reserve(ast.features.size());
    data->clauses.reserve(ast.expression.clauses.size());
    auto symbol_to_feature =
        urd::Store<urd::map<std::string, urd::index<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>>>, urd::stable_refs> {};
    symbol_to_feature.reserve(ast.features.size());

    for (const auto& feature : ast.features)
    {
        const auto wrapper = parse_feature(feature, domain, repository, dl_builder, builder, diagnostics);
        if (!symbol_to_feature.emplace(feature.symbol.text, wrapper.index()).second)
            diagnostics.throw_at(feature.symbol, runir::kr::DuplicateDefinitionError("feature", feature.symbol.text));
        urd::append(data->features, wrapper);
    }

    const auto parse_literal = [&]<typename Literal>(const Literal& literal)
    {
        const auto it = symbol_to_feature.find(literal.symbol.text);
        if (it == symbol_to_feature.end())
            diagnostics.throw_at(literal.symbol, runir::kr::UndefinedSymbolError("feature", literal.symbol.text));

        constexpr auto polarity = std::same_as<Literal, runir::kr::uns::dl::ast::PositiveLiteral>;
        auto literal_data = runir::kr::uns::checkout<runir::kr::uns::ClassifierLiteral>(builder);
        urd::store(literal_data->feature, repository.view(it->second));
        literal_data->polarity = polarity;
        return intern(repository, builder, *literal_data);
    };

    for (const auto& ast_clause : ast.expression.clauses)
    {
        auto clause_data = runir::kr::uns::checkout<runir::kr::uns::ClassifierClause>(builder);
        clause_data->literals.reserve(ast_clause.literals.size());
        for (const auto& ast_literal : ast_clause.literals)
            urd::append(clause_data->literals, boost::apply_visitor(parse_literal, ast_literal.get()));
        urd::append(data->clauses, intern(repository, builder, *clause_data));
    }

    return intern(repository, builder, *data);
}

}  // namespace runir::kr::uns::dl
