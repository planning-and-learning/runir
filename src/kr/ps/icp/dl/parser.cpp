#include "runir/kr/ps/icp/dl/parser.hpp"

#include "kr/parser/ext_constructors.hpp"
#include "kr/parser/observations.hpp"
#include "runir/kr/dl/detail/repository_instantiations.hpp"
#include "runir/kr/dl/repository.hpp"
#include "runir/kr/ps/detail/repository_instantiations.hpp"
#include "runir/kr/ps/icp/canonicalization.hpp"
#include "runir/kr/ps/icp/dl/parser/parser.hpp"
#include "runir/kr/ps/icp/repository.hpp"

#include <algorithm>
#include <sstream>
#include <tyr/formalism/repository.hpp>
#include <unordered_set>
#include <urd/elements/flat.hpp>
#include <urd/repr/maps/std_map.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/repr/values/dense_set.hpp>
#include <urd/repr/values/indexed.hpp>
#include <urd/repr/values/std_set.hpp>
#include <urd/store.hpp>

namespace runir::kr::ps::icp::dl
{

namespace
{

using runir::kr::parser::constructors::parse_constructor;
using runir::kr::parser::ext_constructors::ConstructorContext;
using runir::kr::parser::ext_constructors::ModuleReferences;
using runir::kr::parser::ext_constructors::parse;

template<typename T>
auto intern(Repository& repository, Builder& builder, const urd::builder<T, Repository>& built)
{ return repository.insert(built, builder).first; }

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

template<typename T>
using AstCategoryT = typename AstCategory<std::remove_cvref_t<T>>::Type;

template<typename Map>
auto require(const Map& values, const ast::Identifier& name, const char* kind, const runir::kr::parser::DiagnosticContext& diagnostics)
{
    const auto it = values.find(name.text);
    if (it == values.end())
        diagnostics.throw_at(name, runir::kr::UndefinedSymbolError(kind, name.text));
    return it->second;
}

template<typename FeatureTag, typename Expression>
auto make_feature(Repository& repository, Builder& builder, urd::index<Expression> expression, const std::string& symbol)
{
    auto concrete = checkout<runir::kr::ps::ConcreteFeature<IcpFamilyTag, FeatureTag>>(builder);
    urd::store(concrete->feature, repository.get<runir::kr::dl::ConstructorRepository>().view(expression));
    concrete->symbol = symbol;
    auto data = checkout<runir::kr::ps::Feature<IcpFamilyTag, FeatureTag>>(builder);
    urd::store(data->variant, intern(repository, builder, *concrete));
    return intern(repository, builder, *data).index();
}

ModuleView
lower_module(const ast::Module& module, tyr::formalism::DomainView domain, Repository& repository, const runir::kr::parser::DiagnosticContext& diagnostics)
{
    auto builder = Builder {};
    auto dl_builder = runir::kr::dl::Builder {};
    auto references = ModuleReferences {};
    auto data = checkout<Module>(builder);
    auto symbol = checkout<ModuleSymbol<IcpFamilyTag>>(builder);
    symbol->name = module.name.text;
    urd::store(data->symbol, intern(repository, builder, *symbol));

    auto memories = urd::Store<urd::map<std::string, urd::index<MemoryState<IcpFamilyTag>>>, urd::stable_refs> {};
    for (const auto& memory : module.memory_states)
    {
        auto value = checkout<MemoryState<IcpFamilyTag>>(builder);
        value->name = memory.value.text;
        const auto index = intern(repository, builder, *value).index();
        if (!memories.emplace(memory.value.text, index).second)
            diagnostics.throw_at(memory.value, DuplicateDefinitionError("memory state", memory.value.text));
        urd::append(data->memory_states, repository.view(index));
    }
    urd::store(data->entry_memory_state, repository.view(require(memories, module.entry, "memory state", diagnostics)));

    for (const auto& reg : module.registers)
        boost::apply_visitor(
            [&]<typename Concrete>(const Concrete& concrete)
            {
                using Category = AstCategoryT<Concrete>;
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
                const auto index = repository.get<runir::kr::dl::ConstructorRepository>().insert(*value, dl_builder).first.index();
                declarations.by_name.emplace(concrete.symbol.text, index);
                declarations.by_identifier.push_back(index);
                if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
                    urd::append(data->concept_registers, repository.get<runir::kr::dl::ConstructorRepository>().view(index));
                else
                    urd::append(data->role_registers, repository.get<runir::kr::dl::ConstructorRepository>().view(index));
            },
            reg.get());

    auto concepts = urd::Store<urd::map<std::string, urd::index<ps::Feature<IcpFamilyTag, runir::kr::dl::ConceptTag>>>, urd::stable_refs> {};
    auto roles = urd::Store<urd::map<std::string, urd::index<ps::Feature<IcpFamilyTag, runir::kr::dl::RoleTag>>>, urd::stable_refs> {};
    auto booleans = urd::Store<urd::map<std::string, urd::index<ps::Feature<IcpFamilyTag, runir::kr::dl::BooleanTag>>>, urd::stable_refs> {};
    auto numericals = urd::Store<urd::map<std::string, urd::index<ps::Feature<IcpFamilyTag, runir::kr::dl::NumericalTag>>>, urd::stable_refs> {};
    auto feature_names = urd::Store<urd::set<std::string>, urd::stable_refs> {};
    /// The concept features in their order (the module's), by index: what its reset pairs name.
    auto concept_feature_indices = std::vector<urd::index<ps::Feature<IcpFamilyTag, runir::kr::dl::ConceptTag>>> {};
    const auto context = ConstructorContext { repository.get<runir::kr::dl::ConstructorRepository>(),
                                              dl_builder,
                                              diagnostics,
                                              &references,
                                              runir::kr::parser::ext_constructors::LanguageCapabilities { "ICP programs", true, false } };
    for (const auto& feature : module.features)
        boost::apply_visitor(
            [&]<typename Concrete>(const Concrete& concrete)
            {
                if (!feature_names.emplace(concrete.symbol.text).second)
                    diagnostics.throw_at(concrete.symbol, DuplicateDefinitionError("feature", concrete.symbol.text));
                if constexpr (requires { concrete.head; })
                    diagnostics.throw_at(concrete.head, runir::kr::InvalidExpressionError("Head features need a :datalog section."));
                else if constexpr (std::same_as<Concrete, ast::QueryFeature>)
                {
                    const auto expression = parse(concrete.expression, domain, context);
                    urd::append(data->query_features,
                                repository.view(make_feature<ps::dl::QueryFeature>(repository, builder, expression.index(), concrete.symbol.text)));
                }
                else
                {
                    using Category = AstCategoryT<Concrete>;
                    const auto expression = parse_constructor(concrete.expression, domain, context);
                    if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
                    {
                        const auto index = make_feature<Category>(repository, builder, expression.index(), concrete.symbol.text);
                        concepts.emplace(concrete.symbol.text, index);
                        urd::append(data->concept_features, repository.view(index));
                        concept_feature_indices.push_back(index);
                    }
                    else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
                    {
                        const auto index = make_feature<Category>(repository, builder, expression.index(), concrete.symbol.text);
                        roles.emplace(concrete.symbol.text, index);
                        urd::append(data->role_features, repository.view(index));
                    }
                    else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
                    {
                        const auto index = make_feature<runir::kr::dl::BooleanTag>(repository, builder, expression.index(), concrete.symbol.text);
                        booleans.emplace(concrete.symbol.text, index);
                        urd::append(data->boolean_features, repository.view(index));
                    }
                    else
                    {
                        const auto index = make_feature<runir::kr::dl::NumericalTag>(repository, builder, expression.index(), concrete.symbol.text);
                        numericals.emplace(concrete.symbol.text, index);
                        urd::append(data->numerical_features, repository.view(index));
                    }
                }
            },
            feature.get());

    const auto initialize_rule = [&](auto& target, const auto& source, const ast::RuleEntry& entry)
    {
        urd::store(target.source, repository.view(require(memories, entry.source, "memory state", diagnostics)));
        urd::store(target.target, repository.view(require(memories, entry.target, "memory state", diagnostics)));
        append_conditions(repository, builder, source.conditions, booleans, numericals, diagnostics, target.conditions);
        append_effects(repository, builder, source.effects, booleans, numericals, diagnostics, target.effects);
    };
    auto rule_names = urd::Store<urd::set<std::string>, urd::stable_refs> {};
    for (const auto& entry : module.rule_entries)
    {
        if (!rule_names.emplace(entry.symbol.text).second)
            diagnostics.throw_at(entry.symbol, DuplicateDefinitionError("rule", entry.symbol.text));
        data->memory_transitions.emplace_back();
        for (const auto& body : entry.rules)
            boost::apply_visitor(
                [&]<typename Concrete>(const Concrete& concrete)
                {
                    auto variant = checkout<ps::Rule<IcpFamilyTag>>(builder);
                    variant->symbol = entry.symbol.text;
                    if constexpr (std::same_as<Concrete, ast::Crule>)
                    {
                        auto rule = checkout<Rule<CruleTag>>(builder);
                        initialize_rule(*rule, concrete, entry);
                        auto action = std::optional<tyr::formalism::ActionView<tyr::LiftedTag>> {};
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
                        /// The action's argument names: a name's position is its argument's.
                        auto arguments = urd::Store<urd::set<std::string>, urd::positional>(urd::layer { 0 });
                        for (const auto& argument : concrete.arguments)
                        {
                            if (!arguments.insert(argument.text).second)
                                diagnostics.throw_at(argument, DuplicateDefinitionError("action argument", argument.text));
                            rule->argument_names.emplace_back(argument.text);
                        }
                        const auto object = [&](const ast::Object& source)
                            -> std::variant<ArgumentPosition,
                                            urd::child<runir::kr::dl::Register<runir::kr::dl::ConceptTag>, runir::kr::dl::ConstructorRepository>>
                        {
                            // Not a generic visitor lambda: clang 18 crashes on one nested in this lambda.
                            if (const auto* argument = boost::get<ast::ArgumentObject>(&source.get()))
                            {
                                const auto found = arguments.find(argument->name.text);
                                if (found == arguments.end())
                                    diagnostics.throw_at(argument->name, UndefinedSymbolError("action argument", argument->name.text));
                                return ArgumentPosition { static_cast<ygg::uint_t>(arguments.position(found)) };
                            }
                            const auto& register_ = boost::get<ast::RegisterObject>(source.get());
                            return urd::child<runir::kr::dl::Register<runir::kr::dl::ConceptTag>, runir::kr::dl::ConstructorRepository>(
                                require(references.concept_registers.by_name, register_.name, "concept register", diagnostics));
                        };
                        for (const auto& condition : concrete.xconditions)
                        {
                            auto value = checkout<XCondition>(builder);
                            value->operation = condition.belongs ? ConditionOperation::BELONGS : ConditionOperation::NOT_BELONGS;
                            value->object = object(condition.object);
                            urd::store(value->feature, repository.view(require_feature(concepts, condition.feature, diagnostics)));
                            urd::append(rule->xconditions, intern(repository, builder, *value));
                        }
                        for (const auto& effect : concrete.xeffects)
                        {
                            auto value = checkout<XEffect>(builder);
                            value->operation = effect.enter ? EffectOperation::ENTER : EffectOperation::EXIT;
                            value->object = object(effect.object);
                            urd::store(value->feature, repository.view(require_feature(concepts, effect.feature, diagnostics)));
                            urd::append(rule->xeffects, intern(repository, builder, *value));
                        }
                        urd::store(variant->variant, intern(repository, builder, *rule));
                    }
                    else
                    {
                        using Category = AstCategoryT<Concrete>;
                        auto rule = checkout<Rule<LoadTag<Category>>>(builder);
                        initialize_rule(*rule, concrete, entry);
                        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
                        {
                            urd::store(rule->feature, repository.view(require_feature(concepts, concrete.feature, diagnostics)));
                            urd::store(rule->slot,
                                       repository.get<runir::kr::dl::ConstructorRepository>().view(
                                           require(references.concept_registers.by_name, concrete.slot, "concept register", diagnostics)));
                        }
                        else
                        {
                            urd::store(rule->feature, repository.view(require_feature(roles, concrete.feature, diagnostics)));
                            urd::store(rule->slot,
                                       repository.get<runir::kr::dl::ConstructorRepository>().view(
                                           require(references.role_registers.by_name, concrete.slot, "role register", diagnostics)));
                        }
                        urd::store(variant->variant, intern(repository, builder, *rule));
                    }
                    urd::append(data->memory_transitions.back(), intern(repository, builder, *variant));
                },
                body.get());
    }

    const auto width = concept_feature_indices.size();
    using Successors = urd::Store<urd::set<std::size_t>, urd::dense>;
    auto closure = urd::Store<urd::sequence<Successors>>(width, Successors {});
    const auto ordinal = [&](auto index)
    { return std::find(concept_feature_indices.begin(), concept_feature_indices.end(), index) - concept_feature_indices.begin(); };
    for (const auto& pair : module.reset_pairs)
    {
        const auto before = ordinal(require_feature(concepts, pair.before, diagnostics));
        const auto after = ordinal(require_feature(concepts, pair.after, diagnostics));
        closure[before].insert(after);
    }
    for (std::size_t middle = 0; middle < width; ++middle)
        for (std::size_t before = 0; before < width; ++before)
            if (closure[before].contains(middle))
                closure[before] |= closure[middle];
    for (const auto& pair : module.reset_pairs)
        if (const auto before = ordinal(require_feature(concepts, pair.before, diagnostics)); closure[before].contains(before))
            diagnostics.throw_at(pair, InvalidExpressionError("Reset precedence must be acyclic and irreflexive."));
    for (std::size_t before = 0; before < width; ++before)
        for (const auto after : closure[before])
            data->reset_pairs.push_back({ concept_feature_indices[before], concept_feature_indices[after] });
    return intern(repository, builder, *data);
}

}  // namespace

ModuleView parse_module(const std::string& description, tyr::formalism::DomainView domain, Repository& repository)
{
    auto errors = std::ostringstream {};
    auto handler = runir::kr::parser::ErrorHandlerType(description.cbegin(), description.cend(), errors);
    auto node = ast::Module {};
    parser::parse_module_ast(description, node, handler);
    auto diagnostics = runir::kr::parser::DiagnosticContext {};
    auto scope = runir::kr::parser::DiagnosticContext::Scope(diagnostics, handler);
    return lower_module(node, domain, repository, diagnostics);
}

ProgramView parse_program(const std::string& description, tyr::formalism::DomainView domain, Repository& repository)
{
    auto errors = std::ostringstream {};
    auto handler = runir::kr::parser::ErrorHandlerType(description.cbegin(), description.cend(), errors);
    auto node = ast::Program {};
    parser::parse_program_ast(description, node, handler);
    auto diagnostics = runir::kr::parser::DiagnosticContext {};
    auto scope = runir::kr::parser::DiagnosticContext::Scope(diagnostics, handler);
    if (node.entry.text != node.module.name.text)
        diagnostics.throw_at(node.entry, UndefinedSymbolError("entry module", node.entry.text));
    auto data = urd::data<Program> {};
    data.entry_mod = lower_module(node.module, domain, repository, diagnostics).index();
    return insert(repository, data).first;
}

}  // namespace runir::kr::ps::icp::dl
