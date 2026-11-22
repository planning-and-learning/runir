#include "runir/kr/dl/grammar/parser.hpp"

#include "kr/parser/constructors.hpp"
#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/dl/grammar/canonicalization.hpp"
#include "runir/kr/dl/grammar/constructor_repository.hpp"
#include "runir/kr/dl/grammar/parser/parser.hpp"
#include "runir/kr/parser/diagnostics.hpp"

#include <boost/variant/apply_visitor.hpp>
#include <sstream>
#include <string>
#include <type_traits>
#include <tyr/formalism/repository.hpp>
#include <urd/elements/flat.hpp>
#include <utility>

namespace runir::kr::dl::grammar
{
namespace
{

class ConstructorContext
{
    ConstructorRepository* m_repository;
    Builder* m_builder;
    const runir::kr::parser::DiagnosticContext* m_diagnostics;

public:
    using Target = runir::kr::parser::constructors::GrammarTarget;
    using Repository = ConstructorRepository;

    ConstructorContext(ConstructorRepository& repository, Builder& builder, const runir::kr::parser::DiagnosticContext& diagnostics) noexcept :
        m_repository(&repository),
        m_builder(&builder),
        m_diagnostics(&diagnostics)
    {
    }

    [[nodiscard]] ConstructorRepository& repository() const noexcept { return *m_repository; }

    [[nodiscard]] Builder& builder() const noexcept { return *m_builder; }

    [[nodiscard]] const runir::kr::parser::DiagnosticContext& diagnostics() const noexcept { return *m_diagnostics; }
};

using runir::kr::parser::constructors::parse;
using runir::kr::parser::constructors::parse_constructor;
using runir::kr::parser::constructors::unwrap;
using runir::kr::parser::constructors::Unwrapped;

template<typename T, typename Initialize>
auto intern(const ConstructorContext& context, Initialize&& initialize)
{
    auto data = runir::kr::dl::grammar::checkout<T>(context.builder());
    std::forward<Initialize>(initialize)(*data);
    return context.repository().insert(*data, context.builder()).first;
}

template<runir::kr::dl::CategoryTag Category>
auto parse(const ast::NonTerminal<Category>& node, tyr::formalism::DomainView, const ConstructorContext& context)
{
    return intern<NonTerminal<Category>>(context, [&](auto& data) { data.name = node.name.text; });
}

template<runir::kr::dl::CategoryTag Category, typename Node>
[[noreturn]] auto reject(const Node& node, const ConstructorContext& context, const char* what)
    -> runir::kr::parser::constructors::GrammarTarget::ParsedConstructor<Category>
{ context.diagnostics().throw_at(node, runir::kr::InvalidExpressionError(std::string(what) + " are not supported in generation grammars.")); }

auto parse(const ast::ConceptRegister& node, tyr::formalism::DomainView, const ConstructorContext& context)
{ return reject<ConceptTag>(node, context, "Registers"); }

auto parse(const ast::RoleRegister& node, tyr::formalism::DomainView, const ConstructorContext& context) { return reject<RoleTag>(node, context, "Registers"); }

auto parse(const ast::ConceptArgument& node, tyr::formalism::DomainView, const ConstructorContext& context)
{ return reject<ConceptTag>(node, context, "Arguments"); }

auto parse(const ast::RoleArgument& node, tyr::formalism::DomainView, const ConstructorContext& context) { return reject<RoleTag>(node, context, "Arguments"); }

auto parse(const ast::BooleanArgument& node, tyr::formalism::DomainView, const ConstructorContext& context)
{ return reject<BooleanTag>(node, context, "Arguments"); }

auto parse(const ast::NumericalArgument& node, tyr::formalism::DomainView, const ConstructorContext& context)
{ return reject<NumericalTag>(node, context, "Arguments"); }

template<typename Tag>
    requires(ComparisonTag<Tag> || LogicalBinaryTag<Tag>)
auto parse(const ast::Constructor<BooleanTag, Tag>& node, tyr::formalism::DomainView, const ConstructorContext& context)
{ return reject<BooleanTag>(node, context, "Boolean comparisons and connectives"); }

auto parse(const ast::BooleanNot& node, tyr::formalism::DomainView, const ConstructorContext& context)
{ return reject<BooleanTag>(node, context, "Boolean comparisons and connectives"); }

auto parse(const ast::BooleanConstant& node, tyr::formalism::DomainView, const ConstructorContext& context)
{ return reject<BooleanTag>(node, context, "Boolean constants"); }

auto parse(const ast::NumericalConstant& node, tyr::formalism::DomainView, const ConstructorContext& context)
{ return reject<NumericalTag>(node, context, "Arithmetic expressions"); }

template<NumericalBinaryTag Tag>
auto parse(const ast::Constructor<NumericalTag, Tag>& node, tyr::formalism::DomainView, const ConstructorContext& context)
{ return reject<NumericalTag>(node, context, "Arithmetic expressions"); }

template<runir::kr::dl::CategoryTag Category>
urd::built_t<ConstructorOrNonTerminal<Category>, ConstructorRepository>
parse(const ast::ConstructorOrNonTerminal<Category>& node, tyr::formalism::DomainView domain, const ConstructorContext& context)
{
    return boost::apply_visitor(
        [&]<typename Arg>(const Arg& arg) -> urd::built_t<ConstructorOrNonTerminal<Category>, ConstructorRepository>
        {
            const auto& unwrapped = unwrap(arg);
            if constexpr (std::same_as<Unwrapped<Arg>, ast::NonTerminal<Category>>)
                return urd::as_member<urd::built_t<ConstructorOrNonTerminal<Category>, ConstructorRepository>>(parse(unwrapped, domain, context));
            else if constexpr (std::same_as<Unwrapped<Arg>, ast::FeatureName>)
                context.diagnostics().throw_at(unwrapped.name, runir::kr::InvalidExpressionError("Feature names are not supported in generation grammars."));
            else
                return parse_constructor(unwrapped, domain, context);
        },
        node.get());
}

runir::kr::parser::constructors::GrammarTarget::ParsedConstructor<ConceptTag>
parse(const ast::ConceptProject& node, tyr::formalism::DomainView, const ConstructorContext& context)
{ context.diagnostics().throw_at(node, runir::kr::InvalidExpressionError("Relational expressions are not supported in generation grammars.")); }

runir::kr::parser::constructors::GrammarTarget::ParsedConstructor<RoleTag>
parse(const ast::RoleProject& node, tyr::formalism::DomainView, const ConstructorContext& context)
{ context.diagnostics().throw_at(node, runir::kr::InvalidExpressionError("Relational expressions are not supported in generation grammars.")); }

template<typename Result>
Result parse_collection_operand(const ast::Query& node, tyr::formalism::DomainView, const ConstructorContext& context)
{ context.diagnostics().throw_at(node, runir::kr::InvalidExpressionError("Relational expressions are not supported in generation grammars.")); }

template<runir::kr::dl::CategoryTag Category>
auto parse(const ast::DerivationRule<Category>& node, tyr::formalism::DomainView domain, const ConstructorContext& context)
{
    return intern<DerivationRule<Category>>(context,
                                            [&](auto& data)
                                            {
                                                for (const auto& symbol : node.rhs)
                                                    data.rhs.push_back(parse(symbol, domain, context));
                                                urd::store(data.lhs, parse(node.lhs, domain, context));
                                            });
}

template<runir::kr::dl::CategoryTag Category>
void append_derivation_rule(std::vector<urd::child<DerivationRule<Category>, ConstructorRepository>>& rules,
                            const ast::DerivationRule<Category>& node,
                            tyr::formalism::DomainView domain,
                            const ConstructorContext& context)
{ urd::append(rules, parse(node, domain, context)); }

void append_derivation_rule(urd::builder<GrammarTag, ConstructorRepository>& data,
                            const ast::DerivationRuleVariant& node,
                            tyr::formalism::DomainView domain,
                            const ConstructorContext& context)
{
    boost::apply_visitor(
        [&]<typename Value>(const Value& value)
        {
            const auto& rule = unwrap(value);
            using Category = typename Unwrapped<Value>::CategoryTag;
            if constexpr (std::same_as<Category, ConceptTag>)
                append_derivation_rule(data.concept_derivation_rules, rule, domain, context);
            else if constexpr (std::same_as<Category, RoleTag>)
                append_derivation_rule(data.role_derivation_rules, rule, domain, context);
            else if constexpr (std::same_as<Category, BooleanTag>)
                append_derivation_rule(data.boolean_derivation_rules, rule, domain, context);
            else if constexpr (std::same_as<Category, NumericalTag>)
                append_derivation_rule(data.numerical_derivation_rules, rule, domain, context);
        },
        node.get());
}

template<runir::kr::dl::CategoryTag Category>
auto fixed_start_name()
{
    if constexpr (std::same_as<Category, ConceptTag>)
        return "c_0";
    else if constexpr (std::same_as<Category, RoleTag>)
        return "r_0";
    else if constexpr (std::same_as<Category, BooleanTag>)
        return "b_0";
    else if constexpr (std::same_as<Category, NumericalTag>)
        return "n_0";
}

template<runir::kr::dl::CategoryTag Category>
auto parse_fixed_start(tyr::formalism::DomainView domain, const ConstructorContext& context)
{
    ast::NonTerminal<Category> start;
    start.name.text = fixed_start_name<Category>();
    return parse(start, domain, context);
}

auto parse_grammar_impl(const std::string& description, tyr::formalism::DomainView domain, ConstructorRepository& repository)
{
    auto errors = std::ostringstream {};
    auto error_handler = runir::kr::parser::ErrorHandlerType(description.cbegin(), description.cend(), errors);
    auto ast = runir::kr::dl::grammar::ast::Grammar {};
    parser::parse_grammar_ast(description, ast, error_handler);
    auto diagnostics = runir::kr::parser::DiagnosticContext {};
    const auto scope = runir::kr::parser::DiagnosticContext::Scope(diagnostics, error_handler);
    auto builder = Builder {};
    const auto context = ConstructorContext { repository, builder, diagnostics };

    auto data = runir::kr::dl::grammar::checkout<GrammarTag>(builder);

    urd::store(data->concept_start, parse_fixed_start<ConceptTag>(domain, context));
    urd::store(data->role_start, parse_fixed_start<RoleTag>(domain, context));
    urd::store(data->boolean_start, parse_fixed_start<BooleanTag>(domain, context));
    urd::store(data->numerical_start, parse_fixed_start<NumericalTag>(domain, context));

    for (const auto& rule : ast.rules)
        append_derivation_rule(*data, rule, domain, context);

    urd::store(data->domain, domain);
    return repository.insert(*data, builder).first;
}

}  // namespace

GrammarView parse_grammar(const std::string& description, tyr::formalism::DomainView domain, ConstructorRepository& repository)
{ return parse_grammar_impl(description, domain, repository); }

}  // namespace runir::kr::dl::grammar
