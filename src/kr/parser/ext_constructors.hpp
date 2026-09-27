#ifndef RUNIR_SRC_KR_PARSER_EXT_CONSTRUCTORS_HPP_
#define RUNIR_SRC_KR_PARSER_EXT_CONSTRUCTORS_HPP_

#include "kr/parser/constructors.hpp"
#include "runir/kr/parser/diagnostics.hpp"

#include <unordered_map>

namespace runir::kr::parser::ext_constructors
{

using constructors::intern;
using constructors::intern_constructor;
using constructors::parse;
using constructors::parse_constructor;
using constructors::unwrap;

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
};

struct ConstructorContext
{
    using Target = runir::kr::parser::constructors::SemanticTarget<runir::kr::ExtFamilyTag>;

    runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository;
    runir::kr::dl::Builder<runir::kr::ExtFamilyTag>& builder;
    const runir::kr::parser::DiagnosticContext& diagnostics;
    const ModuleReferences* references;
};

template<runir::kr::dl::CategoryTag Category>
inline auto parse(const runir::kr::dl::grammar::ast::ConstructorOrNonTerminal<runir::kr::ExtFamilyTag, Category>& node,
                  tyr::formalism::planning::DomainView domain,
                  const ConstructorContext& context) -> runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, Category>
{
    return boost::apply_visitor(
        [&](const auto& value) -> runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, Category>
        {
            const auto& unwrapped = unwrap(value);
            if constexpr (std::same_as<std::remove_cvref_t<decltype(unwrapped)>, runir::kr::dl::grammar::ast::NonTerminal<runir::kr::ExtFamilyTag, Category>>)
                context.diagnostics.throw_at(unwrapped.name,
                                             runir::kr::InvalidExpressionError("Ext module DL expressions cannot reference "
                                                                               "grammar nonterminals."));
            else
                return parse_constructor(unwrapped, domain, context);
        },
        node.get());
}

template<typename Entity, typename Identifier, typename Reference>
ygg::Index<Entity>
resolve_reference(const Reference& reference, const ReferenceTable<Entity>* declarations, const char* kind, const ConstructorContext& context)
{
    return boost::apply_visitor(
        [&](const auto& concrete) -> ygg::Index<Entity>
        {
            using Concrete = std::remove_cvref_t<decltype(concrete)>;
            if constexpr (std::same_as<Concrete, runir::kr::dl::grammar::ast::NumericReference>)
            {
                if (declarations)
                {
                    if (concrete.value >= declarations->by_identifier.size())
                        context.diagnostics.throw_at(concrete, runir::kr::UndefinedSymbolError(kind, std::to_string(concrete.value)));
                    return declarations->by_identifier[concrete.value];
                }

                try
                {
                    auto data = runir::kr::dl::checkout<Entity>(context.builder);
                    data->name = std::to_string(concrete.value);
                    data->identifier = Identifier(concrete.value);
                    return intern(context, *data).get_index();
                }
                catch (const std::out_of_range&)
                {
                    context.diagnostics.throw_at(concrete, runir::kr::UndefinedSymbolError(kind, std::to_string(concrete.value)));
                }
            }
            else
            {
                if (declarations)
                {
                    const auto it = declarations->by_name.find(concrete.text);
                    if (it != declarations->by_name.end())
                        return it->second;
                    context.diagnostics.throw_at(concrete, runir::kr::UndefinedSymbolError(kind, concrete.text));
                }
                context.diagnostics.throw_at(concrete, runir::kr::InvalidExpressionError("Expected numeric DL reference: " + concrete.text));
            }
        },
        reference.get());
}

inline auto parse(const runir::kr::dl::grammar::ast::ConceptRegister<runir::kr::ExtFamilyTag>& node,
                  tyr::formalism::planning::DomainView,
                  const ConstructorContext& context)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::RegisterTag>>(context.builder);
    data->reference = resolve_reference<runir::kr::dl::Register<runir::kr::dl::ConceptTag>, runir::kr::dl::RegisterIdentifier<runir::kr::dl::ConceptTag>>(
        node.reference,
        context.references ? &context.references->concept_registers : nullptr,
        "concept register",
        context);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

inline auto parse(const runir::kr::dl::grammar::ast::ConceptArgument<runir::kr::ExtFamilyTag>& node,
                  tyr::formalism::planning::DomainView,
                  const ConstructorContext& context)
{
    auto data =
        runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>>(context.builder);
    data->reference = resolve_reference<runir::kr::dl::Argument<runir::kr::dl::ConceptTag>, runir::kr::dl::ArgumentIdentifier<runir::kr::dl::ConceptTag>>(
        node.reference,
        context.references ? &context.references->concept_arguments : nullptr,
        "concept argument",
        context);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

inline auto
parse(const runir::kr::dl::grammar::ast::RoleRegister<runir::kr::ExtFamilyTag>& node, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::ExtFamilyTag, runir::kr::dl::RegisterTag>>(context.builder);
    data->reference = resolve_reference<runir::kr::dl::Register<runir::kr::dl::RoleTag>, runir::kr::dl::RegisterIdentifier<runir::kr::dl::RoleTag>>(
        node.reference,
        context.references ? &context.references->role_registers : nullptr,
        "role register",
        context);
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

inline auto
parse(const runir::kr::dl::grammar::ast::RoleArgument<runir::kr::ExtFamilyTag>& node, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>>(context.builder);
    data->reference = resolve_reference<runir::kr::dl::Argument<runir::kr::dl::RoleTag>, runir::kr::dl::ArgumentIdentifier<runir::kr::dl::RoleTag>>(
        node.reference,
        context.references ? &context.references->role_arguments : nullptr,
        "role argument",
        context);
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

inline auto parse(const runir::kr::dl::grammar::ast::BooleanArgument<runir::kr::ExtFamilyTag>& node,
                  tyr::formalism::planning::DomainView,
                  const ConstructorContext& context)
{
    auto data =
        runir::kr::dl::checkout<runir::kr::dl::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>>(context.builder);
    data->reference = resolve_reference<runir::kr::dl::Argument<runir::kr::dl::BooleanTag>, runir::kr::dl::ArgumentIdentifier<runir::kr::dl::BooleanTag>>(
        node.reference,
        context.references ? &context.references->boolean_arguments : nullptr,
        "boolean argument",
        context);
    return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).get_index());
}

inline auto parse(const runir::kr::dl::grammar::ast::NumericalArgument<runir::kr::ExtFamilyTag>& node,
                  tyr::formalism::planning::DomainView,
                  const ConstructorContext& context)
{
    auto data =
        runir::kr::dl::checkout<runir::kr::dl::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>>(context.builder);
    data->reference = resolve_reference<runir::kr::dl::Argument<runir::kr::dl::NumericalTag>, runir::kr::dl::ArgumentIdentifier<runir::kr::dl::NumericalTag>>(
        node.reference,
        context.references ? &context.references->numerical_arguments : nullptr,
        "numerical argument",
        context);
    return intern_constructor<runir::kr::dl::NumericalTag>(context, intern(context, *data).get_index());
}

}  // namespace runir::kr::parser::ext_constructors

#endif
