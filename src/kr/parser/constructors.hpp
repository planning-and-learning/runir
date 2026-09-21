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

namespace runir::kr::parser::constructors
{

template<runir::kr::dl::FamilyTag Family>
struct SemanticTarget
{
    template<typename Tag>
    using Concept = runir::kr::dl::Concept<Family, Tag>;

    template<typename Tag>
    using Role = runir::kr::dl::Role<Family, Tag>;

    template<typename Tag>
    using Boolean = runir::kr::dl::Boolean<Family, Tag>;

    template<typename Tag>
    using Numerical = runir::kr::dl::Numerical<Family, Tag>;

    template<runir::kr::dl::CategoryTag Category>
    using Constructor = runir::kr::dl::Constructor<Family, Category>;

    template<runir::kr::dl::CategoryTag Category>
    using ConstructorView = runir::kr::dl::FamilyConstructorView<Family, Category>;
};

template<runir::kr::dl::FamilyTag Family>
struct GrammarTarget
{
    template<typename Tag>
    using Concept = runir::kr::dl::grammar::Concept<Family, Tag>;

    template<typename Tag>
    using Role = runir::kr::dl::grammar::Role<Family, Tag>;

    template<typename Tag>
    using Boolean = runir::kr::dl::grammar::Boolean<Family, Tag>;

    template<typename Tag>
    using Numerical = runir::kr::dl::grammar::Numerical<Family, Tag>;

    template<runir::kr::dl::CategoryTag Category>
    using Constructor = runir::kr::dl::grammar::Constructor<Family, Category>;

    template<runir::kr::dl::CategoryTag Category>
    using ConstructorView = runir::kr::dl::grammar::FamilyConstructorView<Family, Category>;
};

template<typename T>
const T& unwrap(const T& value) noexcept
{
    return value;
}

template<typename T>
const T& unwrap(const boost::spirit::x3::forward_ast<T>& value) noexcept
{
    return value.get();
}

template<typename T, typename Context>
auto checkout(const Context& context)
{
    auto data = context.builder.template get_builder<T>();
    data->clear();
    return data;
}

template<typename Context, typename T>
auto intern(const Context& context, ygg::Data<T>& data)
{
    return get_or_create(context.repository, data).first;
}

template<runir::kr::dl::CategoryTag Category, typename Context, typename T>
auto intern_constructor(const Context& context, ygg::Index<T> index)
{
    auto data = checkout<typename Context::Target::template Constructor<Category>>(context);
    data->variant = index;
    return intern(context, *data);
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename Context>
auto parse_constructor(const runir::kr::dl::grammar::ast::Constructor<Family, Category>& node,
                       tyr::formalism::planning::DomainView domain,
                       const Context& context) -> typename Context::Target::template ConstructorView<Category>;

template<runir::kr::dl::FamilyTag Family>
using QueryView = ygg::View<ygg::Index<runir::kr::dl::Query<Family>>, runir::kr::dl::ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::FamilyTag Family, typename Tag, typename Context>
auto intern_query(const Context& context, ygg::Index<runir::kr::dl::Query<Family, Tag>> index)
{
    auto data = checkout<runir::kr::dl::Query<Family>>(context);
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

template<runir::kr::dl::FamilyTag Family, typename Context>
QueryView<Family> parse(const runir::kr::dl::grammar::ast::Query<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context);

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::QueryAtomicState<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                node.columns.size(),
                                                "q_atomic_state",
                                                context.diagnostics,
                                                [&](auto tag, auto predicate)
                                                {
                                                    auto data = checkout<runir::kr::dl::Query<Family, runir::kr::dl::AtomicStateTag<decltype(tag)>>>(context);
                                                    data->predicate = predicate;
                                                    data->columns = parse_columns(node.columns, context);
                                                    return intern_query(context, intern(context, *data).get_index());
                                                });
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::QueryAtomicGoal<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                node.columns.size(),
                                                "q_atomic_goal",
                                                context.diagnostics,
                                                [&](auto tag, auto predicate)
                                                {
                                                    auto data = checkout<runir::kr::dl::Query<Family, runir::kr::dl::AtomicGoalTag<decltype(tag)>>>(context);
                                                    data->predicate = predicate;
                                                    data->polarity = node.polarity;
                                                    data->columns = parse_columns(node.columns, context);
                                                    return intern_query(context, intern(context, *data).get_index());
                                                });
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::QueryConcept<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto arg = parse(node.arg, domain, context).get_index();
    auto data = checkout<runir::kr::dl::Query<Family, runir::kr::dl::QueryConceptTag>>(context);
    data->arg = arg;
    data->columns = parse_columns(node.columns, context);
    return intern_query(context, intern(context, *data).get_index());
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::QueryRole<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto arg = parse(node.arg, domain, context).get_index();
    auto data = checkout<runir::kr::dl::Query<Family, runir::kr::dl::QueryRoleTag>>(context);
    data->arg = arg;
    data->columns = parse_columns(node.columns, context);
    return intern_query(context, intern(context, *data).get_index());
}

template<runir::kr::dl::FamilyTag Family, typename Tag, typename Context>
auto parse(const runir::kr::dl::grammar::ast::QueryBinary<Family, Tag>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto lhs = parse(node.lhs.get(), domain, context).get_index();
    const auto rhs = parse(node.rhs.get(), domain, context).get_index();
    auto data = checkout<runir::kr::dl::Query<Family, Tag>>(context);
    data->lhs = lhs;
    data->rhs = rhs;
    return intern_query(context, intern(context, *data).get_index());
}

template<runir::kr::dl::FamilyTag Family, typename Tag, typename Context>
auto parse(const runir::kr::dl::grammar::ast::QueryColumns<Family, Tag>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto arg = parse(node.arg.get(), domain, context).get_index();
    auto data = checkout<runir::kr::dl::Query<Family, Tag>>(context);
    data->arg = arg;
    data->columns = parse_columns(node.columns, context);
    return intern_query(context, intern(context, *data).get_index());
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::QuerySelectEqual<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto arg = parse(node.arg.get(), domain, context).get_index();
    auto data = checkout<runir::kr::dl::Query<Family, runir::kr::dl::QuerySelectEqualTag>>(context);
    data->arg = arg;
    data->lhs_column = parse_column(node.lhs_column, context);
    data->rhs_column = parse_column(node.rhs_column, context);
    return intern_query(context, intern(context, *data).get_index());
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::QuerySelectValue<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto arg = parse(node.arg.get(), domain, context).get_index();
    const auto object = runir::kr::parser::require_object(domain, node.object_name, context.diagnostics);
    auto data = checkout<runir::kr::dl::Query<Family, runir::kr::dl::QuerySelectValueTag>>(context);
    data->arg = arg;
    data->column = parse_column(node.column, context);
    data->object = object;
    return intern_query(context, intern(context, *data).get_index());
}

template<runir::kr::dl::FamilyTag Family, typename Context>
QueryView<Family> parse(const runir::kr::dl::grammar::ast::Query<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return boost::apply_visitor(
        [&](const auto& value) -> QueryView<Family>
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

template<typename Node, typename Context>
auto parse_collection_operand(const Node& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return parse(node, domain, context).get_index();
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename Context>
auto parse(const runir::kr::dl::grammar::ast::Constructor<Family, Category, runir::kr::dl::ProjectTag>& node,
           tyr::formalism::planning::DomainView domain,
           const Context& context)
{
    const auto arg = parse(node.arg, domain, context).get_index();
    auto data = checkout<runir::kr::dl::QueryProjection<Family, Category>>(context);
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

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptBot<Family>&, tyr::formalism::planning::DomainView, const Context& context)
{
    auto data = checkout<typename Context::Target::template Concept<runir::kr::dl::BotTag>>(context);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptTop<Family>&, tyr::formalism::planning::DomainView, const Context& context)
{
    auto data = checkout<typename Context::Target::template Concept<runir::kr::dl::TopTag>>(context);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptAtomicState<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                1,
                                                "ConceptAtomicState",
                                                context.diagnostics,
                                                [&](auto tag, auto predicate)
                                                {
                                                    using T = decltype(tag);
                                                    auto data = checkout<typename Context::Target::template Concept<runir::kr::dl::AtomicStateTag<T>>>(context);
                                                    data->predicate = predicate;
                                                    data->polarity = true;
                                                    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
                                                });
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptAtomicGoal<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                1,
                                                "ConceptAtomicGoal",
                                                context.diagnostics,
                                                [&](auto tag, auto predicate)
                                                {
                                                    using T = decltype(tag);
                                                    auto data = checkout<typename Context::Target::template Concept<runir::kr::dl::AtomicGoalTag<T>>>(context);
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
    auto data = checkout<typename Context::Target::template Concept<Tag>>(context);
    data->lhs = lhs.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptIntersection<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return parse_binary_concept<runir::kr::dl::IntersectionTag>(node, domain, context);
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptUnion<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return parse_binary_concept<runir::kr::dl::UnionTag>(node, domain, context);
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptValueRestriction<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return parse_binary_concept<runir::kr::dl::ValueRestrictionTag>(node, domain, context);
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptExistentialQuantification<Family>& node,
           tyr::formalism::planning::DomainView domain,
           const Context& context)
{
    return parse_binary_concept<runir::kr::dl::ExistentialQuantificationTag>(node, domain, context);
}

template<typename Tag, typename Ast, typename Context>
auto parse_number_restriction_concept(const Ast& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto role = parse(node.role, domain, context);
    auto data = checkout<typename Context::Target::template Concept<Tag>>(context);
    data->n = node.n;
    data->role = role.get_index();
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptAtLeastNumberRestriction<Family>& node,
           tyr::formalism::planning::DomainView domain,
           const Context& context)
{
    return parse_number_restriction_concept<runir::kr::dl::AtLeastNumberRestrictionTag>(node, domain, context);
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptAtMostNumberRestriction<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return parse_number_restriction_concept<runir::kr::dl::AtMostNumberRestrictionTag>(node, domain, context);
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptExactNumberRestriction<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return parse_number_restriction_concept<runir::kr::dl::ExactNumberRestrictionTag>(node, domain, context);
}

template<typename Tag, typename Ast, typename Context>
auto parse_qualified_number_restriction_concept(const Ast& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto role = parse(node.role, domain, context);
    const auto concept_view = parse(node.concept_, domain, context);
    auto data = checkout<typename Context::Target::template Concept<Tag>>(context);
    data->n = node.n;
    data->role = role.get_index();
    data->concept_ = concept_view.get_index();
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedAtLeastNumberRestriction<Family>& node,
           tyr::formalism::planning::DomainView domain,
           const Context& context)
{
    return parse_qualified_number_restriction_concept<runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>(node, domain, context);
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedAtMostNumberRestriction<Family>& node,
           tyr::formalism::planning::DomainView domain,
           const Context& context)
{
    return parse_qualified_number_restriction_concept<runir::kr::dl::QualifiedAtMostNumberRestrictionTag>(node, domain, context);
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedExactNumberRestriction<Family>& node,
           tyr::formalism::planning::DomainView domain,
           const Context& context)
{
    return parse_qualified_number_restriction_concept<runir::kr::dl::QualifiedExactNumberRestrictionTag>(node, domain, context);
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptRoleValueMap<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return parse_binary_concept<runir::kr::dl::RoleValueMapTag>(node, domain, context);
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptAgreement<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return parse_binary_concept<runir::kr::dl::AgreementTag>(node, domain, context);
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptRoleFillers<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto role = parse(node.role, domain, context);
    auto data = checkout<typename Context::Target::template Concept<runir::kr::dl::RoleFillersTag>>(context);
    data->role = role.get_index();
    runir::kr::parser::append_objects(domain, node.object_names, context.diagnostics, data->objects);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptOneOf<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    auto data = checkout<typename Context::Target::template Concept<runir::kr::dl::OneOfTag>>(context);
    runir::kr::parser::append_objects(domain, node.object_names, context.diagnostics, data->objects);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptNegation<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto arg = parse(node.arg, domain, context);
    auto data = checkout<typename Context::Target::template Concept<runir::kr::dl::NegationTag>>(context);
    data->arg = arg.get_index();
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::ConceptNominal<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    auto data = checkout<typename Context::Target::template Concept<runir::kr::dl::NominalTag>>(context);
    data->object = runir::kr::parser::require_object(domain, node.object_name, context.diagnostics);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleUniversal<Family>&, tyr::formalism::planning::DomainView, const Context& context)
{
    auto data = checkout<typename Context::Target::template Role<runir::kr::dl::UniversalTag>>(context);
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleAtomicState<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                2,
                                                "RoleAtomicState",
                                                context.diagnostics,
                                                [&](auto tag, auto predicate)
                                                {
                                                    using T = decltype(tag);
                                                    auto data = checkout<typename Context::Target::template Role<runir::kr::dl::AtomicStateTag<T>>>(context);
                                                    data->predicate = predicate;
                                                    data->polarity = true;
                                                    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
                                                });
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleAtomicGoal<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                2,
                                                "RoleAtomicGoal",
                                                context.diagnostics,
                                                [&](auto tag, auto predicate)
                                                {
                                                    using T = decltype(tag);
                                                    auto data = checkout<typename Context::Target::template Role<runir::kr::dl::AtomicGoalTag<T>>>(context);
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
    auto data = checkout<typename Context::Target::template Role<Tag>>(context);
    data->lhs = lhs.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleIntersection<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return parse_binary_role<runir::kr::dl::IntersectionTag>(node, domain, context);
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleUnion<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return parse_binary_role<runir::kr::dl::UnionTag>(node, domain, context);
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleComposition<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return parse_binary_role<runir::kr::dl::CompositionTag>(node, domain, context);
}

template<typename Tag, typename Ast, typename Context>
auto parse_unary_role(const Ast& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto arg = parse(node.arg, domain, context);
    auto data = checkout<typename Context::Target::template Role<Tag>>(context);
    data->arg = arg.get_index();
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleComplement<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return parse_unary_role<runir::kr::dl::ComplementTag>(node, domain, context);
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleInverse<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return parse_unary_role<runir::kr::dl::InverseTag>(node, domain, context);
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleTransitiveClosure<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return parse_unary_role<runir::kr::dl::TransitiveClosureTag>(node, domain, context);
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleReflexiveTransitiveClosure<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return parse_unary_role<runir::kr::dl::ReflexiveTransitiveClosureTag>(node, domain, context);
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleRestriction<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto lhs = parse(node.lhs, domain, context);
    const auto rhs = parse(node.rhs, domain, context);
    auto data = checkout<typename Context::Target::template Role<runir::kr::dl::RestrictionTag>>(context);
    data->lhs = lhs.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::RoleIdentity<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto arg = parse(node.arg, domain, context);
    auto data = checkout<typename Context::Target::template Role<runir::kr::dl::IdentityTag>>(context);
    data->arg = arg.get_index();
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::BooleanAtomicState<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                0,
                                                "BooleanAtomicState",
                                                context.diagnostics,
                                                [&](auto tag, auto predicate)
                                                {
                                                    using T = decltype(tag);
                                                    auto data = checkout<typename Context::Target::template Boolean<runir::kr::dl::AtomicStateTag<T>>>(context);
                                                    data->predicate = predicate;
                                                    data->polarity = node.polarity;
                                                    return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).get_index());
                                                });
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::BooleanAtomicGoal<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    return runir::kr::parser::resolve_predicate(domain,
                                                node.predicate_name,
                                                0,
                                                "BooleanAtomicGoal",
                                                context.diagnostics,
                                                [&](auto tag, auto predicate)
                                                {
                                                    using T = decltype(tag);
                                                    auto data = checkout<typename Context::Target::template Boolean<runir::kr::dl::AtomicGoalTag<T>>>(context);
                                                    data->predicate = predicate;
                                                    data->polarity = node.polarity;
                                                    return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).get_index());
                                                });
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::BooleanNonempty<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    using Data = ygg::Data<typename Context::Target::template Boolean<runir::kr::dl::NonemptyTag>>;
    const auto arg = boost::apply_visitor(
        [&](const auto& value) -> typename Data::ConstructorVariant { return parse_collection_operand(unwrap(value), domain, context); },
        node.arg.get());

    auto data = checkout<typename Context::Target::template Boolean<runir::kr::dl::NonemptyTag>>(context);
    data->arg = arg;
    return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).get_index());
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::NumericalCount<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    using Data = ygg::Data<typename Context::Target::template Numerical<runir::kr::dl::CountTag>>;
    const auto arg = boost::apply_visitor(
        [&](const auto& value) -> typename Data::ConstructorVariant { return parse_collection_operand(unwrap(value), domain, context); },
        node.arg.get());

    auto data = checkout<typename Context::Target::template Numerical<runir::kr::dl::CountTag>>(context);
    data->arg = arg;
    return intern_constructor<runir::kr::dl::NumericalTag>(context, intern(context, *data).get_index());
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::NumericalDistance<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto lhs = parse(node.lhs, domain, context);
    const auto mid = parse(node.mid, domain, context);
    const auto rhs = parse(node.rhs, domain, context);
    auto data = checkout<typename Context::Target::template Numerical<runir::kr::dl::DistanceTag>>(context);
    data->lhs = lhs.get_index();
    data->mid = mid.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::NumericalTag>(context, intern(context, *data).get_index());
}

template<runir::kr::dl::FamilyTag Family, typename Tag, typename Context>
    requires(runir::kr::dl::ComparisonTag<Tag> || runir::kr::dl::LogicalBinaryTag<Tag>)
auto parse(const runir::kr::dl::grammar::ast::Constructor<Family, runir::kr::dl::BooleanTag, Tag>& node,
           tyr::formalism::planning::DomainView domain,
           const Context& context)
{
    const auto lhs = parse(node.lhs, domain, context).get_index();
    const auto rhs = parse(node.rhs, domain, context).get_index();
    auto data = checkout<typename Context::Target::template Boolean<Tag>>(context);
    data->lhs = lhs;
    data->rhs = rhs;
    return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).get_index());
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::NumericalBinaryTag Tag, typename Context>
auto parse(const runir::kr::dl::grammar::ast::Constructor<Family, runir::kr::dl::NumericalTag, Tag>& node,
           tyr::formalism::planning::DomainView domain,
           const Context& context)
{
    const auto lhs = parse(node.lhs, domain, context).get_index();
    const auto rhs = parse(node.rhs, domain, context).get_index();
    auto data = checkout<typename Context::Target::template Numerical<Tag>>(context);
    data->lhs = lhs;
    data->rhs = rhs;
    return intern_constructor<runir::kr::dl::NumericalTag>(context, intern(context, *data).get_index());
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::BooleanConstant<Family>& node, tyr::formalism::planning::DomainView, const Context& context)
{
    auto data = checkout<typename Context::Target::template Boolean<runir::kr::dl::BooleanConstantTag>>(context);
    data->identifier = node.value;
    return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).get_index());
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::NumericalConstant<Family>& node, tyr::formalism::planning::DomainView, const Context& context)
{
    auto data = checkout<typename Context::Target::template Numerical<runir::kr::dl::NumericalConstantTag>>(context);
    data->identifier = node.value;
    return intern_constructor<runir::kr::dl::NumericalTag>(context, intern(context, *data).get_index());
}

template<runir::kr::dl::FamilyTag Family, typename Context>
auto parse(const runir::kr::dl::grammar::ast::BooleanNot<Family>& node, tyr::formalism::planning::DomainView domain, const Context& context)
{
    const auto arg = parse(node.arg, domain, context).get_index();
    auto data = checkout<typename Context::Target::template Boolean<runir::kr::dl::NotTag>>(context);
    data->arg = arg;
    return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).get_index());
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename Context>
auto parse_constructor(const runir::kr::dl::grammar::ast::Constructor<Family, Category>& node,
                       tyr::formalism::planning::DomainView domain,
                       const Context& context) -> typename Context::Target::template ConstructorView<Category>
{
    return boost::apply_visitor([&](const auto& arg) { return parse(unwrap(arg), domain, context); }, node.get());
}

}  // namespace runir::kr::parser::constructors

#endif
