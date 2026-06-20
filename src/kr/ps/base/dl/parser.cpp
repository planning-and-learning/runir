#include "runir/kr/ps/base/dl/parser.hpp"

#include "runir/kr/dl/base/repository.hpp"
#include "runir/kr/ps/base/canonicalization.hpp"
#include "runir/kr/ps/base/dl/parser/parser.hpp"
#include "runir/kr/ps/dl/declarations.hpp"

#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>

namespace runir::kr::ps::base::dl
{
namespace
{

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
    if constexpr (requires { runir::kr::ps::base::canonicalize(data); })
        runir::kr::ps::base::canonicalize(data);
    else if constexpr (requires { runir::kr::ps::canonicalize(data); })
        runir::kr::ps::canonicalize(data);
    else if constexpr (requires { runir::kr::dl::canonicalize(data); })
        runir::kr::dl::canonicalize(data);
    else
        static_assert(ygg::dependent_false<T>::value, "Missing canonicalize overload for repository data.");

    return repository.get_or_create(data).first;
}

template<runir::kr::dl::CategoryTag Category, typename T>
auto intern_constructor(runir::kr::dl::base::ConstructorRepository& repository, ygg::Index<T> index)
{
    ygg::Data<runir::kr::dl::Constructor<runir::kr::BaseFamilyTag, Category>> data(index);
    return intern(repository, data);
}

template<runir::kr::dl::CategoryTag Category>
auto parse_constructor(const runir::kr::dl::grammar::ast::Constructor<runir::kr::BaseFamilyTag, Category>& node,
                       tyr::formalism::planning::DomainView domain,
                       runir::kr::dl::base::ConstructorRepository& repository);

template<runir::kr::dl::CategoryTag Category>
auto parse_constructor_or_non_terminal(const runir::kr::dl::grammar::ast::ConstructorOrNonTerminal<runir::kr::BaseFamilyTag, Category>& node,
                                       tyr::formalism::planning::DomainView domain,
                                       runir::kr::dl::base::ConstructorRepository& repository) -> runir::kr::dl::base::ConstructorView<Category>
{
    return boost::apply_visitor(
        [&](const auto& value) -> runir::kr::dl::base::ConstructorView<Category>
        {
            const auto& unwrapped = unwrap(value);
            if constexpr (std::same_as<std::remove_cvref_t<decltype(unwrapped)>, runir::kr::dl::grammar::ast::NonTerminal<runir::kr::BaseFamilyTag, Category>>)
                throw std::runtime_error("General-sketch DL features cannot reference grammar nonterminals.");
            else
                return parse_constructor(unwrapped, domain, repository);
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
auto require_predicate(tyr::formalism::planning::DomainView domain, const std::string& name, size_t arity, const char* constructor_name)
{
    auto predicate = find_predicate<T>(domain, name);
    if (!predicate)
        return std::optional<ygg::Index<tyr::formalism::Predicate<T>>> {};

    if (predicate->get_arity() != arity)
        throw std::runtime_error(std::string("Cannot construct ") + constructor_name + " from predicates with arity != " + std::to_string(arity) + ".");

    return std::optional(predicate->get_index());
}

template<typename Make>
auto resolve_predicate(tyr::formalism::planning::DomainView domain, const std::string& name, size_t arity, const char* constructor_name, Make&& make)
{
    if (auto predicate = require_predicate<tyr::formalism::StaticTag>(domain, name, arity, constructor_name))
        return make(tyr::formalism::StaticTag {}, *predicate);
    if (auto predicate = require_predicate<tyr::formalism::FluentTag>(domain, name, arity, constructor_name))
        return make(tyr::formalism::FluentTag {}, *predicate);
    if (auto predicate = require_predicate<tyr::formalism::DerivedTag>(domain, name, arity, constructor_name))
        return make(tyr::formalism::DerivedTag {}, *predicate);

    throw std::runtime_error("Predicate \"" + name + "\" is not part of the given domain.");
}

auto require_object(tyr::formalism::planning::DomainView domain, const std::string& name)
{
    for (auto object : domain.get_constants())
        if (object.get_name() == name)
            return object.get_index();

    throw std::runtime_error("Domain has no constant with name \"" + name + "\".");
}

auto require_objects(tyr::formalism::planning::DomainView domain, const std::vector<std::string>& names)
{
    auto result = ygg::IndexList<tyr::formalism::Object> {};
    result.reserve(names.size());
    for (const auto& name : names)
        result.push_back(require_object(domain, name));
    return result;
}

auto parse(const runir::kr::dl::grammar::ast::ConceptBot<runir::kr::BaseFamilyTag>&,
           tyr::formalism::planning::DomainView,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    ygg::Data<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::BotTag>> data;
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptTop<runir::kr::BaseFamilyTag>&,
           tyr::formalism::planning::DomainView,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    ygg::Data<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::TopTag>> data;
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtomicState<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             1,
                             "ConceptAtomicState",
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 ygg::Data<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<T>>> data(predicate);
                                 return intern_constructor<runir::kr::dl::ConceptTag>(repository, intern(repository, data).get_index());
                             });
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtomicGoal<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             1,
                             "ConceptAtomicGoal",
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 ygg::Data<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<T>>> data(predicate, node.polarity);
                                 return intern_constructor<runir::kr::dl::ConceptTag>(repository, intern(repository, data).get_index());
                             });
}

template<typename Tag, typename Ast>
auto parse_binary_concept(const Ast& node, tyr::formalism::planning::DomainView domain, runir::kr::dl::base::ConstructorRepository& repository)
{
    ygg::Data<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, Tag>> data(parse_constructor_or_non_terminal(node.lhs, domain, repository).get_index(),
                                                                          parse_constructor_or_non_terminal(node.rhs, domain, repository).get_index());
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptIntersection<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    return parse_binary_concept<runir::kr::dl::IntersectionTag>(node, domain, repository);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptUnion<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    return parse_binary_concept<runir::kr::dl::UnionTag>(node, domain, repository);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptValueRestriction<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    return parse_binary_concept<runir::kr::dl::ValueRestrictionTag>(node, domain, repository);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptExistentialQuantification<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    return parse_binary_concept<runir::kr::dl::ExistentialQuantificationTag>(node, domain, repository);
}

template<typename Tag, typename Ast>
auto parse_number_restriction_concept(const Ast& node, tyr::formalism::planning::DomainView domain, runir::kr::dl::base::ConstructorRepository& repository)
{
    ygg::Data<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, Tag>> data(node.n, parse_constructor_or_non_terminal(node.role, domain, repository).get_index());
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtLeastNumberRestriction<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    return parse_number_restriction_concept<runir::kr::dl::AtLeastNumberRestrictionTag>(node, domain, repository);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtMostNumberRestriction<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    return parse_number_restriction_concept<runir::kr::dl::AtMostNumberRestrictionTag>(node, domain, repository);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptExactNumberRestriction<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    return parse_number_restriction_concept<runir::kr::dl::ExactNumberRestrictionTag>(node, domain, repository);
}

template<typename Tag, typename Ast>
auto parse_qualified_number_restriction_concept(const Ast& node,
                                                tyr::formalism::planning::DomainView domain,
                                                runir::kr::dl::base::ConstructorRepository& repository)
{
    ygg::Data<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, Tag>> data(node.n,
                                                                          parse_constructor_or_non_terminal(node.role, domain, repository).get_index(),
                                                                          parse_constructor_or_non_terminal(node.concept_, domain, repository).get_index());
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedAtLeastNumberRestriction<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    return parse_qualified_number_restriction_concept<runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>(node, domain, repository);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedAtMostNumberRestriction<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    return parse_qualified_number_restriction_concept<runir::kr::dl::QualifiedAtMostNumberRestrictionTag>(node, domain, repository);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedExactNumberRestriction<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    return parse_qualified_number_restriction_concept<runir::kr::dl::QualifiedExactNumberRestrictionTag>(node, domain, repository);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptRoleValueMap<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    return parse_binary_concept<runir::kr::dl::RoleValueMapTag>(node, domain, repository);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAgreement<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    return parse_binary_concept<runir::kr::dl::AgreementTag>(node, domain, repository);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptRoleFillers<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    ygg::Data<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::RoleFillersTag>> data(
        parse_constructor_or_non_terminal(node.role, domain, repository).get_index(),
        require_objects(domain, node.object_names));
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptOneOf<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    ygg::Data<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::OneOfTag>> data(require_objects(domain, node.object_names));
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptNegation<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    ygg::Data<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::NegationTag>> data(
        parse_constructor_or_non_terminal(node.arg, domain, repository).get_index());
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptNominal<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    ygg::Data<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::NominalTag>> data(require_object(domain, node.object_name));
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleUniversal<runir::kr::BaseFamilyTag>&,
           tyr::formalism::planning::DomainView,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    ygg::Data<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::UniversalTag>> data;
    return intern_constructor<runir::kr::dl::RoleTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleAtomicState<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             2,
                             "RoleAtomicState",
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 ygg::Data<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<T>>> data(predicate);
                                 return intern_constructor<runir::kr::dl::RoleTag>(repository, intern(repository, data).get_index());
                             });
}

auto parse(const runir::kr::dl::grammar::ast::RoleAtomicGoal<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             2,
                             "RoleAtomicGoal",
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 ygg::Data<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<T>>> data(predicate, node.polarity);
                                 return intern_constructor<runir::kr::dl::RoleTag>(repository, intern(repository, data).get_index());
                             });
}

template<typename Tag, typename Ast>
auto parse_binary_role(const Ast& node, tyr::formalism::planning::DomainView domain, runir::kr::dl::base::ConstructorRepository& repository)
{
    ygg::Data<runir::kr::dl::Role<runir::kr::BaseFamilyTag, Tag>> data(parse_constructor_or_non_terminal(node.lhs, domain, repository).get_index(),
                                                                       parse_constructor_or_non_terminal(node.rhs, domain, repository).get_index());
    return intern_constructor<runir::kr::dl::RoleTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleIntersection<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    return parse_binary_role<runir::kr::dl::IntersectionTag>(node, domain, repository);
}

auto parse(const runir::kr::dl::grammar::ast::RoleUnion<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    return parse_binary_role<runir::kr::dl::UnionTag>(node, domain, repository);
}

auto parse(const runir::kr::dl::grammar::ast::RoleComposition<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    return parse_binary_role<runir::kr::dl::CompositionTag>(node, domain, repository);
}

template<typename Tag, typename Ast>
auto parse_unary_role(const Ast& node, tyr::formalism::planning::DomainView domain, runir::kr::dl::base::ConstructorRepository& repository)
{
    ygg::Data<runir::kr::dl::Role<runir::kr::BaseFamilyTag, Tag>> data(parse_constructor_or_non_terminal(node.arg, domain, repository).get_index());
    return intern_constructor<runir::kr::dl::RoleTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleComplement<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    return parse_unary_role<runir::kr::dl::ComplementTag>(node, domain, repository);
}

auto parse(const runir::kr::dl::grammar::ast::RoleInverse<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    return parse_unary_role<runir::kr::dl::InverseTag>(node, domain, repository);
}

auto parse(const runir::kr::dl::grammar::ast::RoleTransitiveClosure<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    return parse_unary_role<runir::kr::dl::TransitiveClosureTag>(node, domain, repository);
}

auto parse(const runir::kr::dl::grammar::ast::RoleReflexiveTransitiveClosure<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    return parse_unary_role<runir::kr::dl::ReflexiveTransitiveClosureTag>(node, domain, repository);
}

auto parse(const runir::kr::dl::grammar::ast::RoleRestriction<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    ygg::Data<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::RestrictionTag>> data(
        parse_constructor_or_non_terminal(node.lhs, domain, repository).get_index(),
        parse_constructor_or_non_terminal(node.rhs, domain, repository).get_index());
    return intern_constructor<runir::kr::dl::RoleTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleIdentity<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    ygg::Data<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::IdentityTag>> data(
        parse_constructor_or_non_terminal(node.arg, domain, repository).get_index());
    return intern_constructor<runir::kr::dl::RoleTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::BooleanAtomicState<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             0,
                             "BooleanAtomicState",
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 ygg::Data<runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<T>>> data(predicate, node.polarity);
                                 return intern_constructor<runir::kr::dl::BooleanTag>(repository, intern(repository, data).get_index());
                             });
}

auto parse(const runir::kr::dl::grammar::ast::BooleanAtomicGoal<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             0,
                             "BooleanAtomicGoal",
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 ygg::Data<runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<T>>> data(predicate, node.polarity);
                                 return intern_constructor<runir::kr::dl::BooleanTag>(repository, intern(repository, data).get_index());
                             });
}

auto parse(const runir::kr::dl::grammar::ast::BooleanNonempty<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    const auto arg = boost::apply_visitor(
        [&](const auto& value) -> ygg::Data<runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::NonemptyTag>>::ConstructorVariant
        { return parse_constructor_or_non_terminal(unwrap(value), domain, repository).get_index(); },
        node.arg.get());

    ygg::Data<runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::NonemptyTag>> data(arg);
    return intern_constructor<runir::kr::dl::BooleanTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::NumericalCount<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    const auto arg = boost::apply_visitor(
        [&](const auto& value) -> ygg::Data<runir::kr::dl::Numerical<runir::kr::BaseFamilyTag, runir::kr::dl::CountTag>>::ConstructorVariant
        { return parse_constructor_or_non_terminal(unwrap(value), domain, repository).get_index(); },
        node.arg.get());

    ygg::Data<runir::kr::dl::Numerical<runir::kr::BaseFamilyTag, runir::kr::dl::CountTag>> data(arg);
    return intern_constructor<runir::kr::dl::NumericalTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::NumericalDistance<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::base::ConstructorRepository& repository)
{
    ygg::Data<runir::kr::dl::Numerical<runir::kr::BaseFamilyTag, runir::kr::dl::DistanceTag>> data(
        parse_constructor_or_non_terminal(node.lhs, domain, repository).get_index(),
        parse_constructor_or_non_terminal(node.mid, domain, repository).get_index(),
        parse_constructor_or_non_terminal(node.rhs, domain, repository).get_index());
    return intern_constructor<runir::kr::dl::NumericalTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::CategoryTag Category>
auto parse_constructor(const runir::kr::dl::grammar::ast::Constructor<runir::kr::BaseFamilyTag, Category>& node,
                       tyr::formalism::planning::DomainView domain,
                       runir::kr::dl::base::ConstructorRepository& repository)
{
    return boost::apply_visitor([&](const auto& arg) { return parse(unwrap(arg), domain, repository); }, node.get());
}

using BooleanFeatureMap = std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>>>;
using NumericalFeatureMap = std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>>>;

auto parse_feature(const runir::kr::ps::base::dl::ast::BooleanFeature<runir::kr::BaseFamilyTag>& node,
                   tyr::formalism::planning::DomainView domain,
                   Repository& repository,
                   BooleanFeatureMap& boolean_features,
                   NumericalFeatureMap&,
                   ygg::Data<runir::kr::ps::base::Sketch>& sketch_data)
{
    ygg::Data<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>> concrete_data(
        parse_constructor(node.feature, domain, repository.get_dl_repository()).get_index(),
        node.symbol,
        node.description);
    auto concrete = intern(repository, concrete_data);
    ygg::Data<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>> data(concrete.get_index());
    auto feature = intern(repository, data);
    boolean_features.emplace(node.symbol, feature.get_index());
    sketch_data.boolean_features.push_back(feature.get_index());
}

auto parse_feature(const runir::kr::ps::base::dl::ast::NumericalFeature<runir::kr::BaseFamilyTag>& node,
                   tyr::formalism::planning::DomainView domain,
                   Repository& repository,
                   BooleanFeatureMap&,
                   NumericalFeatureMap& numerical_features,
                   ygg::Data<runir::kr::ps::base::Sketch>& sketch_data)
{
    ygg::Data<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature>> concrete_data(
        parse_constructor(node.feature, domain, repository.get_dl_repository()).get_index(),
        node.symbol,
        node.description);
    auto concrete = intern(repository, concrete_data);
    ygg::Data<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>> data(concrete.get_index());
    auto feature = intern(repository, data);
    numerical_features.emplace(node.symbol, feature.get_index());
    sketch_data.numerical_features.push_back(feature.get_index());
}

template<typename FeatureTag>
auto require_feature(const std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, FeatureTag>>>& features,
                     const std::string& name)
{
    const auto it = features.find(name);
    if (it == features.end())
        throw std::runtime_error("Unknown feature \"" + name + "\".");
    return it->second;
}

template<typename FeatureTag, typename ObservationTag>
auto make_condition(ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, FeatureTag>> feature, Repository& repository)
{
    ygg::Data<runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>> concrete_data(feature);
    auto concrete = intern(repository, concrete_data);
    ygg::Data<runir::kr::ps::ConcreteConditionVariant<runir::kr::BaseFamilyTag, runir::kr::DlTag>> variant_data(concrete.get_index());
    auto variant = intern(repository, variant_data);
    ygg::Data<runir::kr::ps::ConditionVariant<runir::kr::BaseFamilyTag>> data(variant.get_index());
    return intern(repository, data);
}

template<typename FeatureTag, typename ObservationTag>
auto make_effect(ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, FeatureTag>> feature, Repository& repository)
{
    ygg::Data<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>> concrete_data(feature);
    auto concrete = intern(repository, concrete_data);
    ygg::Data<runir::kr::ps::ConcreteEffectVariant<runir::kr::BaseFamilyTag, runir::kr::DlTag>> variant_data(concrete.get_index());
    auto variant = intern(repository, variant_data);
    ygg::Data<runir::kr::ps::EffectVariant<runir::kr::BaseFamilyTag>> data(variant.get_index());
    return intern(repository, data);
}

auto parse_condition_observation(const runir::kr::ps::base::dl::ast::Positive&,
                                 const std::string& feature,
                                 Repository& repository,
                                 const BooleanFeatureMap& boolean_features,
                                 const NumericalFeatureMap&)
{
    return make_condition<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>(require_feature(boolean_features, feature), repository);
}

auto parse_condition_observation(const runir::kr::ps::base::dl::ast::Negative&,
                                 const std::string& feature,
                                 Repository& repository,
                                 const BooleanFeatureMap& boolean_features,
                                 const NumericalFeatureMap&)
{
    return make_condition<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>(require_feature(boolean_features, feature), repository);
}

auto parse_condition_observation(const runir::kr::ps::base::dl::ast::EqualZero&,
                                 const std::string& feature,
                                 Repository& repository,
                                 const BooleanFeatureMap&,
                                 const NumericalFeatureMap& numerical_features)
{
    return make_condition<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::EqualZero>(require_feature(numerical_features, feature), repository);
}

auto parse_condition_observation(const runir::kr::ps::base::dl::ast::GreaterZero&,
                                 const std::string& feature,
                                 Repository& repository,
                                 const BooleanFeatureMap&,
                                 const NumericalFeatureMap& numerical_features)
{
    return make_condition<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::GreaterZero>(require_feature(numerical_features, feature), repository);
}

auto parse_condition(const runir::kr::ps::base::dl::ast::Condition<runir::kr::BaseFamilyTag>& node,
                     Repository& repository,
                     const BooleanFeatureMap& boolean_features,
                     const NumericalFeatureMap& numerical_features)
{
    return boost::apply_visitor([&](const auto& observation)
                                { return parse_condition_observation(observation, node.feature, repository, boolean_features, numerical_features); },
                                node.observation.get());
}

auto parse_effect_observation(const runir::kr::ps::base::dl::ast::Positive&,
                              const std::string& feature,
                              Repository& repository,
                              const BooleanFeatureMap& boolean_features,
                              const NumericalFeatureMap&)
{
    return make_effect<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>(require_feature(boolean_features, feature), repository);
}

auto parse_effect_observation(const runir::kr::ps::base::dl::ast::Negative&,
                              const std::string& feature,
                              Repository& repository,
                              const BooleanFeatureMap& boolean_features,
                              const NumericalFeatureMap&)
{
    return make_effect<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>(require_feature(boolean_features, feature), repository);
}

auto parse_effect_observation(const runir::kr::ps::base::dl::ast::Unchanged&,
                              const std::string& feature,
                              Repository& repository,
                              const BooleanFeatureMap& boolean_features,
                              const NumericalFeatureMap& numerical_features)
{
    const auto boolean_it = boolean_features.find(feature);
    const auto numerical_it = numerical_features.find(feature);

    if (boolean_it != boolean_features.end() && numerical_it != numerical_features.end())
        throw std::runtime_error("Ambiguous feature \"" + feature + "\".");
    if (boolean_it != boolean_features.end())
        return make_effect<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Unchanged>(boolean_it->second, repository);
    if (numerical_it != numerical_features.end())
        return make_effect<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Unchanged>(numerical_it->second, repository);

    throw std::runtime_error("Unknown feature \"" + feature + "\".");
}

auto parse_effect_observation(const runir::kr::ps::base::dl::ast::Increases&,
                              const std::string& feature,
                              Repository& repository,
                              const BooleanFeatureMap&,
                              const NumericalFeatureMap& numerical_features)
{
    return make_effect<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Increases>(require_feature(numerical_features, feature), repository);
}

auto parse_effect_observation(const runir::kr::ps::base::dl::ast::Decreases&,
                              const std::string& feature,
                              Repository& repository,
                              const BooleanFeatureMap&,
                              const NumericalFeatureMap& numerical_features)
{
    return make_effect<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Decreases>(require_feature(numerical_features, feature), repository);
}

auto parse_effect(const runir::kr::ps::base::dl::ast::Effect<runir::kr::BaseFamilyTag>& node,
                  Repository& repository,
                  const BooleanFeatureMap& boolean_features,
                  const NumericalFeatureMap& numerical_features)
{
    return boost::apply_visitor([&](const auto& observation)
                                { return parse_effect_observation(observation, node.feature, repository, boolean_features, numerical_features); },
                                node.observation.get());
}

auto parse_rule(const runir::kr::ps::base::dl::ast::Rule<runir::kr::BaseFamilyTag>& node,
                Repository& repository,
                const BooleanFeatureMap& boolean_features,
                const NumericalFeatureMap& numerical_features)
{
    auto conditions = ygg::IndexList<runir::kr::ps::ConditionVariant<runir::kr::BaseFamilyTag>> {};
    for (const auto& condition : node.conditions)
        conditions.push_back(parse_condition(condition, repository, boolean_features, numerical_features).get_index());

    auto effects = ygg::IndexList<runir::kr::ps::EffectVariant<runir::kr::BaseFamilyTag>> {};
    for (const auto& effect : node.effects)
        effects.push_back(parse_effect(effect, repository, boolean_features, numerical_features).get_index());

    ygg::Data<runir::kr::ps::base::Rule> data(node.symbol, node.description);
    data.conditions = std::move(conditions);
    data.effects = std::move(effects);
    return intern(repository, data);
}

}  // namespace

SketchView parse_sketch(const std::string& description, tyr::formalism::planning::DomainView domain, Repository& repository)
{
    const auto ast = parser::parse_sketch_ast(description);

    auto boolean_features = BooleanFeatureMap {};
    auto numerical_features = NumericalFeatureMap {};
    ygg::Data<runir::kr::ps::base::Sketch> data;

    for (const auto& feature : ast.features)
        boost::apply_visitor([&](const auto& arg) { parse_feature(arg, domain, repository, boolean_features, numerical_features, data); }, feature.get());

    auto rules = ygg::IndexList<runir::kr::ps::base::Rule> {};
    for (const auto& rule : ast.rules)
        rules.push_back(parse_rule(rule, repository, boolean_features, numerical_features).get_index());

    data.rules = std::move(rules);
    return intern(repository, data);
}

}  // namespace runir::kr::ps::base::dl
