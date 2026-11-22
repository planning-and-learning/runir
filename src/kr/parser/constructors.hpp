#ifndef RUNIR_SRC_KR_PARSER_CONSTRUCTORS_HPP_
#define RUNIR_SRC_KR_PARSER_CONSTRUCTORS_HPP_

#include "kr/parser/resolution.hpp"
#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/dl/grammar/constructor_repository.hpp"
#include "runir/kr/dl/repository.hpp"

#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <stdexcept>
#include <type_traits>
#include <urd/config.hpp>
#include <urd/elements/children.hpp>
#include <urd/elements/flat.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/formalism/builder.hpp>

namespace runir::kr::parser::constructors
{

struct SemanticTarget
{
    using Family = runir::kr::dl::SemanticFamily;

    template<runir::kr::dl::CategoryTag Category>
    using Constructor = runir::kr::dl::Constructor<Category>;

    template<runir::kr::dl::CategoryTag Category>
    using ParsedConstructor = runir::kr::dl::ConstructorView<Category>;
};

struct GrammarTarget
{
    using Family = runir::kr::dl::grammar::GrammarFamily;

    template<runir::kr::dl::CategoryTag Category>
    using Constructor = runir::kr::dl::grammar::Constructor<Category>;

    /// A grammar constructor is not interned: it is its concrete constructor, read as a child (a builder's field
    /// takes it as it is).
    template<runir::kr::dl::CategoryTag Category>
    using ParsedConstructor = urd::built_t<runir::kr::dl::grammar::Constructor<Category>, runir::kr::dl::grammar::ConstructorRepository>;
};

template<typename Context, typename Tag>
using TargetConcept = runir::kr::dl::Concept<typename Context::Target::Family, Tag>;

template<typename Context, typename Tag>
using TargetRole = runir::kr::dl::Role<typename Context::Target::Family, Tag>;

template<typename Context, typename Tag>
using TargetBoolean = runir::kr::dl::Boolean<typename Context::Target::Family, Tag>;

template<typename Context, typename Tag>
using TargetNumerical = runir::kr::dl::Numerical<typename Context::Target::Family, Tag>;

template<typename T>
const T& unwrap(const T& value URD_LIFETIMEBOUND) noexcept
{ return value; }

template<typename T>
const T& unwrap(const boost::spirit::x3::forward_ast<T>& value) noexcept
{ return value.get(); }

template<typename T>
struct UnwrapTraits
{
    using Type = T;
};

template<typename T>
struct UnwrapTraits<boost::spirit::x3::forward_ast<T>>
{
    using Type = T;
};

/// The type `unwrap` returns a reference to.
template<typename T>
using Unwrapped = typename UnwrapTraits<T>::Type;

template<typename T, typename Context>
auto checkout(const Context& context)
{ return ygg::formalism::checkout<T>(context.builder()); }

/// Writes what a parse gives into a builder's member: a built form (a grammar constructor's) as it is, a view (an
/// interned constructor's) as the member stores it, its key (urd::store).
template<typename M, typename P>
void take(M& member, P&& parsed)
{
    if constexpr (std::is_assignable_v<M&, P&&>)
        member = std::forward<P>(parsed);
    else
        urd::store(member, parsed);
}

template<typename Context, typename T>
auto intern(const Context& context, urd::builder<T, typename Context::Repository>& built)
{ return context.repository().insert(built, context.builder()).first; }

/// A semantic constructor is interned; a grammar constructor is stored as it is.
template<runir::kr::dl::CategoryTag Category, typename Context, typename T>
auto intern_constructor(const Context& context, urd::index<T> index)
{
    using Constructor = typename Context::Target::template Constructor<Category>;
    if constexpr (urd::FlatVariant<Constructor>)
    {
        auto result = typename Context::Target::template ParsedConstructor<Category>();
        take(result, context.repository().view(index));
        return result;
    }
    else
    {
        auto data = checkout<Constructor>(context);
        take(data->variant, context.repository().view(index));
        return intern(context, *data);
    }
}

template<runir::kr::dl::CategoryTag Category, typename Context>
auto parse_constructor(const runir::kr::dl::grammar::ast::Constructor<Category>& node, tyr::formalism::DomainView domain, const Context& context) ->
    typename Context::Target::template ParsedConstructor<Category>;

using QueryView = runir::kr::dl::QueryView<>;

template<typename Tag, typename Context>
auto intern_query(const Context& context, urd::index<runir::kr::dl::Query<Tag>> index)
{
    auto data = checkout<runir::kr::dl::Query<>>(context);
    take(data->variant, context.repository().view(index));
    return intern(context, *data);
}

template<typename Context>
auto parse_column(const runir::kr::dl::grammar::ast::Identifier& name, const Context& context)
{
    auto data = checkout<runir::kr::dl::QueryColumn>(context);
    data->name = name.text;
    return intern(context, *data);
}

template<typename Context>
auto parse_columns(const urd::Store<urd::sequence<runir::kr::dl::grammar::ast::Identifier>>& names, const Context& context)
{
    std::vector<urd::child<runir::kr::dl::QueryColumn, typename Context::Repository>> columns;
    for (const auto& name : names)
        take(columns.emplace_back(), parse_column(name, context));
    return columns;
}

template<typename Context>
QueryView parse(const runir::kr::dl::grammar::ast::Query& node, tyr::formalism::DomainView domain, const Context& context);

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::QueryAtomicState& node, tyr::formalism::DomainView domain, const Context& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                node.columns.size(),
                                                "q_atomic_state",
                                                context.diagnostics(),
                                                [&]<typename T>(T, auto predicate)
                                                {
                                                    auto data = checkout<runir::kr::dl::Query<runir::kr::dl::AtomicStateTag<T>>>(context);
                                                    take(data->predicate, std::move(predicate));
                                                    data->columns = parse_columns(node.columns, context);
                                                    return intern_query(context, intern(context, *data).index());
                                                });
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::QueryAtomicGoal& node, tyr::formalism::DomainView domain, const Context& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                node.columns.size(),
                                                "q_atomic_goal",
                                                context.diagnostics(),
                                                [&]<typename T>(T, auto predicate)
                                                {
                                                    auto data = checkout<runir::kr::dl::Query<runir::kr::dl::AtomicGoalTag<T>>>(context);
                                                    take(data->predicate, std::move(predicate));
                                                    data->polarity = node.polarity;
                                                    data->columns = parse_columns(node.columns, context);
                                                    return intern_query(context, intern(context, *data).index());
                                                });
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::QueryConcept& node, tyr::formalism::DomainView domain, const Context& context)
{
    auto arg = parse(node.operand, domain, context);
    auto data = checkout<runir::kr::dl::Query<runir::kr::dl::QueryConceptTag>>(context);
    take(data->operand, std::move(arg));
    data->columns = parse_columns(node.columns, context);
    return intern_query(context, intern(context, *data).index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::QueryRole& node, tyr::formalism::DomainView domain, const Context& context)
{
    auto arg = parse(node.operand, domain, context);
    auto data = checkout<runir::kr::dl::Query<runir::kr::dl::QueryRoleTag>>(context);
    take(data->operand, std::move(arg));
    data->columns = parse_columns(node.columns, context);
    return intern_query(context, intern(context, *data).index());
}

template<typename Tag, typename Context>
auto parse(const runir::kr::dl::grammar::ast::QueryBinary<Tag>& node, tyr::formalism::DomainView domain, const Context& context)
{
    auto lhs = parse(node.left_operand.get(), domain, context);
    auto rhs = parse(node.right_operand.get(), domain, context);
    auto data = checkout<runir::kr::dl::Query<Tag>>(context);
    take(data->left_operand, std::move(lhs));
    take(data->right_operand, std::move(rhs));
    return intern_query(context, intern(context, *data).index());
}

template<typename Tag, typename Context>
auto parse(const runir::kr::dl::grammar::ast::QueryColumns<Tag>& node, tyr::formalism::DomainView domain, const Context& context)
{
    auto arg = parse(node.operand.get(), domain, context);
    auto data = checkout<runir::kr::dl::Query<Tag>>(context);
    take(data->operand, std::move(arg));
    data->columns = parse_columns(node.columns, context);
    return intern_query(context, intern(context, *data).index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::QueryGenericJoin& node, tyr::formalism::DomainView domain, const Context& context)
{
    auto data = checkout<runir::kr::dl::Query<runir::kr::dl::QueryGenericJoinTag>>(context);
    for (const auto& arg : node.args)
        take(data->args.emplace_back(), parse(arg.get(), domain, context));
    data->variable_order = parse_columns(node.variable_order, context);
    data->output_order = parse_columns(node.output_order, context);
    return intern_query(context, intern(context, *data).index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::QueryEmpty& node, tyr::formalism::DomainView, const Context& context)
{
    auto data = checkout<runir::kr::dl::Query<runir::kr::dl::QueryEmptyTag>>(context);
    data->columns = parse_columns(node.columns, context);
    return intern_query(context, intern(context, *data).index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::QuerySelectEqual& node, tyr::formalism::DomainView domain, const Context& context)
{
    auto arg = parse(node.operand.get(), domain, context);
    auto data = checkout<runir::kr::dl::Query<runir::kr::dl::QuerySelectEqualTag>>(context);
    take(data->operand, std::move(arg));
    take(data->left_column, parse_column(node.left_column, context));
    take(data->right_column, parse_column(node.right_column, context));
    return intern_query(context, intern(context, *data).index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::QuerySelectValue& node, tyr::formalism::DomainView domain, const Context& context)
{
    auto arg = parse(node.operand.get(), domain, context);
    auto object = runir::kr::parser::require_object(domain, node.object_name, context.diagnostics());
    auto data = checkout<runir::kr::dl::Query<runir::kr::dl::QuerySelectValueTag>>(context);
    take(data->operand, std::move(arg));
    take(data->column, parse_column(node.column, context));
    take(data->object, std::move(object));
    return intern_query(context, intern(context, *data).index());
}

template<typename Context>
QueryView parse(const runir::kr::dl::grammar::ast::Query& node, tyr::formalism::DomainView domain, const Context& context)
{
    return boost::apply_visitor(
        [&](const auto& value) -> QueryView
        {
            const auto& concrete = unwrap(value);
            try
            {
                return parse(concrete, domain, context);
            }
            catch (const runir::kr::InvalidExpressionError& error)
            {
                if (error.diagnostic().location)
                    throw;
                context.diagnostics().throw_at(concrete, runir::kr::InvalidExpressionError(error.message()));
            }
            catch (const std::logic_error& error)
            {
                context.diagnostics().throw_at(concrete, runir::kr::InvalidExpressionError(error.what()));
            }
        },
        node.get());
}

/// Parses one alternative of a concept/role/query operand into Result, a builder's member (a variant of children):
/// the alternative its view or built form is.
template<typename Result, typename Node, typename Context>
Result parse_collection_operand(const Node& node, tyr::formalism::DomainView domain, const Context& context)
{
    auto result = Result();
    take(result, parse(node, domain, context));
    return result;
}

template<typename Result, typename Operand, typename Context>
Result parse_operand(const Operand& node, tyr::formalism::DomainView domain, const Context& context)
{
    return boost::apply_visitor([&](const auto& value) { return parse_collection_operand<Result>(unwrap(value), domain, context); }, node.get());
}

template<runir::kr::dl::CategoryTag Category, typename Context>
auto parse(const runir::kr::dl::grammar::ast::Constructor<Category, runir::kr::dl::ProjectTag>& node, tyr::formalism::DomainView domain, const Context& context)
{
    auto arg = parse(node.operand, domain, context);
    auto data = checkout<runir::kr::dl::QueryProjection<Category>>(context);
    take(data->operand, std::move(arg));
    data->columns = parse_columns(node.columns, context);
    try
    {
        return intern_constructor<Category>(context, intern(context, *data).index());
    }
    catch (const std::logic_error& error)
    {
        context.diagnostics().throw_at(node, runir::kr::InvalidExpressionError(error.what()));
    }
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptBot&, tyr::formalism::DomainView, const Context& context)
{
    auto data = checkout<TargetConcept<Context, runir::kr::dl::BotTag>>(context);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptTop&, tyr::formalism::DomainView, const Context& context)
{
    auto data = checkout<TargetConcept<Context, runir::kr::dl::TopTag>>(context);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptAtomicState& node, tyr::formalism::DomainView domain, const Context& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                1,
                                                "ConceptAtomicState",
                                                context.diagnostics(),
                                                [&]<typename T>(T, auto predicate)
                                                {
                                                    auto data = checkout<TargetConcept<Context, runir::kr::dl::AtomicStateTag<T>>>(context);
                                                    take(data->predicate, std::move(predicate));
                                                    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).index());
                                                });
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptAtomicGoal& node, tyr::formalism::DomainView domain, const Context& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                1,
                                                "ConceptAtomicGoal",
                                                context.diagnostics(),
                                                [&]<typename T>(T, auto predicate)
                                                {
                                                    auto data = checkout<TargetConcept<Context, runir::kr::dl::AtomicGoalTag<T>>>(context);
                                                    take(data->predicate, std::move(predicate));
                                                    data->polarity = node.polarity;
                                                    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).index());
                                                });
}

template<typename Tag, typename Ast, typename Context>
auto parse_binary_concept(const Ast& node, tyr::formalism::DomainView domain, const Context& context)
{
    auto left = parse(node.left_operand, domain, context);
    auto right = parse(node.right_operand, domain, context);
    auto data = checkout<TargetConcept<Context, Tag>>(context);
    take(data->left_operand, std::move(left));
    take(data->right_operand, std::move(right));
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).index());
}

template<typename Tag, typename Ast, typename Context>
auto parse_quantification_concept(const Ast& node, tyr::formalism::DomainView domain, const Context& context)
{
    auto role = parse(node.role, domain, context);
    auto qualifier = parse(node.qualifier, domain, context);
    auto data = checkout<TargetConcept<Context, Tag>>(context);
    take(data->role, std::move(role));
    take(data->qualifier, std::move(qualifier));
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptIntersection& node, tyr::formalism::DomainView domain, const Context& context)
{ return parse_binary_concept<runir::kr::dl::IntersectionTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptUnion& node, tyr::formalism::DomainView domain, const Context& context)
{ return parse_binary_concept<runir::kr::dl::UnionTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptValueRestriction& node, tyr::formalism::DomainView domain, const Context& context)
{ return parse_quantification_concept<runir::kr::dl::ValueRestrictionTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptExistentialQuantification& node, tyr::formalism::DomainView domain, const Context& context)
{ return parse_quantification_concept<runir::kr::dl::ExistentialQuantificationTag>(node, domain, context); }

template<typename Tag, typename Ast, typename Context>
auto parse_number_restriction_concept(const Ast& node, tyr::formalism::DomainView domain, const Context& context)
{
    auto role = parse(node.role, domain, context);
    auto data = checkout<TargetConcept<Context, Tag>>(context);
    data->n = node.n;
    take(data->role, std::move(role));
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptAtLeastNumberRestriction& node, tyr::formalism::DomainView domain, const Context& context)
{ return parse_number_restriction_concept<runir::kr::dl::AtLeastNumberRestrictionTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptAtMostNumberRestriction& node, tyr::formalism::DomainView domain, const Context& context)
{ return parse_number_restriction_concept<runir::kr::dl::AtMostNumberRestrictionTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptExactNumberRestriction& node, tyr::formalism::DomainView domain, const Context& context)
{ return parse_number_restriction_concept<runir::kr::dl::ExactNumberRestrictionTag>(node, domain, context); }

template<typename Tag, typename Ast, typename Context>
auto parse_qualified_number_restriction_concept(const Ast& node, tyr::formalism::DomainView domain, const Context& context)
{
    auto role = parse(node.role, domain, context);
    auto qualifier = parse(node.qualifier, domain, context);
    auto data = checkout<TargetConcept<Context, Tag>>(context);
    data->n = node.n;
    take(data->role, std::move(role));
    take(data->qualifier, std::move(qualifier));
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedAtLeastNumberRestriction& node, tyr::formalism::DomainView domain, const Context& context)
{ return parse_qualified_number_restriction_concept<runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedAtMostNumberRestriction& node, tyr::formalism::DomainView domain, const Context& context)
{ return parse_qualified_number_restriction_concept<runir::kr::dl::QualifiedAtMostNumberRestrictionTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedExactNumberRestriction& node, tyr::formalism::DomainView domain, const Context& context)
{ return parse_qualified_number_restriction_concept<runir::kr::dl::QualifiedExactNumberRestrictionTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptRoleValueMap& node, tyr::formalism::DomainView domain, const Context& context)
{ return parse_binary_concept<runir::kr::dl::RoleValueMapTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptAgreement& node, tyr::formalism::DomainView domain, const Context& context)
{ return parse_binary_concept<runir::kr::dl::AgreementTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptRoleFillers& node, tyr::formalism::DomainView domain, const Context& context)
{
    auto role = parse(node.role, domain, context);
    auto data = checkout<TargetConcept<Context, runir::kr::dl::RoleFillersTag>>(context);
    take(data->role, std::move(role));
    runir::kr::parser::append_objects(domain, node.object_names, context.diagnostics(), data->objects);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptOneOf& node, tyr::formalism::DomainView domain, const Context& context)
{
    auto data = checkout<TargetConcept<Context, runir::kr::dl::OneOfTag>>(context);
    runir::kr::parser::append_objects(domain, node.object_names, context.diagnostics(), data->objects);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptNegation& node, tyr::formalism::DomainView domain, const Context& context)
{
    auto operand = parse(node.operand, domain, context);
    auto data = checkout<TargetConcept<Context, runir::kr::dl::NegationTag>>(context);
    take(data->operand, std::move(operand));
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptNominal& node, tyr::formalism::DomainView domain, const Context& context)
{
    auto data = checkout<TargetConcept<Context, runir::kr::dl::NominalTag>>(context);
    take(data->object, runir::kr::parser::require_object(domain, node.object_name, context.diagnostics()));
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleUniversal&, tyr::formalism::DomainView, const Context& context)
{
    auto data = checkout<TargetRole<Context, runir::kr::dl::UniversalTag>>(context);
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleAtomicState& node, tyr::formalism::DomainView domain, const Context& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                2,
                                                "RoleAtomicState",
                                                context.diagnostics(),
                                                [&]<typename T>(T, auto predicate)
                                                {
                                                    auto data = checkout<TargetRole<Context, runir::kr::dl::AtomicStateTag<T>>>(context);
                                                    take(data->predicate, std::move(predicate));
                                                    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).index());
                                                });
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleAtomicGoal& node, tyr::formalism::DomainView domain, const Context& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                2,
                                                "RoleAtomicGoal",
                                                context.diagnostics(),
                                                [&]<typename T>(T, auto predicate)
                                                {
                                                    auto data = checkout<TargetRole<Context, runir::kr::dl::AtomicGoalTag<T>>>(context);
                                                    take(data->predicate, std::move(predicate));
                                                    data->polarity = node.polarity;
                                                    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).index());
                                                });
}

template<typename Tag, typename Ast, typename Context>
auto parse_binary_role(const Ast& node, tyr::formalism::DomainView domain, const Context& context)
{
    auto left = parse(node.left_operand, domain, context);
    auto right = parse(node.right_operand, domain, context);
    auto data = checkout<TargetRole<Context, Tag>>(context);
    take(data->left_operand, std::move(left));
    take(data->right_operand, std::move(right));
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleIntersection& node, tyr::formalism::DomainView domain, const Context& context)
{ return parse_binary_role<runir::kr::dl::IntersectionTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleUnion& node, tyr::formalism::DomainView domain, const Context& context)
{ return parse_binary_role<runir::kr::dl::UnionTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleComposition& node, tyr::formalism::DomainView domain, const Context& context)
{ return parse_binary_role<runir::kr::dl::CompositionTag>(node, domain, context); }

template<typename Tag, typename Ast, typename Context>
auto parse_unary_role(const Ast& node, tyr::formalism::DomainView domain, const Context& context)
{
    auto operand = parse(node.operand, domain, context);
    auto data = checkout<TargetRole<Context, Tag>>(context);
    take(data->operand, std::move(operand));
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleComplement& node, tyr::formalism::DomainView domain, const Context& context)
{ return parse_unary_role<runir::kr::dl::ComplementTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleInverse& node, tyr::formalism::DomainView domain, const Context& context)
{ return parse_unary_role<runir::kr::dl::InverseTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleTransitiveClosure& node, tyr::formalism::DomainView domain, const Context& context)
{ return parse_unary_role<runir::kr::dl::TransitiveClosureTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleReflexiveTransitiveClosure& node, tyr::formalism::DomainView domain, const Context& context)
{ return parse_unary_role<runir::kr::dl::ReflexiveTransitiveClosureTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleRestriction& node, tyr::formalism::DomainView domain, const Context& context)
{
    auto role = parse(node.role, domain, context);
    auto qualifier = parse(node.qualifier, domain, context);
    auto data = checkout<TargetRole<Context, runir::kr::dl::RestrictionTag>>(context);
    take(data->role, std::move(role));
    take(data->qualifier, std::move(qualifier));
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleIdentity& node, tyr::formalism::DomainView domain, const Context& context)
{
    auto operand = parse(node.operand, domain, context);
    auto data = checkout<TargetRole<Context, runir::kr::dl::IdentityTag>>(context);
    take(data->operand, std::move(operand));
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::BooleanAtomicState& node, tyr::formalism::DomainView domain, const Context& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                0,
                                                "BooleanAtomicState",
                                                context.diagnostics(),
                                                [&]<typename T>(T, auto predicate)
                                                {
                                                    auto data = checkout<TargetBoolean<Context, runir::kr::dl::AtomicStateTag<T>>>(context);
                                                    take(data->predicate, std::move(predicate));
                                                    data->polarity = node.polarity;
                                                    return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).index());
                                                });
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::BooleanAtomicGoal& node, tyr::formalism::DomainView domain, const Context& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                0,
                                                "BooleanAtomicGoal",
                                                context.diagnostics(),
                                                [&]<typename T>(T, auto predicate)
                                                {
                                                    auto data = checkout<TargetBoolean<Context, runir::kr::dl::AtomicGoalTag<T>>>(context);
                                                    take(data->predicate, std::move(predicate));
                                                    data->polarity = node.polarity;
                                                    return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).index());
                                                });
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::BooleanNonempty& node, tyr::formalism::DomainView domain, const Context& context)
{
    using Data = urd::data<TargetBoolean<Context, runir::kr::dl::NonemptyTag>>;
    auto operand = parse_operand<urd::built_t<decltype(Data::operand), typename Context::Repository>>(node.operand, domain, context);

    auto data = checkout<TargetBoolean<Context, runir::kr::dl::NonemptyTag>>(context);
    data->operand = std::move(operand);
    return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::NumericalCount& node, tyr::formalism::DomainView domain, const Context& context)
{
    using Data = urd::data<TargetNumerical<Context, runir::kr::dl::CountTag>>;
    auto operand = parse_operand<urd::built_t<decltype(Data::operand), typename Context::Repository>>(node.operand, domain, context);

    auto data = checkout<TargetNumerical<Context, runir::kr::dl::CountTag>>(context);
    data->operand = std::move(operand);
    return intern_constructor<runir::kr::dl::NumericalTag>(context, intern(context, *data).index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::NumericalDistance& node, tyr::formalism::DomainView domain, const Context& context)
{
    using Data = urd::data<TargetNumerical<Context, runir::kr::dl::DistanceTag>>;
    auto sources = parse_operand<urd::built_t<decltype(Data::sources), typename Context::Repository>>(node.sources, domain, context);
    auto edges = parse_operand<urd::built_t<decltype(Data::edges), typename Context::Repository>>(node.edges, domain, context);
    auto targets = parse_operand<urd::built_t<decltype(Data::targets), typename Context::Repository>>(node.targets, domain, context);
    auto data = checkout<TargetNumerical<Context, runir::kr::dl::DistanceTag>>(context);
    data->sources = std::move(sources);
    data->edges = std::move(edges);
    data->targets = std::move(targets);
    return intern_constructor<runir::kr::dl::NumericalTag>(context, intern(context, *data).index());
}

template<typename Tag, typename Context>
    requires(runir::kr::dl::ComparisonTag<Tag> || runir::kr::dl::LogicalBinaryTag<Tag>)
auto parse(const runir::kr::dl::grammar::ast::Constructor<runir::kr::dl::BooleanTag, Tag>& node, tyr::formalism::DomainView domain, const Context& context)
{
    auto left = parse(node.left_operand, domain, context);
    auto right = parse(node.right_operand, domain, context);
    auto data = checkout<TargetBoolean<Context, Tag>>(context);
    take(data->left_operand, std::move(left));
    take(data->right_operand, std::move(right));
    return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).index());
}

template<runir::kr::dl::NumericalBinaryTag Tag, typename Context>
auto parse(const runir::kr::dl::grammar::ast::Constructor<runir::kr::dl::NumericalTag, Tag>& node, tyr::formalism::DomainView domain, const Context& context)
{
    auto left = parse(node.left_operand, domain, context);
    auto right = parse(node.right_operand, domain, context);
    auto data = checkout<TargetNumerical<Context, Tag>>(context);
    take(data->left_operand, std::move(left));
    take(data->right_operand, std::move(right));
    return intern_constructor<runir::kr::dl::NumericalTag>(context, intern(context, *data).index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::BooleanConstant& node, tyr::formalism::DomainView, const Context& context)
{
    auto data = checkout<TargetBoolean<Context, runir::kr::dl::BooleanConstantTag>>(context);
    data->value = node.value;
    return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::NumericalConstant& node, tyr::formalism::DomainView, const Context& context)
{
    auto data = checkout<TargetNumerical<Context, runir::kr::dl::NumericalConstantTag>>(context);
    data->value = node.value;
    return intern_constructor<runir::kr::dl::NumericalTag>(context, intern(context, *data).index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::BooleanNot& node, tyr::formalism::DomainView domain, const Context& context)
{
    auto operand = parse(node.operand, domain, context);
    auto data = checkout<TargetBoolean<Context, runir::kr::dl::NotTag>>(context);
    take(data->operand, std::move(operand));
    return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).index());
}

template<runir::kr::dl::CategoryTag Category, typename Context>
auto parse_constructor(const runir::kr::dl::grammar::ast::Constructor<Category>& node, tyr::formalism::DomainView domain, const Context& context) ->
    typename Context::Target::template ParsedConstructor<Category>
{
    return boost::apply_visitor([&](const auto& arg) { return parse(unwrap(arg), domain, context); }, node.get());
}

}  // namespace runir::kr::parser::constructors

#endif
