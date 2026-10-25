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
#include <vector>
#include <yggdrasil/formalism/builder.hpp>

namespace runir::kr::parser::constructors
{

struct SemanticTarget
{
    template<typename Tag>
    using Concept = runir::kr::dl::Concept<Tag>;

    template<typename Tag>
    using Role = runir::kr::dl::Role<Tag>;

    template<typename Tag>
    using Boolean = runir::kr::dl::Boolean<Tag>;

    template<typename Tag>
    using Numerical = runir::kr::dl::Numerical<Tag>;

    template<runir::kr::dl::CategoryTag Category>
    using Constructor = runir::kr::dl::Constructor<Category>;

    template<runir::kr::dl::CategoryTag Category>
    using ConstructorView = runir::kr::dl::ConstructorView<Category>;
};

struct GrammarTarget
{
    template<typename Tag>
    using Concept = runir::kr::dl::grammar::Concept<Tag>;

    template<typename Tag>
    using Role = runir::kr::dl::grammar::Role<Tag>;

    template<typename Tag>
    using Boolean = runir::kr::dl::grammar::Boolean<Tag>;

    template<typename Tag>
    using Numerical = runir::kr::dl::grammar::Numerical<Tag>;

    template<runir::kr::dl::CategoryTag Category>
    using Constructor = runir::kr::dl::grammar::Constructor<Category>;

    template<runir::kr::dl::CategoryTag Category>
    using ConstructorView = runir::kr::dl::grammar::ConstructorView<Category>;
};

template<typename Context, typename Tag>
using TargetConcept = typename Context::Target::template Concept<Tag>;

template<typename Context, typename Tag>
using TargetRole = typename Context::Target::template Role<Tag>;

template<typename Context, typename Tag>
using TargetBoolean = typename Context::Target::template Boolean<Tag>;

template<typename Context, typename Tag>
using TargetNumerical = typename Context::Target::template Numerical<Tag>;

template<typename T>
const T& unwrap(const T& value) noexcept
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
{ return ygg::formalism::checkout<T>(context.builder); }

template<typename Context, typename T>
auto intern(const Context& context, ygg::Data<T>& data)
{ return insert(context.repository, data).first; }

template<runir::kr::dl::CategoryTag Category, typename Context, typename T>
auto intern_constructor(const Context& context, ygg::Index<T> index)
{
    auto data = checkout<typename Context::Target::template Constructor<Category>>(context);
    data->variant = index;
    return intern(context, *data);
}

template<runir::kr::dl::CategoryTag Category, typename Context>
auto parse_constructor(const runir::kr::dl::grammar::ast::Constructor<Category>& node, tyr::formalism::planning::DomainView domain, const Context& context) ->
    typename Context::Target::template ConstructorView<Category>;

using QueryView = runir::kr::dl::QueryView<>;

template<typename Tag, typename Context>
auto intern_query(const Context& context, ygg::Index<runir::kr::dl::Query<Tag>> index)
{
    auto data = checkout<runir::kr::dl::Query<>>(context);
    data->variant = index;
    return intern(context, *data);
}

template<typename Context>
auto parse_column(const runir::kr::dl::grammar::ast::Identifier& name, const Context& context)
{
    auto data = checkout<runir::kr::dl::QueryColumn>(context);
    data->name = name.text;
    return intern(context, *data).get_index();
}

template<typename Context>
auto parse_columns(const std::vector<runir::kr::dl::grammar::ast::Identifier>& names, const Context& context)
{
    ygg::IndexList<runir::kr::dl::QueryColumn> columns;
    for (const auto& name : names)
        columns.push_back(parse_column(name, context));
    return columns;
}

template<typename Context>
QueryView parse(const runir::kr::dl::grammar::ast::Query& node, tyr::formalism::planning::DomainView domain, const Context& context);

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::QueryAtomicState& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                node.columns.size(),
                                                "q_atomic_state",
                                                context.diagnostics,
                                                [&]<typename T>(T, auto predicate)
                                                {
                                                    auto data = checkout<runir::kr::dl::Query<runir::kr::dl::AtomicStateTag<T>>>(context);
                                                    data->predicate = predicate;
                                                    data->columns = parse_columns(node.columns, context);
                                                    return intern_query(context, intern(context, *data).get_index());
                                                });
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::QueryAtomicGoal& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                node.columns.size(),
                                                "q_atomic_goal",
                                                context.diagnostics,
                                                [&]<typename T>(T, auto predicate)
                                                {
                                                    auto data = checkout<runir::kr::dl::Query<runir::kr::dl::AtomicGoalTag<T>>>(context);
                                                    data->predicate = predicate;
                                                    data->polarity = node.polarity;
                                                    data->columns = parse_columns(node.columns, context);
                                                    return intern_query(context, intern(context, *data).get_index());
                                                });
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::QueryConcept& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto arg = parse(node.arg, domain, context).get_index();
    auto data = checkout<runir::kr::dl::Query<runir::kr::dl::QueryConceptTag>>(context);
    data->arg = arg;
    data->columns = parse_columns(node.columns, context);
    return intern_query(context, intern(context, *data).get_index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::QueryRole& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto arg = parse(node.arg, domain, context).get_index();
    auto data = checkout<runir::kr::dl::Query<runir::kr::dl::QueryRoleTag>>(context);
    data->arg = arg;
    data->columns = parse_columns(node.columns, context);
    return intern_query(context, intern(context, *data).get_index());
}

template<typename Tag, typename Context>
auto parse(const runir::kr::dl::grammar::ast::QueryBinary<Tag>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto lhs = parse(node.lhs.get(), domain, context).get_index();
    const auto rhs = parse(node.rhs.get(), domain, context).get_index();
    auto data = checkout<runir::kr::dl::Query<Tag>>(context);
    data->lhs = lhs;
    data->rhs = rhs;
    return intern_query(context, intern(context, *data).get_index());
}

template<typename Tag, typename Context>
auto parse(const runir::kr::dl::grammar::ast::QueryColumns<Tag>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto arg = parse(node.arg.get(), domain, context).get_index();
    auto data = checkout<runir::kr::dl::Query<Tag>>(context);
    data->arg = arg;
    data->columns = parse_columns(node.columns, context);
    return intern_query(context, intern(context, *data).get_index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::QueryGenericJoin& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    ygg::IndexList<runir::kr::dl::Query<>> args;
    for (const auto& arg : node.args)
        args.push_back(parse(arg.get(), domain, context).get_index());
    auto data = checkout<runir::kr::dl::Query<runir::kr::dl::QueryGenericJoinTag>>(context);
    data->args = std::move(args);
    data->variable_order = parse_columns(node.variable_order, context);
    data->output_order = parse_columns(node.output_order, context);
    return intern_query(context, intern(context, *data).get_index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::QueryEmpty& node, tyr::formalism::planning::DomainView, const Context& context)
{
    auto data = checkout<runir::kr::dl::Query<runir::kr::dl::QueryEmptyTag>>(context);
    data->columns = parse_columns(node.columns, context);
    return intern_query(context, intern(context, *data).get_index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::QueryMustPass& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto sources = parse(node.sources.get(), domain, context).get_index();
    const auto edges = parse(node.edges.get(), domain, context).get_index();
    const auto targets = parse(node.targets.get(), domain, context).get_index();
    const auto probes = parse(node.probes.get(), domain, context).get_index();
    auto data = checkout<runir::kr::dl::Query<runir::kr::dl::QueryMustPassTag>>(context);
    data->keys = parse_columns(node.keys, context);
    data->sources = sources;
    data->edges = edges;
    data->targets = targets;
    data->probes = probes;
    return intern_query(context, intern(context, *data).get_index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::QuerySelectEqual& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto arg = parse(node.arg.get(), domain, context).get_index();
    auto data = checkout<runir::kr::dl::Query<runir::kr::dl::QuerySelectEqualTag>>(context);
    data->arg = arg;
    data->lhs_column = parse_column(node.lhs_column, context);
    data->rhs_column = parse_column(node.rhs_column, context);
    return intern_query(context, intern(context, *data).get_index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::QuerySelectValue& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto arg = parse(node.arg.get(), domain, context).get_index();
    const auto object = runir::kr::parser::require_object(domain, node.object_name, context.diagnostics);
    auto data = checkout<runir::kr::dl::Query<runir::kr::dl::QuerySelectValueTag>>(context);
    data->arg = arg;
    data->column = parse_column(node.column, context);
    data->object = object;
    return intern_query(context, intern(context, *data).get_index());
}

template<typename Context>
QueryView parse(const runir::kr::dl::grammar::ast::Query& node, tyr::formalism::planning::DomainView domain, const Context& context)
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
                context.diagnostics.throw_at(concrete, runir::kr::InvalidExpressionError(error.message()));
            }
            catch (const std::logic_error& error)
            {
                context.diagnostics.throw_at(concrete, runir::kr::InvalidExpressionError(error.what()));
            }
        },
        node.get());
}

/// Parses one alternative of a concept/role/query operand into the member variant Result.
template<typename Result, typename Node, typename Context>
Result parse_collection_operand(const Node& node, tyr::formalism::planning::DomainView domain, const Context& context)
{ return Result(parse(node, domain, context).get_index()); }

template<typename Result, typename Operand, typename Context>
Result parse_operand(const Operand& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return boost::apply_visitor([&](const auto& value) { return parse_collection_operand<Result>(unwrap(value), domain, context); }, node.get());
}

template<runir::kr::dl::CategoryTag Category, typename Context>
auto parse(const runir::kr::dl::grammar::ast::Constructor<Category, runir::kr::dl::ProjectTag>& node,
           tyr::formalism::planning::DomainView domain,
           const Context& context)
{
    const auto arg = parse(node.arg, domain, context).get_index();
    auto data = checkout<runir::kr::dl::QueryProjection<Category>>(context);
    data->arg = arg;
    data->columns = parse_columns(node.columns, context);
    try
    {
        return intern_constructor<Category>(context, intern(context, *data).get_index());
    }
    catch (const std::logic_error& error)
    {
        context.diagnostics.throw_at(node, runir::kr::InvalidExpressionError(error.what()));
    }
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptBot&, tyr::formalism::planning::DomainView, const Context& context)
{
    auto data = checkout<TargetConcept<Context, runir::kr::dl::BotTag>>(context);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptTop&, tyr::formalism::planning::DomainView, const Context& context)
{
    auto data = checkout<TargetConcept<Context, runir::kr::dl::TopTag>>(context);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptAtomicState& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                1,
                                                "ConceptAtomicState",
                                                context.diagnostics,
                                                [&]<typename T>(T, auto predicate)
                                                {
                                                    auto data = checkout<TargetConcept<Context, runir::kr::dl::AtomicStateTag<T>>>(context);
                                                    data->predicate = predicate;
                                                    data->polarity = true;
                                                    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
                                                });
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptAtomicGoal& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                1,
                                                "ConceptAtomicGoal",
                                                context.diagnostics,
                                                [&]<typename T>(T, auto predicate)
                                                {
                                                    auto data = checkout<TargetConcept<Context, runir::kr::dl::AtomicGoalTag<T>>>(context);
                                                    data->predicate = predicate;
                                                    data->polarity = node.polarity;
                                                    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
                                                });
}

template<typename Tag, typename Ast, typename Context>
auto parse_binary_concept(const Ast& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto lhs = parse(node.lhs, domain, context);
    const auto rhs = parse(node.rhs, domain, context);
    auto data = checkout<TargetConcept<Context, Tag>>(context);
    data->lhs = lhs.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptIntersection& node, tyr::formalism::planning::DomainView domain, const Context& context)
{ return parse_binary_concept<runir::kr::dl::IntersectionTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptUnion& node, tyr::formalism::planning::DomainView domain, const Context& context)
{ return parse_binary_concept<runir::kr::dl::UnionTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptValueRestriction& node, tyr::formalism::planning::DomainView domain, const Context& context)
{ return parse_binary_concept<runir::kr::dl::ValueRestrictionTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptExistentialQuantification& node, tyr::formalism::planning::DomainView domain, const Context& context)
{ return parse_binary_concept<runir::kr::dl::ExistentialQuantificationTag>(node, domain, context); }

template<typename Tag, typename Ast, typename Context>
auto parse_number_restriction_concept(const Ast& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto role = parse(node.role, domain, context);
    auto data = checkout<TargetConcept<Context, Tag>>(context);
    data->n = node.n;
    data->role = role.get_index();
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptAtLeastNumberRestriction& node, tyr::formalism::planning::DomainView domain, const Context& context)
{ return parse_number_restriction_concept<runir::kr::dl::AtLeastNumberRestrictionTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptAtMostNumberRestriction& node, tyr::formalism::planning::DomainView domain, const Context& context)
{ return parse_number_restriction_concept<runir::kr::dl::AtMostNumberRestrictionTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptExactNumberRestriction& node, tyr::formalism::planning::DomainView domain, const Context& context)
{ return parse_number_restriction_concept<runir::kr::dl::ExactNumberRestrictionTag>(node, domain, context); }

template<typename Tag, typename Ast, typename Context>
auto parse_qualified_number_restriction_concept(const Ast& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto role = parse(node.role, domain, context);
    const auto concept_view = parse(node.concept_, domain, context);
    auto data = checkout<TargetConcept<Context, Tag>>(context);
    data->n = node.n;
    data->role = role.get_index();
    data->concept_ = concept_view.get_index();
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedAtLeastNumberRestriction& node,
           tyr::formalism::planning::DomainView domain,
           const Context& context)
{ return parse_qualified_number_restriction_concept<runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedAtMostNumberRestriction& node,
           tyr::formalism::planning::DomainView domain,
           const Context& context)
{ return parse_qualified_number_restriction_concept<runir::kr::dl::QualifiedAtMostNumberRestrictionTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedExactNumberRestriction& node, tyr::formalism::planning::DomainView domain, const Context& context)
{ return parse_qualified_number_restriction_concept<runir::kr::dl::QualifiedExactNumberRestrictionTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptRoleValueMap& node, tyr::formalism::planning::DomainView domain, const Context& context)
{ return parse_binary_concept<runir::kr::dl::RoleValueMapTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptAgreement& node, tyr::formalism::planning::DomainView domain, const Context& context)
{ return parse_binary_concept<runir::kr::dl::AgreementTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptRoleFillers& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto role = parse(node.role, domain, context);
    auto data = checkout<TargetConcept<Context, runir::kr::dl::RoleFillersTag>>(context);
    data->role = role.get_index();
    runir::kr::parser::append_objects(domain, node.object_names, context.diagnostics, data->objects);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptOneOf& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    auto data = checkout<TargetConcept<Context, runir::kr::dl::OneOfTag>>(context);
    runir::kr::parser::append_objects(domain, node.object_names, context.diagnostics, data->objects);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptNegation& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto arg = parse(node.arg, domain, context);
    auto data = checkout<TargetConcept<Context, runir::kr::dl::NegationTag>>(context);
    data->arg = arg.get_index();
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptNominal& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    auto data = checkout<TargetConcept<Context, runir::kr::dl::NominalTag>>(context);
    data->object = runir::kr::parser::require_object(domain, node.object_name, context.diagnostics);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleUniversal&, tyr::formalism::planning::DomainView, const Context& context)
{
    auto data = checkout<TargetRole<Context, runir::kr::dl::UniversalTag>>(context);
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleAtomicState& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                2,
                                                "RoleAtomicState",
                                                context.diagnostics,
                                                [&]<typename T>(T, auto predicate)
                                                {
                                                    auto data = checkout<TargetRole<Context, runir::kr::dl::AtomicStateTag<T>>>(context);
                                                    data->predicate = predicate;
                                                    data->polarity = true;
                                                    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
                                                });
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleAtomicGoal& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                2,
                                                "RoleAtomicGoal",
                                                context.diagnostics,
                                                [&]<typename T>(T, auto predicate)
                                                {
                                                    auto data = checkout<TargetRole<Context, runir::kr::dl::AtomicGoalTag<T>>>(context);
                                                    data->predicate = predicate;
                                                    data->polarity = node.polarity;
                                                    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
                                                });
}

template<typename Tag, typename Ast, typename Context>
auto parse_binary_role(const Ast& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto lhs = parse(node.lhs, domain, context);
    const auto rhs = parse(node.rhs, domain, context);
    auto data = checkout<TargetRole<Context, Tag>>(context);
    data->lhs = lhs.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleIntersection& node, tyr::formalism::planning::DomainView domain, const Context& context)
{ return parse_binary_role<runir::kr::dl::IntersectionTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleUnion& node, tyr::formalism::planning::DomainView domain, const Context& context)
{ return parse_binary_role<runir::kr::dl::UnionTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleComposition& node, tyr::formalism::planning::DomainView domain, const Context& context)
{ return parse_binary_role<runir::kr::dl::CompositionTag>(node, domain, context); }

template<typename Tag, typename Ast, typename Context>
auto parse_unary_role(const Ast& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto arg = parse(node.arg, domain, context);
    auto data = checkout<TargetRole<Context, Tag>>(context);
    data->arg = arg.get_index();
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleComplement& node, tyr::formalism::planning::DomainView domain, const Context& context)
{ return parse_unary_role<runir::kr::dl::ComplementTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleInverse& node, tyr::formalism::planning::DomainView domain, const Context& context)
{ return parse_unary_role<runir::kr::dl::InverseTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleTransitiveClosure& node, tyr::formalism::planning::DomainView domain, const Context& context)
{ return parse_unary_role<runir::kr::dl::TransitiveClosureTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleReflexiveTransitiveClosure& node, tyr::formalism::planning::DomainView domain, const Context& context)
{ return parse_unary_role<runir::kr::dl::ReflexiveTransitiveClosureTag>(node, domain, context); }

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleRestriction& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto lhs = parse(node.lhs, domain, context);
    const auto rhs = parse(node.rhs, domain, context);
    auto data = checkout<TargetRole<Context, runir::kr::dl::RestrictionTag>>(context);
    data->lhs = lhs.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleIdentity& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto arg = parse(node.arg, domain, context);
    auto data = checkout<TargetRole<Context, runir::kr::dl::IdentityTag>>(context);
    data->arg = arg.get_index();
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::BooleanAtomicState& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                0,
                                                "BooleanAtomicState",
                                                context.diagnostics,
                                                [&]<typename T>(T, auto predicate)
                                                {
                                                    auto data = checkout<TargetBoolean<Context, runir::kr::dl::AtomicStateTag<T>>>(context);
                                                    data->predicate = predicate;
                                                    data->polarity = node.polarity;
                                                    return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).get_index());
                                                });
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::BooleanAtomicGoal& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                0,
                                                "BooleanAtomicGoal",
                                                context.diagnostics,
                                                [&]<typename T>(T, auto predicate)
                                                {
                                                    auto data = checkout<TargetBoolean<Context, runir::kr::dl::AtomicGoalTag<T>>>(context);
                                                    data->predicate = predicate;
                                                    data->polarity = node.polarity;
                                                    return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).get_index());
                                                });
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::BooleanNonempty& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    using Data = ygg::Data<TargetBoolean<Context, runir::kr::dl::NonemptyTag>>;
    const auto arg = parse_operand<typename Data::ConstructorVariant>(node.arg, domain, context);

    auto data = checkout<TargetBoolean<Context, runir::kr::dl::NonemptyTag>>(context);
    data->arg = arg;
    return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).get_index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::NumericalCount& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    using Data = ygg::Data<TargetNumerical<Context, runir::kr::dl::CountTag>>;
    const auto arg = parse_operand<typename Data::ConstructorVariant>(node.arg, domain, context);

    auto data = checkout<TargetNumerical<Context, runir::kr::dl::CountTag>>(context);
    data->arg = arg;
    return intern_constructor<runir::kr::dl::NumericalTag>(context, intern(context, *data).get_index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::NumericalDistance& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    using Data = ygg::Data<TargetNumerical<Context, runir::kr::dl::DistanceTag>>;
    const auto lhs = parse_operand<typename Data::Vertex>(node.lhs, domain, context);
    const auto mid = parse_operand<typename Data::Edge>(node.mid, domain, context);
    const auto rhs = parse_operand<typename Data::Vertex>(node.rhs, domain, context);
    auto data = checkout<TargetNumerical<Context, runir::kr::dl::DistanceTag>>(context);
    data->lhs = lhs;
    data->mid = mid;
    data->rhs = rhs;
    return intern_constructor<runir::kr::dl::NumericalTag>(context, intern(context, *data).get_index());
}

template<typename Tag, typename Context>
    requires(runir::kr::dl::ComparisonTag<Tag> || runir::kr::dl::LogicalBinaryTag<Tag>)
auto parse(const runir::kr::dl::grammar::ast::Constructor<runir::kr::dl::BooleanTag, Tag>& node,
           tyr::formalism::planning::DomainView domain,
           const Context& context)
{
    const auto lhs = parse(node.lhs, domain, context).get_index();
    const auto rhs = parse(node.rhs, domain, context).get_index();
    auto data = checkout<TargetBoolean<Context, Tag>>(context);
    data->lhs = lhs;
    data->rhs = rhs;
    return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).get_index());
}

template<runir::kr::dl::NumericalBinaryTag Tag, typename Context>
auto parse(const runir::kr::dl::grammar::ast::Constructor<runir::kr::dl::NumericalTag, Tag>& node,
           tyr::formalism::planning::DomainView domain,
           const Context& context)
{
    const auto lhs = parse(node.lhs, domain, context).get_index();
    const auto rhs = parse(node.rhs, domain, context).get_index();
    auto data = checkout<TargetNumerical<Context, Tag>>(context);
    data->lhs = lhs;
    data->rhs = rhs;
    return intern_constructor<runir::kr::dl::NumericalTag>(context, intern(context, *data).get_index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::BooleanConstant& node, tyr::formalism::planning::DomainView, const Context& context)
{
    auto data = checkout<TargetBoolean<Context, runir::kr::dl::BooleanConstantTag>>(context);
    data->identifier = node.value;
    return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).get_index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::NumericalConstant& node, tyr::formalism::planning::DomainView, const Context& context)
{
    auto data = checkout<TargetNumerical<Context, runir::kr::dl::NumericalConstantTag>>(context);
    data->identifier = node.value;
    return intern_constructor<runir::kr::dl::NumericalTag>(context, intern(context, *data).get_index());
}

template<typename Context>
auto parse(const runir::kr::dl::grammar::ast::BooleanNot& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto arg = parse(node.arg, domain, context).get_index();
    auto data = checkout<TargetBoolean<Context, runir::kr::dl::NotTag>>(context);
    data->arg = arg;
    return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).get_index());
}

template<runir::kr::dl::CategoryTag Category, typename Context>
auto parse_constructor(const runir::kr::dl::grammar::ast::Constructor<Category>& node, tyr::formalism::planning::DomainView domain, const Context& context) ->
    typename Context::Target::template ConstructorView<Category>
{
    return boost::apply_visitor([&](const auto& arg) { return parse(unwrap(arg), domain, context); }, node.get());
}

}  // namespace runir::kr::parser::constructors

#endif
