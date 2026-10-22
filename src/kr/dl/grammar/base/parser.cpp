#include "runir/kr/dl/grammar/parser.hpp"

#include "kr/parser/constructors.hpp"
#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/dl/grammar/canonicalization.hpp"
#include "runir/kr/dl/grammar/constructor_repository.hpp"
#include "runir/kr/dl/grammar/detail/repository_instantiations_base.hpp"
#include "runir/kr/dl/grammar/parser/parser.hpp"
#include "runir/kr/parser/diagnostics.hpp"

#include <boost/variant/apply_visitor.hpp>
#include <sstream>
#include <string>
#include <type_traits>
#include <tyr/formalism/planning/repository.hpp>
#include <utility>

namespace runir::kr::dl::grammar
{
namespace
{

/// Generation grammars produce Base grammar records from the union AST, rejecting what generation does not support.
using GrammarFamily = runir::kr::BaseFamilyTag;
using AstFamily = runir::kr::ExtFamilyTag;

struct ConstructorContext
{
    using Target = runir::kr::parser::constructors::GrammarTarget<GrammarFamily>;

    ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository;
    Builder<runir::kr::BaseFamilyTag>& builder;
    const runir::kr::parser::DiagnosticContext& diagnostics;
};

using runir::kr::parser::constructors::parse;
using runir::kr::parser::constructors::parse_constructor;
using runir::kr::parser::constructors::unwrap;
using runir::kr::parser::constructors::Unwrapped;

template<typename T, typename Initialize>
auto intern(const ConstructorContext& context, Initialize&& initialize)
{
    auto data = runir::kr::dl::grammar::checkout<T>(context.builder);
    std::forward<Initialize>(initialize)(*data);
    return runir::kr::dl::grammar::insert(context.repository, *data).first;
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
auto parse(const ast::NonTerminal<Family, Category>& node, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    return intern<NonTerminal<GrammarFamily, Category>>(context, [&](auto& data) { data.name = node.name.text; });
}

template<runir::kr::dl::CategoryTag Category, typename Node>
[[noreturn]] auto reject(const Node& node, const ConstructorContext& context, const char* what) -> FamilyConstructorView<GrammarFamily, Category>
{
    context.diagnostics.throw_at(node, runir::kr::InvalidExpressionError(std::string(what) + " are not supported in generation grammars."));
}

auto parse(const ast::ConceptRegister<AstFamily>& node, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    return reject<ConceptTag>(node, context, "Registers");
}

auto parse(const ast::RoleRegister<AstFamily>& node, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    return reject<RoleTag>(node, context, "Registers");
}

auto parse(const ast::ConceptArgument<AstFamily>& node, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    return reject<ConceptTag>(node, context, "Arguments");
}

auto parse(const ast::RoleArgument<AstFamily>& node, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    return reject<RoleTag>(node, context, "Arguments");
}

auto parse(const ast::BooleanArgument<AstFamily>& node, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    return reject<BooleanTag>(node, context, "Arguments");
}

auto parse(const ast::NumericalArgument<AstFamily>& node, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    return reject<NumericalTag>(node, context, "Arguments");
}

template<typename Tag>
    requires(ComparisonTag<Tag> || LogicalBinaryTag<Tag>)
auto parse(const ast::Constructor<AstFamily, BooleanTag, Tag>& node, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    return reject<BooleanTag>(node, context, "Boolean comparisons and connectives");
}

auto parse(const ast::BooleanNot<AstFamily>& node, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    return reject<BooleanTag>(node, context, "Boolean comparisons and connectives");
}

auto parse(const ast::BooleanConstant<AstFamily>& node, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    return reject<BooleanTag>(node, context, "Boolean constants");
}

auto parse(const ast::NumericalConstant<AstFamily>& node, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    return reject<NumericalTag>(node, context, "Arithmetic expressions");
}

template<NumericalBinaryTag Tag>
auto parse(const ast::Constructor<AstFamily, NumericalTag, Tag>& node, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    return reject<NumericalTag>(node, context, "Arithmetic expressions");
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
FamilyConstructorOrNonTerminalView<GrammarFamily, Category>
parse(const ast::ConstructorOrNonTerminal<Family, Category>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto index = boost::apply_visitor(
        [&]<typename Arg>(const Arg& arg) -> ygg::Data<ConstructorOrNonTerminal<GrammarFamily, Category>>::Variant
        {
            const auto& unwrapped = unwrap(arg);
            if constexpr (std::same_as<Unwrapped<Arg>, ast::NonTerminal<Family, Category>>)
                return parse(unwrapped, domain, context).get_index();
            else
                return parse_constructor(unwrapped, domain, context).get_index();
        },
        node.get());

    return intern<ConstructorOrNonTerminal<GrammarFamily, Category>>(context, [&](auto& data) { data.variant = index; });
}

template<runir::kr::dl::FamilyTag Family>
FamilyConstructorView<GrammarFamily, ConceptTag>
parse(const ast::ConceptProject<Family>& node, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    context.diagnostics.throw_at(node, runir::kr::InvalidExpressionError("Relational expressions are not supported in generation grammars."));
}

template<runir::kr::dl::FamilyTag Family>
FamilyConstructorView<GrammarFamily, RoleTag>
parse(const ast::RoleProject<Family>& node, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    context.diagnostics.throw_at(node, runir::kr::InvalidExpressionError("Relational expressions are not supported in generation grammars."));
}

template<typename Result, runir::kr::dl::FamilyTag Family>
Result parse_collection_operand(const ast::Query<Family>& node, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    context.diagnostics.throw_at(node, runir::kr::InvalidExpressionError("Relational expressions are not supported in generation grammars."));
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
auto parse(const ast::DerivationRule<Family, Category>& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    return intern<DerivationRule<GrammarFamily, Category>>(context,
                                                           [&](auto& data)
                                                           {
                                                               for (const auto& symbol : node.rhs)
                                                                   data.rhs.push_back(parse(symbol, domain, context).get_index());
                                                               data.lhs = parse(node.lhs, domain, context).get_index();
                                                           });
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
void append_derivation_rule(ygg::IndexList<DerivationRule<GrammarFamily, Category>>& rules,
                            const ast::DerivationRule<Family, Category>& node,
                            tyr::formalism::planning::DomainView domain,
                            const ConstructorContext& context)
{
    rules.push_back(parse(node, domain, context).get_index());
}

template<runir::kr::dl::FamilyTag Family>
void append_derivation_rule(ygg::Data<GrammarTag<GrammarFamily>>& data,
                            const ast::DerivationRuleVariant<Family>& node,
                            tyr::formalism::planning::DomainView domain,
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
auto parse_fixed_start(tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    ast::NonTerminal<AstFamily, Category> start;
    start.name.text = fixed_start_name<Category>();
    return parse(start, domain, context).get_index();
}

auto parse_grammar_impl(const std::string& description,
                        tyr::formalism::planning::DomainView domain,
                        ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository)
{
    auto errors = std::ostringstream {};
    auto error_handler = runir::kr::parser::ErrorHandlerType(description.cbegin(), description.cend(), errors);
    auto ast = runir::kr::dl::grammar::ast::Grammar<AstFamily> {};
    parser::parse_grammar_ast(description, ast, error_handler);
    auto diagnostics = runir::kr::parser::DiagnosticContext {};
    const auto scope = runir::kr::parser::DiagnosticContext::Scope(diagnostics, error_handler);
    auto builder = Builder<runir::kr::BaseFamilyTag> {};
    const auto context = ConstructorContext { repository, builder, diagnostics };

    auto data = runir::kr::dl::grammar::checkout<GrammarTag<runir::kr::BaseFamilyTag>>(builder);

    data->concept_start = parse_fixed_start<ConceptTag>(domain, context);
    data->role_start = parse_fixed_start<RoleTag>(domain, context);
    data->boolean_start = parse_fixed_start<BooleanTag>(domain, context);
    data->numerical_start = parse_fixed_start<NumericalTag>(domain, context);

    for (const auto& rule : ast.rules)
        append_derivation_rule(*data, rule, domain, context);

    data->domain = domain.get_index();
    return runir::kr::dl::grammar::insert(repository, *data).first;
}

}  // namespace

FamilyGrammarView<runir::kr::BaseFamilyTag>
parse_grammar(const std::string& description, tyr::formalism::planning::DomainView domain, ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository)
{
    return parse_grammar_impl(description, domain, repository);
}

}  // namespace runir::kr::dl::grammar
