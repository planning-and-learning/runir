#include "runir/kr/dl/grammar/parser.hpp"

#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/dl/grammar/canonicalization.hpp"
#include "runir/kr/dl/grammar/parser/parser.hpp"
#include "runir/kr/parser/diagnostics.hpp"

#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <cista/containers/variant.h>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>

namespace runir::kr::dl::grammar
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
}  // namespace runir::kr::dl::grammar

template<typename T>
auto intern(ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository, ygg::Data<T>& data)
{
    canonicalize(data);
    return repository.get_or_create(data).first;
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename T>
auto intern_constructor(ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository, ygg::Index<T> index)
{
    ygg::Data<Constructor<Family, Category>> data(index);
    return intern(repository, data);
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
auto parse(const ast::Constructor<Family, Category>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics);

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
auto parse(const ast::NonTerminal<Family, Category>& node,
           tyr::formalism::planning::DomainView,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext&)
{
    ygg::Data<NonTerminal<Family, Category>> data(node.name.text);
    return intern(repository, data);
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
auto parse(const ast::ConstructorOrNonTerminal<Family, Category>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    const auto index = boost::apply_visitor([&](const auto& arg) -> ygg::Data<ConstructorOrNonTerminal<Family, Category>>::Variant
                                            { return parse(unwrap(arg), domain, repository, diagnostics).get_index(); },
                                            node.get());

    ygg::Data<ConstructorOrNonTerminal<Family, Category>> data(index);
    return intern(repository, data);
}  // namespace runir::kr::dl::grammar

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
                       const ast::Identifier& name,
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
                       const ast::Identifier& name,
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

auto require_object(tyr::formalism::planning::DomainView domain, const ast::Identifier& name, const DiagnosticContext& diagnostics)
{
    for (auto object : domain.get_constants())
        if (object.get_name() == name.text)
            return object.get_index();

    diagnostics.throw_at(name, runir::kr::UndefinedSymbolError("constant", name.text));
}

auto require_objects(tyr::formalism::planning::DomainView domain, const std::vector<ast::Identifier>& names, const DiagnosticContext& diagnostics)
{
    auto objects = ygg::IndexList<tyr::formalism::Object> {};
    for (const auto& name : names)
        objects.push_back(require_object(domain, name, diagnostics));
    return objects;
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptBot<Family>&,
           tyr::formalism::planning::DomainView,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext&)
{
    ygg::Data<Concept<Family, BotTag>> data;
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptTop<Family>&,
           tyr::formalism::planning::DomainView,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext&)
{
    ygg::Data<Concept<Family, TopTag>> data;
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptAtomicState<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
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
                                 ygg::Data<Concept<Family, AtomicStateTag<T>>> data(predicate);
                                 return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
                             });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptAtomicGoal<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
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
                                 ygg::Data<Concept<Family, AtomicGoalTag<T>>> data(predicate, node.polarity);
                                 return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
                             });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptIntersection<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<Concept<Family, IntersectionTag>> data(parse(node.lhs, domain, repository, diagnostics).get_index(),
                                                     parse(node.rhs, domain, repository, diagnostics).get_index());
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptUnion<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<Concept<Family, UnionTag>> data(parse(node.lhs, domain, repository, diagnostics).get_index(),
                                              parse(node.rhs, domain, repository, diagnostics).get_index());
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptNegation<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<Concept<Family, NegationTag>> data(parse(node.arg, domain, repository, diagnostics).get_index());
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptValueRestriction<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<Concept<Family, ValueRestrictionTag>> data(parse(node.lhs, domain, repository, diagnostics).get_index(),
                                                         parse(node.rhs, domain, repository, diagnostics).get_index());
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptExistentialQuantification<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<Concept<Family, ExistentialQuantificationTag>> data(parse(node.lhs, domain, repository, diagnostics).get_index(),
                                                                  parse(node.rhs, domain, repository, diagnostics).get_index());
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptAtLeastNumberRestriction<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<Concept<Family, AtLeastNumberRestrictionTag>> data(node.n, parse(node.role, domain, repository, diagnostics).get_index());
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptAtMostNumberRestriction<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<Concept<Family, AtMostNumberRestrictionTag>> data(node.n, parse(node.role, domain, repository, diagnostics).get_index());
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptExactNumberRestriction<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<Concept<Family, ExactNumberRestrictionTag>> data(node.n, parse(node.role, domain, repository, diagnostics).get_index());
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptQualifiedAtLeastNumberRestriction<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<Concept<Family, QualifiedAtLeastNumberRestrictionTag>> data(node.n,
                                                                          parse(node.role, domain, repository, diagnostics).get_index(),
                                                                          parse(node.concept_, domain, repository, diagnostics).get_index());
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptQualifiedAtMostNumberRestriction<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<Concept<Family, QualifiedAtMostNumberRestrictionTag>> data(node.n,
                                                                         parse(node.role, domain, repository, diagnostics).get_index(),
                                                                         parse(node.concept_, domain, repository, diagnostics).get_index());
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptQualifiedExactNumberRestriction<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<Concept<Family, QualifiedExactNumberRestrictionTag>> data(node.n,
                                                                        parse(node.role, domain, repository, diagnostics).get_index(),
                                                                        parse(node.concept_, domain, repository, diagnostics).get_index());
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptRoleValueMap<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<Concept<Family, RoleValueMapTag>> data(parse(node.lhs, domain, repository, diagnostics).get_index(),
                                                     parse(node.rhs, domain, repository, diagnostics).get_index());
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptAgreement<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<Concept<Family, AgreementTag>> data(parse(node.lhs, domain, repository, diagnostics).get_index(),
                                                  parse(node.rhs, domain, repository, diagnostics).get_index());
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptRoleFillers<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<Concept<Family, RoleFillersTag>> data(parse(node.role, domain, repository, diagnostics).get_index(),
                                                    require_objects(domain, node.object_names, diagnostics));
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptOneOf<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<Concept<Family, OneOfTag>> data(require_objects(domain, node.object_names, diagnostics));
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptNominal<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<Concept<Family, NominalTag>> data(require_object(domain, node.object_name, diagnostics));
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleUniversal<Family>&,
           tyr::formalism::planning::DomainView,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext&)
{
    ygg::Data<Role<Family, UniversalTag>> data;
    return intern_constructor<Family, RoleTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleAtomicState<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
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
                                 ygg::Data<Role<Family, AtomicStateTag<T>>> data(predicate);
                                 return intern_constructor<Family, RoleTag>(repository, intern(repository, data).get_index());
                             });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleAtomicGoal<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
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
                                 ygg::Data<Role<Family, AtomicGoalTag<T>>> data(predicate, node.polarity);
                                 return intern_constructor<Family, RoleTag>(repository, intern(repository, data).get_index());
                             });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleIntersection<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<Role<Family, IntersectionTag>> data(parse(node.lhs, domain, repository, diagnostics).get_index(),
                                                  parse(node.rhs, domain, repository, diagnostics).get_index());
    return intern_constructor<Family, RoleTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleUnion<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<Role<Family, UnionTag>> data(parse(node.lhs, domain, repository, diagnostics).get_index(),
                                           parse(node.rhs, domain, repository, diagnostics).get_index());
    return intern_constructor<Family, RoleTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleComplement<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<Role<Family, ComplementTag>> data(parse(node.arg, domain, repository, diagnostics).get_index());
    return intern_constructor<Family, RoleTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleInverse<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<Role<Family, InverseTag>> data(parse(node.arg, domain, repository, diagnostics).get_index());
    return intern_constructor<Family, RoleTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleComposition<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<Role<Family, CompositionTag>> data(parse(node.lhs, domain, repository, diagnostics).get_index(),
                                                 parse(node.rhs, domain, repository, diagnostics).get_index());
    return intern_constructor<Family, RoleTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleTransitiveClosure<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<Role<Family, TransitiveClosureTag>> data(parse(node.arg, domain, repository, diagnostics).get_index());
    return intern_constructor<Family, RoleTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleReflexiveTransitiveClosure<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<Role<Family, ReflexiveTransitiveClosureTag>> data(parse(node.arg, domain, repository, diagnostics).get_index());
    return intern_constructor<Family, RoleTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleRestriction<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<Role<Family, RestrictionTag>> data(parse(node.lhs, domain, repository, diagnostics).get_index(),
                                                 parse(node.rhs, domain, repository, diagnostics).get_index());
    return intern_constructor<Family, RoleTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleIdentity<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<Role<Family, IdentityTag>> data(parse(node.arg, domain, repository, diagnostics).get_index());
    return intern_constructor<Family, RoleTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::BooleanAtomicState<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
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
                                 ygg::Data<Boolean<Family, AtomicStateTag<T>>> data(predicate, node.polarity);
                                 return intern_constructor<Family, BooleanTag>(repository, intern(repository, data).get_index());
                             });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::BooleanAtomicGoal<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
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
                                 ygg::Data<Boolean<Family, AtomicGoalTag<T>>> data(predicate, node.polarity);
                                 return intern_constructor<Family, BooleanTag>(repository, intern(repository, data).get_index());
                             });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::BooleanNonempty<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    const auto arg = boost::apply_visitor([&](const auto& value) -> ygg::Data<Boolean<Family, NonemptyTag>>::Arg
                                          { return parse(unwrap(value), domain, repository, diagnostics).get_index(); },
                                          node.arg.get());

    ygg::Data<Boolean<Family, NonemptyTag>> data(arg);
    return intern_constructor<Family, BooleanTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::NumericalCount<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    const auto arg = boost::apply_visitor([&](const auto& value) -> ygg::Data<Numerical<Family, CountTag>>::Arg
                                          { return parse(unwrap(value), domain, repository, diagnostics).get_index(); },
                                          node.arg.get());

    ygg::Data<Numerical<Family, CountTag>> data(arg);
    return intern_constructor<Family, NumericalTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::NumericalDistance<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    ygg::Data<Numerical<Family, DistanceTag>> data(parse(node.lhs, domain, repository, diagnostics).get_index(),
                                                   parse(node.mid, domain, repository, diagnostics).get_index(),
                                                   parse(node.rhs, domain, repository, diagnostics).get_index());
    return intern_constructor<Family, NumericalTag>(repository, intern(repository, data).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
auto parse(const ast::Constructor<Family, Category>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    return boost::apply_visitor([&](const auto& arg) { return parse(unwrap(arg), domain, repository, diagnostics); }, node.get());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
auto parse(const ast::DerivationRule<Family, Category>& node,
           tyr::formalism::planning::DomainView domain,
           ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const DiagnosticContext& diagnostics)
{
    auto rhs = ygg::IndexList<ConstructorOrNonTerminal<Family, Category>> {};
    for (const auto& symbol : node.rhs)
        rhs.push_back(parse(symbol, domain, repository, diagnostics).get_index());

    ygg::Data<DerivationRule<Family, Category>> data(parse(node.lhs, domain, repository, diagnostics).get_index(), std::move(rhs));
    return intern(repository, data);
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
void append_derivation_rule(ygg::IndexList<DerivationRule<Family, Category>>& rules,
                            const ast::DerivationRule<Family, Category>& node,
                            tyr::formalism::planning::DomainView domain,
                            ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
                            const DiagnosticContext& diagnostics)
{
    rules.push_back(parse(node, domain, repository, diagnostics).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
void append_derivation_rule(ygg::Data<GrammarTag<Family>>& data,
                            const ast::DerivationRuleVariant<Family>& node,
                            tyr::formalism::planning::DomainView domain,
                            ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
                            const DiagnosticContext& diagnostics)
{
    boost::apply_visitor(
        [&](const auto& value)
        {
            const auto& rule = unwrap(value);
            using Category = typename std::decay_t<decltype(rule)>::CategoryTag;
            if constexpr (std::same_as<Category, ConceptTag>)
                append_derivation_rule(data.concept_derivation_rules, rule, domain, repository, diagnostics);
            else if constexpr (std::same_as<Category, RoleTag>)
                append_derivation_rule(data.role_derivation_rules, rule, domain, repository, diagnostics);
            else if constexpr (std::same_as<Category, BooleanTag>)
                append_derivation_rule(data.boolean_derivation_rules, rule, domain, repository, diagnostics);
            else if constexpr (std::same_as<Category, NumericalTag>)
                append_derivation_rule(data.numerical_derivation_rules, rule, domain, repository, diagnostics);
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
auto parse_fixed_start(tyr::formalism::planning::DomainView domain,
                       ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
                       const DiagnosticContext& diagnostics)
{
    ast::NonTerminal<runir::kr::BaseFamilyTag, Category> start;
    start.name.text = fixed_start_name<Category>();
    return parse(start, domain, repository, diagnostics).get_index();
}

auto parse_grammar_impl(const std::string& description,
                        tyr::formalism::planning::DomainView domain,
                        ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository)
{
    auto errors = std::ostringstream {};
    auto error_handler = runir::kr::parser::ErrorHandlerType(description.cbegin(), description.cend(), errors);
    auto ast = runir::kr::dl::grammar::ast::Grammar<runir::kr::BaseFamilyTag> {};
    parser::parse_grammar_ast(description, ast, error_handler);
    auto diagnostics = DiagnosticContext {};
    const auto scope = DiagnosticContext::Scope(diagnostics, error_handler);

    ygg::Data<GrammarTag<runir::kr::BaseFamilyTag>> data;

    data.concept_start = parse_fixed_start<ConceptTag>(domain, repository, diagnostics);
    data.role_start = parse_fixed_start<RoleTag>(domain, repository, diagnostics);
    data.boolean_start = parse_fixed_start<BooleanTag>(domain, repository, diagnostics);
    data.numerical_start = parse_fixed_start<NumericalTag>(domain, repository, diagnostics);

    for (const auto& rule : ast.rules)
        append_derivation_rule(data, rule, domain, repository, diagnostics);

    data.domain = domain.get_index();
    canonicalize(data);
    return intern(repository, data);
}

}  // namespace

FamilyGrammarView<runir::kr::BaseFamilyTag>
parse_grammar(const std::string& description, tyr::formalism::planning::DomainView domain, ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository)
{
    return parse_grammar_impl(description, domain, repository);
}

}  // namespace runir::kr::dl::grammar
