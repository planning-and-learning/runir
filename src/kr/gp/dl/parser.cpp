#include "runir/kr/gp/dl/parser.hpp"

#include "runir/kr/gp/canonicalization.hpp"
#include "runir/kr/gp/dl/parser/parser.hpp"

#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>

namespace runir::kr::gp::dl
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
auto intern(Repository& repository, tyr::Data<T>& data)
{
    if constexpr (requires { runir::kr::gp::canonicalize(data); })
        runir::kr::gp::canonicalize(data);
    else if constexpr (requires { runir::kr::dl::canonicalize(data); })
        runir::kr::dl::canonicalize(data);
    else
        static_assert(tyr::dependent_false<T>::value, "Missing canonicalize overload for repository data.");

    return repository.get_or_create(data).first;
}

template<runir::kr::dl::CategoryTag Category, typename T>
auto intern_constructor(runir::kr::dl::ConstructorRepository& repository, tyr::Index<T> index)
{
    tyr::Data<runir::kr::dl::Constructor<Category>> data(index);
    return intern(repository, data);
}

template<runir::kr::dl::CategoryTag Category>
auto parse_constructor(const runir::kr::dl::grammar::ast::Constructor<Category>& node,
                       tyr::formalism::planning::DomainView domain,
                       runir::kr::dl::ConstructorRepository& repository);

template<runir::kr::dl::CategoryTag Category>
auto parse_constructor_or_non_terminal(const runir::kr::dl::grammar::ast::ConstructorOrNonTerminal<Category>& node,
                                       tyr::formalism::planning::DomainView domain,
                                       runir::kr::dl::ConstructorRepository& repository) -> runir::kr::dl::ConstructorView<Category>
{
    return boost::apply_visitor(
        [&](const auto& value) -> runir::kr::dl::ConstructorView<Category>
        {
            const auto& unwrapped = unwrap(value);
            if constexpr (std::same_as<std::remove_cvref_t<decltype(unwrapped)>, runir::kr::dl::grammar::ast::NonTerminal<Category>>)
                throw std::runtime_error("General-policy DL features cannot reference grammar nonterminals.");
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

auto parse(const runir::kr::dl::grammar::ast::ConceptBot&, tyr::formalism::planning::DomainView, runir::kr::dl::ConstructorRepository& repository)
{
    tyr::Data<runir::kr::dl::Concept<runir::kr::dl::BotTag>> data;
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptTop&, tyr::formalism::planning::DomainView, runir::kr::dl::ConstructorRepository& repository)
{
    tyr::Data<runir::kr::dl::Concept<runir::kr::dl::TopTag>> data;
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtomicState& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepository& repository)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             1,
                             "ConceptAtomicState",
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 tyr::Data<runir::kr::dl::Concept<runir::kr::dl::AtomicStateTag<T>>> data(predicate);
                                 return intern_constructor<runir::kr::dl::ConceptTag>(repository, intern(repository, data).get_index());
                             });
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtomicGoal& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepository& repository)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             1,
                             "ConceptAtomicGoal",
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 tyr::Data<runir::kr::dl::Concept<runir::kr::dl::AtomicGoalTag<T>>> data(predicate, node.polarity);
                                 return intern_constructor<runir::kr::dl::ConceptTag>(repository, intern(repository, data).get_index());
                             });
}

template<typename Tag, typename Ast>
auto parse_binary_concept(const Ast& node, tyr::formalism::planning::DomainView domain, runir::kr::dl::ConstructorRepository& repository)
{
    tyr::Data<runir::kr::dl::Concept<Tag>> data(parse_constructor_or_non_terminal(node.lhs, domain, repository).get_index(),
                                                parse_constructor_or_non_terminal(node.rhs, domain, repository).get_index());
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptIntersection& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepository& repository)
{
    return parse_binary_concept<runir::kr::dl::IntersectionTag>(node, domain, repository);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptUnion& node, tyr::formalism::planning::DomainView domain, runir::kr::dl::ConstructorRepository& repository)
{
    return parse_binary_concept<runir::kr::dl::UnionTag>(node, domain, repository);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptValueRestriction& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepository& repository)
{
    return parse_binary_concept<runir::kr::dl::ValueRestrictionTag>(node, domain, repository);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptExistentialQuantification& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepository& repository)
{
    return parse_binary_concept<runir::kr::dl::ExistentialQuantificationTag>(node, domain, repository);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptRoleValueMapContainment& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepository& repository)
{
    return parse_binary_concept<runir::kr::dl::RoleValueMapContainmentTag>(node, domain, repository);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptRoleValueMapEquality& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepository& repository)
{
    return parse_binary_concept<runir::kr::dl::RoleValueMapEqualityTag>(node, domain, repository);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptNegation& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepository& repository)
{
    tyr::Data<runir::kr::dl::Concept<runir::kr::dl::NegationTag>> data(parse_constructor_or_non_terminal(node.arg, domain, repository).get_index());
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptNominal& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepository& repository)
{
    tyr::Data<runir::kr::dl::Concept<runir::kr::dl::NominalTag>> data(require_object(domain, node.object_name));
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleUniversal&, tyr::formalism::planning::DomainView, runir::kr::dl::ConstructorRepository& repository)
{
    tyr::Data<runir::kr::dl::Role<runir::kr::dl::UniversalTag>> data;
    return intern_constructor<runir::kr::dl::RoleTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleAtomicState& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepository& repository)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             2,
                             "RoleAtomicState",
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 tyr::Data<runir::kr::dl::Role<runir::kr::dl::AtomicStateTag<T>>> data(predicate);
                                 return intern_constructor<runir::kr::dl::RoleTag>(repository, intern(repository, data).get_index());
                             });
}

auto parse(const runir::kr::dl::grammar::ast::RoleAtomicGoal& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepository& repository)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             2,
                             "RoleAtomicGoal",
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 tyr::Data<runir::kr::dl::Role<runir::kr::dl::AtomicGoalTag<T>>> data(predicate, node.polarity);
                                 return intern_constructor<runir::kr::dl::RoleTag>(repository, intern(repository, data).get_index());
                             });
}

template<typename Tag, typename Ast>
auto parse_binary_role(const Ast& node, tyr::formalism::planning::DomainView domain, runir::kr::dl::ConstructorRepository& repository)
{
    tyr::Data<runir::kr::dl::Role<Tag>> data(parse_constructor_or_non_terminal(node.lhs, domain, repository).get_index(),
                                             parse_constructor_or_non_terminal(node.rhs, domain, repository).get_index());
    return intern_constructor<runir::kr::dl::RoleTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleIntersection& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepository& repository)
{
    return parse_binary_role<runir::kr::dl::IntersectionTag>(node, domain, repository);
}

auto parse(const runir::kr::dl::grammar::ast::RoleUnion& node, tyr::formalism::planning::DomainView domain, runir::kr::dl::ConstructorRepository& repository)
{
    return parse_binary_role<runir::kr::dl::UnionTag>(node, domain, repository);
}

auto parse(const runir::kr::dl::grammar::ast::RoleComposition& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepository& repository)
{
    return parse_binary_role<runir::kr::dl::CompositionTag>(node, domain, repository);
}

template<typename Tag, typename Ast>
auto parse_unary_role(const Ast& node, tyr::formalism::planning::DomainView domain, runir::kr::dl::ConstructorRepository& repository)
{
    tyr::Data<runir::kr::dl::Role<Tag>> data(parse_constructor_or_non_terminal(node.arg, domain, repository).get_index());
    return intern_constructor<runir::kr::dl::RoleTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleComplement& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepository& repository)
{
    return parse_unary_role<runir::kr::dl::ComplementTag>(node, domain, repository);
}

auto parse(const runir::kr::dl::grammar::ast::RoleInverse& node, tyr::formalism::planning::DomainView domain, runir::kr::dl::ConstructorRepository& repository)
{
    return parse_unary_role<runir::kr::dl::InverseTag>(node, domain, repository);
}

auto parse(const runir::kr::dl::grammar::ast::RoleTransitiveClosure& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepository& repository)
{
    return parse_unary_role<runir::kr::dl::TransitiveClosureTag>(node, domain, repository);
}

auto parse(const runir::kr::dl::grammar::ast::RoleReflexiveTransitiveClosure& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepository& repository)
{
    return parse_unary_role<runir::kr::dl::ReflexiveTransitiveClosureTag>(node, domain, repository);
}

auto parse(const runir::kr::dl::grammar::ast::RoleRestriction& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepository& repository)
{
    tyr::Data<runir::kr::dl::Role<runir::kr::dl::RestrictionTag>> data(parse_constructor_or_non_terminal(node.lhs, domain, repository).get_index(),
                                                                       parse_constructor_or_non_terminal(node.rhs, domain, repository).get_index());
    return intern_constructor<runir::kr::dl::RoleTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleIdentity& node, tyr::formalism::planning::DomainView domain, runir::kr::dl::ConstructorRepository& repository)
{
    tyr::Data<runir::kr::dl::Role<runir::kr::dl::IdentityTag>> data(parse_constructor_or_non_terminal(node.arg, domain, repository).get_index());
    return intern_constructor<runir::kr::dl::RoleTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::BooleanAtomicState& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepository& repository)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             0,
                             "BooleanAtomicState",
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 tyr::Data<runir::kr::dl::Boolean<runir::kr::dl::AtomicStateTag<T>>> data(predicate, node.polarity);
                                 return intern_constructor<runir::kr::dl::BooleanTag>(repository, intern(repository, data).get_index());
                             });
}

auto parse(const runir::kr::dl::grammar::ast::BooleanNonempty& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepository& repository)
{
    const auto arg = boost::apply_visitor([&](const auto& value) -> tyr::Data<runir::kr::dl::Boolean<runir::kr::dl::NonemptyTag>>::ConstructorVariant
                                          { return parse_constructor_or_non_terminal(unwrap(value), domain, repository).get_index(); },
                                          node.arg.get());

    tyr::Data<runir::kr::dl::Boolean<runir::kr::dl::NonemptyTag>> data(arg);
    return intern_constructor<runir::kr::dl::BooleanTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::NumericalCount& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepository& repository)
{
    const auto arg = boost::apply_visitor([&](const auto& value) -> tyr::Data<runir::kr::dl::Numerical<runir::kr::dl::CountTag>>::ConstructorVariant
                                          { return parse_constructor_or_non_terminal(unwrap(value), domain, repository).get_index(); },
                                          node.arg.get());

    tyr::Data<runir::kr::dl::Numerical<runir::kr::dl::CountTag>> data(arg);
    return intern_constructor<runir::kr::dl::NumericalTag>(repository, intern(repository, data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::NumericalDistance& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepository& repository)
{
    tyr::Data<runir::kr::dl::Numerical<runir::kr::dl::DistanceTag>> data(parse_constructor_or_non_terminal(node.lhs, domain, repository).get_index(),
                                                                         parse_constructor_or_non_terminal(node.mid, domain, repository).get_index(),
                                                                         parse_constructor_or_non_terminal(node.rhs, domain, repository).get_index());
    return intern_constructor<runir::kr::dl::NumericalTag>(repository, intern(repository, data).get_index());
}

template<runir::kr::dl::CategoryTag Category>
auto parse_constructor(const runir::kr::dl::grammar::ast::Constructor<Category>& node,
                       tyr::formalism::planning::DomainView domain,
                       runir::kr::dl::ConstructorRepository& repository)
{
    return boost::apply_visitor([&](const auto& arg) { return parse(unwrap(arg), domain, repository); }, node.get());
}

using BooleanFeatureMap = std::unordered_map<std::string, tyr::Index<Feature<BooleanFeature>>>;
using NumericalFeatureMap = std::unordered_map<std::string, tyr::Index<Feature<NumericalFeature>>>;

auto parse_feature(const runir::kr::gp::dl::ast::BooleanFeature& node,
                   tyr::formalism::planning::DomainView domain,
                   Repository& repository,
                   BooleanFeatureMap& boolean_features,
                   NumericalFeatureMap&)
{
    tyr::Data<ConcreteFeature<runir::kr::DlTag, BooleanFeature>> concrete_data(
        parse_constructor(node.feature, domain, repository.get_dl_repository()).get_index(),
        node.symbol,
        node.description);
    auto concrete = intern(repository, concrete_data);
    tyr::Data<Feature<BooleanFeature>> data(concrete.get_index());
    auto feature = intern(repository, data);
    boolean_features.emplace(node.name, feature.get_index());
}

auto parse_feature(const runir::kr::gp::dl::ast::NumericalFeature& node,
                   tyr::formalism::planning::DomainView domain,
                   Repository& repository,
                   BooleanFeatureMap&,
                   NumericalFeatureMap& numerical_features)
{
    tyr::Data<ConcreteFeature<runir::kr::DlTag, NumericalFeature>> concrete_data(
        parse_constructor(node.feature, domain, repository.get_dl_repository()).get_index(),
        node.symbol,
        node.description);
    auto concrete = intern(repository, concrete_data);
    tyr::Data<Feature<NumericalFeature>> data(concrete.get_index());
    auto feature = intern(repository, data);
    numerical_features.emplace(node.name, feature.get_index());
}

template<typename FeatureTag>
auto require_feature(const std::unordered_map<std::string, tyr::Index<Feature<FeatureTag>>>& features, const std::string& name)
{
    const auto it = features.find(name);
    if (it == features.end())
        throw std::runtime_error("Unknown feature \"" + name + "\".");
    return it->second;
}

template<typename FeatureTag, typename ObservationTag>
auto make_condition(tyr::Index<Feature<FeatureTag>> feature, Repository& repository)
{
    tyr::Data<ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>> concrete_data(feature);
    auto concrete = intern(repository, concrete_data);
    tyr::Data<ConcreteConditionVariant<runir::kr::DlTag>> variant_data(concrete.get_index());
    auto variant = intern(repository, variant_data);
    tyr::Data<ConditionVariant> data(variant.get_index());
    return intern(repository, data);
}

template<typename FeatureTag, typename ObservationTag>
auto make_effect(tyr::Index<Feature<FeatureTag>> feature, Repository& repository)
{
    tyr::Data<ConcreteEffect<runir::kr::DlTag, FeatureTag, ObservationTag>> concrete_data(feature);
    auto concrete = intern(repository, concrete_data);
    tyr::Data<ConcreteEffectVariant<runir::kr::DlTag>> variant_data(concrete.get_index());
    auto variant = intern(repository, variant_data);
    tyr::Data<EffectVariant> data(variant.get_index());
    return intern(repository, data);
}

auto parse_condition_observation(const Positive&,
                                 const std::string& feature,
                                 Repository& repository,
                                 const BooleanFeatureMap& boolean_features,
                                 const NumericalFeatureMap&)
{
    return make_condition<BooleanFeature, Positive>(require_feature(boolean_features, feature), repository);
}

auto parse_condition_observation(const Negative&,
                                 const std::string& feature,
                                 Repository& repository,
                                 const BooleanFeatureMap& boolean_features,
                                 const NumericalFeatureMap&)
{
    return make_condition<BooleanFeature, Negative>(require_feature(boolean_features, feature), repository);
}

auto parse_condition_observation(const EqualZero&,
                                 const std::string& feature,
                                 Repository& repository,
                                 const BooleanFeatureMap&,
                                 const NumericalFeatureMap& numerical_features)
{
    return make_condition<NumericalFeature, EqualZero>(require_feature(numerical_features, feature), repository);
}

auto parse_condition_observation(const GreaterZero&,
                                 const std::string& feature,
                                 Repository& repository,
                                 const BooleanFeatureMap&,
                                 const NumericalFeatureMap& numerical_features)
{
    return make_condition<NumericalFeature, GreaterZero>(require_feature(numerical_features, feature), repository);
}

auto parse_condition(const runir::kr::gp::dl::ast::Condition& node,
                     Repository& repository,
                     const BooleanFeatureMap& boolean_features,
                     const NumericalFeatureMap& numerical_features)
{
    return boost::apply_visitor([&](const auto& observation)
                                { return parse_condition_observation(observation, node.feature, repository, boolean_features, numerical_features); },
                                node.observation.get());
}

auto parse_effect_observation(const Positive&,
                              const std::string& feature,
                              Repository& repository,
                              const BooleanFeatureMap& boolean_features,
                              const NumericalFeatureMap&)
{
    return make_effect<BooleanFeature, Positive>(require_feature(boolean_features, feature), repository);
}

auto parse_effect_observation(const Negative&,
                              const std::string& feature,
                              Repository& repository,
                              const BooleanFeatureMap& boolean_features,
                              const NumericalFeatureMap&)
{
    return make_effect<BooleanFeature, Negative>(require_feature(boolean_features, feature), repository);
}

auto parse_effect_observation(const Unchanged&,
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
        return make_effect<BooleanFeature, Unchanged>(boolean_it->second, repository);
    if (numerical_it != numerical_features.end())
        return make_effect<NumericalFeature, Unchanged>(numerical_it->second, repository);

    throw std::runtime_error("Unknown feature \"" + feature + "\".");
}

auto parse_effect_observation(const Increases&,
                              const std::string& feature,
                              Repository& repository,
                              const BooleanFeatureMap&,
                              const NumericalFeatureMap& numerical_features)
{
    return make_effect<NumericalFeature, Increases>(require_feature(numerical_features, feature), repository);
}

auto parse_effect_observation(const Decreases&,
                              const std::string& feature,
                              Repository& repository,
                              const BooleanFeatureMap&,
                              const NumericalFeatureMap& numerical_features)
{
    return make_effect<NumericalFeature, Decreases>(require_feature(numerical_features, feature), repository);
}

auto parse_effect(const runir::kr::gp::dl::ast::Effect& node,
                  Repository& repository,
                  const BooleanFeatureMap& boolean_features,
                  const NumericalFeatureMap& numerical_features)
{
    return boost::apply_visitor([&](const auto& observation)
                                { return parse_effect_observation(observation, node.feature, repository, boolean_features, numerical_features); },
                                node.observation.get());
}

auto parse_rule(const runir::kr::gp::dl::ast::Rule& node,
                Repository& repository,
                const BooleanFeatureMap& boolean_features,
                const NumericalFeatureMap& numerical_features)
{
    auto conditions = tyr::IndexList<ConditionVariant> {};
    for (const auto& condition : node.conditions)
        conditions.push_back(parse_condition(condition, repository, boolean_features, numerical_features).get_index());

    auto effects = tyr::IndexList<EffectVariant> {};
    for (const auto& effect : node.effects)
        effects.push_back(parse_effect(effect, repository, boolean_features, numerical_features).get_index());

    tyr::Data<Rule> data;
    data.conditions = std::move(conditions);
    data.effects = std::move(effects);
    return intern(repository, data);
}

}  // namespace

PolicyView parse_policy(const std::string& description, tyr::formalism::planning::DomainView domain, Repository& repository)
{
    const auto ast = parser::parse_policy_ast(description);

    auto boolean_features = BooleanFeatureMap {};
    auto numerical_features = NumericalFeatureMap {};

    for (const auto& feature : ast.features)
        boost::apply_visitor([&](const auto& arg) { parse_feature(arg, domain, repository, boolean_features, numerical_features); }, feature.get());

    auto rules = tyr::IndexList<Rule> {};
    for (const auto& rule : ast.rules)
        rules.push_back(parse_rule(rule, repository, boolean_features, numerical_features).get_index());

    tyr::Data<Policy> data;
    data.rules = std::move(rules);
    return intern(repository, data);
}

}  // namespace runir::kr::gp::dl
