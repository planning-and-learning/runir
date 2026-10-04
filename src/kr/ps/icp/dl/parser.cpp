#include "runir/kr/ps/icp/dl/parser.hpp"

#include "kr/parser/ext_constructors.hpp"
#include "kr/parser/observations.hpp"
#include "runir/kr/ps/icp/canonicalization.hpp"
#include "runir/kr/ps/icp/dl/parser/parser.hpp"
#include "runir/kr/ps/icp/repository.hpp"

#include <algorithm>
#include <cista/containers/variant.h>
#include <sstream>
#include <tyr/formalism/planning/repository.hpp>
#include <unordered_set>

namespace runir::kr::ps::icp::dl
{

namespace
{

using runir::kr::parser::constructors::parse_constructor;
using runir::kr::parser::ext_constructors::ConstructorContext;
using runir::kr::parser::ext_constructors::ModuleReferences;
using runir::kr::parser::ext_constructors::parse;

template<typename T>
auto intern(Repository& repository, ygg::Data<T>& data)
{
    return insert(repository, data).first;
}

using runir::kr::parser::observations::append_conditions;
using runir::kr::parser::observations::append_effects;
using runir::kr::parser::observations::require_feature;

template<typename T>
struct AstCategory;

template<runir::kr::dl::CategoryTag Category>
struct AstCategory<ast::Feature<Category>>
{
    using Type = Category;
};

template<runir::kr::dl::CategoryTag Category>
struct AstCategory<ast::Register<Category>>
{
    using Type = Category;
};

template<runir::kr::dl::CategoryTag Category>
struct AstCategory<ast::LoadRule<Category>>
{
    using Type = Category;
};

template<typename Map>
auto require(const Map& values, const ast::Identifier& name, const char* kind, const runir::kr::parser::DiagnosticContext& diagnostics)
{
    const auto it = values.find(name.text);
    if (it == values.end())
        diagnostics.throw_at(name, runir::kr::UndefinedSymbolError(kind, name.text));
    return it->second;
}

template<typename FeatureTag, typename Expression>
auto make_feature(Repository& repository, Builder& builder, ygg::Index<Expression> expression, const std::string& symbol)
{
    auto concrete = checkout<runir::kr::ps::ConcreteFeature<IcpFamilyTag, DlTag, FeatureTag>>(builder);
    concrete->feature = expression;
    concrete->symbol = symbol;
    auto data = checkout<runir::kr::ps::Feature<IcpFamilyTag, FeatureTag>>(builder);
    data->variant = intern(repository, *concrete).get_index();
    return intern(repository, *data).get_index();
}

ModuleView lower_module(const ast::Module& module,
                        tyr::formalism::planning::DomainView domain,
                        Repository& repository,
                        const runir::kr::parser::DiagnosticContext& diagnostics)
{
    auto builder = Builder {};
    auto dl_builder = runir::kr::dl::Builder<ExtFamilyTag> {};
    auto references = ModuleReferences {};
    auto data = checkout<Module>(builder);
    auto symbol = checkout<ModuleSymbol>(builder);
    symbol->name = module.name.text;
    data->symbol = intern(repository, *symbol).get_index();

    auto memories = std::unordered_map<std::string, ygg::Index<MemoryState>> {};
    for (const auto& memory : module.memory_states)
    {
        auto value = checkout<MemoryState>(builder);
        value->name = memory.value.text;
        const auto index = intern(repository, *value).get_index();
        if (!memories.emplace(memory.value.text, index).second)
            diagnostics.throw_at(memory.value, DuplicateDefinitionError("memory state", memory.value.text));
        data->memory_states.push_back(index);
    }
    data->entry_memory_state = require(memories, module.entry, "memory state", diagnostics);

    for (const auto& reg : module.registers)
        boost::apply_visitor(
            [&](const auto& concrete)
            {
                using Category = typename AstCategory<std::remove_cvref_t<decltype(concrete)>>::Type;
                auto& declarations = [&]() -> auto&
                {
                    if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
                        return references.concept_registers;
                    else
                        return references.role_registers;
                }();
                if (declarations.by_name.contains(concrete.symbol.text))
                    diagnostics.throw_at(concrete.symbol, DuplicateDefinitionError("register", concrete.symbol.text));
                if (declarations.by_identifier.size() == 4)
                    diagnostics.throw_at(concrete.symbol, InvalidExpressionError("At most four registers of each category are supported."));
                auto value = runir::kr::dl::checkout<runir::kr::dl::Register<Category>>(dl_builder);
                value->name = concrete.symbol.text;
                value->identifier = runir::kr::dl::RegisterIdentifier<Category>(declarations.by_identifier.size());
                const auto index = runir::kr::dl::insert(repository.get_dl_repository(), *value).first.get_index();
                declarations.by_name.emplace(concrete.symbol.text, index);
                declarations.by_identifier.push_back(index);
                if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
                    data->concept_registers.push_back(index);
                else
                    data->role_registers.push_back(index);
            },
            reg.get());

    auto concepts = std::unordered_map<std::string, ygg::Index<ps::Feature<IcpFamilyTag, runir::kr::dl::ConceptTag>>> {};
    auto roles = std::unordered_map<std::string, ygg::Index<ps::Feature<IcpFamilyTag, runir::kr::dl::RoleTag>>> {};
    auto booleans = std::unordered_map<std::string, ygg::Index<ps::Feature<IcpFamilyTag, ps::dl::BooleanFeature>>> {};
    auto numericals = std::unordered_map<std::string, ygg::Index<ps::Feature<IcpFamilyTag, ps::dl::NumericalFeature>>> {};
    auto feature_names = std::unordered_set<std::string> {};
    const auto context = ConstructorContext { repository.get_dl_repository(), dl_builder, diagnostics, &references };
    for (const auto& feature : module.features)
        boost::apply_visitor(
            [&](const auto& concrete)
            {
                if (!feature_names.emplace(concrete.symbol.text).second)
                    diagnostics.throw_at(concrete.symbol, DuplicateDefinitionError("feature", concrete.symbol.text));
                using Concrete = std::remove_cvref_t<decltype(concrete)>;
                if constexpr (std::same_as<Concrete, ast::QueryFeature>)
                {
                    const auto expression = parse(concrete.expression, domain, context);
                    data->query_features.push_back(make_feature<ps::dl::QueryFeature>(repository, builder, expression.get_index(), concrete.symbol.text));
                }
                else
                {
                    using Category = typename AstCategory<Concrete>::Type;
                    const auto expression = parse_constructor(concrete.expression, domain, context);
                    if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
                    {
                        const auto index = make_feature<Category>(repository, builder, expression.get_index(), concrete.symbol.text);
                        concepts.emplace(concrete.symbol.text, index);
                        data->concept_features.push_back(index);
                    }
                    else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
                    {
                        const auto index = make_feature<Category>(repository, builder, expression.get_index(), concrete.symbol.text);
                        roles.emplace(concrete.symbol.text, index);
                        data->role_features.push_back(index);
                    }
                    else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
                    {
                        const auto index = make_feature<ps::dl::BooleanFeature>(repository, builder, expression.get_index(), concrete.symbol.text);
                        booleans.emplace(concrete.symbol.text, index);
                        data->boolean_features.push_back(index);
                    }
                    else
                    {
                        const auto index = make_feature<ps::dl::NumericalFeature>(repository, builder, expression.get_index(), concrete.symbol.text);
                        numericals.emplace(concrete.symbol.text, index);
                        data->numerical_features.push_back(index);
                    }
                }
            },
            feature.get());

    const auto initialize_rule = [&](auto& target, const auto& source, const ast::RuleEntry& entry)
    {
        target.source = require(memories, entry.source, "memory state", diagnostics);
        target.target = require(memories, entry.target, "memory state", diagnostics);
        append_conditions(repository, builder, source.conditions, booleans, numericals, diagnostics, target.conditions);
        append_effects(repository, builder, source.effects, booleans, numericals, diagnostics, target.effects);
    };
    auto rule_names = std::unordered_set<std::string> {};
    for (const auto& entry : module.rule_entries)
    {
        if (!rule_names.emplace(entry.symbol.text).second)
            diagnostics.throw_at(entry.symbol, DuplicateDefinitionError("rule", entry.symbol.text));
        data->memory_transitions.emplace_back();
        for (const auto& body : entry.rules)
            boost::apply_visitor(
                [&](const auto& concrete)
                {
                    auto variant = checkout<ps::Rule<IcpFamilyTag>>(builder);
                    variant->symbol = entry.symbol.text;
                    using Concrete = std::remove_cvref_t<decltype(concrete)>;
                    if constexpr (std::same_as<Concrete, ast::Crule>)
                    {
                        auto rule = checkout<Rule<CruleTag>>(builder);
                        initialize_rule(*rule, concrete, entry);
                        auto action = std::optional<tyr::formalism::planning::ActionView<tyr::LiftedTag>> {};
                        for (const auto candidate : domain.get_actions())
                            if (candidate.get_name() == concrete.action.text)
                            {
                                action = candidate;
                                break;
                            }
                        if (!action)
                            diagnostics.throw_at(concrete.action, UndefinedSymbolError("action", concrete.action.text));
                        if (concrete.arguments.size() != action->get_original_arity())
                            diagnostics.throw_at(concrete.action,
                                                 ArityMismatchError("action " + concrete.action.text, action->get_original_arity(), concrete.arguments.size()));
                        rule->action_name = concrete.action.text;
                        auto arguments = std::unordered_map<std::string, ygg::uint_t> {};
                        for (const auto& argument : concrete.arguments)
                        {
                            if (!arguments.emplace(argument.text, arguments.size()).second)
                                diagnostics.throw_at(argument, DuplicateDefinitionError("action argument", argument.text));
                            rule->argument_names.emplace_back(argument.text);
                        }
                        const auto object = [&](const ast::Object& source)
                        {
                            return boost::apply_visitor(
                                [&](const auto& value)
                                    -> ::cista::offset::variant<ArgumentPosition, ygg::Index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>>>
                                {
                                    if constexpr (std::same_as<std::remove_cvref_t<decltype(value)>, ast::ArgumentObject>)
                                        return ArgumentPosition { require(arguments, value.name, "action argument", diagnostics) };
                                    else
                                        return require(references.concept_registers.by_name, value.name, "concept register", diagnostics);
                                },
                                source.get());
                        };
                        for (const auto& condition : concrete.xconditions)
                        {
                            auto value = checkout<XCondition>(builder);
                            value->operation = condition.belongs ? ConditionOperation::BELONGS : ConditionOperation::NOT_BELONGS;
                            value->object = object(condition.object);
                            value->feature = require_feature(concepts, condition.feature, diagnostics);
                            rule->xconditions.push_back(intern(repository, *value).get_index());
                        }
                        for (const auto& effect : concrete.xeffects)
                        {
                            auto value = checkout<XEffect>(builder);
                            value->operation = effect.enter ? EffectOperation::ENTER : EffectOperation::EXIT;
                            value->object = object(effect.object);
                            value->feature = require_feature(concepts, effect.feature, diagnostics);
                            rule->xeffects.push_back(intern(repository, *value).get_index());
                        }
                        variant->variant = intern(repository, *rule).get_index();
                    }
                    else
                    {
                        using Category = typename AstCategory<Concrete>::Type;
                        auto rule = checkout<Rule<LoadTag<Category>>>(builder);
                        initialize_rule(*rule, concrete, entry);
                        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
                        {
                            rule->feature = require_feature(concepts, concrete.feature, diagnostics);
                            rule->reg = require(references.concept_registers.by_name, concrete.reg, "concept register", diagnostics);
                        }
                        else
                        {
                            rule->feature = require_feature(roles, concrete.feature, diagnostics);
                            rule->reg = require(references.role_registers.by_name, concrete.reg, "role register", diagnostics);
                        }
                        variant->variant = intern(repository, *rule).get_index();
                    }
                    data->memory_transitions.back().push_back(intern(repository, *variant).get_index());
                },
                body.get());
    }

    const auto width = data->concept_features.size();
    auto closure = std::vector<std::vector<bool>>(width, std::vector<bool>(width));
    const auto ordinal = [&](auto index)
    { return std::find(data->concept_features.begin(), data->concept_features.end(), index) - data->concept_features.begin(); };
    for (const auto& pair : module.reset_pairs)
    {
        const auto before = ordinal(require_feature(concepts, pair.before, diagnostics));
        const auto after = ordinal(require_feature(concepts, pair.after, diagnostics));
        closure[before][after] = true;
    }
    for (std::size_t middle = 0; middle < width; ++middle)
        for (std::size_t before = 0; before < width; ++before)
            if (closure[before][middle])
                for (std::size_t after = 0; after < width; ++after)
                    closure[before][after] = closure[before][after] || closure[middle][after];
    for (const auto& pair : module.reset_pairs)
        if (closure[ordinal(require_feature(concepts, pair.before, diagnostics))][ordinal(require_feature(concepts, pair.before, diagnostics))])
            diagnostics.throw_at(pair, InvalidExpressionError("Reset precedence must be acyclic and irreflexive."));
    for (std::size_t before = 0; before < width; ++before)
        for (std::size_t after = 0; after < width; ++after)
            if (closure[before][after])
                data->reset_pairs.push_back({ data->concept_features[before], data->concept_features[after] });
    return intern(repository, *data);
}

}  // namespace

ModuleView parse_module(const std::string& description, tyr::formalism::planning::DomainView domain, Repository& repository)
{
    auto errors = std::ostringstream {};
    auto handler = runir::kr::parser::ErrorHandlerType(description.cbegin(), description.cend(), errors);
    auto node = ast::Module {};
    parser::parse_module_ast(description, node, handler);
    auto diagnostics = runir::kr::parser::DiagnosticContext {};
    auto scope = runir::kr::parser::DiagnosticContext::Scope(diagnostics, handler);
    return lower_module(node, domain, repository, diagnostics);
}

ProgramView parse_program(const std::string& description, tyr::formalism::planning::DomainView domain, Repository& repository)
{
    auto errors = std::ostringstream {};
    auto handler = runir::kr::parser::ErrorHandlerType(description.cbegin(), description.cend(), errors);
    auto node = ast::Program {};
    parser::parse_program_ast(description, node, handler);
    auto diagnostics = runir::kr::parser::DiagnosticContext {};
    auto scope = runir::kr::parser::DiagnosticContext::Scope(diagnostics, handler);
    if (node.entry.text != node.module.name.text)
        diagnostics.throw_at(node.entry, UndefinedSymbolError("entry module", node.entry.text));
    auto data = ygg::Data<Program> {};
    data.module = lower_module(node.module, domain, repository, diagnostics).get_index();
    return intern(repository, data);
}

}  // namespace runir::kr::ps::icp::dl
