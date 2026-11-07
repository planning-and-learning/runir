#include "runir/kr/ps/base/dl/parser.hpp"
#include <urd/repr/maps/std_map.hpp>
#include <urd/repr/values/std_set.hpp>
#include <urd/store.hpp>

#include "kr/parser/ext_constructors.hpp"
#include "kr/parser/observations.hpp"
#include "kr/parser/resolution.hpp"
#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/dl/repository.hpp"
#include "runir/kr/parser/diagnostics.hpp"
#include "runir/kr/parser/config.hpp"
#include "runir/kr/ps/base/canonicalization.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/base/dl/parser/parser.hpp"
#include "runir/kr/ps/base/dl/parser/parsers.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/dl/declarations.hpp"

#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <sstream>
#include <string>
#include <type_traits>
#include <tyr/formalism/planning/repository.hpp>
#include <unordered_map>
#include <unordered_set>

namespace runir::kr::ps::base::dl
{
namespace
{

struct Builders
{
    runir::kr::dl::Builder* dl;
    runir::kr::ps::base::Builder* ps;
};

using runir::kr::parser::constructors::parse_constructor;
using runir::kr::parser::ext_constructors::ConstructorContext;
using runir::kr::parser::ext_constructors::LanguageCapabilities;

inline constexpr auto sketch_capabilities = LanguageCapabilities { "sketches", false, false };

template<typename T>
auto intern(runir::kr::ps::base::Repository& repository, urd::data<T>& data)
{ return runir::kr::ps::base::insert(repository, data).first; }

auto parse_feature(const runir::kr::ps::base::dl::ast::BooleanFeature<runir::kr::BaseFamilyTag>& node,
                   tyr::formalism::planning::DomainView domain,
                   Repository& repository,
                   Builders& builders,
                   urd::Store<urd::map<std::string, urd::index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::BooleanTag>>>, urd::stable_refs>& boolean_features,
                   urd::Store<urd::map<std::string, urd::index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::NumericalTag>>>, urd::stable_refs>&,
                   urd::data<runir::kr::ps::base::Sketch>& sketch_data,
                   const runir::kr::parser::DiagnosticContext& diagnostics)
{
    const auto constructor =
        parse_constructor(node.feature, domain, ConstructorContext { repository.get_dl_repository(), *builders.dl, diagnostics, nullptr, sketch_capabilities });
    auto concrete_data =
        runir::kr::ps::base::checkout<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag>>(*builders.ps);
    concrete_data->feature = constructor.index();
    concrete_data->symbol = node.symbol.text;
    const auto concrete = intern(repository, *concrete_data);
    auto data = runir::kr::ps::base::checkout<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::BooleanTag>>(*builders.ps);
    data->variant = concrete.index();
    const auto feature = intern(repository, *data);
    boolean_features.emplace(node.symbol.text, feature.index());
    sketch_data.boolean_features.push_back(feature.index());
}

auto parse_feature(
    const runir::kr::ps::base::dl::ast::NumericalFeature<runir::kr::BaseFamilyTag>& node,
    tyr::formalism::planning::DomainView domain,
    Repository& repository,
    Builders& builders,
    urd::Store<urd::map<std::string, urd::index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::BooleanTag>>>, urd::stable_refs>&,
    urd::Store<urd::map<std::string, urd::index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::NumericalTag>>>, urd::stable_refs>& numerical_features,
    urd::data<runir::kr::ps::base::Sketch>& sketch_data,
    const runir::kr::parser::DiagnosticContext& diagnostics)
{
    const auto constructor =
        parse_constructor(node.feature, domain, ConstructorContext { repository.get_dl_repository(), *builders.dl, diagnostics, nullptr, sketch_capabilities });
    auto concrete_data =
        runir::kr::ps::base::checkout<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag>>(*builders.ps);
    concrete_data->feature = constructor.index();
    concrete_data->symbol = node.symbol.text;
    const auto concrete = intern(repository, *concrete_data);
    auto data = runir::kr::ps::base::checkout<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::NumericalTag>>(*builders.ps);
    data->variant = concrete.index();
    const auto feature = intern(repository, *data);
    numerical_features.emplace(node.symbol.text, feature.index());
    sketch_data.numerical_features.push_back(feature.index());
}

using runir::kr::parser::observations::parse_condition;
using runir::kr::parser::observations::parse_effect;

auto parse_rule(
    const runir::kr::ps::base::dl::ast::Rule<runir::kr::BaseFamilyTag>& node,
    Repository& repository,
    runir::kr::ps::base::Builder& builder,
    const urd::Store<urd::map<std::string, urd::index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::BooleanTag>>>, urd::stable_refs>& boolean_features,
    const urd::Store<urd::map<std::string, urd::index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::NumericalTag>>>, urd::stable_refs>& numerical_features,
    const runir::kr::parser::DiagnosticContext& diagnostics)
{
    auto data = runir::kr::ps::base::checkout<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>>(builder);
    data->symbol = node.symbol.text;
    data->conditions.reserve(node.conditions.size());
    for (const auto& condition : node.conditions)
        data->conditions.push_back(parse_condition(repository, builder, condition, boolean_features, numerical_features, diagnostics));

    data->effects.reserve(node.effects.size());
    for (const auto& effect : node.effects)
        data->effects.push_back(parse_effect(repository, builder, effect, boolean_features, numerical_features, diagnostics));
    return intern(repository, *data);
}

}  // namespace

SketchView parse_sketch(const std::string& description, tyr::formalism::planning::DomainView domain, Repository& repository)
{
    auto first = description.cbegin();
    const auto last = description.cend();
    auto ast = runir::kr::ps::base::dl::ast::Sketch<runir::kr::BaseFamilyTag> {};
    auto errors = std::ostringstream {};
    auto error_handler = runir::kr::parser::ErrorHandlerType(first, last, errors);
    if (!ygg::diagnostics::parse_full(first, last, parser::sketch_root_parser(), ast, error_handler))
        throw runir::kr::parser::DiagnosticContext::parse_error(error_handler, "Failed to parse DL general sketch description.", first);
    auto diagnostics = runir::kr::parser::DiagnosticContext {};
    const auto scope = runir::kr::parser::DiagnosticContext::Scope(diagnostics, error_handler);
    auto dl_builder = runir::kr::dl::Builder {};
    auto ps_builder = runir::kr::ps::base::Builder {};
    auto builders = Builders { &dl_builder, &ps_builder };

    auto boolean_features = urd::Store<urd::map<std::string, urd::index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::BooleanTag>>>, urd::stable_refs> {};
    auto numerical_features = urd::Store<urd::map<std::string, urd::index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::NumericalTag>>>, urd::stable_refs> {};
    auto feature_symbols = urd::Store<urd::set<std::string>, urd::stable_refs> {};
    auto data = runir::kr::ps::base::checkout<runir::kr::ps::base::Sketch>(ps_builder);
    data->boolean_features.reserve(ast.features.size());
    data->numerical_features.reserve(ast.features.size());
    data->rules.reserve(ast.rules.size());

    for (const auto& feature : ast.features)
        boost::apply_visitor(
            [&](const auto& arg)
            {
                if (!feature_symbols.emplace(arg.symbol.text).second)
                    diagnostics.throw_at(arg.symbol, runir::kr::DuplicateDefinitionError("feature", arg.symbol.text));
                parse_feature(arg, domain, repository, builders, boolean_features, numerical_features, *data, diagnostics);
            },
            feature.get());

    auto rule_symbols = urd::Store<urd::set<std::string>, urd::stable_refs> {};
    for (const auto& rule : ast.rules)
    {
        if (!rule_symbols.emplace(rule.symbol.text).second)
            diagnostics.throw_at(rule.symbol, runir::kr::DuplicateDefinitionError("rule", rule.symbol.text));
        data->rules.push_back(parse_rule(rule, repository, ps_builder, boolean_features, numerical_features, diagnostics).index());
    }

    return intern(repository, *data);
}

}  // namespace runir::kr::ps::base::dl
