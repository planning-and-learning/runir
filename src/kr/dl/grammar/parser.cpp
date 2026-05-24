#include "runir/kr/dl/grammar/base/parser.hpp"

#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/dl/grammar/canonicalization.hpp"
#include "runir/kr/dl/grammar/parser/base/parser.hpp"

#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <cista/containers/variant.h>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace runir::kr::dl::grammar
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
auto intern(base::ConstructorRepository& repository, tyr::Data<T>& data)
{
    canonicalize(data);
    return repository.get_or_create(data).first;
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename T>
auto intern_constructor(base::ConstructorRepository& repository, tyr::Index<T> index)
{
    tyr::Data<Constructor<Family, Category>> data(index);
    return intern(repository, data);
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
auto parse(const ast::Constructor<Family, Category>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository);

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
auto parse(const ast::NonTerminal<Family, Category>& node, tyr::formalism::planning::DomainView, base::ConstructorRepository& repository)
{
    tyr::Data<NonTerminal<Family, Category>> data(node.name);
    return intern(repository, data);
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
auto parse(const ast::ConstructorOrNonTerminal<Family, Category>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    const auto index = boost::apply_visitor([&](const auto& arg) -> tyr::Data<ConstructorOrNonTerminal<Family, Category>>::Variant
                                            { return parse(unwrap(arg), domain, repository).get_index(); },
                                            node.get());

    tyr::Data<ConstructorOrNonTerminal<Family, Category>> data(index);
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

auto require_objects(tyr::formalism::planning::DomainView domain, const std::vector<std::string>& names)
{
    auto objects = tyr::IndexList<tyr::formalism::Object> {};
    for (const auto& name : names)
        objects.push_back(require_object(domain, name));
    return objects;
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptBot<Family>&, tyr::formalism::planning::DomainView, base::ConstructorRepository& repository)
{
    tyr::Data<Concept<Family, BotTag>> data;
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptTop<Family>&, tyr::formalism::planning::DomainView, base::ConstructorRepository& repository)
{
    tyr::Data<Concept<Family, TopTag>> data;
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptAtomicState<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             1,
                             "ConceptAtomicState",
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 tyr::Data<Concept<Family, AtomicStateTag<T>>> data(predicate);
                                 return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
                             });
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptAtomicGoal<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             1,
                             "ConceptAtomicGoal",
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 tyr::Data<Concept<Family, AtomicGoalTag<T>>> data(predicate, node.polarity);
                                 return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
                             });
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptIntersection<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    tyr::Data<Concept<Family, IntersectionTag>> data(parse(node.lhs, domain, repository).get_index(), parse(node.rhs, domain, repository).get_index());
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptUnion<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    tyr::Data<Concept<Family, UnionTag>> data(parse(node.lhs, domain, repository).get_index(), parse(node.rhs, domain, repository).get_index());
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptNegation<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    tyr::Data<Concept<Family, NegationTag>> data(parse(node.arg, domain, repository).get_index());
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptValueRestriction<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    tyr::Data<Concept<Family, ValueRestrictionTag>> data(parse(node.lhs, domain, repository).get_index(), parse(node.rhs, domain, repository).get_index());
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptExistentialQuantification<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    tyr::Data<Concept<Family, ExistentialQuantificationTag>> data(parse(node.lhs, domain, repository).get_index(),
                                                                  parse(node.rhs, domain, repository).get_index());
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptAtLeastNumberRestriction<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    tyr::Data<Concept<Family, AtLeastNumberRestrictionTag>> data(node.n, parse(node.role, domain, repository).get_index());
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptAtMostNumberRestriction<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    tyr::Data<Concept<Family, AtMostNumberRestrictionTag>> data(node.n, parse(node.role, domain, repository).get_index());
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptExactNumberRestriction<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    tyr::Data<Concept<Family, ExactNumberRestrictionTag>> data(node.n, parse(node.role, domain, repository).get_index());
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptQualifiedAtLeastNumberRestriction<Family>& node,
           tyr::formalism::planning::DomainView domain,
           base::ConstructorRepository& repository)
{
    tyr::Data<Concept<Family, QualifiedAtLeastNumberRestrictionTag>> data(node.n,
                                                                          parse(node.role, domain, repository).get_index(),
                                                                          parse(node.concept_, domain, repository).get_index());
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptQualifiedAtMostNumberRestriction<Family>& node,
           tyr::formalism::planning::DomainView domain,
           base::ConstructorRepository& repository)
{
    tyr::Data<Concept<Family, QualifiedAtMostNumberRestrictionTag>> data(node.n,
                                                                         parse(node.role, domain, repository).get_index(),
                                                                         parse(node.concept_, domain, repository).get_index());
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptQualifiedExactNumberRestriction<Family>& node,
           tyr::formalism::planning::DomainView domain,
           base::ConstructorRepository& repository)
{
    tyr::Data<Concept<Family, QualifiedExactNumberRestrictionTag>> data(node.n,
                                                                        parse(node.role, domain, repository).get_index(),
                                                                        parse(node.concept_, domain, repository).get_index());
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptRoleValueMap<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    tyr::Data<Concept<Family, RoleValueMapTag>> data(parse(node.lhs, domain, repository).get_index(), parse(node.rhs, domain, repository).get_index());
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptAgreement<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    tyr::Data<Concept<Family, AgreementTag>> data(parse(node.lhs, domain, repository).get_index(), parse(node.rhs, domain, repository).get_index());
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptRoleFillers<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    tyr::Data<Concept<Family, RoleFillersTag>> data(parse(node.role, domain, repository).get_index(), require_objects(domain, node.object_names));
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptOneOf<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    tyr::Data<Concept<Family, OneOfTag>> data(require_objects(domain, node.object_names));
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptNominal<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    tyr::Data<Concept<Family, NominalTag>> data(require_object(domain, node.object_name));
    return intern_constructor<Family, ConceptTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleUniversal<Family>&, tyr::formalism::planning::DomainView, base::ConstructorRepository& repository)
{
    tyr::Data<Role<Family, UniversalTag>> data;
    return intern_constructor<Family, RoleTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleAtomicState<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             2,
                             "RoleAtomicState",
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 tyr::Data<Role<Family, AtomicStateTag<T>>> data(predicate);
                                 return intern_constructor<Family, RoleTag>(repository, intern(repository, data).get_index());
                             });
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleAtomicGoal<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             2,
                             "RoleAtomicGoal",
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 tyr::Data<Role<Family, AtomicGoalTag<T>>> data(predicate, node.polarity);
                                 return intern_constructor<Family, RoleTag>(repository, intern(repository, data).get_index());
                             });
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleIntersection<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    tyr::Data<Role<Family, IntersectionTag>> data(parse(node.lhs, domain, repository).get_index(), parse(node.rhs, domain, repository).get_index());
    return intern_constructor<Family, RoleTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleUnion<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    tyr::Data<Role<Family, UnionTag>> data(parse(node.lhs, domain, repository).get_index(), parse(node.rhs, domain, repository).get_index());
    return intern_constructor<Family, RoleTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleComplement<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    tyr::Data<Role<Family, ComplementTag>> data(parse(node.arg, domain, repository).get_index());
    return intern_constructor<Family, RoleTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleInverse<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    tyr::Data<Role<Family, InverseTag>> data(parse(node.arg, domain, repository).get_index());
    return intern_constructor<Family, RoleTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleComposition<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    tyr::Data<Role<Family, CompositionTag>> data(parse(node.lhs, domain, repository).get_index(), parse(node.rhs, domain, repository).get_index());
    return intern_constructor<Family, RoleTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleTransitiveClosure<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    tyr::Data<Role<Family, TransitiveClosureTag>> data(parse(node.arg, domain, repository).get_index());
    return intern_constructor<Family, RoleTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleReflexiveTransitiveClosure<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    tyr::Data<Role<Family, ReflexiveTransitiveClosureTag>> data(parse(node.arg, domain, repository).get_index());
    return intern_constructor<Family, RoleTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleRestriction<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    tyr::Data<Role<Family, RestrictionTag>> data(parse(node.lhs, domain, repository).get_index(), parse(node.rhs, domain, repository).get_index());
    return intern_constructor<Family, RoleTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleIdentity<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    tyr::Data<Role<Family, IdentityTag>> data(parse(node.arg, domain, repository).get_index());
    return intern_constructor<Family, RoleTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::BooleanAtomicState<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             0,
                             "BooleanAtomicState",
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 tyr::Data<Boolean<Family, AtomicStateTag<T>>> data(predicate, node.polarity);
                                 return intern_constructor<Family, BooleanTag>(repository, intern(repository, data).get_index());
                             });
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::BooleanAtomicGoal<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             0,
                             "BooleanAtomicGoal",
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 tyr::Data<Boolean<Family, AtomicGoalTag<T>>> data(predicate, node.polarity);
                                 return intern_constructor<Family, BooleanTag>(repository, intern(repository, data).get_index());
                             });
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::BooleanNonempty<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    const auto arg = boost::apply_visitor([&](const auto& value) -> tyr::Data<Boolean<Family, NonemptyTag>>::Arg
                                          { return parse(unwrap(value), domain, repository).get_index(); },
                                          node.arg.get());

    tyr::Data<Boolean<Family, NonemptyTag>> data(arg);
    return intern_constructor<Family, BooleanTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::NumericalCount<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    const auto arg = boost::apply_visitor([&](const auto& value) -> tyr::Data<Numerical<Family, CountTag>>::Arg
                                          { return parse(unwrap(value), domain, repository).get_index(); },
                                          node.arg.get());

    tyr::Data<Numerical<Family, CountTag>> data(arg);
    return intern_constructor<Family, NumericalTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::NumericalDistance<Family>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    tyr::Data<Numerical<Family, DistanceTag>> data(parse(node.lhs, domain, repository).get_index(),
                                                   parse(node.mid, domain, repository).get_index(),
                                                   parse(node.rhs, domain, repository).get_index());
    return intern_constructor<Family, NumericalTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
auto parse(const ast::Constructor<Family, Category>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    return boost::apply_visitor([&](const auto& arg) { return parse(unwrap(arg), domain, repository); }, node.get());
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
auto parse(const ast::DerivationRule<Family, Category>& node, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    auto rhs = tyr::IndexList<ConstructorOrNonTerminal<Family, Category>> {};
    for (const auto& symbol : node.rhs)
        rhs.push_back(parse(symbol, domain, repository).get_index());

    tyr::Data<DerivationRule<Family, Category>> data(parse(node.lhs, domain, repository).get_index(), std::move(rhs));
    return intern(repository, data);
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
void append_derivation_rule(tyr::IndexList<DerivationRule<Family, Category>>& rules,
                            const ast::DerivationRule<Family, Category>& node,
                            tyr::formalism::planning::DomainView domain,
                            base::ConstructorRepository& repository)
{
    rules.push_back(parse(node, domain, repository).get_index());
}

template<runir::kr::dl::FamilyTag Family>
void append_derivation_rule(tyr::Data<GrammarTag<Family>>& data,
                            const ast::DerivationRuleVariant<Family>& node,
                            tyr::formalism::planning::DomainView domain,
                            base::ConstructorRepository& repository)
{
    boost::apply_visitor(
        [&](const auto& value)
        {
            const auto& rule = unwrap(value);
            using Category = typename std::decay_t<decltype(rule)>::CategoryTag;
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
auto parse_fixed_start(tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    ast::NonTerminal<runir::kr::dl::BaseFamilyTag, Category> start;
    start.name = fixed_start_name<Category>();
    return parse(start, domain, repository).get_index();
}

auto parse_grammar_impl(const std::string& description, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    const auto ast = parser::base::parse_grammar_ast(description);

    tyr::Data<GrammarTag<runir::kr::dl::BaseFamilyTag>> data;

    data.concept_start = parse_fixed_start<ConceptTag>(domain, repository);
    data.role_start = parse_fixed_start<RoleTag>(domain, repository);
    data.boolean_start = parse_fixed_start<BooleanTag>(domain, repository);
    data.numerical_start = parse_fixed_start<NumericalTag>(domain, repository);

    for (const auto& rule : ast.rules)
        append_derivation_rule(data, rule, domain, repository);

    data.domain = domain.get_index();
    canonicalize(data);
    return intern(repository, data);
}

}  // namespace

base::GrammarView base::parse_grammar(const std::string& description, tyr::formalism::planning::DomainView domain, base::ConstructorRepository& repository)
{
    return parse_grammar_impl(description, domain, repository);
}

}
