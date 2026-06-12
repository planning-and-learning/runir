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
auto intern_dl(runir::kr::dl::ext::ConstructorRepository& repository, ygg::Data<T>& data)
{
    if constexpr (requires { runir::kr::dl::canonicalize(data); })
        runir::kr::dl::canonicalize(data);
    return repository.get_or_create(data).first;
}

template<dl_::CategoryTag Category, typename T>
auto intern_constructor(runir::kr::dl::ext::ConstructorRepository& repository, ygg::Index<T> index)
{
    ygg::Data<dl_::Constructor<runir::kr::ExtFamilyTag, Category>> data(index);
    return intern_dl(repository, data);
}

template<dl_::CategoryTag Category>
auto parse_constructor(const dl_ast::Constructor<runir::kr::ExtFamilyTag, Category>& node,
                       tyr::formalism::planning::DomainView domain,
                       runir::kr::dl::ext::ConstructorRepository& repository) -> dl_::FamilyConstructorView<runir::kr::ExtFamilyTag, Category>;

template<dl_::CategoryTag Category>
auto parse_choice(const dl_ast::ConstructorOrNonTerminal<runir::kr::ExtFamilyTag, Category>& node,
                  tyr::formalism::planning::DomainView domain,
                  runir::kr::dl::ext::ConstructorRepository& repository) -> dl_::FamilyConstructorView<runir::kr::ExtFamilyTag, Category>
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

auto parse_dl(const dl_ast::ConceptBot&, tyr::formalism::planning::DomainView, runir::kr::dl::ext::ConstructorRepository& repository)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, dl_::BotTag>> data;
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::ConceptTop&, tyr::formalism::planning::DomainView, runir::kr::dl::ext::ConstructorRepository& repository)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, dl_::TopTag>> data;
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::ConceptAtomicState& node, tyr::formalism::planning::DomainView domain, runir::kr::dl::ext::ConstructorRepository& repository)
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

auto parse_dl(const dl_ast::ConceptAtomicGoal& node, tyr::formalism::planning::DomainView domain, runir::kr::dl::ext::ConstructorRepository& repository)
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
auto parse_binary_concept(const Ast& node, tyr::formalism::planning::DomainView domain, runir::kr::dl::ext::ConstructorRepository& repository)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, Tag>> data(parse_choice(node.lhs, domain, repository).get_index(),
                                                               parse_choice(node.rhs, domain, repository).get_index());
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::ConceptIntersection<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ext::ConstructorRepository& repository)
{
    return parse_binary_concept<dl_::IntersectionTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::ConceptUnion<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ext::ConstructorRepository& repository)
{
    return parse_binary_concept<dl_::UnionTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::ConceptValueRestriction<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ext::ConstructorRepository& repository)
{
    return parse_binary_concept<dl_::ValueRestrictionTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::ConceptExistentialQuantification<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ext::ConstructorRepository& repository)
{
    return parse_binary_concept<dl_::ExistentialQuantificationTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::ConceptNegation<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ext::ConstructorRepository& repository)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, dl_::NegationTag>> data(parse_choice(node.arg, domain, repository).get_index());
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

template<typename Tag, typename Ast>
auto parse_number_restriction(const Ast& node, tyr::formalism::planning::DomainView domain, runir::kr::dl::ext::ConstructorRepository& repository)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, Tag>> data(node.n, parse_choice(node.role, domain, repository).get_index());
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::ConceptAtLeastNumberRestriction<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ext::ConstructorRepository& repository)
{
    return parse_number_restriction<dl_::AtLeastNumberRestrictionTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::ConceptAtMostNumberRestriction<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ext::ConstructorRepository& repository)
{
    return parse_number_restriction<dl_::AtMostNumberRestrictionTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::ConceptExactNumberRestriction<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ext::ConstructorRepository& repository)
{
    return parse_number_restriction<dl_::ExactNumberRestrictionTag>(node, domain, repository);
}

template<typename Tag, typename Ast>
auto parse_qualified_number_restriction(const Ast& node, tyr::formalism::planning::DomainView domain, runir::kr::dl::ext::ConstructorRepository& repository)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, Tag>> data(node.n,
                                                               parse_choice(node.role, domain, repository).get_index(),
                                                               parse_choice(node.concept_, domain, repository).get_index());
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::ConceptQualifiedAtLeastNumberRestriction<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ext::ConstructorRepository& repository)
{
    return parse_qualified_number_restriction<dl_::QualifiedAtLeastNumberRestrictionTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::ConceptQualifiedAtMostNumberRestriction<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ext::ConstructorRepository& repository)
{
    return parse_qualified_number_restriction<dl_::QualifiedAtMostNumberRestrictionTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::ConceptQualifiedExactNumberRestriction<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ext::ConstructorRepository& repository)
{
    return parse_qualified_number_restriction<dl_::QualifiedExactNumberRestrictionTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::ConceptRoleValueMap<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ext::ConstructorRepository& repository)
{
    return parse_binary_concept<dl_::RoleValueMapTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::ConceptAgreement<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ext::ConstructorRepository& repository)
{
    return parse_binary_concept<dl_::AgreementTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::ConceptRoleFillers<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ext::ConstructorRepository& repository)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, dl_::RoleFillersTag>> data(parse_choice(node.role, domain, repository).get_index(),
                                                                               require_objects(domain, node.object_names));
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::ConceptOneOf& node, tyr::formalism::planning::DomainView domain, runir::kr::dl::ext::ConstructorRepository& repository)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, dl_::OneOfTag>> data(require_objects(domain, node.object_names));
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::ConceptNominal& node, tyr::formalism::planning::DomainView domain, runir::kr::dl::ext::ConstructorRepository& repository)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, dl_::NominalTag>> data(require_object(domain, node.object_name));
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ext_ast::ConceptRegister& node, tyr::formalism::planning::DomainView, runir::kr::dl::ext::ConstructorRepository& repository)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, dl_::RegisterTag>> data(dl_::RegisterIdentifier<dl_::ConceptTag>(node.identifier));
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ext_ast::Argument<dl_::ConceptTag>& node, tyr::formalism::planning::DomainView, runir::kr::dl::ext::ConstructorRepository& repository)
{
    ygg::Data<dl_::Concept<runir::kr::ExtFamilyTag, dl_::ArgumentTag<dl_::ConceptTag>>> data(dl_::ArgumentIdentifier<dl_::ConceptTag>(node.identifier));
    return intern_constructor<dl_::ConceptTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::RoleUniversal&, tyr::formalism::planning::DomainView, runir::kr::dl::ext::ConstructorRepository& repository)
{
    ygg::Data<dl_::Role<runir::kr::ExtFamilyTag, dl_::UniversalTag>> data;
    return intern_constructor<dl_::RoleTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::RoleAtomicState& node, tyr::formalism::planning::DomainView domain, runir::kr::dl::ext::ConstructorRepository& repository)
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

auto parse_dl(const dl_ast::RoleAtomicGoal& node, tyr::formalism::planning::DomainView domain, runir::kr::dl::ext::ConstructorRepository& repository)
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
auto parse_binary_role(const Ast& node, tyr::formalism::planning::DomainView domain, runir::kr::dl::ext::ConstructorRepository& repository)
{
    ygg::Data<dl_::Role<runir::kr::ExtFamilyTag, Tag>> data(parse_choice(node.lhs, domain, repository).get_index(),
                                                            parse_choice(node.rhs, domain, repository).get_index());
    return intern_constructor<dl_::RoleTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::RoleIntersection<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ext::ConstructorRepository& repository)
{
    return parse_binary_role<dl_::IntersectionTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::RoleUnion<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ext::ConstructorRepository& repository)
{
    return parse_binary_role<dl_::UnionTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::RoleComposition<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ext::ConstructorRepository& repository)
{
    return parse_binary_role<dl_::CompositionTag>(node, domain, repository);
}

template<typename Tag, typename Ast>
auto parse_unary_role(const Ast& node, tyr::formalism::planning::DomainView domain, runir::kr::dl::ext::ConstructorRepository& repository)
{
    ygg::Data<dl_::Role<runir::kr::ExtFamilyTag, Tag>> data(parse_choice(node.arg, domain, repository).get_index());
    return intern_constructor<dl_::RoleTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::RoleComplement<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ext::ConstructorRepository& repository)
{
    return parse_unary_role<dl_::ComplementTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::RoleInverse<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ext::ConstructorRepository& repository)
{
    return parse_unary_role<dl_::InverseTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::RoleTransitiveClosure<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ext::ConstructorRepository& repository)
{
    return parse_unary_role<dl_::TransitiveClosureTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::RoleReflexiveTransitiveClosure<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ext::ConstructorRepository& repository)
{
    return parse_unary_role<dl_::ReflexiveTransitiveClosureTag>(node, domain, repository);
}

auto parse_dl(const dl_ast::RoleRestriction<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ext::ConstructorRepository& repository)
{
    ygg::Data<dl_::Role<runir::kr::ExtFamilyTag, dl_::RestrictionTag>> data(parse_choice(node.lhs, domain, repository).get_index(),
                                                                            parse_choice(node.rhs, domain, repository).get_index());
    return intern_constructor<dl_::RoleTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ast::RoleIdentity<runir::kr::ExtFamilyTag>& node,
              tyr::formalism::planning::DomainView domain,
              runir::kr::dl::ext::ConstructorRepository& repository)
{
    ygg::Data<dl_::Role<runir::kr::ExtFamilyTag, dl_::IdentityTag>> data(parse_choice(node.arg, domain, repository).get_index());
    return intern_constructor<dl_::RoleTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ext_ast::RoleRegister& node, tyr::formalism::planning::DomainView, runir::kr::dl::ext::ConstructorRepository& repository)
{
    ygg::Data<dl_::Role<runir::kr::ExtFamilyTag, dl_::RegisterTag>> data(dl_::RegisterIdentifier<dl_::RoleTag>(node.identifier));
    return intern_constructor<dl_::RoleTag>(repository, intern_dl(repository, data).get_index());
}

auto parse_dl(const dl_ext_ast::Argument<dl_::RoleTag>& node, tyr::formalism::planning::DomainView, runir::kr::dl::ext::ConstructorRepository& repository)
{
    ygg::Data<dl_::Role<runir::kr::ExtFamilyTag, dl_::ArgumentTag<dl_::RoleTag>>> data(dl_::ArgumentIdentifier<dl_::RoleTag>(node.identifier));
    return intern_constructor<dl_::RoleTag>(repository, intern_dl(repository, data).get_index());
}

template<dl_::CategoryTag Category>
auto parse_constructor(const dl_ast::Constructor<runir::kr::ExtFamilyTag, Category>& node,
                       tyr::formalism::planning::DomainView domain,
                       runir::kr::dl::ext::ConstructorRepository& repository) -> dl_::FamilyConstructorView<runir::kr::ExtFamilyTag, Category>
{
    return boost::apply_visitor([&](const auto& arg) { return parse_dl(unwrap(arg), domain, repository); }, node.get());
}

template<runir::kr::dl::CategoryTag Category>
auto intern_argument(Repository& repository, const ast::Argument& argument)
{
    auto data = ygg::Data<Argument<Category>>(argument.name, runir::kr::dl::ArgumentIdentifier<Category>(argument.identifier));
    return intern(repository, data);
}

using ConceptFeatureMap = std::unordered_map<std::string, ygg::Index<runir::kr::ps::ext::Feature<runir::kr::dl::ConceptTag>>>;
using BooleanFeatureMap = std::unordered_map<std::string, ygg::Index<runir::kr::ps::ext::Feature<runir::kr::ps::dl::BooleanFeature>>>;
using NumericalFeatureMap = std::unordered_map<std::string, ygg::Index<runir::kr::ps::ext::Feature<runir::kr::ps::dl::NumericalFeature>>>;
using ConceptAliasMap = std::unordered_map<std::string, ygg::Index<runir::kr::dl::FamilyConstructor<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>>;
using MemoryStateMap = std::unordered_map<std::string, ygg::Index<MemoryState>>;
using RegisterMap = std::unordered_map<std::string, ygg::Index<Register>>;
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

[[noreturn]] void fail_at(const std::string& message, std::size_t offset) { throw std::runtime_error(message + " at offset " + std::to_string(offset) + "."); }

SignatureCounts signature_counts(const ast::Module& module)
{
    auto result = SignatureCounts {};
    for (const auto& argument : module.arguments)
    {
        switch (argument.kind)
        {
            case ast::ArgumentKind::CONCEPT:
                ++result.concepts;
                break;
            case ast::ArgumentKind::ROLE:
                ++result.roles;
                break;
            case ast::ArgumentKind::BOOLEAN:
                ++result.booleans;
                break;
            case ast::ArgumentKind::NUMERICAL:
                ++result.numericals;
                break;
        }
    }
    return result;
}

std::size_t argument_count(const SignatureCounts& counts, ast::ArgumentKind kind)
{
    switch (kind)
    {
        case ast::ArgumentKind::CONCEPT:
            return counts.concepts;
        case ast::ArgumentKind::ROLE:
            return counts.roles;
        case ast::ArgumentKind::BOOLEAN:
            return counts.booleans;
        case ast::ArgumentKind::NUMERICAL:
            return counts.numericals;
    }
    return 0;
}

const char* argument_kind_name(ast::ArgumentKind kind)
{
    switch (kind)
    {
        case ast::ArgumentKind::CONCEPT:
            return "concept";
        case ast::ArgumentKind::ROLE:
            return "role";
        case ast::ArgumentKind::BOOLEAN:
            return "boolean";
        case ast::ArgumentKind::NUMERICAL:
            return "numerical";
    }
    return "unknown";
}

void validate_module_declarations(const ast::Module& module)
{
    const auto counts = signature_counts(module);
    auto concept_arguments = std::unordered_set<std::uint64_t> {};
    auto role_arguments = std::unordered_set<std::uint64_t> {};
    auto boolean_arguments = std::unordered_set<std::uint64_t> {};
    auto numerical_arguments = std::unordered_set<std::uint64_t> {};

    for (const auto& argument : module.arguments)
    {
        const auto max_identifier = argument_count(counts, argument.kind);
        if (argument.identifier >= max_identifier)
            fail_at(std::string(argument_kind_name(argument.kind)) + " argument identifier " + std::to_string(argument.identifier)
                        + " is out of range; declared arguments of this kind: " + std::to_string(max_identifier) + ".",
                    argument.source_offset);

        auto& seen = argument.kind == ast::ArgumentKind::CONCEPT ? concept_arguments :
                     argument.kind == ast::ArgumentKind::ROLE    ? role_arguments :
                     argument.kind == ast::ArgumentKind::BOOLEAN ? boolean_arguments :
                                                                   numerical_arguments;
        if (!seen.emplace(argument.identifier).second)
            fail_at(std::string("Duplicate ") + argument_kind_name(argument.kind) + " argument identifier " + std::to_string(argument.identifier) + ".",
                    argument.source_offset);
    }

    auto register_identifiers = std::unordered_set<std::uint64_t> {};
    for (const auto& reg : module.registers)
    {
        if (reg.identifier >= runir::kr::dl::num_registers)
            fail_at("Register identifier " + std::to_string(reg.identifier) + " is out of range; max registers is "
                        + std::to_string(runir::kr::dl::num_registers) + ".",
                    reg.source_offset);
        if (!register_identifiers.emplace(reg.identifier).second)
            fail_at("Duplicate register identifier " + std::to_string(reg.identifier) + ".", reg.source_offset);
    }
}

void append_argument(Repository& repository, ygg::Data<Module>& data, const ast::Argument& argument)
{
    switch (argument.kind)
    {
        case ast::ArgumentKind::CONCEPT:
            data.concept_arguments.push_back(intern_argument<runir::kr::dl::ConceptTag>(repository, argument).get_index());
            break;
        case ast::ArgumentKind::ROLE:
            data.role_arguments.push_back(intern_argument<runir::kr::dl::RoleTag>(repository, argument).get_index());
            break;
        case ast::ArgumentKind::BOOLEAN:
            data.boolean_arguments.push_back(intern_argument<runir::kr::dl::BooleanTag>(repository, argument).get_index());
            break;
        case ast::ArgumentKind::NUMERICAL:
            data.numerical_arguments.push_back(intern_argument<runir::kr::dl::NumericalTag>(repository, argument).get_index());
            break;
    }
}

template<typename FeatureTag, typename ConcreteFeatureTag>
auto intern_dl_feature(Repository& repository,
                       ygg::Index<runir::kr::dl::FamilyConstructor<runir::kr::ExtFamilyTag, ConcreteFeatureTag>> constructor,
                       const ast::Feature& feature)
{
    ygg::Data<ConcreteFeature<runir::kr::DlTag, FeatureTag>> concrete_data(constructor, feature.symbol, feature.description);
    const auto concrete = intern(repository, concrete_data);
    ygg::Data<Feature<FeatureTag>> feature_data(concrete.get_index());
    return intern(repository, feature_data);
}

void append_feature(Repository& repository,
                    const ast::Feature& feature,
                    tyr::formalism::planning::DomainView domain,
                    ConceptFeatureMap& concept_features,
                    BooleanFeatureMap& boolean_features,
                    NumericalFeatureMap& numerical_features,
                    ConceptAliasMap& concept_aliases)
{
    try
    {
        switch (feature.kind)
        {
            case ast::FeatureKind::CONCEPT:
            {
                const auto constructor = parse_concept(feature.expression, domain, repository.get_dl_repository());
                const auto view = intern_dl_feature<runir::kr::dl::ConceptTag>(repository, constructor.get_index(), feature);
                if (!concept_features.emplace(feature.name, view.get_index()).second || !concept_aliases.emplace(feature.name, constructor.get_index()).second)
                    fail_at("Duplicate feature name \"" + feature.name + "\".", feature.name_offset);
                break;
            }
            case ast::FeatureKind::BOOLEAN:
            {
                const auto constructor = parse_boolean(feature.expression, domain, repository.get_dl_repository());
                const auto view = intern_dl_feature<runir::kr::ps::dl::BooleanFeature>(repository, constructor.get_index(), feature);
                if (!boolean_features.emplace(feature.name, view.get_index()).second)
                    fail_at("Duplicate feature name \"" + feature.name + "\".", feature.name_offset);
                break;
            }
            case ast::FeatureKind::NUMERICAL:
            {
                const auto constructor = parse_numerical(feature.expression, domain, repository.get_dl_repository());
                const auto view = intern_dl_feature<runir::kr::ps::dl::NumericalFeature>(repository, constructor.get_index(), feature);
                if (!numerical_features.emplace(feature.name, view.get_index()).second)
                    fail_at("Duplicate feature name \"" + feature.name + "\".", feature.name_offset);
                break;
            }
        }
    }
    catch (const std::exception& e)
    {
        fail_at("Invalid feature \"" + feature.name + "\": " + e.what(), feature.expression_offset);
    }
}

template<typename FeatureTag>
auto require_feature(const std::unordered_map<std::string, ygg::Index<Feature<FeatureTag>>>& features, const std::string& name, std::size_t offset)
{
    const auto it = features.find(name);
    if (it == features.end())
        fail_at("Unknown feature \"" + name + "\".", offset);
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
                     const ast::Observation& observation,
                     const ConceptFeatureMap& concept_features,
                     const BooleanFeatureMap& boolean_features,
                     const NumericalFeatureMap& numerical_features)
{
    switch (observation.kind)
    {
        case ast::ObservationKind::POSITIVE:
            return make_condition<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>(
                repository,
                require_feature(boolean_features, observation.feature, observation.feature_offset));
        case ast::ObservationKind::NEGATIVE:
            return make_condition<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>(
                repository,
                require_feature(boolean_features, observation.feature, observation.feature_offset));
        case ast::ObservationKind::EQUAL_ZERO:
            if (concept_features.contains(observation.feature))
                return make_condition<runir::kr::dl::ConceptTag, runir::kr::ps::dl::EqualZero>(
                    repository,
                    require_feature(concept_features, observation.feature, observation.feature_offset));
            return make_condition<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::EqualZero>(
                repository,
                require_feature(numerical_features, observation.feature, observation.feature_offset));
        case ast::ObservationKind::GREATER_ZERO:
            if (concept_features.contains(observation.feature))
                return make_condition<runir::kr::dl::ConceptTag, runir::kr::ps::dl::GreaterZero>(
                    repository,
                    require_feature(concept_features, observation.feature, observation.feature_offset));
            return make_condition<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::GreaterZero>(
                repository,
                require_feature(numerical_features, observation.feature, observation.feature_offset));
        case ast::ObservationKind::UNCHANGED:
        case ast::ObservationKind::INCREASES:
        case ast::ObservationKind::DECREASES:
            fail_at("Effect observations are not valid rule conditions.", observation.source_offset);
    }
    fail_at("Unknown condition observation kind.", observation.source_offset);
}

auto parse_effect(Repository& repository,
                  const ast::Observation& observation,
                  const ConceptFeatureMap& concept_features,
                  const BooleanFeatureMap& boolean_features,
                  const NumericalFeatureMap& numerical_features)
{
    switch (observation.kind)
    {
        case ast::ObservationKind::POSITIVE:
            return make_effect<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>(
                repository,
                require_feature(boolean_features, observation.feature, observation.feature_offset));
        case ast::ObservationKind::NEGATIVE:
            return make_effect<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>(
                repository,
                require_feature(boolean_features, observation.feature, observation.feature_offset));
        case ast::ObservationKind::UNCHANGED:
            if (concept_features.contains(observation.feature))
                return make_effect<runir::kr::dl::ConceptTag, runir::kr::ps::dl::Unchanged>(
                    repository,
                    require_feature(concept_features, observation.feature, observation.feature_offset));
            if (boolean_features.contains(observation.feature))
                return make_effect<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Unchanged>(
                    repository,
                    require_feature(boolean_features, observation.feature, observation.feature_offset));
            return make_effect<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Unchanged>(
                repository,
                require_feature(numerical_features, observation.feature, observation.feature_offset));
        case ast::ObservationKind::INCREASES:
            if (concept_features.contains(observation.feature))
                return make_effect<runir::kr::dl::ConceptTag, runir::kr::ps::dl::Increases>(
                    repository,
                    require_feature(concept_features, observation.feature, observation.feature_offset));
            return make_effect<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Increases>(
                repository,
                require_feature(numerical_features, observation.feature, observation.feature_offset));
        case ast::ObservationKind::DECREASES:
            if (concept_features.contains(observation.feature))
                return make_effect<runir::kr::dl::ConceptTag, runir::kr::ps::dl::Decreases>(
                    repository,
                    require_feature(concept_features, observation.feature, observation.feature_offset));
            return make_effect<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Decreases>(
                repository,
                require_feature(numerical_features, observation.feature, observation.feature_offset));
        case ast::ObservationKind::EQUAL_ZERO:
        case ast::ObservationKind::GREATER_ZERO:
            fail_at("Condition observations are not valid rule effects.", observation.source_offset);
    }
    fail_at("Unknown effect observation kind.", observation.source_offset);
}

auto parse_conditions(Repository& repository,
                      const std::vector<ast::Observation>& observations,
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
                   const std::vector<ast::Observation>& observations,
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

std::string trim_copy(std::string value)
{
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front())))
        value.erase(value.begin());
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())))
        value.pop_back();
    return value;
}

bool starts_with_expression(const std::string& value)
{
    const auto trimmed = trim_copy(value);
    return !trimmed.empty() && trimmed.front() == '(';
}

auto parse_concept_argument(Repository& repository,
                            const std::string& expression,
                            std::size_t offset,
                            tyr::formalism::planning::DomainView domain,
                            const ConceptAliasMap& concept_aliases)
{
    try
    {
        if (starts_with_expression(expression))
            return parse_concept(expression, domain, repository.get_dl_repository()).get_index();

        const auto it = concept_aliases.find(expression);
        if (it == concept_aliases.end())
            fail_at("Unknown concept expression or feature alias \"" + expression + "\".", offset);
        return it->second;
    }
    catch (const std::exception& e)
    {
        fail_at(e.what(), offset);
    }
}

auto parse_do_argument(const ast::Expression& expression, const ConceptFeatureMap& concept_features)
{
    const auto name = trim_copy(expression.text);
    if (starts_with_expression(name))
        fail_at("Do-rule action arguments must reference declared concept features by symbol.", expression.source_offset);
    return require_feature(concept_features, name, expression.source_offset);
}

auto parse_call_argument(Repository& repository,
                         const ast::Expression& expression,
                         tyr::formalism::planning::DomainView domain,
                         const ConceptAliasMap& concept_aliases) -> CallArgument
{
    const auto trimmed = trim_copy(expression.text);
    try
    {
        if (trimmed.starts_with("(r_"))
            return parse_role(trimmed, domain, repository.get_dl_repository()).get_index();
        if (trimmed.starts_with("(b_"))
            return parse_boolean(trimmed, domain, repository.get_dl_repository()).get_index();
        if (trimmed.starts_with("(n_"))
            return parse_numerical(trimmed, domain, repository.get_dl_repository()).get_index();
        return parse_concept_argument(repository, trimmed, expression.source_offset, domain, concept_aliases);
    }
    catch (const std::exception& e)
    {
        fail_at(e.what(), expression.source_offset);
    }
}

SignatureCounts call_argument_signature_counts(const ast::Rule& rule)
{
    auto result = SignatureCounts {};
    for (const auto& argument : rule.arguments)
    {
        const auto trimmed = trim_copy(argument.text);
        if (trimmed.starts_with("(r_"))
            ++result.roles;
        else if (trimmed.starts_with("(b_"))
            ++result.booleans;
        else if (trimmed.starts_with("(n_"))
            ++result.numericals;
        else
            ++result.concepts;
    }
    return result;
}

struct SExpr
{
    std::size_t source_offset = 0;
    std::string atom;
    std::vector<SExpr> list;

    bool is_atom() const noexcept { return list.empty(); }
};

struct ExpressionToken
{
    std::size_t source_offset = 0;
    std::string text;
};

std::vector<ExpressionToken> tokenize_expression(const std::string& text)
{
    auto tokens = std::vector<ExpressionToken> {};
    for (size_t i = 0; i < text.size();)
    {
        if (std::isspace(static_cast<unsigned char>(text[i])))
        {
            ++i;
        }
        else if (text[i] == '(' || text[i] == ')')
        {
            tokens.push_back(ExpressionToken { i, std::string(1, text[i++]) });
        }
        else if (text[i] == '"')
        {
            const auto start = i++;
            while (i < text.size() && text[i] != '"')
                ++i;
            if (i == text.size())
                throw std::runtime_error("Unterminated string in DL expression.");
            ++i;
            tokens.push_back(ExpressionToken { start, text.substr(start, i - start) });
        }
        else
        {
            const auto start = i;
            while (i < text.size() && !std::isspace(static_cast<unsigned char>(text[i])) && text[i] != '(' && text[i] != ')')
                ++i;
            tokens.push_back(ExpressionToken { start, text.substr(start, i - start) });
        }
    }
    return tokens;
}

SExpr parse_expression_tokens(const std::vector<ExpressionToken>& tokens, size_t& pos)
{
    if (pos >= tokens.size())
        throw std::runtime_error("Unexpected end of DL expression.");

    const auto source_offset = tokens[pos].source_offset;
    if (tokens[pos].text != "(")
        return SExpr { source_offset, tokens[pos++].text, {} };

    ++pos;
    auto result = SExpr { source_offset, {}, {} };
    while (pos < tokens.size() && tokens[pos].text != ")")
        result.list.push_back(parse_expression_tokens(tokens, pos));

    if (pos >= tokens.size())
        throw std::runtime_error("Unclosed list in DL expression.");
    ++pos;

    if (result.list.empty())
        throw std::runtime_error("Empty list in DL expression.");
    return result;
}

SExpr parse_expression(const std::string& text)
{
    const auto tokens = tokenize_expression(text);
    size_t pos = 0;
    auto result = parse_expression_tokens(tokens, pos);
    if (pos != tokens.size())
        throw std::runtime_error("Trailing tokens in DL expression.");
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

const std::string& expression_operator(const SExpr& expr);

ygg::uint_t parse_uint_atom(const SExpr& expr, const char* context)
{
    const auto& value = require_atom(expr, context);
    return static_cast<ygg::uint_t>(std::stoull(value));
}

ygg::uint_t parse_uint_atom_at(const SExpr& expr, std::size_t base_offset, const char* context)
{
    if (!expr.is_atom())
        fail_at(std::string("Expected atom in ") + context + ".", base_offset + expr.source_offset);

    try
    {
        std::size_t parsed = 0;
        const auto value = std::stoull(expr.atom, &parsed);
        if (parsed != expr.atom.size())
            fail_at(std::string("Expected unsigned integer in ") + context + ".", base_offset + expr.source_offset);
        return static_cast<ygg::uint_t>(value);
    }
    catch (const std::invalid_argument&)
    {
        fail_at(std::string("Expected unsigned integer in ") + context + ".", base_offset + expr.source_offset);
    }
    catch (const std::out_of_range&)
    {
        fail_at(std::string("Unsigned integer in ") + context + " is out of range.", base_offset + expr.source_offset);
    }
}

void validate_argument_reference_at(ygg::uint_t identifier, std::size_t count, const char* kind, std::size_t offset)
{
    if (identifier >= count)
        fail_at(std::string(kind) + " argument reference " + std::to_string(identifier)
                    + " is out of range; declared arguments of this kind: " + std::to_string(count) + ".",
                offset);
}

void validate_register_reference_at(ygg::uint_t identifier, const std::unordered_set<std::uint64_t>& concept_registers, const char* kind, std::size_t offset)
{
    if (identifier >= runir::kr::dl::num_registers)
        fail_at(std::string(kind) + " register reference " + std::to_string(identifier) + " is out of range; max registers is "
                    + std::to_string(runir::kr::dl::num_registers) + ".",
                offset);
    if (!concept_registers.contains(identifier))
        fail_at(std::string(kind) + " register reference " + std::to_string(identifier) + " is not declared by the module.", offset);
}

void validate_expression_references(const SExpr& expr,
                                    std::size_t base_offset,
                                    const SignatureCounts& signature,
                                    const std::unordered_set<std::uint64_t>& concept_registers)
{
    if (expr.is_atom())
        return;

    const auto fail_here = [&](const std::string& message) { fail_at(message, base_offset + expr.source_offset); };
    const auto& op = expression_operator(expr);
    if (op == "c_argument")
    {
        if (expr.list.size() != 2)
            fail_here("c_argument expects one identifier.");
        const auto identifier = parse_uint_atom_at(expr.list[1], base_offset, "concept argument identifier");
        validate_argument_reference_at(identifier, signature.concepts, "Concept", base_offset + expr.source_offset);
    }
    else if (op == "r_argument")
    {
        if (expr.list.size() != 2)
            fail_here("r_argument expects one identifier.");
        const auto identifier = parse_uint_atom_at(expr.list[1], base_offset, "role argument identifier");
        validate_argument_reference_at(identifier, signature.roles, "Role", base_offset + expr.source_offset);
    }
    else if (op == "b_argument")
    {
        if (expr.list.size() != 2)
            fail_here("b_argument expects one identifier.");
        const auto identifier = parse_uint_atom_at(expr.list[1], base_offset, "boolean argument identifier");
        validate_argument_reference_at(identifier, signature.booleans, "Boolean", base_offset + expr.source_offset);
    }
    else if (op == "n_argument")
    {
        if (expr.list.size() != 2)
            fail_here("n_argument expects one identifier.");
        const auto identifier = parse_uint_atom_at(expr.list[1], base_offset, "numerical argument identifier");
        validate_argument_reference_at(identifier, signature.numericals, "Numerical", base_offset + expr.source_offset);
    }
    else if (op == "c_register")
    {
        if (expr.list.size() != 2)
            fail_here("c_register expects one identifier.");
        const auto identifier = parse_uint_atom_at(expr.list[1], base_offset, "concept register identifier");
        validate_register_reference_at(identifier, concept_registers, "Concept", base_offset + expr.source_offset);
    }
    else if (op == "r_register")
    {
        if (expr.list.size() != 2)
            fail_here("r_register expects one identifier.");
        const auto identifier = parse_uint_atom_at(expr.list[1], base_offset, "role register identifier");
        validate_register_reference_at(identifier, concept_registers, "Role", base_offset + expr.source_offset);
    }

    for (std::size_t i = 1; i < expr.list.size(); ++i)
        validate_expression_references(expr.list[i], base_offset, signature, concept_registers);
}

void validate_expression_references(const std::string& expression,
                                    std::size_t offset,
                                    const SignatureCounts& signature,
                                    const std::unordered_set<std::uint64_t>& concept_registers)
{
    if (!starts_with_expression(expression))
        return;

    auto expr = SExpr {};
    try
    {
        expr = parse_expression(expression);
    }
    catch (const std::exception& e)
    {
        fail_at(e.what(), offset);
    }
    validate_expression_references(expr, offset, signature, concept_registers);
}

void validate_module_expression_references(const ast::Module& module)
{
    const auto signature = signature_counts(module);
    auto concept_registers = std::unordered_set<std::uint64_t> {};
    for (const auto& reg : module.registers)
        concept_registers.insert(reg.identifier);

    for (const auto& feature : module.features)
        validate_expression_references(feature.expression, feature.expression_offset, signature, concept_registers);

    for (const auto& transition : module.rule_entries)
    {
        for (const auto& rule : transition.rules)
        {
            if (rule.kind == ast::RuleKind::LOAD)
                validate_expression_references(rule.concept_expression, rule.concept_expression_offset, signature, concept_registers);
            for (const auto& argument : rule.arguments)
                validate_expression_references(argument.text, argument.source_offset, signature, concept_registers);
        }
    }
}

const std::string& expression_operator(const SExpr& expr)
{
    if (expr.is_atom() || expr.list.empty())
        throw std::runtime_error("Expected constructor expression list.");
    return require_atom(expr.list.front(), "constructor operator");
}

template<typename Variant>
Variant parse_constructor_variant_child(runir::kr::dl::ext::ConstructorRepository& repository, const SExpr& expr, tyr::formalism::planning::DomainView domain)
{
    const auto text = format_expression(expr);
    const auto op = expression_operator(expr);
    if (op.starts_with("r_"))
        return parse_role(text, domain, repository).get_index();
    return parse_concept(text, domain, repository).get_index();
}

template<typename RuleTag>
auto intern_rule_variant(Repository& repository, ygg::Data<Rule<RuleTag>>& data, const std::string& symbol, const std::string& description)
{
    const auto rule = intern(repository, data);
    ygg::Data<RuleVariant> variant_data(symbol, description, rule.get_index());
    return intern(repository, variant_data);
}

auto require_memory_state(const MemoryStateMap& memory_states, const std::string& name, std::size_t offset)
{
    const auto it = memory_states.find(name);
    if (it == memory_states.end())
        fail_at("Unknown memory state \"" + name + "\".", offset);
    return it->second;
}

auto require_register(const RegisterMap& registers, const std::string& name, std::size_t offset)
{
    const auto it = registers.find(name);
    if (it == registers.end())
        fail_at("Unknown register \"" + name + "\".", offset);
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

void validate_do_action(tyr::formalism::planning::DomainView domain, const ast::Rule& rule)
{
    const auto arity = find_action_arity(domain, rule.action);
    if (!arity)
        fail_at("Unknown action \"" + rule.action + "\".", rule.action_offset);

    if (rule.arguments.size() != *arity)
        fail_at("Do rule for action \"" + rule.action + "\" has " + std::to_string(rule.arguments.size()) + " arguments; expected " + std::to_string(*arity)
                    + ".",
                rule.arguments_offset);
}

auto parse_rule(Repository& repository,
                const ast::Rule& rule,
                ygg::Index<MemoryState> source,
                ygg::Index<MemoryState> target,
                tyr::formalism::planning::DomainView domain,
                const RegisterMap& registers,
                const ModuleMap& modules,
                const ConceptFeatureMap& concept_features,
                const BooleanFeatureMap& boolean_features,
                const NumericalFeatureMap& numerical_features,
                const ConceptAliasMap& concept_aliases,
                const std::string& symbol,
                const std::string& description)
{
    const auto conditions = parse_conditions(repository, rule.conditions, concept_features, boolean_features, numerical_features);

    switch (rule.kind)
    {
        case ast::RuleKind::LOAD:
        {
            ygg::Data<Rule<LoadTag>> data;
            data.source = source;
            data.target = target;
            data.conditions = conditions;
            data.load_concept = parse_concept_argument(repository, rule.concept_expression, rule.concept_expression_offset, domain, concept_aliases);
            data.reg = require_register(registers, rule.reg, rule.register_offset);
            return intern_rule_variant(repository, data, symbol, description);
        }
        case ast::RuleKind::SKETCH:
        {
            ygg::Data<Rule<SketchTag>> data;
            data.source = source;
            data.target = target;
            data.conditions = conditions;
            data.effects = parse_effects(repository, rule.effects, concept_features, boolean_features, numerical_features);
            return intern_rule_variant(repository, data, symbol, description);
        }
        case ast::RuleKind::DO:
        {
            validate_do_action(domain, rule);
            ygg::Data<Rule<DoTag>> data(rule.action);
            data.source = source;
            data.target = target;
            data.conditions = conditions;
            data.effects = parse_effects(repository, rule.effects, concept_features, boolean_features, numerical_features);
            for (const auto& argument : rule.arguments)
                data.arguments.push_back(parse_do_argument(argument, concept_features));
            return intern_rule_variant(repository, data, symbol, description);
        }
        case ast::RuleKind::CALL:
        {
            ygg::Data<Rule<CallTag>> data;
            data.source = source;
            data.target = target;
            data.conditions = conditions;
            data.callee_name = rule.callee;
            if (const auto callee = find_module(modules, rule.callee))
                data.callee = *callee;
            for (const auto& argument : rule.arguments)
                data.arguments.push_back(parse_call_argument(repository, argument, domain, concept_aliases));
            return intern_rule_variant(repository, data, symbol, description);
        }
    }
    throw std::runtime_error("Unknown rule kind.");
}

}  // namespace

runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>
parse_concept(const std::string& description, tyr::formalism::planning::DomainView domain, runir::kr::dl::ext::ConstructorRepository& repository)
{
    return parse_constructor(runir::kr::dl::grammar::parser::ext::parse_concept_ast(description), domain, repository);
}

runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>
parse_role(const std::string& description, tyr::formalism::planning::DomainView domain, runir::kr::dl::ext::ConstructorRepository& repository)
{
    return parse_constructor(runir::kr::dl::grammar::parser::ext::parse_role_ast(description), domain, repository);
}

runir::kr::dl::FamilyConstructorView<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanTag>
parse_boolean(const std::string& description, tyr::formalism::planning::DomainView domain, runir::kr::dl::ext::ConstructorRepository& repository)
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
parse_numerical(const std::string& description, tyr::formalism::planning::DomainView domain, runir::kr::dl::ext::ConstructorRepository& repository)
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
    validate_module_expression_references(ast);

    auto memory_states = MemoryStateMap {};
    for (const auto& state : ast.memory_states)
    {
        auto state_data = ygg::Data<MemoryState>(state.value);
        if (!memory_states.emplace(state.value, intern(repository, state_data).get_index()).second)
            fail_at("Duplicate memory state \"" + state.value + "\".", state.source_offset);
    }

    const auto entry = memory_states.find(ast.entry);
    if (entry == memory_states.end())
        fail_at("Module entry memory state \"" + ast.entry + "\" is not declared in :memory.", ast.entry_offset);

    auto data = ygg::Data<Module>(ast.name);
    data.entry_memory_state = entry->second;

    for (const auto& argument : ast.arguments)
        append_argument(repository, data, argument);

    auto registers = RegisterMap {};
    for (const auto& reg : ast.registers)
    {
        auto reg_data = ygg::Data<Register>(reg.name, runir::kr::dl::RegisterIdentifier<runir::kr::dl::ConceptTag>(reg.identifier));
        const auto view = intern(repository, reg_data);
        data.registers.push_back(view.get_index());
        if (!registers.emplace(std::to_string(reg.identifier), view.get_index()).second)
            fail_at("Duplicate register name \"" + reg.name + "\".", reg.name_offset);
    }

    for (const auto& state : ast.memory_states)
        data.memory_states.push_back(memory_states.at(state.value));

    auto concept_features = ConceptFeatureMap {};
    auto boolean_features = BooleanFeatureMap {};
    auto numerical_features = NumericalFeatureMap {};
    auto concept_aliases = ConceptAliasMap {};
    for (const auto& feature : ast.features)
        append_feature(repository, feature, domain, concept_features, boolean_features, numerical_features, concept_aliases);

    auto modules = known_modules;
    if (auto callee = repository.find(data))
        modules.emplace(ast.name, callee->get_index());

    data.memory_transitions.reserve(ast.rule_entries.size());
    for (const auto& transition : ast.rule_entries)
    {
        auto parsed_transition = ygg::IndexList<RuleVariant> {};
        const auto source = require_memory_state(memory_states, transition.source, transition.source_name_offset);
        const auto target = require_memory_state(memory_states, transition.target, transition.target_name_offset);
        for (const auto& rule : transition.rules)
            parsed_transition.push_back(parse_rule(repository,
                                                   rule,
                                                   source,
                                                   target,
                                                   domain,
                                                   registers,
                                                   modules,
                                                   concept_features,
                                                   boolean_features,
                                                   numerical_features,
                                                   concept_aliases,
                                                   transition.symbol,
                                                   transition.description)
                                            .get_index());
        ygg::canonicalize(parsed_transition);
        data.memory_transitions.push_back(std::move(parsed_transition));
    }

    return intern(repository, data);
}

void validate_module_set(const std::vector<ast::Module>& modules)
{
    auto signatures_by_name = std::unordered_map<std::string, SignatureCounts> {};
    for (const auto& module : modules)
    {
        if (!signatures_by_name.emplace(module.name, signature_counts(module)).second)
            fail_at("Duplicate module name \"" + module.name + "\" in module set", module.source_offset);
    }

    for (const auto& module : modules)
    {
        for (const auto& transition : module.rule_entries)
        {
            for (const auto& rule : transition.rules)
            {
                if (rule.kind != ast::RuleKind::CALL)
                    continue;

                const auto callee = signatures_by_name.find(rule.callee);
                if (callee == signatures_by_name.end())
                    fail_at("Module \"" + module.name + "\" calls unknown module \"" + rule.callee + "\"", rule.callee_offset);

                const auto actual = call_argument_signature_counts(rule);
                if (actual != callee->second)
                    fail_at("Call from module \"" + module.name + "\" to module \"" + rule.callee + "\" has argument signature " + signature_text(actual)
                                + "; expected " + signature_text(callee->second),
                            rule.source_offset);
            }
        }
    }
}

void validate_module_program(const ast::ModuleProgram& program)
{
    validate_module_set(program.modules);

    auto module_names = std::unordered_map<std::string, std::size_t> {};
    for (const auto& module : program.modules)
        module_names.emplace(module.name, module.source_offset);

    if (!module_names.contains(program.entry))
        fail_at("Module program entry module \"" + program.entry + "\" is not declared.", program.entry_offset);
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
