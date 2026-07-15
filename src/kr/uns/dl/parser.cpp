#include "runir/kr/uns/dl/parser.hpp"

#include "runir/kr/dl/repository.hpp"
#include "runir/kr/parser/diagnostics.hpp"
#include "runir/kr/parser/parser.hpp"
#include "runir/kr/uns/dl/parser/parser.hpp"
#include "runir/kr/uns/dl/parser/parsers.hpp"
#include "runir/kr/uns/repository.hpp"

#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <optional>
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

using DiagnosticContext = runir::kr::parser::DiagnosticContext;

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
template<typename T, typename Repository>
auto intern(Repository& repository, ygg::Data<T>& data)
{
    if constexpr (requires { runir::kr::dl::canonicalize(data); })
        runir::kr::dl::canonicalize(data);

    return repository.get_or_create(data).first;
}

template<runir::kr::dl::CategoryTag Category, typename T>
auto intern_constructor(runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository, ygg::Index<T> index)
{
    ygg::Data<runir::kr::dl::Constructor<runir::kr::UnsFamilyTag, Category>> data(index);
    return intern(repository, data);
}

template<runir::kr::dl::CategoryTag Category>
auto parse_constructor(const runir::kr::dl::grammar::ast::Constructor<runir::kr::UnsFamilyTag, Category>& node,
                       tyr::formalism::planning::DomainView domain,
                       runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
                       const DiagnosticContext& diagnostics);

template<runir::kr::dl::CategoryTag Category>
auto parse_constructor_or_non_terminal(const runir::kr::dl::grammar::ast::ConstructorOrNonTerminal<runir::kr::UnsFamilyTag, Category>& node,
                                       tyr::formalism::planning::DomainView domain,
                                       runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
                                       const DiagnosticContext& diagnostics) -> runir::kr::dl::FamilyConstructorView<runir::kr::UnsFamilyTag, Category>
{
    return boost::apply_visitor(
        [&](const auto& value) -> runir::kr::dl::FamilyConstructorView<runir::kr::UnsFamilyTag, Category>
        {
            const auto& unwrapped = unwrap(value);
            if constexpr (std::same_as<std::remove_cvref_t<decltype(unwrapped)>, runir::kr::dl::grammar::ast::NonTerminal<runir::kr::UnsFamilyTag, Category>>)
                diagnostics.throw_at(unwrapped.name, runir::kr::InvalidExpressionError("Classifier DL features cannot reference grammar nonterminals."));
            else
                return parse_constructor(unwrapped, domain, repository, diagnostics);
        },
        node.get());
}

template<tyr::formalism::FactKind T>
auto find_predicate(tyr::formalism::planning::DomainView domain, const std::string& name)
{
    for (auto predicate : domain.template get_predicates<T>())
        if (predicate.get_name() == name)
            return std::optional(predicate);

    return std::optional<tyr::formalism::planning::PredicateView<T>> {};
}

template<tyr::formalism::FactKind T>
auto require_predicate(tyr::formalism::planning::DomainView domain,
                       const runir::kr::parser::ast::Identifier& name,
                       size_t arity,
                       const char* constructor_name,
                       const DiagnosticContext& diagnostics)
{
    auto predicate = find_predicate<T>(domain, name.text);
    if (!predicate)
        return std::optional<ygg::Index<tyr::formalism::Predicate<T>>> {};

    if (predicate->get_arity() != arity)
        diagnostics.throw_at(name, runir::kr::ArityMismatchError(constructor_name, arity, predicate->get_arity()));

    return std::optional(predicate->get_index());
}

template<typename Make>
auto resolve_predicate(tyr::formalism::planning::DomainView domain,
                       const runir::kr::parser::ast::Identifier& name,
                       size_t arity,
                       const char* constructor_name,
                       const DiagnosticContext& diagnostics,
                       Make&& make)
{
    if (auto predicate = require_predicate<tyr::formalism::StaticTag>(domain, name, arity, constructor_name, diagnostics))
        return make(tyr::formalism::StaticTag {}, *predicate);
    if (auto predicate = require_predicate<tyr::formalism::FluentTag>(domain, name, arity, constructor_name, diagnostics))
        return make(tyr::formalism::FluentTag {}, *predicate);
    if (auto predicate = require_predicate<tyr::formalism::DerivedTag>(domain, name, arity, constructor_name, diagnostics))
        return make(tyr::formalism::DerivedTag {}, *predicate);

    diagnostics.throw_at(name, runir::kr::UndefinedSymbolError("predicate", name.text));
}

auto require_object(tyr::formalism::planning::DomainView domain, const runir::kr::parser::ast::Identifier& name, const DiagnosticContext& diagnostics)
{
    for (auto object : domain.get_constants())
        if (object.get_name() == name.text)
            return object.get_index();

    diagnostics.throw_at(name, runir::kr::UndefinedSymbolError("constant", name.text));
}

auto require_objects(tyr::formalism::planning::DomainView domain,
                     const std::vector<runir::kr::dl::grammar::ast::Identifier>& names,
                     const DiagnosticContext& diagnostics)
{
    auto result = ygg::IndexList<tyr::formalism::Object> {};
    result.reserve(names.size());
    for (const auto& name : names)
        result.push_back(require_object(domain, name, diagnostics));
    return result;
}

auto parse(const runir::kr::dl::grammar::ast::ConceptBot<runir::kr::UnsFamilyTag>&,
           tyr::formalism::planning::DomainView,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext&)
{
    ygg::Data<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::BotTag>> data;
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptTop<runir::kr::UnsFamilyTag>&,
           tyr::formalism::planning::DomainView,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext&)
{
    ygg::Data<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::TopTag>> data;
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtomicState<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             1,
                             "ConceptAtomicState",
                             diagnostics,
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 ygg::Data<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<T>>> data(predicate);
                                 return intern_constructor<runir::kr::dl::ConceptTag>(repository, intern(repository, data).get_index());
                             });
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtomicGoal<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             1,
                             "ConceptAtomicGoal",
                             diagnostics,
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 ygg::Data<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<T>>> data(predicate, node.polarity);
                                 return intern_constructor<runir::kr::dl::ConceptTag>(repository, intern(repository, data).get_index());
                             });
}

template<typename Tag, typename Ast>
auto parse_binary_concept(const Ast& node,
                          tyr::formalism::planning::DomainView domain,
                          runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
                          const DiagnosticContext& diagnostics)
{
    ygg::Data<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, Tag>> data(
        parse_constructor_or_non_terminal(node.lhs, domain, repository, diagnostics).get_index(),
        parse_constructor_or_non_terminal(node.rhs, domain, repository, diagnostics).get_index());
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptIntersection<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_binary_concept<runir::kr::dl::IntersectionTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptUnion<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_binary_concept<runir::kr::dl::UnionTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptValueRestriction<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_binary_concept<runir::kr::dl::ValueRestrictionTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptExistentialQuantification<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_binary_concept<runir::kr::dl::ExistentialQuantificationTag>(node, domain, repository, diagnostics);
}

template<typename Tag, typename Ast>
auto parse_number_restriction_concept(const Ast& node,
                                      tyr::formalism::planning::DomainView domain,
                                      runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
                                      const DiagnosticContext& diagnostics)
{
    ygg::Data<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, Tag>> data(
        node.n,
        parse_constructor_or_non_terminal(node.role, domain, repository, diagnostics).get_index());
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtLeastNumberRestriction<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_number_restriction_concept<runir::kr::dl::AtLeastNumberRestrictionTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtMostNumberRestriction<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_number_restriction_concept<runir::kr::dl::AtMostNumberRestrictionTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptExactNumberRestriction<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_number_restriction_concept<runir::kr::dl::ExactNumberRestrictionTag>(node, domain, repository, diagnostics);
}

template<typename Tag, typename Ast>
auto parse_qualified_number_restriction_concept(const Ast& node,
                                                tyr::formalism::planning::DomainView domain,
                                                runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
                                                const DiagnosticContext& diagnostics)
{
    ygg::Data<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, Tag>> data(
        node.n,
        parse_constructor_or_non_terminal(node.role, domain, repository, diagnostics).get_index(),
        parse_constructor_or_non_terminal(node.concept_, domain, repository, diagnostics).get_index());
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedAtLeastNumberRestriction<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_qualified_number_restriction_concept<runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedAtMostNumberRestriction<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_qualified_number_restriction_concept<runir::kr::dl::QualifiedAtMostNumberRestrictionTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedExactNumberRestriction<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_qualified_number_restriction_concept<runir::kr::dl::QualifiedExactNumberRestrictionTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptRoleValueMap<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_binary_concept<runir::kr::dl::RoleValueMapTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAgreement<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_binary_concept<runir::kr::dl::AgreementTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptRoleFillers<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::RoleFillersTag>> data(
        parse_constructor_or_non_terminal(node.role, domain, repository, diagnostics).get_index(),
        require_objects(domain, node.object_names, diagnostics));
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptOneOf<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::OneOfTag>> data(require_objects(domain, node.object_names, diagnostics));
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptNegation<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::NegationTag>> data(
        parse_constructor_or_non_terminal(node.arg, domain, repository, diagnostics).get_index());
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptNominal<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::NominalTag>> data(require_object(domain, node.object_name, diagnostics));
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleUniversal<runir::kr::UnsFamilyTag>&,
           tyr::formalism::planning::DomainView,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext&)
{
    ygg::Data<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::UniversalTag>> data;
    return intern_constructor<runir::kr::dl::RoleTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleAtomicState<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             2,
                             "RoleAtomicState",
                             diagnostics,
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 ygg::Data<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<T>>> data(predicate);
                                 return intern_constructor<runir::kr::dl::RoleTag>(repository, intern(repository, data).get_index());
                             });
}

auto parse(const runir::kr::dl::grammar::ast::RoleAtomicGoal<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             2,
                             "RoleAtomicGoal",
                             diagnostics,
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 ygg::Data<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<T>>> data(predicate, node.polarity);
                                 return intern_constructor<runir::kr::dl::RoleTag>(repository, intern(repository, data).get_index());
                             });
}

template<typename Tag, typename Ast>
auto parse_binary_role(const Ast& node,
                       tyr::formalism::planning::DomainView domain,
                       runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
                       const DiagnosticContext& diagnostics)
{
    ygg::Data<runir::kr::dl::Role<runir::kr::UnsFamilyTag, Tag>> data(parse_constructor_or_non_terminal(node.lhs, domain, repository, diagnostics).get_index(),
                                                                      parse_constructor_or_non_terminal(node.rhs, domain, repository, diagnostics).get_index());
    return intern_constructor<runir::kr::dl::RoleTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleIntersection<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_binary_role<runir::kr::dl::IntersectionTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::RoleUnion<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_binary_role<runir::kr::dl::UnionTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::RoleComposition<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_binary_role<runir::kr::dl::CompositionTag>(node, domain, repository, diagnostics);
}

template<typename Tag, typename Ast>
auto parse_unary_role(const Ast& node,
                      tyr::formalism::planning::DomainView domain,
                      runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
                      const DiagnosticContext& diagnostics)
{
    ygg::Data<runir::kr::dl::Role<runir::kr::UnsFamilyTag, Tag>> data(parse_constructor_or_non_terminal(node.arg, domain, repository, diagnostics).get_index());
    return intern_constructor<runir::kr::dl::RoleTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleComplement<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_unary_role<runir::kr::dl::ComplementTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::RoleInverse<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_unary_role<runir::kr::dl::InverseTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::RoleTransitiveClosure<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_unary_role<runir::kr::dl::TransitiveClosureTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::RoleReflexiveTransitiveClosure<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_unary_role<runir::kr::dl::ReflexiveTransitiveClosureTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::RoleRestriction<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::RestrictionTag>> data(
        parse_constructor_or_non_terminal(node.lhs, domain, repository, diagnostics).get_index(),
        parse_constructor_or_non_terminal(node.rhs, domain, repository, diagnostics).get_index());
    return intern_constructor<runir::kr::dl::RoleTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleIdentity<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::IdentityTag>> data(
        parse_constructor_or_non_terminal(node.arg, domain, repository, diagnostics).get_index());
    return intern_constructor<runir::kr::dl::RoleTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::BooleanAtomicState<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             0,
                             "BooleanAtomicState",
                             diagnostics,
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 ygg::Data<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<T>>> data(predicate, node.polarity);
                                 return intern_constructor<runir::kr::dl::BooleanTag>(repository, intern(repository, data).get_index());
                             });
}

auto parse(const runir::kr::dl::grammar::ast::BooleanAtomicGoal<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             0,
                             "BooleanAtomicGoal",
                             diagnostics,
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 ygg::Data<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<T>>> data(predicate, node.polarity);
                                 return intern_constructor<runir::kr::dl::BooleanTag>(repository, intern(repository, data).get_index());
                             });
}

auto parse(const runir::kr::dl::grammar::ast::BooleanNonempty<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    const auto arg = boost::apply_visitor(
        [&](const auto& value) -> ygg::Data<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::NonemptyTag>>::ConstructorVariant
        { return parse_constructor_or_non_terminal(unwrap(value), domain, repository, diagnostics).get_index(); },
        node.arg.get());

    ygg::Data<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::NonemptyTag>> data(arg);
    return intern_constructor<runir::kr::dl::BooleanTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::NumericalCount<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    const auto arg =
        boost::apply_visitor([&](const auto& value) -> ygg::Data<runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, runir::kr::dl::CountTag>>::ConstructorVariant
                             { return parse_constructor_or_non_terminal(unwrap(value), domain, repository, diagnostics).get_index(); },
                             node.arg.get());

    ygg::Data<runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, runir::kr::dl::CountTag>> data(arg);
    return intern_constructor<runir::kr::dl::NumericalTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::NumericalDistance<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, runir::kr::dl::DistanceTag>> data(
        parse_constructor_or_non_terminal(node.lhs, domain, repository, diagnostics).get_index(),
        parse_constructor_or_non_terminal(node.mid, domain, repository, diagnostics).get_index(),
        parse_constructor_or_non_terminal(node.rhs, domain, repository, diagnostics).get_index());
    return intern_constructor<runir::kr::dl::NumericalTag>(repository, intern(repository, data).get_index());
}

template<typename Tag, typename Ast>
auto parse_comparison(const Ast& node,
                      tyr::formalism::planning::DomainView domain,
                      runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
                      const DiagnosticContext& diagnostics)
{
    ygg::Data<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, Tag>> data(
        parse_constructor_or_non_terminal(node.lhs, domain, repository, diagnostics).get_index(),
        parse_constructor_or_non_terminal(node.rhs, domain, repository, diagnostics).get_index());
    return intern_constructor<runir::kr::dl::BooleanTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::BooleanEq<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_comparison<runir::kr::dl::EqTag<runir::kr::dl::BooleanTag>>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::BooleanNeq<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_comparison<runir::kr::dl::NeqTag<runir::kr::dl::BooleanTag>>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::BooleanLt<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_comparison<runir::kr::dl::LtTag<runir::kr::dl::BooleanTag>>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::BooleanLe<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_comparison<runir::kr::dl::LeTag<runir::kr::dl::BooleanTag>>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::BooleanGt<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_comparison<runir::kr::dl::GtTag<runir::kr::dl::BooleanTag>>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::BooleanGe<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_comparison<runir::kr::dl::GeTag<runir::kr::dl::BooleanTag>>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalEq<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_comparison<runir::kr::dl::EqTag<runir::kr::dl::NumericalTag>>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalNeq<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_comparison<runir::kr::dl::NeqTag<runir::kr::dl::NumericalTag>>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalLt<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_comparison<runir::kr::dl::LtTag<runir::kr::dl::NumericalTag>>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalLe<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_comparison<runir::kr::dl::LeTag<runir::kr::dl::NumericalTag>>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalGt<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_comparison<runir::kr::dl::GtTag<runir::kr::dl::NumericalTag>>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalGe<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_comparison<runir::kr::dl::GeTag<runir::kr::dl::NumericalTag>>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::BooleanConstant<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext&)
{
    ygg::Data<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanConstantTag>> data(node.value);
    return intern_constructor<runir::kr::dl::BooleanTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::NumericalConstant<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext&)
{
    ygg::Data<runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, runir::kr::dl::NumericalConstantTag>> data(node.value);
    return intern_constructor<runir::kr::dl::NumericalTag>(repository, intern(repository, data).get_index());
}

template<typename Tag, typename Ast>
auto parse_numerical_binary(const Ast& node,
                            tyr::formalism::planning::DomainView domain,
                            runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
                            const DiagnosticContext& diagnostics)
{
    ygg::Data<runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, Tag>> data(
        parse_constructor_or_non_terminal(node.lhs, domain, repository, diagnostics).get_index(),
        parse_constructor_or_non_terminal(node.rhs, domain, repository, diagnostics).get_index());
    return intern_constructor<runir::kr::dl::NumericalTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::NumericalAdd<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_numerical_binary<runir::kr::dl::AddTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalSub<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_numerical_binary<runir::kr::dl::SubTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalMul<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_numerical_binary<runir::kr::dl::MulTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalDiv<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_numerical_binary<runir::kr::dl::DivTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalMin<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_numerical_binary<runir::kr::dl::MinTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalMax<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_numerical_binary<runir::kr::dl::MaxTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::BooleanAnd<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_comparison<runir::kr::dl::AndTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::BooleanOr<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return parse_comparison<runir::kr::dl::OrTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::BooleanNot<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::NotTag>> data(
        parse_constructor_or_non_terminal(node.arg, domain, repository, diagnostics).get_index());
    return intern_constructor<runir::kr::dl::BooleanTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::CategoryTag Category>
auto parse_constructor(const runir::kr::dl::grammar::ast::Constructor<runir::kr::UnsFamilyTag, Category>& node,
                       tyr::formalism::planning::DomainView domain,
                       runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
                       const DiagnosticContext& diagnostics)
{
    return boost::apply_visitor([&](const auto& arg) { return parse(unwrap(arg), domain, repository, diagnostics); }, node.get());
}

auto parse_feature(const runir::kr::uns::dl::ast::BooleanFeature& node,
                   tyr::formalism::planning::DomainView domain,
                   Repository& repository,
                   const DiagnosticContext& diagnostics)
{
    ygg::Data<runir::kr::uns::dl::Feature> concrete_data(parse_constructor(node.feature, domain, repository.get_dl_repository(), diagnostics).get_index(),
                                                         node.symbol.text);
    auto concrete = intern(repository, concrete_data);

    ygg::Data<runir::kr::uns::Feature> data(concrete.get_index());
    return intern(repository, data);
}

}  // namespace

ClassifierView parse_classifier(const std::string& description, tyr::formalism::planning::DomainView domain, Repository& repository)
{
    auto first = description.cbegin();
    const auto last = description.cend();
    auto ast = runir::kr::uns::dl::ast::Classifier {};
    auto errors = std::ostringstream {};
    auto error_handler = runir::kr::parser::ErrorHandlerType(first, last, errors);
    if (!runir::kr::parser::parse_full(first, last, parser::classifier_root_parser(), ast, error_handler))
        throw runir::kr::parser::DiagnosticContext::parse_error(error_handler, "Failed to parse DL classifier description.", first);
    auto diagnostics = DiagnosticContext {};
    const auto scope = DiagnosticContext::Scope(diagnostics, error_handler);

    auto features = ygg::IndexList<runir::kr::uns::Feature> {};
    auto symbol_to_feature = std::unordered_map<std::string, ygg::Index<runir::kr::uns::Feature>> {};

    for (const auto& feature : ast.features)
    {
        auto wrapper = parse_feature(feature, domain, repository, diagnostics);
        if (!symbol_to_feature.emplace(feature.symbol.text, wrapper.get_index()).second)
            diagnostics.throw_at(feature.symbol, runir::kr::DuplicateDefinitionError("feature", feature.symbol.text));
        features.push_back(wrapper.get_index());
    }

    const auto parse_literal = [&](const auto& literal)
    {
        using Literal = std::remove_cvref_t<decltype(literal)>;
        const auto it = symbol_to_feature.find(literal.symbol.text);
        if (it == symbol_to_feature.end())
            diagnostics.throw_at(literal.symbol, runir::kr::UndefinedSymbolError("feature", literal.symbol.text));

        constexpr auto polarity = std::same_as<Literal, runir::kr::uns::dl::ast::PositiveLiteral>;
        ygg::Data<runir::kr::uns::ClassifierLiteral> literal_data(ygg::Data<runir::kr::uns::ClassifierLiteral>::Variant(it->second), polarity);
        return intern(repository, literal_data).get_index();
    };

    auto clauses = ygg::IndexList<runir::kr::uns::ClassifierClause> {};
    for (const auto& ast_clause : ast.expression.clauses)
    {
        auto literals = ygg::IndexList<runir::kr::uns::ClassifierLiteral> {};
        for (const auto& ast_literal : ast_clause.literals)
            literals.push_back(boost::apply_visitor(parse_literal, ast_literal.get()));

        ygg::Data<runir::kr::uns::ClassifierClause> clause_data;
        clause_data.literals = std::move(literals);
        clauses.push_back(intern(repository, clause_data).get_index());
    }

    ygg::Data<runir::kr::uns::Classifier> data(ast.symbol.text);
    data.features = std::move(features);
    data.clauses = std::move(clauses);
    return intern(repository, data);
}

}  // namespace runir::kr::uns::dl
