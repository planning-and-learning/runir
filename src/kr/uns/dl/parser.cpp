#include "runir/kr/uns/dl/parser.hpp"

#include "kr/parser/ext_constructors.hpp"
#include "kr/parser/resolution.hpp"
#include "runir/kr/dl/detail/repository_instantiations.hpp"
#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/dl/repository.hpp"
#include "runir/kr/parser/diagnostics.hpp"
#include "runir/kr/parser/parser.hpp"
#include "runir/kr/uns/detail/repository_instantiations.hpp"
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
#include <utility>

namespace runir::kr::uns::dl
{
namespace
{

using runir::kr::parser::constructors::parse_constructor;
using runir::kr::parser::ext_constructors::ConstructorContext;
using runir::kr::parser::ext_constructors::LanguageCapabilities;
using runir::kr::parser::ext_constructors::parse;

template<typename T>
auto intern(runir::kr::uns::Repository& repository, ygg::Data<T>& data)
{
    return runir::kr::uns::insert(repository, data).first;
}

auto parse_feature(const runir::kr::uns::dl::ast::BooleanFeature& node,
                   tyr::formalism::planning::DomainView domain,
                   Repository& repository,
                   runir::kr::dl::Builder& dl_builder,
                   runir::kr::uns::Builder& builder,
                   const runir::kr::parser::DiagnosticContext& diagnostics)
{
    const auto context =
        ConstructorContext { repository.get_dl_repository(), dl_builder, diagnostics, nullptr, LanguageCapabilities { "classifiers", false, false } };
    const auto feature = parse_constructor(node.feature, domain, context);
    auto concrete_data =
        runir::kr::uns::checkout<runir::kr::ps::ConcreteFeature<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag>>(builder);
    concrete_data->feature = feature.get_index();
    concrete_data->symbol = node.symbol.text;
    const auto concrete = intern(repository, *concrete_data);

    auto data = runir::kr::uns::checkout<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>>(builder);
    data->variant = concrete.get_index();
    return intern(repository, *data);
}

}  // namespace

ClassifierView parse_classifier(const std::string& description, tyr::formalism::planning::DomainView domain, Repository& repository)
{
    auto dl_builder = runir::kr::dl::Builder {};
    auto builder = runir::kr::uns::Builder {};
    auto first = description.cbegin();
    const auto last = description.cend();
    auto ast = runir::kr::uns::dl::ast::Classifier {};
    auto errors = std::ostringstream {};
    auto error_handler = runir::kr::parser::ErrorHandlerType(first, last, errors);
    if (!runir::kr::parser::parse_full(first, last, parser::classifier_root_parser(), ast, error_handler))
        throw runir::kr::parser::DiagnosticContext::parse_error(error_handler, "Failed to parse DL classifier description.", first);
    auto diagnostics = runir::kr::parser::DiagnosticContext {};
    const auto scope = runir::kr::parser::DiagnosticContext::Scope(diagnostics, error_handler);

    auto data = runir::kr::uns::checkout<runir::kr::uns::Classifier>(builder);
    data->symbol = ast.symbol.text;
    data->features.reserve(ast.features.size());
    data->clauses.reserve(ast.expression.clauses.size());
    auto symbol_to_feature = std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>>> {};
    symbol_to_feature.reserve(ast.features.size());

    for (const auto& feature : ast.features)
    {
        const auto wrapper = parse_feature(feature, domain, repository, dl_builder, builder, diagnostics);
        if (!symbol_to_feature.emplace(feature.symbol.text, wrapper.get_index()).second)
            diagnostics.throw_at(feature.symbol, runir::kr::DuplicateDefinitionError("feature", feature.symbol.text));
        data->features.push_back(wrapper.get_index());
    }

    const auto parse_literal = [&]<typename Literal>(const Literal& literal)
    {
        const auto it = symbol_to_feature.find(literal.symbol.text);
        if (it == symbol_to_feature.end())
            diagnostics.throw_at(literal.symbol, runir::kr::UndefinedSymbolError("feature", literal.symbol.text));

        constexpr auto polarity = std::same_as<Literal, runir::kr::uns::dl::ast::PositiveLiteral>;
        auto literal_data = runir::kr::uns::checkout<runir::kr::uns::ClassifierLiteral>(builder);
        literal_data->variant = it->second;
        literal_data->polarity = polarity;
        return intern(repository, *literal_data).get_index();
    };

    for (const auto& ast_clause : ast.expression.clauses)
    {
        auto clause_data = runir::kr::uns::checkout<runir::kr::uns::ClassifierClause>(builder);
        clause_data->literals.reserve(ast_clause.literals.size());
        for (const auto& ast_literal : ast_clause.literals)
            clause_data->literals.push_back(boost::apply_visitor(parse_literal, ast_literal.get()));
        data->clauses.push_back(intern(repository, *clause_data).get_index());
    }

    return intern(repository, *data);
}

}  // namespace runir::kr::uns::dl
