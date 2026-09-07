#include "runir/kr/ps/base/dl/parser.hpp"

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
#include <optional>
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

using DiagnosticContext = runir::kr::parser::DiagnosticContext;
using DlBuilder = runir::kr::dl::Builder<runir::kr::BaseFamilyTag>;
using PsBuilder = runir::kr::ps::base::Builder;

struct ConstructorContext
{
    const DiagnosticContext& diagnostics;
    DlBuilder& builder;

    template<typename Position, typename Error>
    [[noreturn]] void throw_at(const Position& position, Error error) const
    {
        diagnostics.throw_at(position, std::move(error));
    }
};

struct BuildContext
{
    DlBuilder& dl;
    PsBuilder& ps;
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
auto intern(runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository, ygg::Data<T>& data)
{
    return runir::kr::dl::get_or_create(repository, data).first;
}

template<runir::kr::dl::CategoryTag Category, typename T>
auto intern_constructor(runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository, DlBuilder& builder, ygg::Index<T> index)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Constructor<runir::kr::BaseFamilyTag, Category>>(builder);
    data->variant = index;
    return intern(repository, *data);
}

template<runir::kr::dl::CategoryTag Category>
auto parse_constructor(const runir::kr::dl::grammar::ast::Constructor<runir::kr::BaseFamilyTag, Category>& node,
                       tyr::formalism::planning::DomainView domain,
                       runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
                       const ConstructorContext& diagnostics);

template<runir::kr::dl::CategoryTag Category>
auto parse_constructor_or_non_terminal(const runir::kr::dl::grammar::ast::ConstructorOrNonTerminal<runir::kr::BaseFamilyTag, Category>& node,
                                       tyr::formalism::planning::DomainView domain,
                                       runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
                                       const ConstructorContext& diagnostics) -> runir::kr::dl::FamilyConstructorView<runir::kr::BaseFamilyTag, Category>
{
    return boost::apply_visitor(
        [&](const auto& value) -> runir::kr::dl::FamilyConstructorView<runir::kr::BaseFamilyTag, Category>
        {
            const auto& unwrapped = unwrap(value);
            if constexpr (std::same_as<std::remove_cvref_t<decltype(unwrapped)>, runir::kr::dl::grammar::ast::NonTerminal<runir::kr::BaseFamilyTag, Category>>)
                diagnostics.throw_at(unwrapped.name,
                                     runir::kr::InvalidExpressionError("General-sketch DL features cannot "
                                                                       "reference grammar nonterminals."));
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
                       const ConstructorContext& diagnostics)
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
                       const ConstructorContext& diagnostics,
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

auto require_object(tyr::formalism::planning::DomainView domain, const runir::kr::parser::ast::Identifier& name, const ConstructorContext& diagnostics)
{
    for (auto object : domain.get_constants())
        if (object.get_name() == name.text)
            return object.get_index();

    diagnostics.throw_at(name, runir::kr::UndefinedSymbolError("constant", name.text));
}

void append_objects(tyr::formalism::planning::DomainView domain,
                    const std::vector<runir::kr::dl::grammar::ast::Identifier>& names,
                    const ConstructorContext& diagnostics,
                    ygg::IndexList<tyr::formalism::Object>& result)
{
    result.reserve(result.size() + names.size());
    for (const auto& name : names)
        result.push_back(require_object(domain, name, diagnostics));
}

auto parse(const runir::kr::dl::grammar::ast::ConceptBot<runir::kr::BaseFamilyTag>&,
           tyr::formalism::planning::DomainView,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::BotTag>>(diagnostics.builder);
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, diagnostics.builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptTop<runir::kr::BaseFamilyTag>&,
           tyr::formalism::planning::DomainView,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::TopTag>>(diagnostics.builder);
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, diagnostics.builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtomicState<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             1,
                             "ConceptAtomicState",
                             diagnostics,
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<T>>>(
                                     diagnostics.builder);
                                 data->predicate = predicate;
                                 data->polarity = true;
                                 return intern_constructor<runir::kr::dl::ConceptTag>(repository, diagnostics.builder, intern(repository, *data).get_index());
                             });
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtomicGoal<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             1,
                             "ConceptAtomicGoal",
                             diagnostics,
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<T>>>(
                                     diagnostics.builder);
                                 data->predicate = predicate;
                                 data->polarity = node.polarity;
                                 return intern_constructor<runir::kr::dl::ConceptTag>(repository, diagnostics.builder, intern(repository, *data).get_index());
                             });
}

template<typename Tag, typename Ast>
auto parse_binary_concept(const Ast& node,
                          tyr::formalism::planning::DomainView domain,
                          runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
                          const ConstructorContext& diagnostics)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, repository, diagnostics);
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, repository, diagnostics);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, Tag>>(diagnostics.builder);
    data->lhs = lhs.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, diagnostics.builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptIntersection<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    return parse_binary_concept<runir::kr::dl::IntersectionTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptUnion<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    return parse_binary_concept<runir::kr::dl::UnionTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptValueRestriction<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    return parse_binary_concept<runir::kr::dl::ValueRestrictionTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptExistentialQuantification<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    return parse_binary_concept<runir::kr::dl::ExistentialQuantificationTag>(node, domain, repository, diagnostics);
}

template<typename Tag, typename Ast>
auto parse_number_restriction_concept(const Ast& node,
                                      tyr::formalism::planning::DomainView domain,
                                      runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
                                      const ConstructorContext& diagnostics)
{
    const auto role = parse_constructor_or_non_terminal(node.role, domain, repository, diagnostics);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, Tag>>(diagnostics.builder);
    data->n = node.n;
    data->role = role.get_index();
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, diagnostics.builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtLeastNumberRestriction<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    return parse_number_restriction_concept<runir::kr::dl::AtLeastNumberRestrictionTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtMostNumberRestriction<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    return parse_number_restriction_concept<runir::kr::dl::AtMostNumberRestrictionTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptExactNumberRestriction<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    return parse_number_restriction_concept<runir::kr::dl::ExactNumberRestrictionTag>(node, domain, repository, diagnostics);
}

template<typename Tag, typename Ast>
auto parse_qualified_number_restriction_concept(const Ast& node,
                                                tyr::formalism::planning::DomainView domain,
                                                runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
                                                const ConstructorContext& diagnostics)
{
    const auto role = parse_constructor_or_non_terminal(node.role, domain, repository, diagnostics);
    const auto concept_view = parse_constructor_or_non_terminal(node.concept_, domain, repository, diagnostics);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, Tag>>(diagnostics.builder);
    data->n = node.n;
    data->role = role.get_index();
    data->concept_ = concept_view.get_index();
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, diagnostics.builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedAtLeastNumberRestriction<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    return parse_qualified_number_restriction_concept<runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedAtMostNumberRestriction<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    return parse_qualified_number_restriction_concept<runir::kr::dl::QualifiedAtMostNumberRestrictionTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedExactNumberRestriction<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    return parse_qualified_number_restriction_concept<runir::kr::dl::QualifiedExactNumberRestrictionTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptRoleValueMap<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    return parse_binary_concept<runir::kr::dl::RoleValueMapTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAgreement<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    return parse_binary_concept<runir::kr::dl::AgreementTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptRoleFillers<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    const auto role = parse_constructor_or_non_terminal(node.role, domain, repository, diagnostics);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::RoleFillersTag>>(diagnostics.builder);
    data->role = role.get_index();
    append_objects(domain, node.object_names, diagnostics, data->objects);
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, diagnostics.builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptOneOf<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::OneOfTag>>(diagnostics.builder);
    append_objects(domain, node.object_names, diagnostics, data->objects);
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, diagnostics.builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptNegation<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    const auto arg = parse_constructor_or_non_terminal(node.arg, domain, repository, diagnostics);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::NegationTag>>(diagnostics.builder);
    data->arg = arg.get_index();
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, diagnostics.builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptNominal<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::NominalTag>>(diagnostics.builder);
    data->object = require_object(domain, node.object_name, diagnostics);
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, diagnostics.builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleUniversal<runir::kr::BaseFamilyTag>&,
           tyr::formalism::planning::DomainView,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::UniversalTag>>(diagnostics.builder);
    return intern_constructor<runir::kr::dl::RoleTag>(repository, diagnostics.builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleAtomicState<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             2,
                             "RoleAtomicState",
                             diagnostics,
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<T>>>(
                                     diagnostics.builder);
                                 data->predicate = predicate;
                                 data->polarity = true;
                                 return intern_constructor<runir::kr::dl::RoleTag>(repository, diagnostics.builder, intern(repository, *data).get_index());
                             });
}

auto parse(const runir::kr::dl::grammar::ast::RoleAtomicGoal<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             2,
                             "RoleAtomicGoal",
                             diagnostics,
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<T>>>(
                                     diagnostics.builder);
                                 data->predicate = predicate;
                                 data->polarity = node.polarity;
                                 return intern_constructor<runir::kr::dl::RoleTag>(repository, diagnostics.builder, intern(repository, *data).get_index());
                             });
}

template<typename Tag, typename Ast>
auto parse_binary_role(const Ast& node,
                       tyr::formalism::planning::DomainView domain,
                       runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
                       const ConstructorContext& diagnostics)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, repository, diagnostics);
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, repository, diagnostics);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::BaseFamilyTag, Tag>>(diagnostics.builder);
    data->lhs = lhs.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::RoleTag>(repository, diagnostics.builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleIntersection<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    return parse_binary_role<runir::kr::dl::IntersectionTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::RoleUnion<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    return parse_binary_role<runir::kr::dl::UnionTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::RoleComposition<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    return parse_binary_role<runir::kr::dl::CompositionTag>(node, domain, repository, diagnostics);
}

template<typename Tag, typename Ast>
auto parse_unary_role(const Ast& node,
                      tyr::formalism::planning::DomainView domain,
                      runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
                      const ConstructorContext& diagnostics)
{
    const auto arg = parse_constructor_or_non_terminal(node.arg, domain, repository, diagnostics);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::BaseFamilyTag, Tag>>(diagnostics.builder);
    data->arg = arg.get_index();
    return intern_constructor<runir::kr::dl::RoleTag>(repository, diagnostics.builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleComplement<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    return parse_unary_role<runir::kr::dl::ComplementTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::RoleInverse<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    return parse_unary_role<runir::kr::dl::InverseTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::RoleTransitiveClosure<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    return parse_unary_role<runir::kr::dl::TransitiveClosureTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::RoleReflexiveTransitiveClosure<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    return parse_unary_role<runir::kr::dl::ReflexiveTransitiveClosureTag>(node, domain, repository, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::RoleRestriction<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, repository, diagnostics);
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, repository, diagnostics);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::RestrictionTag>>(diagnostics.builder);
    data->lhs = lhs.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::RoleTag>(repository, diagnostics.builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleIdentity<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    const auto arg = parse_constructor_or_non_terminal(node.arg, domain, repository, diagnostics);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::IdentityTag>>(diagnostics.builder);
    data->arg = arg.get_index();
    return intern_constructor<runir::kr::dl::RoleTag>(repository, diagnostics.builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::BooleanAtomicState<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             0,
                             "BooleanAtomicState",
                             diagnostics,
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 auto data = runir::kr::dl::checkout<runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<T>>>(
                                     diagnostics.builder);
                                 data->predicate = predicate;
                                 data->polarity = node.polarity;
                                 return intern_constructor<runir::kr::dl::BooleanTag>(repository, diagnostics.builder, intern(repository, *data).get_index());
                             });
}

auto parse(const runir::kr::dl::grammar::ast::BooleanAtomicGoal<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             0,
                             "BooleanAtomicGoal",
                             diagnostics,
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 auto data = runir::kr::dl::checkout<runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<T>>>(
                                     diagnostics.builder);
                                 data->predicate = predicate;
                                 data->polarity = node.polarity;
                                 return intern_constructor<runir::kr::dl::BooleanTag>(repository, diagnostics.builder, intern(repository, *data).get_index());
                             });
}

auto parse(const runir::kr::dl::grammar::ast::BooleanNonempty<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    const auto arg = boost::apply_visitor(
        [&](const auto& value) -> ygg::Data<runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::NonemptyTag>>::ConstructorVariant
        { return parse_constructor_or_non_terminal(unwrap(value), domain, repository, diagnostics).get_index(); },
        node.arg.get());

    auto data = runir::kr::dl::checkout<runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::NonemptyTag>>(diagnostics.builder);
    data->arg = arg;
    return intern_constructor<runir::kr::dl::BooleanTag>(repository, diagnostics.builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::NumericalCount<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    const auto arg = boost::apply_visitor(
        [&](const auto& value) -> ygg::Data<runir::kr::dl::Numerical<runir::kr::BaseFamilyTag, runir::kr::dl::CountTag>>::ConstructorVariant
        { return parse_constructor_or_non_terminal(unwrap(value), domain, repository, diagnostics).get_index(); },
        node.arg.get());

    auto data = runir::kr::dl::checkout<runir::kr::dl::Numerical<runir::kr::BaseFamilyTag, runir::kr::dl::CountTag>>(diagnostics.builder);
    data->arg = arg;
    return intern_constructor<runir::kr::dl::NumericalTag>(repository, diagnostics.builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::NumericalDistance<runir::kr::BaseFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
           const ConstructorContext& diagnostics)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, repository, diagnostics);
    const auto mid = parse_constructor_or_non_terminal(node.mid, domain, repository, diagnostics);
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, repository, diagnostics);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Numerical<runir::kr::BaseFamilyTag, runir::kr::dl::DistanceTag>>(diagnostics.builder);
    data->lhs = lhs.get_index();
    data->mid = mid.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::NumericalTag>(repository, diagnostics.builder, intern(repository, *data).get_index());
}

template<runir::kr::dl::CategoryTag Category>
auto parse_constructor(const runir::kr::dl::grammar::ast::Constructor<runir::kr::BaseFamilyTag, Category>& node,
                       tyr::formalism::planning::DomainView domain,
                       runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository,
                       const ConstructorContext& diagnostics)
{
    return boost::apply_visitor([&](const auto& arg) { return parse(unwrap(arg), domain, repository, diagnostics); }, node.get());
}

using BooleanFeatureMap = std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>>>;
using NumericalFeatureMap = std::unordered_map<std::string, ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>>>;

auto parse_feature(const runir::kr::ps::base::dl::ast::BooleanFeature<runir::kr::BaseFamilyTag>& node,
                   tyr::formalism::planning::DomainView domain,
                   Repository& repository,
                   BuildContext& builders,
                   BooleanFeatureMap& boolean_features,
                   NumericalFeatureMap&,
                   ygg::Data<runir::kr::ps::base::Sketch>& sketch_data,
                   const DiagnosticContext& diagnostics)
{
    const auto constructor = parse_constructor(node.feature, domain, repository.get_dl_repository(), ConstructorContext { diagnostics, builders.dl });
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

auto parse_feature(const runir::kr::ps::base::dl::ast::NumericalFeature<runir::kr::BaseFamilyTag>& node,
                   tyr::formalism::planning::DomainView domain,
                   Repository& repository,
                   BuildContext& builders,
                   BooleanFeatureMap&,
                   NumericalFeatureMap& numerical_features,
                   ygg::Data<runir::kr::ps::base::Sketch>& sketch_data,
                   const DiagnosticContext& diagnostics)
{
    const auto constructor = parse_constructor(node.feature, domain, repository.get_dl_repository(), ConstructorContext { diagnostics, builders.dl });
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
                     const DiagnosticContext& diagnostics)
{
    const auto it = features.find(name.text);
    if (it == features.end())
        diagnostics.throw_at(name, runir::kr::UndefinedSymbolError("feature", name.text));
    return it->second;
}

template<typename FeatureTag, typename ObservationTag>
auto make_condition(ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, FeatureTag>> feature, Repository& repository, PsBuilder& builder)
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
auto make_effect(ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, FeatureTag>> feature, Repository& repository, PsBuilder& builder)
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

auto parse_condition_observation(const runir::kr::ps::base::dl::ast::Positive&,
                                 const runir::kr::parser::ast::Identifier& feature,
                                 Repository& repository,
                                 PsBuilder& builder,
                                 const BooleanFeatureMap& boolean_features,
                                 const NumericalFeatureMap&,
                                 const DiagnosticContext& diagnostics)
{
    return make_condition<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>(require_feature(boolean_features, feature, diagnostics),
                                                                                          repository,
                                                                                          builder);
}

auto parse_condition_observation(const runir::kr::ps::base::dl::ast::Negative&,
                                 const runir::kr::parser::ast::Identifier& feature,
                                 Repository& repository,
                                 PsBuilder& builder,
                                 const BooleanFeatureMap& boolean_features,
                                 const NumericalFeatureMap&,
                                 const DiagnosticContext& diagnostics)
{
    return make_condition<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>(require_feature(boolean_features, feature, diagnostics),
                                                                                          repository,
                                                                                          builder);
}

auto parse_condition_observation(const runir::kr::ps::base::dl::ast::EqualZero&,
                                 const runir::kr::parser::ast::Identifier& feature,
                                 Repository& repository,
                                 PsBuilder& builder,
                                 const BooleanFeatureMap&,
                                 const NumericalFeatureMap& numerical_features,
                                 const DiagnosticContext& diagnostics)
{
    return make_condition<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::EqualZero>(require_feature(numerical_features, feature, diagnostics),
                                                                                             repository,
                                                                                             builder);
}

auto parse_condition_observation(const runir::kr::ps::base::dl::ast::GreaterZero&,
                                 const runir::kr::parser::ast::Identifier& feature,
                                 Repository& repository,
                                 PsBuilder& builder,
                                 const BooleanFeatureMap&,
                                 const NumericalFeatureMap& numerical_features,
                                 const DiagnosticContext& diagnostics)
{
    return make_condition<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::GreaterZero>(require_feature(numerical_features, feature, diagnostics),
                                                                                               repository,
                                                                                               builder);
}

auto parse_condition(const runir::kr::ps::base::dl::ast::Condition<runir::kr::BaseFamilyTag>& node,
                     Repository& repository,
                     PsBuilder& builder,
                     const BooleanFeatureMap& boolean_features,
                     const NumericalFeatureMap& numerical_features,
                     const DiagnosticContext& diagnostics)
{
    return boost::apply_visitor(
        [&](const auto& observation)
        { return parse_condition_observation(observation, node.feature, repository, builder, boolean_features, numerical_features, diagnostics); },
        node.observation.get());
}

auto parse_effect_observation(const runir::kr::ps::base::dl::ast::Positive&,
                              const runir::kr::parser::ast::Identifier& feature,
                              Repository& repository,
                              PsBuilder& builder,
                              const BooleanFeatureMap& boolean_features,
                              const NumericalFeatureMap&,
                              const DiagnosticContext& diagnostics)
{
    return make_effect<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>(require_feature(boolean_features, feature, diagnostics),
                                                                                       repository,
                                                                                       builder);
}

auto parse_effect_observation(const runir::kr::ps::base::dl::ast::Negative&,
                              const runir::kr::parser::ast::Identifier& feature,
                              Repository& repository,
                              PsBuilder& builder,
                              const BooleanFeatureMap& boolean_features,
                              const NumericalFeatureMap&,
                              const DiagnosticContext& diagnostics)
{
    return make_effect<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>(require_feature(boolean_features, feature, diagnostics),
                                                                                       repository,
                                                                                       builder);
}

auto parse_effect_observation(const runir::kr::ps::base::dl::ast::Unchanged&,
                              const runir::kr::parser::ast::Identifier& feature,
                              Repository& repository,
                              PsBuilder& builder,
                              const BooleanFeatureMap& boolean_features,
                              const NumericalFeatureMap& numerical_features,
                              const DiagnosticContext& diagnostics)
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

auto parse_effect_observation(const runir::kr::ps::base::dl::ast::Increases&,
                              const runir::kr::parser::ast::Identifier& feature,
                              Repository& repository,
                              PsBuilder& builder,
                              const BooleanFeatureMap&,
                              const NumericalFeatureMap& numerical_features,
                              const DiagnosticContext& diagnostics)
{
    return make_effect<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Increases>(require_feature(numerical_features, feature, diagnostics),
                                                                                          repository,
                                                                                          builder);
}

auto parse_effect_observation(const runir::kr::ps::base::dl::ast::Decreases&,
                              const runir::kr::parser::ast::Identifier& feature,
                              Repository& repository,
                              PsBuilder& builder,
                              const BooleanFeatureMap&,
                              const NumericalFeatureMap& numerical_features,
                              const DiagnosticContext& diagnostics)
{
    return make_effect<runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Decreases>(require_feature(numerical_features, feature, diagnostics),
                                                                                          repository,
                                                                                          builder);
}

auto parse_effect(const runir::kr::ps::base::dl::ast::Effect<runir::kr::BaseFamilyTag>& node,
                  Repository& repository,
                  PsBuilder& builder,
                  const BooleanFeatureMap& boolean_features,
                  const NumericalFeatureMap& numerical_features,
                  const DiagnosticContext& diagnostics)
{
    return boost::apply_visitor(
        [&](const auto& observation)
        { return parse_effect_observation(observation, node.feature, repository, builder, boolean_features, numerical_features, diagnostics); },
        node.observation.get());
}

auto parse_rule(const runir::kr::ps::base::dl::ast::Rule<runir::kr::BaseFamilyTag>& node,
                Repository& repository,
                PsBuilder& builder,
                const BooleanFeatureMap& boolean_features,
                const NumericalFeatureMap& numerical_features,
                const DiagnosticContext& diagnostics)
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
    auto diagnostics = DiagnosticContext {};
    const auto scope = DiagnosticContext::Scope(diagnostics, error_handler);
    auto dl_builder = DlBuilder {};
    auto ps_builder = PsBuilder {};
    auto builders = BuildContext { dl_builder, ps_builder };

    auto boolean_features = BooleanFeatureMap {};
    auto numerical_features = NumericalFeatureMap {};
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
