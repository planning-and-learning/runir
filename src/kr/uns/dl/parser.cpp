#include "runir/kr/uns/dl/parser.hpp"

#include "kr/parser/resolution.hpp"
#include "runir/kr/dl/repository.hpp"
#include "runir/kr/parser/diagnostics.hpp"
#include "runir/kr/parser/parser.hpp"
#include "runir/kr/uns/dl/parser/parser.hpp"
#include "runir/kr/uns/dl/parser/parsers.hpp"
#include "runir/kr/uns/repository.hpp"

#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <sstream>
#include <string>
#include <type_traits>
#include <tyr/formalism/planning/repository.hpp>
#include <unordered_map>
#include <utility>

namespace runir::kr::uns::dl
{
namespace
{

struct ConstructorContext
{
    runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository;
    runir::kr::dl::Builder<runir::kr::UnsFamilyTag>& builder;
    const runir::kr::parser::DiagnosticContext& diagnostics;
};

template<typename T>
struct IsForwardAst : std::false_type
{
};

template<typename T>
struct IsForwardAst<boost::spirit::x3::forward_ast<T>> : std::true_type
{
};

template<typename T>
decltype(auto) unwrap(const T& value) noexcept
{
    if constexpr (IsForwardAst<T>::value)
        return value.get();
    else
        return (value);
}
template<typename T>
auto intern(runir::kr::uns::Repository& repository, ygg::Data<T>& data)
{
    return runir::kr::uns::get_or_create(repository, data).first;
}

template<typename T>
auto intern(const ConstructorContext& context, ygg::Data<T>& data)
{
    return runir::kr::dl::get_or_create(context.repository, data).first;
}

template<runir::kr::dl::CategoryTag Category, typename T>
auto intern_constructor(const ConstructorContext& context, ygg::Index<T> index)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Constructor<runir::kr::UnsFamilyTag, Category>>(context.builder);
    data->variant = index;
    return intern(context, *data);
}

template<runir::kr::dl::CategoryTag Category>
auto parse_constructor(const runir::kr::dl::grammar::ast::Constructor<runir::kr::UnsFamilyTag, Category>& node,
                       tyr::formalism::planning::DomainView domain,
                       const ConstructorContext& context);

template<runir::kr::dl::CategoryTag Category>
auto parse_constructor_or_non_terminal(const runir::kr::dl::grammar::ast::ConstructorOrNonTerminal<runir::kr::UnsFamilyTag, Category>& node,
                                       tyr::formalism::planning::DomainView domain,
                                       const ConstructorContext& context) -> runir::kr::dl::FamilyConstructorView<runir::kr::UnsFamilyTag, Category>
{
    return boost::apply_visitor(
        [&](const auto& value) -> runir::kr::dl::FamilyConstructorView<runir::kr::UnsFamilyTag, Category>
        {
            const auto& unwrapped = unwrap(value);
            if constexpr (std::same_as<std::remove_cvref_t<decltype(unwrapped)>, runir::kr::dl::grammar::ast::NonTerminal<runir::kr::UnsFamilyTag, Category>>)
                context.diagnostics.throw_at(unwrapped.name,
                                             runir::kr::InvalidExpressionError("Classifier DL features cannot reference grammar nonterminals."));
            else
                return parse_constructor(unwrapped, domain, context);
        },
        node.get());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptBot<runir::kr::UnsFamilyTag>&, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::BotTag>>(context.builder);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptTop<runir::kr::UnsFamilyTag>&, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::TopTag>>(context.builder);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtomicState<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return runir::kr::parser::resolve_predicate(
        domain,
        node.predicate_name,
        1,
        "ConceptAtomicState",
        context.diagnostics,
        [&](auto tag, auto predicate)
        {
            using T = decltype(tag);
            auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<T>>>(context.builder);
            data->predicate = predicate;
            data->polarity = true;
            return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
        });
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtomicGoal<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return runir::kr::parser::resolve_predicate(
        domain,
        node.predicate_name,
        1,
        "ConceptAtomicGoal",
        context.diagnostics,
        [&](auto tag, auto predicate)
        {
            using T = decltype(tag);
            auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<T>>>(context.builder);
            data->predicate = predicate;
            data->polarity = node.polarity;
            return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
        });
}

template<typename Tag, typename Ast>
auto parse_binary_concept(const Ast& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, context);
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, Tag>>(context.builder);
    data->lhs = lhs.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptIntersection<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_binary_concept<runir::kr::dl::IntersectionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptUnion<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_binary_concept<runir::kr::dl::UnionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptValueRestriction<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_binary_concept<runir::kr::dl::ValueRestrictionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptExistentialQuantification<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_binary_concept<runir::kr::dl::ExistentialQuantificationTag>(node, domain, context);
}

template<typename Tag, typename Ast>
auto parse_number_restriction_concept(const Ast& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto role = parse_constructor_or_non_terminal(node.role, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, Tag>>(context.builder);
    data->n = node.n;
    data->role = role.get_index();
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtLeastNumberRestriction<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_number_restriction_concept<runir::kr::dl::AtLeastNumberRestrictionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtMostNumberRestriction<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_number_restriction_concept<runir::kr::dl::AtMostNumberRestrictionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptExactNumberRestriction<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_number_restriction_concept<runir::kr::dl::ExactNumberRestrictionTag>(node, domain, context);
}

template<typename Tag, typename Ast>
auto parse_qualified_number_restriction_concept(const Ast& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto role = parse_constructor_or_non_terminal(node.role, domain, context);
    const auto concept_view = parse_constructor_or_non_terminal(node.concept_, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, Tag>>(context.builder);
    data->n = node.n;
    data->role = role.get_index();
    data->concept_ = concept_view.get_index();
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedAtLeastNumberRestriction<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_qualified_number_restriction_concept<runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedAtMostNumberRestriction<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_qualified_number_restriction_concept<runir::kr::dl::QualifiedAtMostNumberRestrictionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedExactNumberRestriction<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_qualified_number_restriction_concept<runir::kr::dl::QualifiedExactNumberRestrictionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptRoleValueMap<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_binary_concept<runir::kr::dl::RoleValueMapTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAgreement<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_binary_concept<runir::kr::dl::AgreementTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptRoleFillers<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    const auto role = parse_constructor_or_non_terminal(node.role, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::RoleFillersTag>>(context.builder);
    data->role = role.get_index();
    runir::kr::parser::append_objects(domain, node.object_names, context.diagnostics, data->objects);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptOneOf<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::OneOfTag>>(context.builder);
    runir::kr::parser::append_objects(domain, node.object_names, context.diagnostics, data->objects);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptNegation<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    const auto arg = parse_constructor_or_non_terminal(node.arg, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::NegationTag>>(context.builder);
    data->arg = arg.get_index();
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptNominal<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::NominalTag>>(context.builder);
    data->object = runir::kr::parser::require_object(domain, node.object_name, context.diagnostics);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleUniversal<runir::kr::UnsFamilyTag>&, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::UniversalTag>>(context.builder);
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleAtomicState<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return runir::kr::parser::resolve_predicate(
        domain,
        node.predicate_name,
        2,
        "RoleAtomicState",
        context.diagnostics,
        [&](auto tag, auto predicate)
        {
            using T = decltype(tag);
            auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<T>>>(context.builder);
            data->predicate = predicate;
            data->polarity = true;
            return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
        });
}

auto parse(const runir::kr::dl::grammar::ast::RoleAtomicGoal<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return runir::kr::parser::resolve_predicate(
        domain,
        node.predicate_name,
        2,
        "RoleAtomicGoal",
        context.diagnostics,
        [&](auto tag, auto predicate)
        {
            using T = decltype(tag);
            auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<T>>>(context.builder);
            data->predicate = predicate;
            data->polarity = node.polarity;
            return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
        });
}

template<typename Tag, typename Ast>
auto parse_binary_role(const Ast& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, context);
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::UnsFamilyTag, Tag>>(context.builder);
    data->lhs = lhs.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleIntersection<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_binary_role<runir::kr::dl::IntersectionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::RoleUnion<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_binary_role<runir::kr::dl::UnionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::RoleComposition<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_binary_role<runir::kr::dl::CompositionTag>(node, domain, context);
}

template<typename Tag, typename Ast>
auto parse_unary_role(const Ast& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto arg = parse_constructor_or_non_terminal(node.arg, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::UnsFamilyTag, Tag>>(context.builder);
    data->arg = arg.get_index();
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleComplement<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_unary_role<runir::kr::dl::ComplementTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::RoleInverse<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_unary_role<runir::kr::dl::InverseTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::RoleTransitiveClosure<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_unary_role<runir::kr::dl::TransitiveClosureTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::RoleReflexiveTransitiveClosure<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_unary_role<runir::kr::dl::ReflexiveTransitiveClosureTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::RoleRestriction<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, context);
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::RestrictionTag>>(context.builder);
    data->lhs = lhs.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleIdentity<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    const auto arg = parse_constructor_or_non_terminal(node.arg, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::IdentityTag>>(context.builder);
    data->arg = arg.get_index();
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::BooleanAtomicState<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return runir::kr::parser::resolve_predicate(
        domain,
        node.predicate_name,
        0,
        "BooleanAtomicState",
        context.diagnostics,
        [&](auto tag, auto predicate)
        {
            using T = decltype(tag);
            auto data = runir::kr::dl::checkout<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<T>>>(context.builder);
            data->predicate = predicate;
            data->polarity = node.polarity;
            return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).get_index());
        });
}

auto parse(const runir::kr::dl::grammar::ast::BooleanAtomicGoal<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return runir::kr::parser::resolve_predicate(
        domain,
        node.predicate_name,
        0,
        "BooleanAtomicGoal",
        context.diagnostics,
        [&](auto tag, auto predicate)
        {
            using T = decltype(tag);
            auto data = runir::kr::dl::checkout<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<T>>>(context.builder);
            data->predicate = predicate;
            data->polarity = node.polarity;
            return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).get_index());
        });
}

auto parse(const runir::kr::dl::grammar::ast::BooleanNonempty<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    const auto arg = boost::apply_visitor(
        [&](const auto& value) -> ygg::Data<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::NonemptyTag>>::ConstructorVariant
        { return parse_constructor_or_non_terminal(unwrap(value), domain, context).get_index(); },
        node.arg.get());

    auto data = runir::kr::dl::checkout<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::NonemptyTag>>(context.builder);
    data->arg = arg;
    return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::NumericalCount<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    const auto arg =
        boost::apply_visitor([&](const auto& value) -> ygg::Data<runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, runir::kr::dl::CountTag>>::ConstructorVariant
                             { return parse_constructor_or_non_terminal(unwrap(value), domain, context).get_index(); },
                             node.arg.get());

    auto data = runir::kr::dl::checkout<runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, runir::kr::dl::CountTag>>(context.builder);
    data->arg = arg;
    return intern_constructor<runir::kr::dl::NumericalTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::NumericalDistance<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, context);
    const auto mid = parse_constructor_or_non_terminal(node.mid, domain, context);
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, runir::kr::dl::DistanceTag>>(context.builder);
    data->lhs = lhs.get_index();
    data->mid = mid.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::NumericalTag>(context, intern(context, *data).get_index());
}

template<typename Tag, typename Ast>
auto parse_comparison(const Ast& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, context);
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, Tag>>(context.builder);
    data->lhs = lhs.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::BooleanEq<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_comparison<runir::kr::dl::EqTag<runir::kr::dl::BooleanTag>>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::BooleanNeq<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_comparison<runir::kr::dl::NeqTag<runir::kr::dl::BooleanTag>>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::BooleanLt<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_comparison<runir::kr::dl::LtTag<runir::kr::dl::BooleanTag>>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::BooleanLe<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_comparison<runir::kr::dl::LeTag<runir::kr::dl::BooleanTag>>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::BooleanGt<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_comparison<runir::kr::dl::GtTag<runir::kr::dl::BooleanTag>>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::BooleanGe<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_comparison<runir::kr::dl::GeTag<runir::kr::dl::BooleanTag>>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalEq<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_comparison<runir::kr::dl::EqTag<runir::kr::dl::NumericalTag>>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalNeq<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_comparison<runir::kr::dl::NeqTag<runir::kr::dl::NumericalTag>>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalLt<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_comparison<runir::kr::dl::LtTag<runir::kr::dl::NumericalTag>>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalLe<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_comparison<runir::kr::dl::LeTag<runir::kr::dl::NumericalTag>>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalGt<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_comparison<runir::kr::dl::GtTag<runir::kr::dl::NumericalTag>>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalGe<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_comparison<runir::kr::dl::GeTag<runir::kr::dl::NumericalTag>>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::BooleanConstant<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView,
           const ConstructorContext& context)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanConstantTag>>(context.builder);
    data->identifier = node.value;
    return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::NumericalConstant<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView,
           const ConstructorContext& context)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, runir::kr::dl::NumericalConstantTag>>(context.builder);
    data->identifier = node.value;
    return intern_constructor<runir::kr::dl::NumericalTag>(context, intern(context, *data).get_index());
}

template<typename Tag, typename Ast>
auto parse_numerical_binary(const Ast& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, context);
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, Tag>>(context.builder);
    data->lhs = lhs.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::NumericalTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::NumericalAdd<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_numerical_binary<runir::kr::dl::AddTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalSub<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_numerical_binary<runir::kr::dl::SubTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalMul<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_numerical_binary<runir::kr::dl::MulTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalDiv<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_numerical_binary<runir::kr::dl::DivTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalMin<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_numerical_binary<runir::kr::dl::MinTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalMax<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_numerical_binary<runir::kr::dl::MaxTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::BooleanAnd<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_comparison<runir::kr::dl::AndTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::BooleanOr<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_comparison<runir::kr::dl::OrTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::BooleanNot<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    const auto arg = parse_constructor_or_non_terminal(node.arg, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::NotTag>>(context.builder);
    data->arg = arg.get_index();
    return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).get_index());
}

template<runir::kr::dl::CategoryTag Category>
auto parse_constructor(const runir::kr::dl::grammar::ast::Constructor<runir::kr::UnsFamilyTag, Category>& node,
                       tyr::formalism::planning::DomainView domain,
                       const ConstructorContext& context)
{
    return boost::apply_visitor([&](const auto& arg) { return parse(unwrap(arg), domain, context); }, node.get());
}

auto parse_feature(const runir::kr::uns::dl::ast::BooleanFeature& node,
                   tyr::formalism::planning::DomainView domain,
                   Repository& repository,
                   runir::kr::dl::Builder<runir::kr::UnsFamilyTag>& dl_builder,
                   runir::kr::uns::Builder& builder,
                   const runir::kr::parser::DiagnosticContext& diagnostics)
{
    const auto context = ConstructorContext { repository.get_dl_repository(), dl_builder, diagnostics };
    const auto feature = parse_constructor(node.feature, domain, context);
    auto concrete_data =
        runir::kr::uns::checkout<runir::kr::ps::ConcreteFeature<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>>(builder);
    concrete_data->feature = feature.get_index();
    concrete_data->symbol = node.symbol.text;
    const auto concrete = intern(repository, *concrete_data);

    auto data = runir::kr::uns::checkout<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, runir::kr::ps::dl::BooleanFeature>>(builder);
    data->variant = concrete.get_index();
    return intern(repository, *data);
}

}  // namespace

ClassifierView parse_classifier(const std::string& description, tyr::formalism::planning::DomainView domain, Repository& repository)
{
    auto dl_builder = runir::kr::dl::Builder<runir::kr::UnsFamilyTag> {};
    auto builder = runir::kr::uns::Builder {};
    auto first = description.cbegin();
    const auto last = description.cend();
    auto ast = runir::kr::uns::dl::ast::Classifier {};
    auto errors = std::ostringstream {};
    auto error_handler = runir::kr::parser::ErrorHandlerType(first, last, errors);
    if (!runir::kr::parser::parse_full(first, last, parser::classifier_root_parser(), ast, error_handler))
        throw runir::kr::parser::DiagnosticContext::parse_error(error_handler, "Failed to parse DL classifier description.", first);
    auto diagnostics = runir::kr::parser::DiagnosticContext {};
    const auto scope = runir::kr::parser::DiagnosticContext::Scope(diagnostics, error_handler);

    auto data = runir::kr::uns::checkout<runir::kr::uns::Classifier>(builder);
    data->symbol = ast.symbol.text;
    data->features.reserve(ast.features.size());
    data->clauses.reserve(ast.expression.clauses.size());
    auto symbol_to_feature = std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, runir::kr::ps::dl::BooleanFeature>>> {};
    symbol_to_feature.reserve(ast.features.size());

    for (const auto& feature : ast.features)
    {
        const auto wrapper = parse_feature(feature, domain, repository, dl_builder, builder, diagnostics);
        if (!symbol_to_feature.emplace(feature.symbol.text, wrapper.get_index()).second)
            diagnostics.throw_at(feature.symbol, runir::kr::DuplicateDefinitionError("feature", feature.symbol.text));
        data->features.push_back(wrapper.get_index());
    }

    const auto parse_literal = [&](const auto& literal)
    {
        using Literal = std::remove_cvref_t<decltype(literal)>;
        const auto it = symbol_to_feature.find(literal.symbol.text);
        if (it == symbol_to_feature.end())
            diagnostics.throw_at(literal.symbol, runir::kr::UndefinedSymbolError("feature", literal.symbol.text));

        constexpr auto polarity = std::same_as<Literal, runir::kr::uns::dl::ast::PositiveLiteral>;
        auto literal_data = runir::kr::uns::checkout<runir::kr::uns::ClassifierLiteral>(builder);
        literal_data->variant = it->second;
        literal_data->polarity = polarity;
        return intern(repository, *literal_data).get_index();
    };

    for (const auto& ast_clause : ast.expression.clauses)
    {
        auto clause_data = runir::kr::uns::checkout<runir::kr::uns::ClassifierClause>(builder);
        clause_data->literals.reserve(ast_clause.literals.size());
        for (const auto& ast_literal : ast_clause.literals)
            clause_data->literals.push_back(boost::apply_visitor(parse_literal, ast_literal.get()));
        data->clauses.push_back(intern(repository, *clause_data).get_index());
    }

    return intern(repository, *data);
}

}  // namespace runir::kr::uns::dl
