#include "runir/kr/ps/base/dl/parser.hpp"

#include "kr/parser/resolution.hpp"
#include "runir/kr/dl/repository.hpp"
#include "runir/kr/parser/diagnostics.hpp"
#include "runir/kr/parser/parser.hpp"
#include "runir/kr/ps/base/canonicalization.hpp"
#include "runir/kr/ps/base/dl/parser/parser.hpp"
#include "runir/kr/ps/base/dl/parser/parsers.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/dl/declarations.hpp"

#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <sstream>
#include <string>
#include <type_traits>
#include <tyr/formalism/planning/repository.hpp>
#include <unordered_map>
#include <unordered_set>

namespace runir::kr::ps::base::dl
{
namespace
{

struct ConstructorContext
{
    runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository;
    runir::kr::dl::Builder<runir::kr::BaseFamilyTag>& builder;
    const runir::kr::parser::DiagnosticContext& diagnostics;
};

struct Builders
{
    runir::kr::dl::Builder<runir::kr::BaseFamilyTag>& dl;
    runir::kr::ps::base::Builder& ps;
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
auto intern(runir::kr::ps::base::Repository& repository, ygg::Data<T>& data)
{
    return runir::kr::ps::base::get_or_create(repository, data).first;
}

template<typename T>
auto intern(const ConstructorContext& context, ygg::Data<T>& data)
{
    return runir::kr::dl::get_or_create(context.repository, data).first;
}

template<runir::kr::dl::CategoryTag Category, typename T>
auto intern_constructor(const ConstructorContext& context, ygg::Index<T> index)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Constructor<runir::kr::BaseFamilyTag, Category>>(context.builder);
    data->variant = index;
    return intern(context, *data);
}

template<runir::kr::dl::CategoryTag Category>
auto parse_constructor(const runir::kr::dl::grammar::ast::Constructor<runir::kr::BaseFamilyTag, Category>& node,
                       tyr::formalism::planning::DomainView domain,
                       const ConstructorContext& context);

template<runir::kr::dl::CategoryTag Category>
auto parse_constructor_or_non_terminal(const runir::kr::dl::grammar::ast::ConstructorOrNonTerminal<runir::kr::BaseFamilyTag, Category>& node,
                                       tyr::formalism::planning::DomainView domain,
                                       const ConstructorContext& context) -> runir::kr::dl::FamilyConstructorView<runir::kr::BaseFamilyTag, Category>
{
    return boost::apply_visitor(
        [&](const auto& value) -> runir::kr::dl::FamilyConstructorView<runir::kr::BaseFamilyTag, Category>
        {
            const auto& unwrapped = unwrap(value);
            if constexpr (std::same_as<std::remove_cvref_t<decltype(unwrapped)>, runir::kr::dl::grammar::ast::NonTerminal<runir::kr::BaseFamilyTag, Category>>)
                context.diagnostics.throw_at(unwrapped.name,
                                             runir::kr::InvalidExpressionError("General-sketch DL features cannot "
                                                                               "reference grammar nonterminals."));
            else
                return parse_constructor(unwrapped, domain, context);
        },
        node.get());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptBot<runir::kr::BaseFamilyTag>&, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::BotTag>>(context.builder);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptTop<runir::kr::BaseFamilyTag>&, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::TopTag>>(context.builder);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtomicState<runir::kr::BaseFamilyTag>& node,
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
            auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<T>>>(context.builder);
            data->predicate = predicate;
            data->polarity = true;
            return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
        });
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtomicGoal<runir::kr::BaseFamilyTag>& node,
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
            auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<T>>>(context.builder);
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
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, Tag>>(context.builder);
    data->lhs = lhs.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptIntersection<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_binary_concept<runir::kr::dl::IntersectionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptUnion<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_binary_concept<runir::kr::dl::UnionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptValueRestriction<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_binary_concept<runir::kr::dl::ValueRestrictionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptExistentialQuantification<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_binary_concept<runir::kr::dl::ExistentialQuantificationTag>(node, domain, context);
}

template<typename Tag, typename Ast>
auto parse_number_restriction_concept(const Ast& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto role = parse_constructor_or_non_terminal(node.role, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, Tag>>(context.builder);
    data->n = node.n;
    data->role = role.get_index();
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtLeastNumberRestriction<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_number_restriction_concept<runir::kr::dl::AtLeastNumberRestrictionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtMostNumberRestriction<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_number_restriction_concept<runir::kr::dl::AtMostNumberRestrictionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptExactNumberRestriction<runir::kr::BaseFamilyTag>& node,
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
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, Tag>>(context.builder);
    data->n = node.n;
    data->role = role.get_index();
    data->concept_ = concept_view.get_index();
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedAtLeastNumberRestriction<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_qualified_number_restriction_concept<runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedAtMostNumberRestriction<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_qualified_number_restriction_concept<runir::kr::dl::QualifiedAtMostNumberRestrictionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedExactNumberRestriction<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_qualified_number_restriction_concept<runir::kr::dl::QualifiedExactNumberRestrictionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptRoleValueMap<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_binary_concept<runir::kr::dl::RoleValueMapTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAgreement<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_binary_concept<runir::kr::dl::AgreementTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptRoleFillers<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    const auto role = parse_constructor_or_non_terminal(node.role, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::RoleFillersTag>>(context.builder);
    data->role = role.get_index();
    runir::kr::parser::append_objects(domain, node.object_names, context.diagnostics, data->objects);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptOneOf<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::OneOfTag>>(context.builder);
    runir::kr::parser::append_objects(domain, node.object_names, context.diagnostics, data->objects);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptNegation<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    const auto arg = parse_constructor_or_non_terminal(node.arg, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::NegationTag>>(context.builder);
    data->arg = arg.get_index();
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptNominal<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::NominalTag>>(context.builder);
    data->object = runir::kr::parser::require_object(domain, node.object_name, context.diagnostics);
    return intern_constructor<runir::kr::dl::ConceptTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleUniversal<runir::kr::BaseFamilyTag>&, tyr::formalism::planning::DomainView, const ConstructorContext& context)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::UniversalTag>>(context.builder);
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleAtomicState<runir::kr::BaseFamilyTag>& node,
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
            auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<T>>>(context.builder);
            data->predicate = predicate;
            data->polarity = true;
            return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
        });
}

auto parse(const runir::kr::dl::grammar::ast::RoleAtomicGoal<runir::kr::BaseFamilyTag>& node,
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
            auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<T>>>(context.builder);
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
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::BaseFamilyTag, Tag>>(context.builder);
    data->lhs = lhs.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleIntersection<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_binary_role<runir::kr::dl::IntersectionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::RoleUnion<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_binary_role<runir::kr::dl::UnionTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::RoleComposition<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_binary_role<runir::kr::dl::CompositionTag>(node, domain, context);
}

template<typename Tag, typename Ast>
auto parse_unary_role(const Ast& node, tyr::formalism::planning::DomainView domain, const ConstructorContext& context)
{
    const auto arg = parse_constructor_or_non_terminal(node.arg, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::BaseFamilyTag, Tag>>(context.builder);
    data->arg = arg.get_index();
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleComplement<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_unary_role<runir::kr::dl::ComplementTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::RoleInverse<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_unary_role<runir::kr::dl::InverseTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::RoleTransitiveClosure<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_unary_role<runir::kr::dl::TransitiveClosureTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::RoleReflexiveTransitiveClosure<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    return parse_unary_role<runir::kr::dl::ReflexiveTransitiveClosureTag>(node, domain, context);
}

auto parse(const runir::kr::dl::grammar::ast::RoleRestriction<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, context);
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::RestrictionTag>>(context.builder);
    data->lhs = lhs.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleIdentity<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    const auto arg = parse_constructor_or_non_terminal(node.arg, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::IdentityTag>>(context.builder);
    data->arg = arg.get_index();
    return intern_constructor<runir::kr::dl::RoleTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::BooleanAtomicState<runir::kr::BaseFamilyTag>& node,
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
            auto data = runir::kr::dl::checkout<runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<T>>>(context.builder);
            data->predicate = predicate;
            data->polarity = node.polarity;
            return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).get_index());
        });
}

auto parse(const runir::kr::dl::grammar::ast::BooleanAtomicGoal<runir::kr::BaseFamilyTag>& node,
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
            auto data = runir::kr::dl::checkout<runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<T>>>(context.builder);
            data->predicate = predicate;
            data->polarity = node.polarity;
            return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).get_index());
        });
}

auto parse(const runir::kr::dl::grammar::ast::BooleanNonempty<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    const auto arg = boost::apply_visitor(
        [&](const auto& value) -> ygg::Data<runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::NonemptyTag>>::ConstructorVariant
        { return parse_constructor_or_non_terminal(unwrap(value), domain, context).get_index(); },
        node.arg.get());

    auto data = runir::kr::dl::checkout<runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::NonemptyTag>>(context.builder);
    data->arg = arg;
    return intern_constructor<runir::kr::dl::BooleanTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::NumericalCount<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    const auto arg = boost::apply_visitor(
        [&](const auto& value) -> ygg::Data<runir::kr::dl::Numerical<runir::kr::BaseFamilyTag, runir::kr::dl::CountTag>>::ConstructorVariant
        { return parse_constructor_or_non_terminal(unwrap(value), domain, context).get_index(); },
        node.arg.get());

    auto data = runir::kr::dl::checkout<runir::kr::dl::Numerical<runir::kr::BaseFamilyTag, runir::kr::dl::CountTag>>(context.builder);
    data->arg = arg;
    return intern_constructor<runir::kr::dl::NumericalTag>(context, intern(context, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::NumericalDistance<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           const ConstructorContext& context)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, context);
    const auto mid = parse_constructor_or_non_terminal(node.mid, domain, context);
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, context);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Numerical<runir::kr::BaseFamilyTag, runir::kr::dl::DistanceTag>>(context.builder);
    data->lhs = lhs.get_index();
    data->mid = mid.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::NumericalTag>(context, intern(context, *data).get_index());
}

template<runir::kr::dl::CategoryTag Category>
auto parse_constructor(const runir::kr::dl::grammar::ast::Constructor<runir::kr::BaseFamilyTag, Category>& node,
                       tyr::formalism::planning::DomainView domain,
                       const ConstructorContext& context)
{
    return boost::apply_visitor([&](const auto& arg) { return parse(unwrap(arg), domain, context); }, node.get());
}

auto parse_feature(
    const runir::kr::ps::base::dl::ast::BooleanFeature<runir::kr::BaseFamilyTag>& node,
    tyr::formalism::planning::DomainView domain,
    Repository& repository,
    Builders& builders,
    std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>>>& boolean_features,
    std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>>>&,
    ygg::Data<runir::kr::ps::base::Sketch>& sketch_data,
    const runir::kr::parser::DiagnosticContext& diagnostics)
{
    const auto constructor = parse_constructor(node.feature, domain, ConstructorContext { repository.get_dl_repository(), builders.dl, diagnostics });
    auto concrete_data =
        runir::kr::ps::base::checkout<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>>(
            builders.ps);
    concrete_data->feature = constructor.get_index();
    concrete_data->symbol = node.symbol.text;
    const auto concrete = intern(repository, *concrete_data);
    auto data = runir::kr::ps::base::checkout<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>>(builders.ps);
    data->variant = concrete.get_index();
    const auto feature = intern(repository, *data);
    boolean_features.emplace(node.symbol.text, feature.get_index());
    sketch_data.boolean_features.push_back(feature.get_index());
}

auto parse_feature(
    const runir::kr::ps::base::dl::ast::NumericalFeature<runir::kr::BaseFamilyTag>& node,
    tyr::formalism::planning::DomainView domain,
    Repository& repository,
    Builders& builders,
    std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>>>&,
    std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>>>& numerical_features,
    ygg::Data<runir::kr::ps::base::Sketch>& sketch_data,
    const runir::kr::parser::DiagnosticContext& diagnostics)
{
    const auto constructor = parse_constructor(node.feature, domain, ConstructorContext { repository.get_dl_repository(), builders.dl, diagnostics });
    auto concrete_data =
        runir::kr::ps::base::checkout<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature>>(
            builders.ps);
    concrete_data->feature = constructor.get_index();
    concrete_data->symbol = node.symbol.text;
    const auto concrete = intern(repository, *concrete_data);
    auto data = runir::kr::ps::base::checkout<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>>(builders.ps);
    data->variant = concrete.get_index();
    const auto feature = intern(repository, *data);
    numerical_features.emplace(node.symbol.text, feature.get_index());
    sketch_data.numerical_features.push_back(feature.get_index());
}

template<typename FeatureTag>
auto require_feature(const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, FeatureTag>>>& features,
                     const runir::kr::parser::ast::Identifier& name,
                     const runir::kr::parser::DiagnosticContext& diagnostics)
{
    const auto it = features.find(name.text);
    if (it == features.end())
        diagnostics.throw_at(name, runir::kr::UndefinedSymbolError("feature", name.text));
    return it->second;
}

template<typename FeatureTag, typename ObservationTag>
auto make_condition(ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, FeatureTag>> feature,
                    Repository& repository,
                    runir::kr::ps::base::Builder& builder)
{
    auto concrete_data =
        runir::kr::ps::base::checkout<runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>(builder);
    concrete_data->feature = feature;
    const auto concrete = intern(repository, *concrete_data);
    auto variant_data = runir::kr::ps::base::checkout<runir::kr::ps::ConcreteConditionVariant<runir::kr::BaseFamilyTag, runir::kr::DlTag>>(builder);
    variant_data->variant = concrete.get_index();
    const auto variant = intern(repository, *variant_data);
    auto data = runir::kr::ps::base::checkout<runir::kr::ps::ConditionVariant<runir::kr::BaseFamilyTag>>(builder);
    data->variant = variant.get_index();
    return intern(repository, *data);
}

template<typename FeatureTag, typename ObservationTag>
auto make_effect(ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, FeatureTag>> feature,
                 Repository& repository,
                 runir::kr::ps::base::Builder& builder)
{
    auto concrete_data =
        runir::kr::ps::base::checkout<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>(builder);
    concrete_data->feature = feature;
    const auto concrete = intern(repository, *concrete_data);
    auto variant_data = runir::kr::ps::base::checkout<runir::kr::ps::ConcreteEffectVariant<runir::kr::BaseFamilyTag, runir::kr::DlTag>>(builder);
    variant_data->variant = concrete.get_index();
    const auto variant = intern(repository, *variant_data);
    auto data = runir::kr::ps::base::checkout<runir::kr::ps::EffectVariant<runir::kr::BaseFamilyTag>>(builder);
    data->variant = variant.get_index();
    return intern(repository, *data);
}

auto parse_condition_observation(
    const runir::kr::ps::base::dl::ast::Positive&,
    const runir::kr::parser::ast::Identifier& feature,
    Repository& repository,
    runir::kr::ps::base::Builder& builder,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>>>& boolean_features,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>>>&,
    const runir::kr::parser::DiagnosticContext& diagnostics)
{
    return make_condition<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>(require_feature(boolean_features, feature, diagnostics),
                                                                                          repository,
                                                                                          builder);
}

auto parse_condition_observation(
    const runir::kr::ps::base::dl::ast::Negative&,
    const runir::kr::parser::ast::Identifier& feature,
    Repository& repository,
    runir::kr::ps::base::Builder& builder,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>>>& boolean_features,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>>>&,
    const runir::kr::parser::DiagnosticContext& diagnostics)
{
    return make_condition<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>(require_feature(boolean_features, feature, diagnostics),
                                                                                          repository,
                                                                                          builder);
}

auto parse_condition_observation(
    const runir::kr::ps::base::dl::ast::EqualZero&,
    const runir::kr::parser::ast::Identifier& feature,
    Repository& repository,
    runir::kr::ps::base::Builder& builder,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>>>&,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>>>&
        numerical_features,
    const runir::kr::parser::DiagnosticContext& diagnostics)
{
    return make_condition<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::EqualZero>(require_feature(numerical_features, feature, diagnostics),
                                                                                             repository,
                                                                                             builder);
}

auto parse_condition_observation(
    const runir::kr::ps::base::dl::ast::GreaterZero&,
    const runir::kr::parser::ast::Identifier& feature,
    Repository& repository,
    runir::kr::ps::base::Builder& builder,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>>>&,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>>>&
        numerical_features,
    const runir::kr::parser::DiagnosticContext& diagnostics)
{
    return make_condition<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::GreaterZero>(require_feature(numerical_features, feature, diagnostics),
                                                                                               repository,
                                                                                               builder);
}

auto parse_condition(
    const runir::kr::ps::base::dl::ast::Condition<runir::kr::BaseFamilyTag>& node,
    Repository& repository,
    runir::kr::ps::base::Builder& builder,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>>>& boolean_features,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>>>&
        numerical_features,
    const runir::kr::parser::DiagnosticContext& diagnostics)
{
    return boost::apply_visitor(
        [&](const auto& observation)
        { return parse_condition_observation(observation, node.feature, repository, builder, boolean_features, numerical_features, diagnostics); },
        node.observation.get());
}

auto parse_effect_observation(
    const runir::kr::ps::base::dl::ast::Positive&,
    const runir::kr::parser::ast::Identifier& feature,
    Repository& repository,
    runir::kr::ps::base::Builder& builder,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>>>& boolean_features,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>>>&,
    const runir::kr::parser::DiagnosticContext& diagnostics)
{
    return make_effect<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>(require_feature(boolean_features, feature, diagnostics),
                                                                                       repository,
                                                                                       builder);
}

auto parse_effect_observation(
    const runir::kr::ps::base::dl::ast::Negative&,
    const runir::kr::parser::ast::Identifier& feature,
    Repository& repository,
    runir::kr::ps::base::Builder& builder,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>>>& boolean_features,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>>>&,
    const runir::kr::parser::DiagnosticContext& diagnostics)
{
    return make_effect<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>(require_feature(boolean_features, feature, diagnostics),
                                                                                       repository,
                                                                                       builder);
}

auto parse_effect_observation(
    const runir::kr::ps::base::dl::ast::Unchanged&,
    const runir::kr::parser::ast::Identifier& feature,
    Repository& repository,
    runir::kr::ps::base::Builder& builder,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>>>& boolean_features,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>>>&
        numerical_features,
    const runir::kr::parser::DiagnosticContext& diagnostics)
{
    const auto boolean_it = boolean_features.find(feature.text);
    const auto numerical_it = numerical_features.find(feature.text);

    if (boolean_it != boolean_features.end() && numerical_it != numerical_features.end())
        diagnostics.throw_at(feature, runir::kr::InvalidExpressionError("Ambiguous feature \"" + feature.text + "\"."));
    if (boolean_it != boolean_features.end())
        return make_effect<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Unchanged>(boolean_it->second, repository, builder);
    if (numerical_it != numerical_features.end())
        return make_effect<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Unchanged>(numerical_it->second, repository, builder);

    diagnostics.throw_at(feature, runir::kr::UndefinedSymbolError("feature", feature.text));
}

auto parse_effect_observation(
    const runir::kr::ps::base::dl::ast::Increases&,
    const runir::kr::parser::ast::Identifier& feature,
    Repository& repository,
    runir::kr::ps::base::Builder& builder,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>>>&,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>>>&
        numerical_features,
    const runir::kr::parser::DiagnosticContext& diagnostics)
{
    return make_effect<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Increases>(require_feature(numerical_features, feature, diagnostics),
                                                                                          repository,
                                                                                          builder);
}

auto parse_effect_observation(
    const runir::kr::ps::base::dl::ast::Decreases&,
    const runir::kr::parser::ast::Identifier& feature,
    Repository& repository,
    runir::kr::ps::base::Builder& builder,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>>>&,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>>>&
        numerical_features,
    const runir::kr::parser::DiagnosticContext& diagnostics)
{
    return make_effect<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Decreases>(require_feature(numerical_features, feature, diagnostics),
                                                                                          repository,
                                                                                          builder);
}

auto parse_effect(
    const runir::kr::ps::base::dl::ast::Effect<runir::kr::BaseFamilyTag>& node,
    Repository& repository,
    runir::kr::ps::base::Builder& builder,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>>>& boolean_features,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>>>&
        numerical_features,
    const runir::kr::parser::DiagnosticContext& diagnostics)
{
    return boost::apply_visitor(
        [&](const auto& observation)
        { return parse_effect_observation(observation, node.feature, repository, builder, boolean_features, numerical_features, diagnostics); },
        node.observation.get());
}

auto parse_rule(
    const runir::kr::ps::base::dl::ast::Rule<runir::kr::BaseFamilyTag>& node,
    Repository& repository,
    runir::kr::ps::base::Builder& builder,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>>>& boolean_features,
    const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>>>&
        numerical_features,
    const runir::kr::parser::DiagnosticContext& diagnostics)
{
    auto data = runir::kr::ps::base::checkout<runir::kr::ps::base::Rule>(builder);
    data->symbol = node.symbol.text;
    data->conditions.reserve(node.conditions.size());
    for (const auto& condition : node.conditions)
        data->conditions.push_back(parse_condition(condition, repository, builder, boolean_features, numerical_features, diagnostics).get_index());

    data->effects.reserve(node.effects.size());
    for (const auto& effect : node.effects)
        data->effects.push_back(parse_effect(effect, repository, builder, boolean_features, numerical_features, diagnostics).get_index());
    return intern(repository, *data);
}

}  // namespace

SketchView parse_sketch(const std::string& description, tyr::formalism::planning::DomainView domain, Repository& repository)
{
    auto first = description.cbegin();
    const auto last = description.cend();
    auto ast = runir::kr::ps::base::dl::ast::Sketch<runir::kr::BaseFamilyTag> {};
    auto errors = std::ostringstream {};
    auto error_handler = runir::kr::parser::ErrorHandlerType(first, last, errors);
    if (!runir::kr::parser::parse_full(first, last, parser::sketch_root_parser(), ast, error_handler))
        throw runir::kr::parser::DiagnosticContext::parse_error(error_handler, "Failed to parse DL general sketch description.", first);
    auto diagnostics = runir::kr::parser::DiagnosticContext {};
    const auto scope = runir::kr::parser::DiagnosticContext::Scope(diagnostics, error_handler);
    auto dl_builder = runir::kr::dl::Builder<runir::kr::BaseFamilyTag> {};
    auto ps_builder = runir::kr::ps::base::Builder {};
    auto builders = Builders { dl_builder, ps_builder };

    auto boolean_features = std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>>> {};
    auto numerical_features =
        std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>>> {};
    auto feature_symbols = std::unordered_set<std::string> {};
    auto data = runir::kr::ps::base::checkout<runir::kr::ps::base::Sketch>(ps_builder);
    data->boolean_features.reserve(ast.features.size());
    data->numerical_features.reserve(ast.features.size());
    data->rules.reserve(ast.rules.size());

    for (const auto& feature : ast.features)
        boost::apply_visitor(
            [&](const auto& arg)
            {
                if (!feature_symbols.emplace(arg.symbol.text).second)
                    diagnostics.throw_at(arg.symbol, runir::kr::DuplicateDefinitionError("feature", arg.symbol.text));
                parse_feature(arg, domain, repository, builders, boolean_features, numerical_features, *data, diagnostics);
            },
            feature.get());

    auto rule_symbols = std::unordered_set<std::string> {};
    for (const auto& rule : ast.rules)
    {
        if (!rule_symbols.emplace(rule.symbol.text).second)
            diagnostics.throw_at(rule.symbol, runir::kr::DuplicateDefinitionError("rule", rule.symbol.text));
        data->rules.push_back(parse_rule(rule, repository, ps_builder, boolean_features, numerical_features, diagnostics).get_index());
    }

    return intern(repository, *data);
}

}  // namespace runir::kr::ps::base::dl
