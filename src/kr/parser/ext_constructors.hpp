#ifndef RUNIR_SRC_KR_PARSER_EXT_CONSTRUCTORS_HPP_
#define RUNIR_SRC_KR_PARSER_EXT_CONSTRUCTORS_HPP_

#include "kr/parser/constructors.hpp"
#include "kr/parser/datalog.hpp"
#include "runir/kr/parser/diagnostics.hpp"

#include <string>
#include <unordered_map>

namespace runir::kr::parser::ext_constructors
{

using constructors::intern;
using constructors::intern_constructor;
using constructors::parse;
using constructors::parse_constructor;
using constructors::unwrap;
using constructors::Unwrapped;

template<typename Entity>
struct ReferenceTable
{
    std::unordered_map<std::string, ygg::Index<Entity>> by_name;
    std::vector<ygg::Index<Entity>> by_identifier;
};

struct ModuleReferences
{
    ReferenceTable<runir::kr::dl::Argument<runir::kr::dl::ConceptTag>> concept_arguments;
    ReferenceTable<runir::kr::dl::Argument<runir::kr::dl::RoleTag>> role_arguments;
    ReferenceTable<runir::kr::dl::Argument<runir::kr::dl::BooleanTag>> boolean_arguments;
    ReferenceTable<runir::kr::dl::Argument<runir::kr::dl::NumericalTag>> numerical_arguments;
    ReferenceTable<runir::kr::dl::Register<runir::kr::dl::ConceptTag>> concept_registers;
    ReferenceTable<runir::kr::dl::Register<runir::kr::dl::RoleTag>> role_registers;
    /// The module's features by name and its :datalog section, while the module is parsed.
    datalog::FeatureResolver* features = nullptr;
    datalog::DatalogSection* datalog = nullptr;
};

/// What a language binds: sketches and classifiers bind neither registers nor arguments, ICP programs only registers.
struct LanguageCapabilities
{
    const char* language;
    bool registers;
    bool arguments;
};

inline constexpr auto all_capabilities = LanguageCapabilities { "Ext module DL expressions", true, true };

class ConstructorContext
{
    runir::kr::dl::ConstructorRepository* m_repository;
    runir::kr::dl::Builder* m_builder;
    const runir::kr::parser::DiagnosticContext* m_diagnostics;
    const ModuleReferences* m_references;
    LanguageCapabilities m_capabilities;

public:
    using Target = runir::kr::parser::constructors::SemanticTarget;

    ConstructorContext(runir::kr::dl::ConstructorRepository& repository,
                       runir::kr::dl::Builder& builder,
                       const runir::kr::parser::DiagnosticContext& diagnostics,
                       const ModuleReferences* references,
                       LanguageCapabilities capabilities = all_capabilities) noexcept :
        m_repository(&repository),
        m_builder(&builder),
        m_diagnostics(&diagnostics),
        m_references(references),
        m_capabilities(capabilities)
    {
    }

    [[nodiscard]] runir::kr::dl::ConstructorRepository& repository() const noexcept { return *m_repository; }
    [[nodiscard]] runir::kr::dl::Builder& builder() const noexcept { return *m_builder; }
    [[nodiscard]] const runir::kr::parser::DiagnosticContext& diagnostics() const noexcept { return *m_diagnostics; }
    [[nodiscard]] const ModuleReferences* references() const noexcept { return m_references; }
    [[nodiscard]] const LanguageCapabilities& capabilities() const noexcept { return m_capabilities; }
};

/// The feature a bare name refers to.
inline datalog::FeatureReference resolve_feature(const runir::kr::parser::ast::Identifier& name, const ConstructorContext& context)
{
    const auto* references = context.references();
    if (!references || !references->features)
        context.diagnostics().throw_at(
            name,
            runir::kr::InvalidExpressionError(std::string("Feature names are not supported in ") + context.capabilities().language + "."));
    const auto feature = references->features->resolve(name);
    if (!feature)
        context.diagnostics().throw_at(name, runir::kr::UndefinedSymbolError("feature", name.text));
    return *feature;
}

template<typename Node>
void require_registers(const Node& node, const ConstructorContext& context)
{
    if (!context.capabilities().registers)
        context.diagnostics().throw_at(
            node,
            runir::kr::InvalidExpressionError(std::string("Registers are not supported in ") + context.capabilities().language + "."));
}

template<typename Node>
void require_arguments(const Node& node, const ConstructorContext& context)
{
    if (!context.capabilities().arguments)
        context.diagnostics().throw_at(
            node,
            runir::kr::InvalidExpressionError(std::string("Arguments are not supported in ") + context.capabilities().language + "."));
}

/// q_head: the rows of a head of the module's :datalog section.
inline auto parse(const runir::kr::dl::grammar::ast::QueryHead& node, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    const auto* references = context.references();
    if (!references || !references->datalog)
        context.diagnostics().throw_at(node,
                                       runir::kr::InvalidExpressionError(std::string("q_head is not supported in ") + context.capabilities().language + "."));
    const auto head = references->datalog->head(node.name);
    if (!head)
        context.diagnostics().throw_at(node.name, runir::kr::UndefinedSymbolError("Datalog head", node.name.text));
    auto data = constructors::checkout<runir::kr::dl::Query<runir::kr::dl::QueryHeadTag>>(context);
    data->program = head->program;
    data->relation = head->relation;
    data->columns = constructors::parse_columns(node.columns, context);
    try
    {
        return constructors::intern_query(context, intern(context, *data).get_index());
    }
    catch (const std::invalid_argument& error)
    {
        context.diagnostics().throw_at(node, runir::kr::InvalidExpressionError(std::string("q_head: ") + error.what()));
    }
}

template<runir::kr::dl::CategoryTag Category>
inline auto parse(const runir::kr::dl::grammar::ast::ConstructorOrNonTerminal<Category>& node,
                  tyr::formalism::planning::DomainView domain,
                  const ConstructorContext& context) -> runir::kr::dl::ConstructorView<Category>
{
    return boost::apply_visitor(
        [&]<typename Value>(const Value& value) -> runir::kr::dl::ConstructorView<Category>
        {
            const auto& unwrapped = unwrap(value);
            if constexpr (std::same_as<Unwrapped<Value>, runir::kr::dl::grammar::ast::NonTerminal<Category>>)
                context.diagnostics().throw_at(
                    unwrapped.name,
                    runir::kr::InvalidExpressionError(std::string(context.capabilities().language) + " cannot reference grammar nonterminals."));
            else if constexpr (std::same_as<Unwrapped<Value>, runir::kr::dl::grammar::ast::FeatureName>)
            {
                const auto feature = resolve_feature(unwrapped.name, context);
                if (const auto* index = std::get_if<ygg::Index<runir::kr::dl::Constructor<Category>>>(&feature))
                    return ygg::make_view(*index, context.repository());
                context.diagnostics().throw_at(
                    unwrapped.name,
                    runir::kr::InvalidExpressionError("Feature `" + unwrapped.name.text + "` is not a " + Category::name + " feature."));
            }
            else
                return parse_constructor(unwrapped, domain, context);
        },
        node.get());
}

/// An operand that may be of several categories (n_count, b_nonempty, n_distance): a bare name is parsed as a concept
/// choice, so it may name a feature of any of the operand's categories.
template<typename Result, runir::kr::dl::CategoryTag Category>
Result parse_collection_operand(const runir::kr::dl::grammar::ast::ConstructorOrNonTerminal<Category>& node,
                                tyr::formalism::planning::DomainView domain,
                                const ConstructorContext& context)
{
    if (const auto* name = boost::get<runir::kr::dl::grammar::ast::FeatureName>(&node.get()))
    {
        const auto feature = resolve_feature(name->name, context);
        return std::visit(
            [&]<typename T>(ygg::Index<T> index) -> Result
            {
                if constexpr (std::is_constructible_v<Result, ygg::Index<T>>)
                    return Result(index);
                else
                    context.diagnostics().throw_at(name->name, runir::kr::InvalidExpressionError("Feature `" + name->name.text + "` cannot be used here."));
            },
            feature);
    }
    return Result(parse(node, domain, context).get_index());
}

template<typename Entity, typename Identifier, typename Reference>
ygg::Index<Entity>
resolve_reference(const Reference& reference, const ReferenceTable<Entity>* declarations, const char* kind, const ConstructorContext& context)
{
    return boost::apply_visitor(
        [&]<typename Concrete>(const Concrete& concrete) -> ygg::Index<Entity>
        {
            if constexpr (std::same_as<Concrete, runir::kr::dl::grammar::ast::NumericReference>)
            {
                if (declarations)
                {
                    if (concrete.value >= declarations->by_identifier.size())
                        context.diagnostics().throw_at(concrete, runir::kr::UndefinedSymbolError(kind, std::to_string(concrete.value)));
                    return declarations->by_identifier[concrete.value];
                }

                try
                {
                    auto data = runir::kr::dl::checkout<Entity>(context.builder());
                    data->name = std::to_string(concrete.value);
                    data->identifier = Identifier(concrete.value);
                    return intern(context, *data).get_index();
                }
                catch (const std::out_of_range&)
                {
                    context.diagnostics().throw_at(concrete, runir::kr::UndefinedSymbolError(kind, std::to_string(concrete.value)));
                }
            }
            else
            {
                if (declarations)
                {
                    const auto it = declarations->by_name.find(concrete.text);
                    if (it != declarations->by_name.end())
                        return it->second;
                    context.diagnostics().throw_at(concrete, runir::kr::UndefinedSymbolError(kind, concrete.text));
                }
                context.diagnostics().throw_at(concrete, runir::kr::InvalidExpressionError("Expected numeric DL reference: " + concrete.text));
            }
        },
        reference.get());
}

inline auto parse(const runir::kr::dl::grammar::ast::ConceptRegister& node, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    require_registers(node, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::dl::RegisterTag>>(context.builder());
    data->reference = resolve_reference<runir::kr::dl::Register<runir::kr::dl::ConceptTag>, runir::kr::dl::RegisterIdentifier<runir::kr::dl::ConceptTag>>(
        node.reference,
        context.references() ? &context.references()->concept_registers : nullptr,
        "concept register",
        context);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

inline auto parse(const runir::kr::dl::grammar::ast::ConceptArgument& node, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    require_arguments(node, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>>(context.builder());
    data->reference = resolve_reference<runir::kr::dl::Argument<runir::kr::dl::ConceptTag>, runir::kr::dl::ArgumentIdentifier<runir::kr::dl::ConceptTag>>(
        node.reference,
        context.references() ? &context.references()->concept_arguments : nullptr,
        "concept argument",
        context);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

inline auto parse(const runir::kr::dl::grammar::ast::RoleRegister& node, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    require_registers(node, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::dl::RegisterTag>>(context.builder());
    data->reference = resolve_reference<runir::kr::dl::Register<runir::kr::dl::RoleTag>, runir::kr::dl::RegisterIdentifier<runir::kr::dl::RoleTag>>(
        node.reference,
        context.references() ? &context.references()->role_registers : nullptr,
        "role register",
        context);
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

inline auto parse(const runir::kr::dl::grammar::ast::RoleArgument& node, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    require_arguments(node, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>>(context.builder());
    data->reference = resolve_reference<runir::kr::dl::Argument<runir::kr::dl::RoleTag>, runir::kr::dl::ArgumentIdentifier<runir::kr::dl::RoleTag>>(
        node.reference,
        context.references() ? &context.references()->role_arguments : nullptr,
        "role argument",
        context);
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

inline auto parse(const runir::kr::dl::grammar::ast::BooleanArgument& node, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    require_arguments(node, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Boolean<runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>>(context.builder());
    data->reference = resolve_reference<runir::kr::dl::Argument<runir::kr::dl::BooleanTag>, runir::kr::dl::ArgumentIdentifier<runir::kr::dl::BooleanTag>>(
        node.reference,
        context.references() ? &context.references()->boolean_arguments : nullptr,
        "boolean argument",
        context);
    return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).get_index());
}

inline auto parse(const runir::kr::dl::grammar::ast::NumericalArgument& node, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    require_arguments(node, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Numerical<runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>>(context.builder());
    data->reference = resolve_reference<runir::kr::dl::Argument<runir::kr::dl::NumericalTag>, runir::kr::dl::ArgumentIdentifier<runir::kr::dl::NumericalTag>>(
        node.reference,
        context.references() ? &context.references()->numerical_arguments : nullptr,
        "numerical argument",
        context);
    return intern_constructor<runir::kr::dl::NumericalTag>(context, intern(context, *data).get_index());
}

}  // namespace runir::kr::parser::ext_constructors

#endif
