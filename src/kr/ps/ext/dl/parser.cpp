#include "runir/kr/ps/ext/dl/parser.hpp"

#include "runir/kr/dl/canonicalization.hpp"
#include "runir/kr/dl/grammar/parser/ext/parser.hpp"
#include "runir/kr/ps/ext/canonicalization.hpp"

#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <cctype>
#include <cstdint>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <yggdrasil/containers/variant.hpp>

namespace runir::kr::ps::ext::dl
{
namespace
{
namespace dl_ = runir::kr::dl;
namespace dl_ast = runir::kr::dl::grammar::parser::base::ast;
namespace dl_ext_ast = runir::kr::dl::grammar::parser::ext::ast;

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
auto intern(Repository& repository, ygg::Data<T>& data)
{
    if constexpr (requires { runir::kr::ps::ext::canonicalize(data); })
        runir::kr::ps::ext::canonicalize(data);
    return repository.get_or_create(data).first;
}

template<typename T>
auto intern_dl(runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository, ygg::Data<T>& data)
{
    if constexpr (requires { runir::kr::dl::canonicalize(data); })
        runir::kr::dl::canonicalize(data);
    return repository.get_or_create(data).first;
}

template<dl_::CategoryTag Category, typename T>
auto intern_constructor(runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository, ygg::Index<T> index)
{
    ygg::Data<dl_::Constructor<runir::kr::ExtFamilyTag, Category>> data(index);
    return intern_dl(repository, data);
}

template<dl_::CategoryTag Category>
auto parse_constructor(const dl_ast::Constructor<runir::kr::ExtFamilyTag, Category>& node,
                       tyr::formalism::planning::DomainView domain,
                       runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
    -> dl_::FamilyConstructorView<runir::kr::ExtFamilyTag, Category>;

template<dl_::CategoryTag Category>
auto parse_choice(const dl_ast::ConstructorOrNonTerminal<runir::kr::ExtFamilyTag, Category>& node,
                  tyr::formalism::planning::DomainView domain,
                  runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository) -> dl_::FamilyConstructorView<runir::kr::ExtFamilyTag, Category>
{
    return boost::apply_visitor(
        [&](const auto& value) -> dl_::FamilyConstructorView<runir::kr::ExtFamilyTag, Category>
        {
            const auto& unwrapped = unwrap(value);
            if constexpr (std::same_as<std::remove_cvref_t<decltype(unwrapped)>, dl_ast::NonTerminal<runir::kr::ExtFamilyTag, Category>>)
                throw std::runtime_error("Ext module DL expressions cannot reference grammar nonterminals.");
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

auto parse_dl(const dl_ast::ConceptBot&, tyr::formalism::planning::DomainView, runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, dl_::BotTag>> data;
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::ConceptTop&, tyr::formalism::planning::DomainView, runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, dl_::TopTag>> data;
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::ConceptAtomicState& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             1,
                             "ConceptAtomicState",
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, dl_::AtomicStateTag<T>>> data(predicate);
                                 return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
                             });
}

auto parse_dl(const dl_ast::ConceptAtomicGoal& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             1,
                             "ConceptAtomicGoal",
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, dl_::AtomicGoalTag<T>>> data(predicate, node.polarity);
                                 return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
                             });
}

template<typename Tag, typename Ast>
auto parse_binary_concept(const Ast& node,
                          tyr::formalism::planning::DomainView domain,
                          runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, Tag>> data(parse_choice(node.lhs, domain, repository).get_index(),
                                                               parse_choice(node.rhs, domain, repository).get_index());
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::ConceptIntersection<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    return parse_binary_concept<dl_::IntersectionTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::ConceptUnion<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    return parse_binary_concept<dl_::UnionTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::ConceptValueRestriction<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    return parse_binary_concept<dl_::ValueRestrictionTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::ConceptExistentialQuantification<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    return parse_binary_concept<dl_::ExistentialQuantificationTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::ConceptNegation<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, dl_::NegationTag>> data(parse_choice(node.arg, domain, repository).get_index());
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

template<typename Tag, typename Ast>
auto parse_number_restriction(const Ast& node,
                              tyr::formalism::planning::DomainView domain,
                              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, Tag>> data(node.n, parse_choice(node.role, domain, repository).get_index());
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::ConceptAtLeastNumberRestriction<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    return parse_number_restriction<dl_::AtLeastNumberRestrictionTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::ConceptAtMostNumberRestriction<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    return parse_number_restriction<dl_::AtMostNumberRestrictionTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::ConceptExactNumberRestriction<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    return parse_number_restriction<dl_::ExactNumberRestrictionTag>(node, domain, repository);
}

template<typename Tag, typename Ast>
auto parse_qualified_number_restriction(const Ast& node,
                                        tyr::formalism::planning::DomainView domain,
                                        runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, Tag>> data(node.n,
                                                               parse_choice(node.role, domain, repository).get_index(),
                                                               parse_choice(node.concept_, domain, repository).get_index());
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::ConceptQualifiedAtLeastNumberRestriction<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    return parse_qualified_number_restriction<dl_::QualifiedAtLeastNumberRestrictionTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::ConceptQualifiedAtMostNumberRestriction<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    return parse_qualified_number_restriction<dl_::QualifiedAtMostNumberRestrictionTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::ConceptQualifiedExactNumberRestriction<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    return parse_qualified_number_restriction<dl_::QualifiedExactNumberRestrictionTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::ConceptRoleValueMap<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    return parse_binary_concept<dl_::RoleValueMapTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::ConceptAgreement<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    return parse_binary_concept<dl_::AgreementTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::ConceptRoleFillers<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, dl_::RoleFillersTag>> data(parse_choice(node.role, domain, repository).get_index(),
                                                                               require_objects(domain, node.object_names));
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::ConceptOneOf& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, dl_::OneOfTag>> data(require_objects(domain, node.object_names));
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::ConceptNominal& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, dl_::NominalTag>> data(require_object(domain, node.object_name));
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ext_ast::ConceptRegister& node,
              tyr::formalism::planning::DomainView,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, dl_::RegisterTag>> data(dl_::RegisterIdentifier<dl_::ConceptTag>(node.identifier));
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ext_ast::Argument<dl_::ConceptTag>& node,
              tyr::formalism::planning::DomainView,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, dl_::ArgumentTag<dl_::ConceptTag>>> data(dl_::ArgumentIdentifier<dl_::ConceptTag>(node.identifier));
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::RoleUniversal&, tyr::formalism::planning::DomainView, runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    ygg::Data<dl_::Role<runir::kr::ExtFamilyTag, dl_::UniversalTag>> data;
    return intern_constructor<dl_::RoleTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::RoleAtomicState& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             2,
                             "RoleAtomicState",
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 ygg::Data<dl_::Role<runir::kr::ExtFamilyTag, dl_::AtomicStateTag<T>>> data(predicate);
                                 return intern_constructor<dl_::RoleTag>(repository, intern_dl(repository, data).get_index());
                             });
}

auto parse_dl(const dl_ast::RoleAtomicGoal& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             2,
                             "RoleAtomicGoal",
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 ygg::Data<dl_::Role<runir::kr::ExtFamilyTag, dl_::AtomicGoalTag<T>>> data(predicate, node.polarity);
                                 return intern_constructor<dl_::RoleTag>(repository, intern_dl(repository, data).get_index());
                             });
}

template<typename Tag, typename Ast>
auto parse_binary_role(const Ast& node,
                       tyr::formalism::planning::DomainView domain,
                       runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    ygg::Data<dl_::Role<runir::kr::ExtFamilyTag, Tag>> data(parse_choice(node.lhs, domain, repository).get_index(),
                                                            parse_choice(node.rhs, domain, repository).get_index());
    return intern_constructor<dl_::RoleTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::RoleIntersection<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    return parse_binary_role<dl_::IntersectionTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::RoleUnion<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    return parse_binary_role<dl_::UnionTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::RoleComposition<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    return parse_binary_role<dl_::CompositionTag>(node, domain, repository);
}

template<typename Tag, typename Ast>
auto parse_unary_role(const Ast& node,
                      tyr::formalism::planning::DomainView domain,
                      runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    ygg::Data<dl_::Role<runir::kr::ExtFamilyTag, Tag>> data(parse_choice(node.arg, domain, repository).get_index());
    return intern_constructor<dl_::RoleTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::RoleComplement<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    return parse_unary_role<dl_::ComplementTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::RoleInverse<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    return parse_unary_role<dl_::InverseTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::RoleTransitiveClosure<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    return parse_unary_role<dl_::TransitiveClosureTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::RoleReflexiveTransitiveClosure<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    return parse_unary_role<dl_::ReflexiveTransitiveClosureTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::RoleRestriction<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    ygg::Data<dl_::Role<runir::kr::ExtFamilyTag, dl_::RestrictionTag>> data(parse_choice(node.lhs, domain, repository).get_index(),
                                                                            parse_choice(node.rhs, domain, repository).get_index());
    return intern_constructor<dl_::RoleTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::RoleIdentity<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    ygg::Data<dl_::Role<runir::kr::ExtFamilyTag, dl_::IdentityTag>> data(parse_choice(node.arg, domain, repository).get_index());
    return intern_constructor<dl_::RoleTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ext_ast::RoleRegister& node,
              tyr::formalism::planning::DomainView,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    ygg::Data<dl_::Role<runir::kr::ExtFamilyTag, dl_::RegisterTag>> data(dl_::RegisterIdentifier<dl_::RoleTag>(node.identifier));
    return intern_constructor<dl_::RoleTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ext_ast::Argument<dl_::RoleTag>& node,
              tyr::formalism::planning::DomainView,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    ygg::Data<dl_::Role<runir::kr::ExtFamilyTag, dl_::ArgumentTag<dl_::RoleTag>>> data(dl_::ArgumentIdentifier<dl_::RoleTag>(node.identifier));
    return intern_constructor<dl_::RoleTag>(repository, intern_dl(repository, data).get_index());
}

template<dl_::CategoryTag Category>
auto parse_constructor(const dl_ast::Constructor<runir::kr::ExtFamilyTag, Category>& node,
                       tyr::formalism::planning::DomainView domain,
                       runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
    -> dl_::FamilyConstructorView<runir::kr::ExtFamilyTag, Category>
{
    return boost::apply_visitor([&](const auto& arg) { return parse_dl(unwrap(arg), domain, repository); }, node.get());
}

template<typename T>
struct AstCategory
{
};

template<dl_::CategoryTag Category>
struct AstCategory<ast::Argument<Category>>
{
    using Type = Category;
};

template<dl_::CategoryTag Category>
struct AstCategory<ast::Register<Category>>
{
    using Type = Category;
};

template<dl_::CategoryTag Category>
struct AstCategory<ast::Feature<Category>>
{
    using Type = Category;
};

template<dl_::CategoryTag Category>
struct AstCategory<ast::LoadRule<Category>>
{
    using Type = Category;
};

template<dl_::CategoryTag Category>
auto intern_argument(Repository& repository, const ast::Argument<Category>& argument, ygg::uint_t identifier)
{
    auto data = ygg::Data<Argument<Category>>(argument.symbol, runir::kr::dl::ArgumentIdentifier<Category>(identifier));
    return intern(repository, data);
}

using ConceptFeatureMap = std::unordered_map<std::string, ygg::Index<runir::kr::ps::ext::Feature<runir::kr::dl::ConceptTag>>>;
using RoleFeatureMap = std::unordered_map<std::string, ygg::Index<runir::kr::ps::ext::Feature<runir::kr::dl::RoleTag>>>;
using BooleanFeatureMap = std::unordered_map<std::string, ygg::Index<runir::kr::ps::ext::Feature<runir::kr::ps::dl::BooleanFeature>>>;
using NumericalFeatureMap = std::unordered_map<std::string, ygg::Index<runir::kr::ps::ext::Feature<runir::kr::ps::dl::NumericalFeature>>>;
using ConceptAliasMap = std::unordered_map<std::string, ygg::Index<runir::kr::dl::FamilyConstructor<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>>;
using MemoryStateMap = std::unordered_map<std::string, ygg::Index<MemoryState>>;
using ConceptRegisterMap = std::unordered_map<std::string, ygg::Index<Register<runir::kr::dl::ConceptTag>>>;
using RoleRegisterMap = std::unordered_map<std::string, ygg::Index<Register<runir::kr::dl::RoleTag>>>;
using ModuleMap = std::unordered_map<std::string, ygg::Index<Module>>;

struct SignatureCounts
{
    std::size_t concepts = 0;
    std::size_t roles = 0;
    std::size_t booleans = 0;
    std::size_t numericals = 0;

    bool operator==(const SignatureCounts&) const noexcept = default;
};

std::string signature_text(const SignatureCounts& counts)
{
    return "concept=" + std::to_string(counts.concepts) + ", role=" + std::to_string(counts.roles) + ", boolean=" + std::to_string(counts.booleans)
           + ", numerical=" + std::to_string(counts.numericals);
}

[[noreturn]] void fail(const std::string& message) { throw std::runtime_error(message + "."); }

template<dl_::CategoryTag Category>
void increment(SignatureCounts& counts)
{
    if constexpr (std::same_as<Category, dl_::ConceptTag>)
        ++counts.concepts;
    else if constexpr (std::same_as<Category, dl_::RoleTag>)
        ++counts.roles;
    else if constexpr (std::same_as<Category, dl_::BooleanTag>)
        ++counts.booleans;
    else if constexpr (std::same_as<Category, dl_::NumericalTag>)
        ++counts.numericals;
}

SignatureCounts signature_counts(const ast::Module& module)
{
    auto result = SignatureCounts {};
    for (const auto& argument : module.arguments)
        boost::apply_visitor([&](const auto& concrete) { increment<typename AstCategory<std::remove_cvref_t<decltype(concrete)>>::Type>(result); },
                             argument.get());
    return result;
}

template<dl_::CategoryTag Category>
void validate_unique_names(const std::vector<ast::Register<Category>>& declarations, const char* kind)
{
    if (declarations.size() > runir::kr::dl::num_registers)
        fail(std::string(kind) + " declares more registers than the supported maximum of " + std::to_string(runir::kr::dl::num_registers));

    auto names = std::unordered_set<std::string> {};
    for (const auto& declaration : declarations)
        if (!names.emplace(declaration.symbol).second)
            fail(std::string("Duplicate ") + kind + " register name \"" + declaration.symbol + "\"");
}

template<dl_::CategoryTag Category>
std::vector<ast::Register<Category>> collect_registers(const std::vector<ast::RegisterVariant>& registers)
{
    auto result = std::vector<ast::Register<Category>> {};
    for (const auto& reg : registers)
    {
        boost::apply_visitor(
            [&](const auto& concrete)
            {
                using ConcreteCategory = typename AstCategory<std::remove_cvref_t<decltype(concrete)>>::Type;
                if constexpr (std::same_as<ConcreteCategory, Category>)
                    result.push_back(concrete);
            },
            reg.get());
    }
    return result;
}

void validate_module_declarations(const ast::Module& module)
{
    auto concept_arguments = std::unordered_set<std::string> {};
    auto role_arguments = std::unordered_set<std::string> {};
    auto boolean_arguments = std::unordered_set<std::string> {};
    auto numerical_arguments = std::unordered_set<std::string> {};

    const auto add_argument = [&](auto& names, const std::string& symbol, const char* kind)
    {
        if (!names.emplace(symbol).second)
            fail(std::string("Duplicate ") + kind + " argument name \"" + symbol + "\"");
    };

    for (const auto& argument : module.arguments)
    {
        boost::apply_visitor(
            [&](const auto& concrete)
            {
                using Category = typename AstCategory<std::remove_cvref_t<decltype(concrete)>>::Type;
                if constexpr (std::same_as<Category, dl_::ConceptTag>)
                    add_argument(concept_arguments, concrete.symbol, "concept");
                else if constexpr (std::same_as<Category, dl_::RoleTag>)
                    add_argument(role_arguments, concrete.symbol, "role");
                else if constexpr (std::same_as<Category, dl_::BooleanTag>)
                    add_argument(boolean_arguments, concrete.symbol, "boolean");
                else if constexpr (std::same_as<Category, dl_::NumericalTag>)
                    add_argument(numerical_arguments, concrete.symbol, "numerical");
            },
            argument.get());
    }

    validate_unique_names(collect_registers<dl_::ConceptTag>(module.registers), "concept");
    validate_unique_names(collect_registers<dl_::RoleTag>(module.registers), "role");
}

void append_argument(Repository& repository,
                     ygg::Data<Module>& data,
                     const ast::ArgumentVariant& argument,
                     ygg::uint_t& concept_identifier,
                     ygg::uint_t& role_identifier,
                     ygg::uint_t& boolean_identifier,
                     ygg::uint_t& numerical_identifier)
{
    boost::apply_visitor(
        [&](const auto& concrete)
        {
            using Category = typename AstCategory<std::remove_cvref_t<decltype(concrete)>>::Type;
            if constexpr (std::same_as<Category, dl_::ConceptTag>)
                data.concept_arguments.push_back(intern_argument<Category>(repository, concrete, concept_identifier++).get_index());
            else if constexpr (std::same_as<Category, dl_::RoleTag>)
                data.role_arguments.push_back(intern_argument<Category>(repository, concrete, role_identifier++).get_index());
            else if constexpr (std::same_as<Category, dl_::BooleanTag>)
                data.boolean_arguments.push_back(intern_argument<Category>(repository, concrete, boolean_identifier++).get_index());
            else if constexpr (std::same_as<Category, dl_::NumericalTag>)
                data.numerical_arguments.push_back(intern_argument<Category>(repository, concrete, numerical_identifier++).get_index());
        },
        argument.get());
}

template<typename FeatureTag, typename ConcreteFeatureTag>
auto intern_dl_feature(Repository& repository,
                       ygg::Index<runir::kr::dl::FamilyConstructor<runir::kr::ExtFamilyTag, ConcreteFeatureTag>> constructor,
                       const std::string& symbol)
{
    ygg::Data<ConcreteFeature<runir::kr::DlTag, FeatureTag>> concrete_data(constructor, symbol);
    const auto concrete = intern(repository, concrete_data);
    ygg::Data<Feature<FeatureTag>> feature_data(concrete.get_index());
    return intern(repository, feature_data);
}

template<dl_::CategoryTag Category>
void append_feature(Repository& repository,
                    ygg::Data<Module>& module_data,
                    const ast::Feature<Category>& feature,
                    tyr::formalism::planning::DomainView domain,
                    ConceptFeatureMap& concept_features,
                    RoleFeatureMap& role_features,
                    BooleanFeatureMap& boolean_features,
                    NumericalFeatureMap& numerical_features,
                    ConceptAliasMap& concept_aliases)
{
    try
    {
        if constexpr (std::same_as<Category, dl_::ConceptTag>)
        {
            const auto constructor = parse_concept(feature.expression, domain, repository.get_dl_repository());
            const auto view = intern_dl_feature<runir::kr::dl::ConceptTag>(repository, constructor.get_index(), feature.symbol);
            if (!concept_features.emplace(feature.symbol, view.get_index()).second || !concept_aliases.emplace(feature.symbol, constructor.get_index()).second)
                fail("Duplicate feature name \"" + feature.symbol + "\"");
            module_data.concept_features.push_back(view.get_index());
        }
        else if constexpr (std::same_as<Category, dl_::RoleTag>)
        {
            const auto constructor = parse_role(feature.expression, domain, repository.get_dl_repository());
            const auto view = intern_dl_feature<runir::kr::dl::RoleTag>(repository, constructor.get_index(), feature.symbol);
            if (!role_features.emplace(feature.symbol, view.get_index()).second)
                fail("Duplicate feature name \"" + feature.symbol + "\"");
            module_data.role_features.push_back(view.get_index());
        }
        else if constexpr (std::same_as<Category, dl_::BooleanTag>)
        {
            const auto constructor = parse_boolean(feature.expression, domain, repository.get_dl_repository());
            const auto view = intern_dl_feature<runir::kr::ps::dl::BooleanFeature>(repository, constructor.get_index(), feature.symbol);
            if (!boolean_features.emplace(feature.symbol, view.get_index()).second)
                fail("Duplicate feature name \"" + feature.symbol + "\"");
            module_data.boolean_features.push_back(view.get_index());
        }
        else if constexpr (std::same_as<Category, dl_::NumericalTag>)
        {
            const auto constructor = parse_numerical(feature.expression, domain, repository.get_dl_repository());
            const auto view = intern_dl_feature<runir::kr::ps::dl::NumericalFeature>(repository, constructor.get_index(), feature.symbol);
            if (!numerical_features.emplace(feature.symbol, view.get_index()).second)
                fail("Duplicate feature name \"" + feature.symbol + "\"");
            module_data.numerical_features.push_back(view.get_index());
        }
    }
    catch (const std::exception& e)
    {
        fail("Invalid feature \"" + feature.symbol + "\": " + e.what());
    }
}

template<typename FeatureTag>
auto require_feature(const std::unordered_map<std::string, ygg::Index<Feature<FeatureTag>>>& features, const std::string& name)
{
    const auto it = features.find(name);
    if (it == features.end())
        fail("Unknown feature \"" + name + "\"");
    return it->second;
}

template<typename FeatureTag, typename ObservationTag>
auto make_condition(Repository& repository, ygg::Index<Feature<FeatureTag>> feature)
{
    ygg::Data<ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>> concrete_data(feature);
    const auto concrete = intern(repository, concrete_data);
    ygg::Data<ConcreteConditionVariant<runir::kr::DlTag>> concrete_variant_data(concrete.get_index());
    const auto concrete_variant = intern(repository, concrete_variant_data);
    ygg::Data<ConditionVariant> variant_data(concrete_variant.get_index());
    return intern(repository, variant_data);
}

template<typename FeatureTag, typename ObservationTag>
auto make_effect(Repository& repository, ygg::Index<Feature<FeatureTag>> feature)
{
    ygg::Data<ConcreteEffect<runir::kr::DlTag, FeatureTag, ObservationTag>> concrete_data(feature);
    const auto concrete = intern(repository, concrete_data);
    ygg::Data<ConcreteEffectVariant<runir::kr::DlTag>> concrete_variant_data(concrete.get_index());
    const auto concrete_variant = intern(repository, concrete_variant_data);
    ygg::Data<EffectVariant> variant_data(concrete_variant.get_index());
    return intern(repository, variant_data);
}

auto parse_condition(Repository& repository,
                     const ast::Condition& condition,
                     const ConceptFeatureMap& concept_features,
                     const BooleanFeatureMap& boolean_features,
                     const NumericalFeatureMap& numerical_features)
{
    return boost::apply_visitor(
        [&](const auto& observation)
        {
            using Observation = std::remove_cvref_t<decltype(observation)>;
            if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::Positive>)
            {
                return make_condition<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>(repository,
                                                                                                      require_feature(boolean_features, condition.feature));
            }
            else if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::Negative>)
            {
                return make_condition<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>(repository,
                                                                                                      require_feature(boolean_features, condition.feature));
            }
            else if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::EqualZero>)
            {
                if (concept_features.contains(condition.feature))
                    return make_condition<runir::kr::dl::ConceptTag, runir::kr::ps::dl::EqualZero>(repository,
                                                                                                   require_feature(concept_features, condition.feature));
                return make_condition<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::EqualZero>(
                    repository,
                    require_feature(numerical_features, condition.feature));
            }
            else if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::GreaterZero>)
            {
                if (concept_features.contains(condition.feature))
                    return make_condition<runir::kr::dl::ConceptTag, runir::kr::ps::dl::GreaterZero>(repository,
                                                                                                     require_feature(concept_features, condition.feature));
                return make_condition<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::GreaterZero>(
                    repository,
                    require_feature(numerical_features, condition.feature));
            }
        },
        condition.observation.get());
}

auto parse_effect(Repository& repository,
                  const ast::Effect& effect,
                  const ConceptFeatureMap& concept_features,
                  const BooleanFeatureMap& boolean_features,
                  const NumericalFeatureMap& numerical_features)
{
    return boost::apply_visitor(
        [&](const auto& observation)
        {
            using Observation = std::remove_cvref_t<decltype(observation)>;
            if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::Positive>)
            {
                return make_effect<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>(repository,
                                                                                                   require_feature(boolean_features, effect.feature));
            }
            else if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::Negative>)
            {
                return make_effect<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>(repository,
                                                                                                   require_feature(boolean_features, effect.feature));
            }
            else if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::Unchanged>)
            {
                if (concept_features.contains(effect.feature))
                    return make_effect<runir::kr::dl::ConceptTag, runir::kr::ps::dl::Unchanged>(repository, require_feature(concept_features, effect.feature));
                if (boolean_features.contains(effect.feature))
                    return make_effect<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Unchanged>(repository,
                                                                                                        require_feature(boolean_features, effect.feature));
                return make_effect<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Unchanged>(repository,
                                                                                                      require_feature(numerical_features, effect.feature));
            }
            else if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::Increases>)
            {
                if (concept_features.contains(effect.feature))
                    return make_effect<runir::kr::dl::ConceptTag, runir::kr::ps::dl::Increases>(repository, require_feature(concept_features, effect.feature));
                return make_effect<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Increases>(repository,
                                                                                                      require_feature(numerical_features, effect.feature));
            }
            else if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::Decreases>)
            {
                if (concept_features.contains(effect.feature))
                    return make_effect<runir::kr::dl::ConceptTag, runir::kr::ps::dl::Decreases>(repository, require_feature(concept_features, effect.feature));
                return make_effect<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Decreases>(repository,
                                                                                                      require_feature(numerical_features, effect.feature));
            }
        },
        effect.observation.get());
}

auto parse_conditions(Repository& repository,
                      const std::vector<ast::Condition>& observations,
                      const ConceptFeatureMap& concept_features,
                      const BooleanFeatureMap& boolean_features,
                      const NumericalFeatureMap& numerical_features)
{
    auto result = ygg::IndexList<ConditionVariant> {};
    result.reserve(observations.size());
    for (const auto& observation : observations)
        result.push_back(parse_condition(repository, observation, concept_features, boolean_features, numerical_features).get_index());
    return result;
}

auto parse_effects(Repository& repository,
                   const std::vector<ast::Effect>& observations,
                   const ConceptFeatureMap& concept_features,
                   const BooleanFeatureMap& boolean_features,
                   const NumericalFeatureMap& numerical_features)
{
    auto result = ygg::IndexList<EffectVariant> {};
    result.reserve(observations.size());
    for (const auto& observation : observations)
        result.push_back(parse_effect(repository, observation, concept_features, boolean_features, numerical_features).get_index());
    return result;
}

bool is_expression_text(const std::string& value) { return !value.empty() && value.front() == '('; }

auto parse_concept_argument(Repository& repository,
                            const ast::Expression& expression,
                            tyr::formalism::planning::DomainView domain,
                            const ConceptAliasMap& concept_aliases)
{
    try
    {
        return boost::apply_visitor(
            [&](const auto& concrete)
            {
                using Expression = std::remove_cvref_t<decltype(concrete)>;
                if constexpr (std::same_as<Expression, ast::ConstructorExpression>)
                {
                    return parse_concept(concrete.text, domain, repository.get_dl_repository()).get_index();
                }
                else
                {
                    const auto it = concept_aliases.find(concrete.symbol);
                    if (it == concept_aliases.end())
                        fail("Unknown concept expression or feature alias \"" + concrete.symbol + "\"");
                    return it->second;
                }
            },
            expression.get());
    }
    catch (const std::exception& e)
    {
        fail(e.what());
    }
}

template<typename FeatureTag>
auto constructor_from_feature(Repository& repository, ygg::Index<Feature<FeatureTag>> feature)
{
    const auto view = ygg::make_view(feature, repository);
    return ygg::visit([](auto concrete_feature) { return concrete_feature.get_feature().get_index(); }, view.get_variant());
}

auto parse_role_argument(Repository& repository,
                         const ast::Expression& expression,
                         tyr::formalism::planning::DomainView domain,
                         const RoleFeatureMap& role_features)
{
    try
    {
        return boost::apply_visitor(
            [&](const auto& concrete)
            {
                using Expression = std::remove_cvref_t<decltype(concrete)>;
                if constexpr (std::same_as<Expression, ast::ConstructorExpression>)
                {
                    return parse_role(concrete.text, domain, repository.get_dl_repository()).get_index();
                }
                else
                {
                    const auto it = role_features.find(concrete.symbol);
                    if (it == role_features.end())
                        fail("Unknown role expression or feature alias \"" + concrete.symbol + "\"");
                    return constructor_from_feature(repository, it->second);
                }
            },
            expression.get());
    }
    catch (const std::exception& e)
    {
        fail(e.what());
    }
}

auto parse_do_argument(const ast::Expression& expression, const ConceptFeatureMap& concept_features)
{
    return boost::apply_visitor(
        [&](const auto& concrete) -> ygg::Index<Feature<runir::kr::dl::ConceptTag>>
        {
            using Expression = std::remove_cvref_t<decltype(concrete)>;
            if constexpr (std::same_as<Expression, ast::ConstructorExpression>)
            {
                fail("Do-rule action arguments must reference declared concept features by symbol");
            }
            else
            {
                return require_feature(concept_features, concrete.symbol);
            }
        },
        expression.get());
}

auto parse_call_argument(Repository& repository,
                         const ast::Expression& expression,
                         const ConceptFeatureMap& concept_features,
                         const RoleFeatureMap& role_features,
                         const BooleanFeatureMap& boolean_features,
                         const NumericalFeatureMap& numerical_features) -> CallArgument
{
    return boost::apply_visitor(
        [&](const auto& concrete) -> CallArgument
        {
            using Expression = std::remove_cvref_t<decltype(concrete)>;
            if constexpr (std::same_as<Expression, ast::ConstructorExpression>)
            {
                fail("Call-rule arguments must reference declared features by symbol");
            }
            else
            {
                const auto& name = concrete.symbol;
                auto matches = 0;
                auto result = CallArgument {};
                if (const auto it = concept_features.find(name); it != concept_features.end())
                {
                    result = constructor_from_feature(repository, it->second);
                    ++matches;
                }
                if (const auto it = role_features.find(name); it != role_features.end())
                {
                    result = constructor_from_feature(repository, it->second);
                    ++matches;
                }
                if (const auto it = boolean_features.find(name); it != boolean_features.end())
                {
                    result = constructor_from_feature(repository, it->second);
                    ++matches;
                }
                if (const auto it = numerical_features.find(name); it != numerical_features.end())
                {
                    result = constructor_from_feature(repository, it->second);
                    ++matches;
                }

                if (matches == 0)
                    fail("Unknown call-rule argument feature \"" + name + "\"");
                if (matches > 1)
                    fail("Ambiguous call-rule argument feature \"" + name + "\"");
                return result;
            }
        },
        expression.get());
}

struct FeatureSymbolSets
{
    std::unordered_set<std::string> concepts;
    std::unordered_set<std::string> roles;
    std::unordered_set<std::string> booleans;
    std::unordered_set<std::string> numericals;
};

FeatureSymbolSets feature_symbols(const ast::Module& module)
{
    auto result = FeatureSymbolSets {};
    for (const auto& feature : module.features)
    {
        boost::apply_visitor(
            [&](const auto& concrete)
            {
                using Category = typename AstCategory<std::remove_cvref_t<decltype(concrete)>>::Type;
                if constexpr (std::same_as<Category, dl_::ConceptTag>)
                    result.concepts.emplace(concrete.symbol);
                else if constexpr (std::same_as<Category, dl_::RoleTag>)
                    result.roles.emplace(concrete.symbol);
                else if constexpr (std::same_as<Category, dl_::BooleanTag>)
                    result.booleans.emplace(concrete.symbol);
                else if constexpr (std::same_as<Category, dl_::NumericalTag>)
                    result.numericals.emplace(concrete.symbol);
            },
            feature.get());
    }
    return result;
}

void increment_signature_count(SignatureCounts& counts, const FeatureSymbolSets& features, const std::string& symbol)
{
    auto matches = 0;
    if (features.concepts.contains(symbol))
    {
        ++counts.concepts;
        ++matches;
    }
    if (features.roles.contains(symbol))
    {
        ++counts.roles;
        ++matches;
    }
    if (features.booleans.contains(symbol))
    {
        ++counts.booleans;
        ++matches;
    }
    if (features.numericals.contains(symbol))
    {
        ++counts.numericals;
        ++matches;
    }
    if (matches == 0)
        fail("Unknown call-rule argument feature \"" + symbol + "\"");
    if (matches > 1)
        fail("Ambiguous call-rule argument feature \"" + symbol + "\"");
}

SignatureCounts call_argument_signature_counts(const ast::CallRule& rule, const FeatureSymbolSets& features)
{
    auto result = SignatureCounts {};
    for (const auto& argument : rule.arguments)
    {
        boost::apply_visitor(
            [&](const auto& concrete)
            {
                using Expression = std::remove_cvref_t<decltype(concrete)>;
                if constexpr (std::same_as<Expression, ast::ConstructorExpression>)
                    fail("Call-rule arguments must reference declared features by symbol");
                else
                    increment_signature_count(result, features, concrete.symbol);
            },
            argument.get());
    }
    return result;
}

struct SExpr
{
    std::string atom;
    std::vector<SExpr> list;

    bool is_atom() const noexcept { return list.empty(); }
};

using IdentifierMap = std::unordered_map<std::string, ygg::uint_t>;

struct ModuleReferenceMaps
{
    IdentifierMap concept_arguments;
    IdentifierMap role_arguments;
    IdentifierMap boolean_arguments;
    IdentifierMap numerical_arguments;
    IdentifierMap concept_registers;
    IdentifierMap role_registers;
};

void add_identifier(IdentifierMap& map, const std::string& name, ygg::uint_t identifier, const char* kind)
{
    if (!map.emplace(name, identifier).second)
        fail(std::string("Duplicate ") + kind + " name \"" + name + "\"");
}

ModuleReferenceMaps reference_maps(const ast::Module& module)
{
    auto result = ModuleReferenceMaps {};
    auto concept_argument = ygg::uint_t(0);
    auto role_argument = ygg::uint_t(0);
    auto boolean_argument = ygg::uint_t(0);
    auto numerical_argument = ygg::uint_t(0);

    for (const auto& argument : module.arguments)
    {
        boost::apply_visitor(
            [&](const auto& concrete)
            {
                using Category = typename AstCategory<std::remove_cvref_t<decltype(concrete)>>::Type;
                if constexpr (std::same_as<Category, dl_::ConceptTag>)
                    add_identifier(result.concept_arguments, concrete.symbol, concept_argument++, "concept argument");
                else if constexpr (std::same_as<Category, dl_::RoleTag>)
                    add_identifier(result.role_arguments, concrete.symbol, role_argument++, "role argument");
                else if constexpr (std::same_as<Category, dl_::BooleanTag>)
                    add_identifier(result.boolean_arguments, concrete.symbol, boolean_argument++, "boolean argument");
                else if constexpr (std::same_as<Category, dl_::NumericalTag>)
                    add_identifier(result.numerical_arguments, concrete.symbol, numerical_argument++, "numerical argument");
            },
            argument.get());
    }

    auto concept_register = ygg::uint_t(0);
    auto role_register = ygg::uint_t(0);
    for (const auto& reg : module.registers)
    {
        boost::apply_visitor(
            [&](const auto& concrete)
            {
                using Category = typename AstCategory<std::remove_cvref_t<decltype(concrete)>>::Type;
                if constexpr (std::same_as<Category, dl_::ConceptTag>)
                    add_identifier(result.concept_registers, concrete.symbol, concept_register++, "concept register");
                else if constexpr (std::same_as<Category, dl_::RoleTag>)
                    add_identifier(result.role_registers, concrete.symbol, role_register++, "role register");
            },
            reg.get());
    }

    return result;
}

ygg::uint_t require_identifier(const IdentifierMap& map, const std::string& name, const char* kind)
{
    const auto it = map.find(name);
    if (it == map.end())
        fail(std::string("Unknown ") + kind + " \"" + name + "\"");
    return it->second;
}

std::vector<std::string> tokenize_expression(const std::string& text)
{
    auto tokens = std::vector<std::string> {};
    for (size_t i = 0; i < text.size();)
    {
        if (std::isspace(static_cast<unsigned char>(text[i])))
        {
            ++i;
        }
        else if (text[i] == '(' || text[i] == ')')
        {
            tokens.emplace_back(1, text[i++]);
        }
        else if (text[i] == '"')
        {
            const auto start = i++;
            while (i < text.size() && text[i] != '"')
                ++i;
            if (i == text.size())
                fail("Unterminated string in DL expression");
            ++i;
            tokens.push_back(text.substr(start, i - start));
        }
        else
        {
            const auto start = i;
            while (i < text.size() && !std::isspace(static_cast<unsigned char>(text[i])) && text[i] != '(' && text[i] != ')')
                ++i;
            tokens.push_back(text.substr(start, i - start));
        }
    }
    return tokens;
}

SExpr parse_expression_tokens(const std::vector<std::string>& tokens, size_t& pos)
{
    if (pos >= tokens.size())
        fail("Unexpected end of DL expression");

    if (tokens[pos] != "(")
        return SExpr { tokens[pos++], {} };

    ++pos;
    auto result = SExpr {};
    while (pos < tokens.size() && tokens[pos] != ")")
        result.list.push_back(parse_expression_tokens(tokens, pos));

    if (pos >= tokens.size())
        fail("Unclosed list in DL expression");
    ++pos;

    if (result.list.empty())
        fail("Empty list in DL expression");
    return result;
}

SExpr parse_expression(const std::string& text)
{
    const auto tokens = tokenize_expression(text);
    size_t pos = 0;
    auto result = parse_expression_tokens(tokens, pos);
    if (pos != tokens.size())
        fail("Trailing tokens in DL expression");
    return result;
}

std::string format_expression(const SExpr& expr)
{
    if (expr.is_atom())
        return expr.atom;

    auto out = std::ostringstream {};
    out << '(';
    for (size_t i = 0; i < expr.list.size(); ++i)
    {
        if (i != 0)
            out << ' ';
        out << format_expression(expr.list[i]);
    }
    out << ')';
    return out.str();
}

const std::string& require_atom(const SExpr& expr, const char* context)
{
    if (!expr.is_atom())
        throw std::runtime_error(std::string("Expected atom in ") + context + ".");
    return expr.atom;
}

const std::string& expression_operator(const SExpr& expr)
{
    if (expr.is_atom() || expr.list.empty())
        throw std::runtime_error("Expected constructor expression list.");
    return require_atom(expr.list.front(), "constructor operator");
}

void normalize_identifier_expression(SExpr& expr, const IdentifierMap& map, const char* constructor, const char* kind)
{
    if (expr.list.size() != 2)
        fail(std::string(constructor) + " expects one symbol");
    auto& identifier = expr.list[1];
    if (!identifier.is_atom())
        fail(std::string(constructor) + " expects one symbol");
    identifier.atom = std::to_string(require_identifier(map, identifier.atom, kind));
}

void normalize_expression_references(SExpr& expr, const ModuleReferenceMaps& maps)
{
    if (expr.is_atom())
        return;

    const auto& op = expression_operator(expr);
    if (op == "c_argument")
        normalize_identifier_expression(expr, maps.concept_arguments, "c_argument", "concept argument");
    else if (op == "r_argument")
        normalize_identifier_expression(expr, maps.role_arguments, "r_argument", "role argument");
    else if (op == "b_argument")
        normalize_identifier_expression(expr, maps.boolean_arguments, "b_argument", "boolean argument");
    else if (op == "n_argument")
        normalize_identifier_expression(expr, maps.numerical_arguments, "n_argument", "numerical argument");
    else if (op == "c_register")
        normalize_identifier_expression(expr, maps.concept_registers, "c_register", "concept register");
    else if (op == "r_register")
        normalize_identifier_expression(expr, maps.role_registers, "r_register", "role register");

    for (std::size_t i = 1; i < expr.list.size(); ++i)
        normalize_expression_references(expr.list[i], maps);
}

std::string normalize_expression_references(const std::string& expression, const ModuleReferenceMaps& maps)
{
    if (!is_expression_text(expression))
        return expression;

    auto expr = parse_expression(expression);
    normalize_expression_references(expr, maps);
    return format_expression(expr);
}

ast::Expression normalize_expression_references(ast::Expression expression, const ModuleReferenceMaps& maps)
{
    boost::apply_visitor(
        [&](auto& concrete)
        {
            using Expression = std::remove_cvref_t<decltype(concrete)>;
            if constexpr (std::same_as<Expression, ast::ConstructorExpression>)
                concrete.text = normalize_expression_references(concrete.text, maps);
        },
        expression.get());
    return expression;
}

ast::Module normalize_module_expressions(ast::Module module, const ModuleReferenceMaps& maps)
{
    for (auto& feature : module.features)
    {
        boost::apply_visitor([&](auto& concrete) { concrete.expression = normalize_expression_references(concrete.expression, maps); }, feature.get());
    }

    for (auto& transition : module.rule_entries)
    {
        for (auto& rule : transition.rules)
        {
            boost::apply_visitor(
                [&](auto& concrete)
                {
                    using Rule = std::remove_cvref_t<decltype(concrete)>;
                    if constexpr (requires { concrete.expression; })
                        concrete.expression = normalize_expression_references(concrete.expression, maps);
                    if constexpr (requires { concrete.arguments; })
                        for (auto& argument : concrete.arguments)
                            argument = normalize_expression_references(argument, maps);
                    if constexpr (std::same_as<Rule, ast::CallRule>) {}
                },
                rule.get());
        }
    }
    return module;
}

std::string unquote(std::string value)
{
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"')
        return value.substr(1, value.size() - 2);
    return value;
}

bool parse_truth_atom(const SExpr& expr)
{
    const auto& value = require_atom(expr, "truth value");
    if (value == runir::kr::dl::TrueTag::keyword)
        return true;
    if (value == runir::kr::dl::FalseTag::keyword)
        return false;
    throw std::runtime_error("Expected true or false truth value.");
}

ygg::uint_t parse_uint_atom(const SExpr& expr, const char* context)
{
    const auto& value = require_atom(expr, context);
    return static_cast<ygg::uint_t>(std::stoull(value));
}

template<typename Variant>
Variant parse_constructor_variant_child(runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository,
                                        const SExpr& expr,
                                        tyr::formalism::planning::DomainView domain)
{
    const auto text = format_expression(expr);
    const auto op = expression_operator(expr);
    if (op.starts_with("r_"))
        return parse_role(text, domain, repository).get_index();
    return parse_concept(text, domain, repository).get_index();
}

template<typename RuleTag, typename Category>
auto intern_rule_variant(Repository& repository, ygg::Data<Rule<RuleTag, Category>>& data, const std::string& symbol)
{
    const auto rule = intern(repository, data);
    ygg::Data<RuleVariant> variant_data(symbol, rule.get_index());
    return intern(repository, variant_data);
}

auto require_memory_state(const MemoryStateMap& memory_states, const std::string& name)
{
    const auto it = memory_states.find(name);
    if (it == memory_states.end())
        fail("Unknown memory state \"" + name + "\"");
    return it->second;
}

template<typename RegisterMap>
auto require_register(const RegisterMap& registers, const std::string& name)
{
    const auto it = registers.find(name);
    if (it == registers.end())
        fail("Unknown register \"" + name + "\"");
    return it->second;
}

auto find_module(const ModuleMap& modules, const std::string& name)
{
    const auto it = modules.find(name);
    return it == modules.end() ? std::optional<ygg::Index<Module>> {} : std::optional(it->second);
}

auto find_action_arity(tyr::formalism::planning::DomainView domain, const std::string& name)
{
    for (const auto action : domain.get_actions())
        if (action.get_name() == name)
            return std::optional<ygg::uint_t>(action.get_original_arity());
    return std::optional<ygg::uint_t> {};
}

void validate_do_action(tyr::formalism::planning::DomainView domain, const ast::DoRule& rule)
{
    const auto arity = find_action_arity(domain, rule.action);
    if (!arity)
        fail("Unknown action \"" + rule.action + "\"");

    if (rule.arguments.size() != *arity)
        fail("Do rule for action \"" + rule.action + "\" has " + std::to_string(rule.arguments.size()) + " arguments; expected " + std::to_string(*arity));
}

template<dl_::CategoryTag Category>
auto parse_load_rule(Repository& repository,
                     const ast::LoadRule<Category>& rule,
                     ygg::Index<MemoryState> source,
                     ygg::Index<MemoryState> target,
                     tyr::formalism::planning::DomainView domain,
                     const ConceptRegisterMap& concept_registers,
                     const RoleRegisterMap& role_registers,
                     const ConceptFeatureMap& concept_features,
                     const RoleFeatureMap& role_features,
                     const BooleanFeatureMap& boolean_features,
                     const NumericalFeatureMap& numerical_features,
                     const ConceptAliasMap& concept_aliases,
                     const std::string& symbol)
{
    ygg::Data<Rule<LoadTag, Category>> data;
    data.source = source;
    data.target = target;
    data.conditions = parse_conditions(repository, rule.conditions, concept_features, boolean_features, numerical_features);
    if constexpr (std::same_as<Category, dl_::ConceptTag>)
    {
        data.load_expression = parse_concept_argument(repository, rule.expression, domain, concept_aliases);
        data.reg = require_register(concept_registers, rule.reg);
    }
    else
    {
        data.load_expression = parse_role_argument(repository, rule.expression, domain, role_features);
        data.reg = require_register(role_registers, rule.reg);
    }
    return intern_rule_variant(repository, data, symbol);
}

auto parse_rule(Repository& repository,
                const ast::Rule& rule,
                ygg::Index<MemoryState> source,
                ygg::Index<MemoryState> target,
                tyr::formalism::planning::DomainView domain,
                const ConceptRegisterMap& concept_registers,
                const RoleRegisterMap& role_registers,
                const ModuleMap& modules,
                const ConceptFeatureMap& concept_features,
                const RoleFeatureMap& role_features,
                const BooleanFeatureMap& boolean_features,
                const NumericalFeatureMap& numerical_features,
                const ConceptAliasMap& concept_aliases,
                const std::string& symbol)
{
    return boost::apply_visitor(
        [&](const auto& concrete) -> RuleVariantView
        {
            using RuleAst = std::remove_cvref_t<decltype(concrete)>;
            if constexpr (requires { typename AstCategory<RuleAst>::Type; })
            {
                using Category = typename AstCategory<RuleAst>::Type;
                return parse_load_rule<Category>(repository,
                                                 concrete,
                                                 source,
                                                 target,
                                                 domain,
                                                 concept_registers,
                                                 role_registers,
                                                 concept_features,
                                                 role_features,
                                                 boolean_features,
                                                 numerical_features,
                                                 concept_aliases,
                                                 symbol);
            }
            else if constexpr (std::same_as<RuleAst, ast::SketchRule>)
            {
                ygg::Data<Rule<SketchTag>> data;
                data.source = source;
                data.target = target;
                data.conditions = parse_conditions(repository, concrete.conditions, concept_features, boolean_features, numerical_features);
                data.effects = parse_effects(repository, concrete.effects, concept_features, boolean_features, numerical_features);
                return intern_rule_variant(repository, data, symbol);
            }
            else if constexpr (std::same_as<RuleAst, ast::DoRule>)
            {
                validate_do_action(domain, concrete);
                ygg::Data<Rule<DoTag>> data(concrete.action);
                data.source = source;
                data.target = target;
                data.conditions = parse_conditions(repository, concrete.conditions, concept_features, boolean_features, numerical_features);
                data.effects = parse_effects(repository, concrete.effects, concept_features, boolean_features, numerical_features);
                for (const auto& argument : concrete.arguments)
                    data.arguments.push_back(parse_do_argument(argument, concept_features));
                return intern_rule_variant(repository, data, symbol);
            }
            else if constexpr (std::same_as<RuleAst, ast::CallRule>)
            {
                ygg::Data<Rule<CallTag>> data;
                data.source = source;
                data.target = target;
                data.conditions = parse_conditions(repository, concrete.conditions, concept_features, boolean_features, numerical_features);
                data.callee_name = concrete.callee;
                if (const auto callee = find_module(modules, concrete.callee))
                    data.callee = *callee;
                for (const auto& argument : concrete.arguments)
                    data.arguments.push_back(parse_call_argument(repository, argument, concept_features, role_features, boolean_features, numerical_features));
                return intern_rule_variant(repository, data, symbol);
            }
        },
        rule.get());
}

}  // namespace

runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>
parse_concept(const std::string& description,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    return parse_constructor(runir::kr::dl::grammar::parser::ext::parse_concept_ast(description), domain, repository);
}

runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>
parse_role(const std::string& description,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    return parse_constructor(runir::kr::dl::grammar::parser::ext::parse_role_ast(description), domain, repository);
}

runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanTag>
parse_boolean(const std::string& description,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    const auto expr = parse_expression(description);
    const auto& op = expression_operator(expr);

    if (op == "b_argument")
    {
        if (expr.list.size() != 2)
            throw std::runtime_error("b_argument expects one identifier.");
        ygg::Data<runir::kr::dl::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>> data(
            runir::kr::dl::ArgumentIdentifier<runir::kr::dl::BooleanTag>(parse_uint_atom(expr.list[1], "boolean argument identifier")));
        return intern_constructor<runir::kr::dl::BooleanTag>(repository, intern_dl(repository, data).get_index());
    }

    if (op == "b_nonempty")
    {
        if (expr.list.size() != 2)
            throw std::runtime_error("b_nonempty expects one concept or role argument.");
        using Data = ygg::Data<runir::kr::dl::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::NonemptyTag>>;
        Data data(parse_constructor_variant_child<typename Data::ConstructorVariant>(repository, expr.list[1], domain));
        return intern_constructor<runir::kr::dl::BooleanTag>(repository, intern_dl(repository, data).get_index());
    }

    if (op == "b_atomic_state" || op == "b_atomic_goal")
    {
        if (expr.list.size() != 3)
            throw std::runtime_error(op + " expects a predicate name and polarity.");
        const auto predicate_name = unquote(require_atom(expr.list[1], "boolean predicate name"));
        const auto polarity = parse_truth_atom(expr.list[2]);
        if (op == "b_atomic_state")
        {
            return resolve_predicate(domain,
                                     predicate_name,
                                     0,
                                     "BooleanAtomicState",
                                     [&](auto tag, auto predicate)
                                     {
                                         using T = decltype(tag);
                                         ygg::Data<runir::kr::dl::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::AtomicStateTag<T>>> data(predicate, polarity);
                                         return intern_constructor<runir::kr::dl::BooleanTag>(repository, intern_dl(repository, data).get_index());
                                     });
        }
        return resolve_predicate(domain,
                                 predicate_name,
                                 0,
                                 "BooleanAtomicGoal",
                                 [&](auto tag, auto predicate)
                                 {
                                     using T = decltype(tag);
                                     ygg::Data<runir::kr::dl::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::AtomicGoalTag<T>>> data(predicate, polarity);
                                     return intern_constructor<runir::kr::dl::BooleanTag>(repository, intern_dl(repository, data).get_index());
                                 });
    }

    throw std::runtime_error("Unsupported boolean DL expression \"" + op + "\".");
}

runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalTag>
parse_numerical(const std::string& description,
                tyr::formalism::planning::DomainView domain,
                runir::kr::dl::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>& repository)
{
    const auto expr = parse_expression(description);
    const auto& op = expression_operator(expr);

    if (op == "n_argument")
    {
        if (expr.list.size() != 2)
            throw std::runtime_error("n_argument expects one identifier.");
        ygg::Data<runir::kr::dl::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>> data(
            runir::kr::dl::ArgumentIdentifier<runir::kr::dl::NumericalTag>(parse_uint_atom(expr.list[1], "numerical argument identifier")));
        return intern_constructor<runir::kr::dl::NumericalTag>(repository, intern_dl(repository, data).get_index());
    }

    if (op == "n_count")
    {
        if (expr.list.size() != 2)
            throw std::runtime_error("n_count expects one concept or role argument.");
        using Data = ygg::Data<runir::kr::dl::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::CountTag>>;
        Data data(parse_constructor_variant_child<typename Data::ConstructorVariant>(repository, expr.list[1], domain));
        return intern_constructor<runir::kr::dl::NumericalTag>(repository, intern_dl(repository, data).get_index());
    }

    if (op == "n_distance")
    {
        if (expr.list.size() != 4)
            throw std::runtime_error("n_distance expects concept, role, and concept arguments.");
        ygg::Data<runir::kr::dl::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::DistanceTag>> data(
            parse_concept(format_expression(expr.list[1]), domain, repository).get_index(),
            parse_role(format_expression(expr.list[2]), domain, repository).get_index(),
            parse_concept(format_expression(expr.list[3]), domain, repository).get_index());
        return intern_constructor<runir::kr::dl::NumericalTag>(repository, intern_dl(repository, data).get_index());
    }

    throw std::runtime_error("Unsupported numerical DL expression \"" + op + "\".");
}

ModuleView lower_module(const ast::Module& ast, tyr::formalism::planning::DomainView domain, Repository& repository, const ModuleMap& known_modules = {})
{
    validate_module_declarations(ast);
    const auto maps = reference_maps(ast);
    const auto normalized_ast = normalize_module_expressions(ast, maps);

    auto memory_states = MemoryStateMap {};
    for (const auto& state : normalized_ast.memory_states)
    {
        auto state_data = ygg::Data<MemoryState>(state.value);
        if (!memory_states.emplace(state.value, intern(repository, state_data).get_index()).second)
            fail("Duplicate memory state \"" + state.value + "\"");
    }

    const auto entry = memory_states.find(normalized_ast.entry);
    if (entry == memory_states.end())
        fail("Module entry memory state \"" + normalized_ast.entry + "\" is not declared in :memory");

    auto data = ygg::Data<Module>(normalized_ast.name);
    data.entry_memory_state = entry->second;

    auto concept_argument = ygg::uint_t(0);
    auto role_argument = ygg::uint_t(0);
    auto boolean_argument = ygg::uint_t(0);
    auto numerical_argument = ygg::uint_t(0);
    for (const auto& argument : normalized_ast.arguments)
        append_argument(repository, data, argument, concept_argument, role_argument, boolean_argument, numerical_argument);

    auto concept_registers = ConceptRegisterMap {};
    auto role_registers = RoleRegisterMap {};
    auto concept_register = ygg::uint_t(0);
    auto role_register = ygg::uint_t(0);
    for (const auto& reg : normalized_ast.registers)
    {
        boost::apply_visitor(
            [&](const auto& concrete)
            {
                using Category = typename AstCategory<std::remove_cvref_t<decltype(concrete)>>::Type;
                if constexpr (std::same_as<Category, dl_::ConceptTag>)
                {
                    auto reg_data = ygg::Data<Register<Category>>(concrete.symbol, runir::kr::dl::RegisterIdentifier<Category>(concept_register++));
                    const auto view = intern(repository, reg_data);
                    data.concept_registers.push_back(view.get_index());
                    if (!concept_registers.emplace(concrete.symbol, view.get_index()).second)
                        fail("Duplicate concept register name \"" + concrete.symbol + "\"");
                }
                else if constexpr (std::same_as<Category, dl_::RoleTag>)
                {
                    auto reg_data = ygg::Data<Register<Category>>(concrete.symbol, runir::kr::dl::RegisterIdentifier<Category>(role_register++));
                    const auto view = intern(repository, reg_data);
                    data.role_registers.push_back(view.get_index());
                    if (!role_registers.emplace(concrete.symbol, view.get_index()).second)
                        fail("Duplicate role register name \"" + concrete.symbol + "\"");
                }
            },
            reg.get());
    }

    for (const auto& state : normalized_ast.memory_states)
        data.memory_states.push_back(memory_states.at(state.value));

    auto concept_features = ConceptFeatureMap {};
    auto role_features = RoleFeatureMap {};
    auto boolean_features = BooleanFeatureMap {};
    auto numerical_features = NumericalFeatureMap {};
    auto concept_aliases = ConceptAliasMap {};
    for (const auto& feature : normalized_ast.features)
    {
        boost::apply_visitor(
            [&](const auto& concrete)
            { append_feature(repository, data, concrete, domain, concept_features, role_features, boolean_features, numerical_features, concept_aliases); },
            feature.get());
    }

    auto modules = known_modules;
    if (auto callee = repository.find(data))
        modules.emplace(normalized_ast.name, callee->get_index());

    data.memory_transitions.reserve(normalized_ast.rule_entries.size());
    for (const auto& transition : normalized_ast.rule_entries)
    {
        auto parsed_transition = ygg::IndexList<RuleVariant> {};
        const auto source = require_memory_state(memory_states, transition.source);
        const auto target = require_memory_state(memory_states, transition.target);
        for (const auto& rule : transition.rules)
            parsed_transition.push_back(parse_rule(repository,
                                                   rule,
                                                   source,
                                                   target,
                                                   domain,
                                                   concept_registers,
                                                   role_registers,
                                                   modules,
                                                   concept_features,
                                                   role_features,
                                                   boolean_features,
                                                   numerical_features,
                                                   concept_aliases,
                                                   transition.symbol)
                                            .get_index());
        ygg::canonicalize(parsed_transition);
        data.memory_transitions.push_back(std::move(parsed_transition));
    }

    return intern(repository, data);
}

void validate_module_set(const std::vector<ast::Module>& modules)
{
    auto signatures_by_name = std::unordered_map<std::string, SignatureCounts> {};
    auto feature_symbols_by_module = std::unordered_map<std::string, FeatureSymbolSets> {};
    for (const auto& module : modules)
    {
        if (!signatures_by_name.emplace(module.name, signature_counts(module)).second)
            fail("Duplicate module name \"" + module.name + "\" in module set");
        feature_symbols_by_module.emplace(module.name, feature_symbols(module));
    }

    for (const auto& module : modules)
    {
        const auto& caller_features = feature_symbols_by_module.at(module.name);
        for (const auto& transition : module.rule_entries)
        {
            for (const auto& rule : transition.rules)
            {
                boost::apply_visitor(
                    [&](const auto& concrete)
                    {
                        using RuleAst = std::remove_cvref_t<decltype(concrete)>;
                        if constexpr (std::same_as<RuleAst, ast::CallRule>)
                        {
                            const auto callee = signatures_by_name.find(concrete.callee);
                            if (callee == signatures_by_name.end())
                                fail("Module \"" + module.name + "\" calls unknown module \"" + concrete.callee + "\"");

                            const auto actual = call_argument_signature_counts(concrete, caller_features);
                            if (actual != callee->second)
                                fail("Call from module \"" + module.name + "\" to module \"" + concrete.callee + "\" has argument signature "
                                     + signature_text(actual) + "; expected " + signature_text(callee->second));
                        }
                    },
                    rule.get());
            }
        }
    }
}

void validate_module_program(const ast::ModuleProgram& program)
{
    validate_module_set(program.modules);

    auto module_names = std::unordered_set<std::string> {};
    for (const auto& module : program.modules)
        module_names.emplace(module.name);

    if (!module_names.contains(program.entry))
        fail("Module program entry module \"" + program.entry + "\" is not declared");
}

ModuleView parse_module(const std::string& description, tyr::formalism::planning::DomainView domain, Repository& repository)
{
    return lower_module(parser::parse_module_ast(description), domain, repository);
}

ModuleProgramView parse_module_program(const std::string& description, tyr::formalism::planning::DomainView domain, Repository& repository)
{
    const auto ast = parser::parse_module_program_ast(description);
    validate_module_program(ast);

    auto module_views = std::vector<ModuleView> {};
    module_views.reserve(ast.modules.size());
    auto module_indices_by_name = ModuleMap {};

    for (const auto& module : ast.modules)
    {
        auto view = lower_module(module, domain, repository, module_indices_by_name);
        module_views.push_back(view);
        module_indices_by_name.emplace(module.name, view.get_index());
    }

    auto data = ygg::Data<ModuleProgram> {};
    data.entry_module = module_indices_by_name.at(ast.entry);
    for (const auto module : module_views)
        data.modules.push_back(module.get_index());

    return intern(repository, data);
}

std::vector<ModuleView> parse_modules(const std::vector<std::string>& descriptions, tyr::formalism::planning::DomainView domain, Repository& repository)
{
    auto asts = std::vector<ast::Module> {};
    asts.reserve(descriptions.size());
    for (const auto& description : descriptions)
        asts.push_back(parser::parse_module_ast(description));

    validate_module_set(asts);

    auto result = std::vector<ModuleView> {};
    result.reserve(asts.size());
    auto modules = ModuleMap {};
    for (const auto& ast : asts)
    {
        auto view = lower_module(ast, domain, repository, modules);
        result.push_back(view);
        modules.emplace(ast.name, view.get_index());
    }
    return result;
}

}  // namespace runir::kr::ps::ext::dl
