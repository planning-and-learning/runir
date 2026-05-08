#include "runir/grammar/parser.hpp"

#include "runir/grammar/canonicalization.hpp"

#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <cista/containers/variant.h>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace runir::grammar
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

template<typename T>
auto intern(ConstructorRepository& repository, tyr::Data<T>& data)
{
    canonicalize(data);
    return repository.get_or_create(data).first;
}

template<runir::CategoryTag Category, typename T>
auto intern_constructor(ConstructorRepository& repository, tyr::Index<T> index)
{
    tyr::Data<Constructor<Category>> data(index);
    return intern(repository, data);
}

template<runir::CategoryTag Category>
auto parse(const ast::Constructor<Category>& node, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository);

template<runir::CategoryTag Category>
auto parse(const ast::NonTerminal<Category>& node, tyr::formalism::planning::DomainView, ConstructorRepository& repository)
{
    tyr::Data<NonTerminal<Category>> data(node.name);
    return intern(repository, data);
}

template<runir::CategoryTag Category>
auto parse(const ast::ConstructorOrNonTerminal<Category>& node, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    const auto index = boost::apply_visitor([&](const auto& arg) -> tyr::Data<ConstructorOrNonTerminal<Category>>::Variant
                                            { return parse(unwrap(arg), domain, repository).get_index(); },
                                            node.get());

    tyr::Data<ConstructorOrNonTerminal<Category>> data(index);
    return intern(repository, data);
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
        return std::optional<tyr::Index<tyr::formalism::Predicate<T>>> {};

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

auto parse(const ast::ConceptBot&, tyr::formalism::planning::DomainView, ConstructorRepository& repository)
{
    tyr::Data<Concept<BotTag>> data;
    return intern_constructor<ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const ast::ConceptTop&, tyr::formalism::planning::DomainView, ConstructorRepository& repository)
{
    tyr::Data<Concept<TopTag>> data;
    return intern_constructor<ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const ast::ConceptAtomicState& node, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             1,
                             "ConceptAtomicState",
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 tyr::Data<Concept<AtomicStateTag<T>>> data(predicate);
                                 return intern_constructor<ConceptTag>(repository, intern(repository, data).get_index());
                             });
}

auto parse(const ast::ConceptAtomicGoal& node, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             1,
                             "ConceptAtomicGoal",
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 tyr::Data<Concept<AtomicGoalTag<T>>> data(predicate, node.polarity);
                                 return intern_constructor<ConceptTag>(repository, intern(repository, data).get_index());
                             });
}

auto parse(const ast::ConceptIntersection& node, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    tyr::Data<Concept<IntersectionTag>> data(parse(node.lhs, domain, repository).get_index(), parse(node.rhs, domain, repository).get_index());
    return intern_constructor<ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const ast::ConceptUnion& node, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    tyr::Data<Concept<UnionTag>> data(parse(node.lhs, domain, repository).get_index(), parse(node.rhs, domain, repository).get_index());
    return intern_constructor<ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const ast::ConceptNegation& node, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    tyr::Data<Concept<NegationTag>> data(parse(node.arg, domain, repository).get_index());
    return intern_constructor<ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const ast::ConceptValueRestriction& node, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    tyr::Data<Concept<ValueRestrictionTag>> data(parse(node.lhs, domain, repository).get_index(), parse(node.rhs, domain, repository).get_index());
    return intern_constructor<ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const ast::ConceptExistentialQuantification& node, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    tyr::Data<Concept<ExistentialQuantificationTag>> data(parse(node.lhs, domain, repository).get_index(), parse(node.rhs, domain, repository).get_index());
    return intern_constructor<ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const ast::ConceptRoleValueMapContainment& node, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    tyr::Data<Concept<RoleValueMapContainmentTag>> data(parse(node.lhs, domain, repository).get_index(), parse(node.rhs, domain, repository).get_index());
    return intern_constructor<ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const ast::ConceptRoleValueMapEquality& node, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    tyr::Data<Concept<RoleValueMapEqualityTag>> data(parse(node.lhs, domain, repository).get_index(), parse(node.rhs, domain, repository).get_index());
    return intern_constructor<ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const ast::ConceptNominal& node, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    tyr::Data<Concept<NominalTag>> data(require_object(domain, node.object_name));
    return intern_constructor<ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const ast::RoleUniversal&, tyr::formalism::planning::DomainView, ConstructorRepository& repository)
{
    tyr::Data<Role<UniversalTag>> data;
    return intern_constructor<RoleTag>(repository, intern(repository, data).get_index());
}

auto parse(const ast::RoleAtomicState& node, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             2,
                             "RoleAtomicState",
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 tyr::Data<Role<AtomicStateTag<T>>> data(predicate);
                                 return intern_constructor<RoleTag>(repository, intern(repository, data).get_index());
                             });
}

auto parse(const ast::RoleAtomicGoal& node, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             2,
                             "RoleAtomicGoal",
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 tyr::Data<Role<AtomicGoalTag<T>>> data(predicate, node.polarity);
                                 return intern_constructor<RoleTag>(repository, intern(repository, data).get_index());
                             });
}

auto parse(const ast::RoleIntersection& node, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    tyr::Data<Role<IntersectionTag>> data(parse(node.lhs, domain, repository).get_index(), parse(node.rhs, domain, repository).get_index());
    return intern_constructor<RoleTag>(repository, intern(repository, data).get_index());
}

auto parse(const ast::RoleUnion& node, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    tyr::Data<Role<UnionTag>> data(parse(node.lhs, domain, repository).get_index(), parse(node.rhs, domain, repository).get_index());
    return intern_constructor<RoleTag>(repository, intern(repository, data).get_index());
}

auto parse(const ast::RoleComplement& node, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    tyr::Data<Role<ComplementTag>> data(parse(node.arg, domain, repository).get_index());
    return intern_constructor<RoleTag>(repository, intern(repository, data).get_index());
}

auto parse(const ast::RoleInverse& node, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    tyr::Data<Role<InverseTag>> data(parse(node.arg, domain, repository).get_index());
    return intern_constructor<RoleTag>(repository, intern(repository, data).get_index());
}

auto parse(const ast::RoleComposition& node, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    tyr::Data<Role<CompositionTag>> data(parse(node.lhs, domain, repository).get_index(), parse(node.rhs, domain, repository).get_index());
    return intern_constructor<RoleTag>(repository, intern(repository, data).get_index());
}

auto parse(const ast::RoleTransitiveClosure& node, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    tyr::Data<Role<TransitiveClosureTag>> data(parse(node.arg, domain, repository).get_index());
    return intern_constructor<RoleTag>(repository, intern(repository, data).get_index());
}

auto parse(const ast::RoleReflexiveTransitiveClosure& node, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    tyr::Data<Role<ReflexiveTransitiveClosureTag>> data(parse(node.arg, domain, repository).get_index());
    return intern_constructor<RoleTag>(repository, intern(repository, data).get_index());
}

auto parse(const ast::RoleRestriction& node, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    tyr::Data<Role<RestrictionTag>> data(parse(node.lhs, domain, repository).get_index(), parse(node.rhs, domain, repository).get_index());
    return intern_constructor<RoleTag>(repository, intern(repository, data).get_index());
}

auto parse(const ast::RoleIdentity& node, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    tyr::Data<Role<IdentityTag>> data(parse(node.arg, domain, repository).get_index());
    return intern_constructor<RoleTag>(repository, intern(repository, data).get_index());
}

auto parse(const ast::BooleanAtomicState& node, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             0,
                             "BooleanAtomicState",
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 tyr::Data<Boolean<AtomicStateTag<T>>> data(predicate, node.polarity);
                                 return intern_constructor<BooleanTag>(repository, intern(repository, data).get_index());
                             });
}

auto parse(const ast::BooleanNonempty& node, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    const auto arg =
        boost::apply_visitor([&](const auto& value) -> tyr::Data<Boolean<NonemptyTag>>::Arg { return parse(unwrap(value), domain, repository).get_index(); },
                             node.arg.get());

    tyr::Data<Boolean<NonemptyTag>> data(arg);
    return intern_constructor<BooleanTag>(repository, intern(repository, data).get_index());
}

auto parse(const ast::NumericalCount& node, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    const auto arg =
        boost::apply_visitor([&](const auto& value) -> tyr::Data<Numerical<CountTag>>::Arg { return parse(unwrap(value), domain, repository).get_index(); },
                             node.arg.get());

    tyr::Data<Numerical<CountTag>> data(arg);
    return intern_constructor<NumericalTag>(repository, intern(repository, data).get_index());
}

auto parse(const ast::NumericalDistance& node, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    tyr::Data<Numerical<DistanceTag>> data(parse(node.lhs, domain, repository).get_index(),
                                           parse(node.mid, domain, repository).get_index(),
                                           parse(node.rhs, domain, repository).get_index());
    return intern_constructor<NumericalTag>(repository, intern(repository, data).get_index());
}

template<runir::CategoryTag Category>
auto parse(const ast::Constructor<Category>& node, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    return boost::apply_visitor([&](const auto& arg) { return parse(unwrap(arg), domain, repository); }, node.get());
}

template<runir::CategoryTag Category>
auto parse(const ast::DerivationRule<Category>& node, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    auto rhs = tyr::IndexList<ConstructorOrNonTerminal<Category>> {};
    for (const auto& symbol : node.rhs)
        rhs.push_back(parse(symbol, domain, repository).get_index());

    tyr::Data<DerivationRule<Category>> data(parse(node.lhs, domain, repository).get_index(), std::move(rhs));
    return intern(repository, data);
}

template<runir::CategoryTag Category>
void append_derivation_rule(tyr::IndexList<DerivationRule<Category>>& rules,
                            const ast::DerivationRule<Category>& node,
                            tyr::formalism::planning::DomainView domain,
                            ConstructorRepository& repository)
{
    rules.push_back(parse(node, domain, repository).get_index());
}

void append_derivation_rule(tyr::Data<GrammarTag>& data,
                            const ast::DerivationRuleVariant& node,
                            tyr::formalism::planning::DomainView domain,
                            ConstructorRepository& repository)
{
    boost::apply_visitor(
        [&](const auto& value)
        {
            const auto& rule = unwrap(value);
            using Category = typename std::decay_t<decltype(rule)>::Category;
            if constexpr (std::same_as<Category, ConceptTag>)
                append_derivation_rule(data.concept_derivation_rules, rule, domain, repository);
            else if constexpr (std::same_as<Category, RoleTag>)
                append_derivation_rule(data.role_derivation_rules, rule, domain, repository);
            else if constexpr (std::same_as<Category, BooleanTag>)
                append_derivation_rule(data.boolean_derivation_rules, rule, domain, repository);
            else if constexpr (std::same_as<Category, NumericalTag>)
                append_derivation_rule(data.numerical_derivation_rules, rule, domain, repository);
        },
        node.get());
}

}

GrammarView parse_grammar(const std::string& description, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    const auto ast = parser::parse_grammar_ast(description);

    tyr::Data<GrammarTag> data;

    if (ast.head.concept_start)
        data.concept_start = parse(*ast.head.concept_start, domain, repository).get_index();
    if (ast.head.role_start)
        data.role_start = parse(*ast.head.role_start, domain, repository).get_index();
    if (ast.head.boolean_start)
        data.boolean_start = parse(*ast.head.boolean_start, domain, repository).get_index();
    if (ast.head.numerical_start)
        data.numerical_start = parse(*ast.head.numerical_start, domain, repository).get_index();

    for (const auto& rule : ast.body.rules)
        append_derivation_rule(data, rule, domain, repository);

    data.domain = domain.get_index();
    canonicalize(data);
    return intern(repository, data);
}

}
