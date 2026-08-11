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
using DlBuilder = runir::kr::dl::Builder<runir::kr::UnsFamilyTag>;
using UnsBuilder = runir::kr::uns::Builder;

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
auto intern(runir::kr::uns::Repository& repository, ygg::Data<T>& data)
{
    return runir::kr::uns::get_or_create(repository, data).first;
}

template<typename T>
auto intern(runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository, ygg::Data<T>& data)
{
    return runir::kr::dl::get_or_create(repository, data).first;
}

template<runir::kr::dl::CategoryTag Category, typename T>
auto intern_constructor(runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository, DlBuilder& builder, ygg::Index<T> index)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Constructor<runir::kr::UnsFamilyTag, Category>>(builder);
    data->value = index;
    return intern(repository, *data);
}

template<runir::kr::dl::CategoryTag Category>
auto parse_constructor(const runir::kr::dl::grammar::ast::Constructor<runir::kr::UnsFamilyTag, Category>& node,
                       tyr::formalism::planning::DomainView domain,
                       runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
                       DlBuilder& builder,
                       const DiagnosticContext& diagnostics);

template<runir::kr::dl::CategoryTag Category>
auto parse_constructor_or_non_terminal(const runir::kr::dl::grammar::ast::ConstructorOrNonTerminal<runir::kr::UnsFamilyTag, Category>& node,
                                       tyr::formalism::planning::DomainView domain,
                                       runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
                                       DlBuilder& builder,
                                       const DiagnosticContext& diagnostics) -> runir::kr::dl::FamilyConstructorView<runir::kr::UnsFamilyTag, Category>
{
    return boost::apply_visitor(
        [&](const auto& value) -> runir::kr::dl::FamilyConstructorView<runir::kr::UnsFamilyTag, Category>
        {
            const auto& unwrapped = unwrap(value);
            if constexpr (std::same_as<std::remove_cvref_t<decltype(unwrapped)>, runir::kr::dl::grammar::ast::NonTerminal<runir::kr::UnsFamilyTag, Category>>)
                diagnostics.throw_at(unwrapped.name, runir::kr::InvalidExpressionError("Classifier DL features cannot reference grammar nonterminals."));
            else
                return parse_constructor(unwrapped, domain, repository, builder, diagnostics);
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

void append_objects(tyr::formalism::planning::DomainView domain,
                    const std::vector<runir::kr::dl::grammar::ast::Identifier>& names,
                    const DiagnosticContext& diagnostics,
                    ygg::IndexList<tyr::formalism::Object>& result)
{
    result.reserve(result.size() + names.size());
    for (const auto& name : names)
        result.push_back(require_object(domain, name, diagnostics));
}

auto parse(const runir::kr::dl::grammar::ast::ConceptBot<runir::kr::UnsFamilyTag>&,
           tyr::formalism::planning::DomainView,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext&)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::BotTag>>(builder);
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptTop<runir::kr::UnsFamilyTag>&,
           tyr::formalism::planning::DomainView,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext&)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::TopTag>>(builder);
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtomicState<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
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
                                 auto data =
                                     runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<T>>>(builder);
                                 data->predicate = predicate;
                                 data->polarity = true;
                                 return intern_constructor<runir::kr::dl::ConceptTag>(repository, builder, intern(repository, *data).get_index());
                             });
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtomicGoal<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
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
                                 auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<T>>>(builder);
                                 data->predicate = predicate;
                                 data->polarity = node.polarity;
                                 return intern_constructor<runir::kr::dl::ConceptTag>(repository, builder, intern(repository, *data).get_index());
                             });
}

template<typename Tag, typename Ast>
auto parse_binary_concept(const Ast& node,
                          tyr::formalism::planning::DomainView domain,
                          runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
                          DlBuilder& builder,
                          const DiagnosticContext& diagnostics)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, repository, builder, diagnostics);
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, repository, builder, diagnostics);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, Tag>>(builder);
    data->lhs = lhs.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptIntersection<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_binary_concept<runir::kr::dl::IntersectionTag>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptUnion<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_binary_concept<runir::kr::dl::UnionTag>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptValueRestriction<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_binary_concept<runir::kr::dl::ValueRestrictionTag>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptExistentialQuantification<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_binary_concept<runir::kr::dl::ExistentialQuantificationTag>(node, domain, repository, builder, diagnostics);
}

template<typename Tag, typename Ast>
auto parse_number_restriction_concept(const Ast& node,
                                      tyr::formalism::planning::DomainView domain,
                                      runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
                                      DlBuilder& builder,
                                      const DiagnosticContext& diagnostics)
{
    const auto role = parse_constructor_or_non_terminal(node.role, domain, repository, builder, diagnostics);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, Tag>>(builder);
    data->n = node.n;
    data->role = role.get_index();
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtLeastNumberRestriction<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_number_restriction_concept<runir::kr::dl::AtLeastNumberRestrictionTag>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAtMostNumberRestriction<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_number_restriction_concept<runir::kr::dl::AtMostNumberRestrictionTag>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptExactNumberRestriction<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_number_restriction_concept<runir::kr::dl::ExactNumberRestrictionTag>(node, domain, repository, builder, diagnostics);
}

template<typename Tag, typename Ast>
auto parse_qualified_number_restriction_concept(const Ast& node,
                                                tyr::formalism::planning::DomainView domain,
                                                runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
                                                DlBuilder& builder,
                                                const DiagnosticContext& diagnostics)
{
    const auto role = parse_constructor_or_non_terminal(node.role, domain, repository, builder, diagnostics);
    const auto concept_view = parse_constructor_or_non_terminal(node.concept_, domain, repository, builder, diagnostics);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, Tag>>(builder);
    data->n = node.n;
    data->role = role.get_index();
    data->concept_ = concept_view.get_index();
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedAtLeastNumberRestriction<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_qualified_number_restriction_concept<runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedAtMostNumberRestriction<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_qualified_number_restriction_concept<runir::kr::dl::QualifiedAtMostNumberRestrictionTag>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptQualifiedExactNumberRestriction<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_qualified_number_restriction_concept<runir::kr::dl::QualifiedExactNumberRestrictionTag>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptRoleValueMap<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_binary_concept<runir::kr::dl::RoleValueMapTag>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptAgreement<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_binary_concept<runir::kr::dl::AgreementTag>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::ConceptRoleFillers<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    const auto role = parse_constructor_or_non_terminal(node.role, domain, repository, builder, diagnostics);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::RoleFillersTag>>(builder);
    data->role = role.get_index();
    append_objects(domain, node.object_names, diagnostics, data->objects);
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptOneOf<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::OneOfTag>>(builder);
    append_objects(domain, node.object_names, diagnostics, data->objects);
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptNegation<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    const auto arg = parse_constructor_or_non_terminal(node.arg, domain, repository, builder, diagnostics);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::NegationTag>>(builder);
    data->arg = arg.get_index();
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::ConceptNominal<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::NominalTag>>(builder);
    data->object = require_object(domain, node.object_name, diagnostics);
    return intern_constructor<runir::kr::dl::ConceptTag>(repository, builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleUniversal<runir::kr::UnsFamilyTag>&,
           tyr::formalism::planning::DomainView,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext&)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::UniversalTag>>(builder);
    return intern_constructor<runir::kr::dl::RoleTag>(repository, builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleAtomicState<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
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
                                 auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<T>>>(builder);
                                 data->predicate = predicate;
                                 data->polarity = true;
                                 return intern_constructor<runir::kr::dl::RoleTag>(repository, builder, intern(repository, *data).get_index());
                             });
}

auto parse(const runir::kr::dl::grammar::ast::RoleAtomicGoal<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
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
                                 auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<T>>>(builder);
                                 data->predicate = predicate;
                                 data->polarity = node.polarity;
                                 return intern_constructor<runir::kr::dl::RoleTag>(repository, builder, intern(repository, *data).get_index());
                             });
}

template<typename Tag, typename Ast>
auto parse_binary_role(const Ast& node,
                       tyr::formalism::planning::DomainView domain,
                       runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
                       DlBuilder& builder,
                       const DiagnosticContext& diagnostics)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, repository, builder, diagnostics);
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, repository, builder, diagnostics);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::UnsFamilyTag, Tag>>(builder);
    data->lhs = lhs.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::RoleTag>(repository, builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleIntersection<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_binary_role<runir::kr::dl::IntersectionTag>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::RoleUnion<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_binary_role<runir::kr::dl::UnionTag>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::RoleComposition<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_binary_role<runir::kr::dl::CompositionTag>(node, domain, repository, builder, diagnostics);
}

template<typename Tag, typename Ast>
auto parse_unary_role(const Ast& node,
                      tyr::formalism::planning::DomainView domain,
                      runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
                      DlBuilder& builder,
                      const DiagnosticContext& diagnostics)
{
    const auto arg = parse_constructor_or_non_terminal(node.arg, domain, repository, builder, diagnostics);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::UnsFamilyTag, Tag>>(builder);
    data->arg = arg.get_index();
    return intern_constructor<runir::kr::dl::RoleTag>(repository, builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleComplement<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_unary_role<runir::kr::dl::ComplementTag>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::RoleInverse<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_unary_role<runir::kr::dl::InverseTag>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::RoleTransitiveClosure<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_unary_role<runir::kr::dl::TransitiveClosureTag>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::RoleReflexiveTransitiveClosure<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_unary_role<runir::kr::dl::ReflexiveTransitiveClosureTag>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::RoleRestriction<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, repository, builder, diagnostics);
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, repository, builder, diagnostics);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::RestrictionTag>>(builder);
    data->lhs = lhs.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::RoleTag>(repository, builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::RoleIdentity<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    const auto arg = parse_constructor_or_non_terminal(node.arg, domain, repository, builder, diagnostics);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::IdentityTag>>(builder);
    data->arg = arg.get_index();
    return intern_constructor<runir::kr::dl::RoleTag>(repository, builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::BooleanAtomicState<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
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
                                 auto data =
                                     runir::kr::dl::checkout<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<T>>>(builder);
                                 data->predicate = predicate;
                                 data->polarity = node.polarity;
                                 return intern_constructor<runir::kr::dl::BooleanTag>(repository, builder, intern(repository, *data).get_index());
                             });
}

auto parse(const runir::kr::dl::grammar::ast::BooleanAtomicGoal<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
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
                                 auto data = runir::kr::dl::checkout<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<T>>>(builder);
                                 data->predicate = predicate;
                                 data->polarity = node.polarity;
                                 return intern_constructor<runir::kr::dl::BooleanTag>(repository, builder, intern(repository, *data).get_index());
                             });
}

auto parse(const runir::kr::dl::grammar::ast::BooleanNonempty<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    const auto arg = boost::apply_visitor(
        [&](const auto& value) -> ygg::Data<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::NonemptyTag>>::ConstructorVariant
        { return parse_constructor_or_non_terminal(unwrap(value), domain, repository, builder, diagnostics).get_index(); },
        node.arg.get());

    auto data = runir::kr::dl::checkout<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::NonemptyTag>>(builder);
    data->arg = arg;
    return intern_constructor<runir::kr::dl::BooleanTag>(repository, builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::NumericalCount<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    const auto arg =
        boost::apply_visitor([&](const auto& value) -> ygg::Data<runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, runir::kr::dl::CountTag>>::ConstructorVariant
                             { return parse_constructor_or_non_terminal(unwrap(value), domain, repository, builder, diagnostics).get_index(); },
                             node.arg.get());

    auto data = runir::kr::dl::checkout<runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, runir::kr::dl::CountTag>>(builder);
    data->arg = arg;
    return intern_constructor<runir::kr::dl::NumericalTag>(repository, builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::NumericalDistance<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, repository, builder, diagnostics);
    const auto mid = parse_constructor_or_non_terminal(node.mid, domain, repository, builder, diagnostics);
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, repository, builder, diagnostics);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, runir::kr::dl::DistanceTag>>(builder);
    data->lhs = lhs.get_index();
    data->mid = mid.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::NumericalTag>(repository, builder, intern(repository, *data).get_index());
}

template<typename Tag, typename Ast>
auto parse_comparison(const Ast& node,
                      tyr::formalism::planning::DomainView domain,
                      runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
                      DlBuilder& builder,
                      const DiagnosticContext& diagnostics)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, repository, builder, diagnostics);
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, repository, builder, diagnostics);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, Tag>>(builder);
    data->lhs = lhs.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::BooleanTag>(repository, builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::BooleanEq<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_comparison<runir::kr::dl::EqTag<runir::kr::dl::BooleanTag>>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::BooleanNeq<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_comparison<runir::kr::dl::NeqTag<runir::kr::dl::BooleanTag>>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::BooleanLt<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_comparison<runir::kr::dl::LtTag<runir::kr::dl::BooleanTag>>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::BooleanLe<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_comparison<runir::kr::dl::LeTag<runir::kr::dl::BooleanTag>>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::BooleanGt<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_comparison<runir::kr::dl::GtTag<runir::kr::dl::BooleanTag>>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::BooleanGe<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_comparison<runir::kr::dl::GeTag<runir::kr::dl::BooleanTag>>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalEq<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_comparison<runir::kr::dl::EqTag<runir::kr::dl::NumericalTag>>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalNeq<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_comparison<runir::kr::dl::NeqTag<runir::kr::dl::NumericalTag>>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalLt<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_comparison<runir::kr::dl::LtTag<runir::kr::dl::NumericalTag>>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalLe<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_comparison<runir::kr::dl::LeTag<runir::kr::dl::NumericalTag>>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalGt<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_comparison<runir::kr::dl::GtTag<runir::kr::dl::NumericalTag>>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalGe<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_comparison<runir::kr::dl::GeTag<runir::kr::dl::NumericalTag>>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::BooleanConstant<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext&)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanConstantTag>>(builder);
    data->identifier = node.value;
    return intern_constructor<runir::kr::dl::BooleanTag>(repository, builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::NumericalConstant<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext&)
{
    auto data = runir::kr::dl::checkout<runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, runir::kr::dl::NumericalConstantTag>>(builder);
    data->identifier = node.value;
    return intern_constructor<runir::kr::dl::NumericalTag>(repository, builder, intern(repository, *data).get_index());
}

template<typename Tag, typename Ast>
auto parse_numerical_binary(const Ast& node,
                            tyr::formalism::planning::DomainView domain,
                            runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
                            DlBuilder& builder,
                            const DiagnosticContext& diagnostics)
{
    const auto lhs = parse_constructor_or_non_terminal(node.lhs, domain, repository, builder, diagnostics);
    const auto rhs = parse_constructor_or_non_terminal(node.rhs, domain, repository, builder, diagnostics);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, Tag>>(builder);
    data->lhs = lhs.get_index();
    data->rhs = rhs.get_index();
    return intern_constructor<runir::kr::dl::NumericalTag>(repository, builder, intern(repository, *data).get_index());
}

auto parse(const runir::kr::dl::grammar::ast::NumericalAdd<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_numerical_binary<runir::kr::dl::AddTag>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalSub<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_numerical_binary<runir::kr::dl::SubTag>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalMul<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_numerical_binary<runir::kr::dl::MulTag>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalDiv<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_numerical_binary<runir::kr::dl::DivTag>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalMin<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_numerical_binary<runir::kr::dl::MinTag>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::NumericalMax<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_numerical_binary<runir::kr::dl::MaxTag>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::BooleanAnd<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_comparison<runir::kr::dl::AndTag>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::BooleanOr<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    return parse_comparison<runir::kr::dl::OrTag>(node, domain, repository, builder, diagnostics);
}

auto parse(const runir::kr::dl::grammar::ast::BooleanNot<runir::kr::UnsFamilyTag>& node,
           tyr::formalism::planning::DomainView domain,
           runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
           DlBuilder& builder,
           const DiagnosticContext& diagnostics)
{
    const auto arg = parse_constructor_or_non_terminal(node.arg, domain, repository, builder, diagnostics);
    auto data = runir::kr::dl::checkout<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::NotTag>>(builder);
    data->arg = arg.get_index();
    return intern_constructor<runir::kr::dl::BooleanTag>(repository, builder, intern(repository, *data).get_index());
}

template<runir::kr::dl::CategoryTag Category>
auto parse_constructor(const runir::kr::dl::grammar::ast::Constructor<runir::kr::UnsFamilyTag, Category>& node,
                       tyr::formalism::planning::DomainView domain,
                       runir::kr::dl::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>& repository,
                       DlBuilder& builder,
                       const DiagnosticContext& diagnostics)
{
    return boost::apply_visitor([&](const auto& arg) { return parse(unwrap(arg), domain, repository, builder, diagnostics); }, node.get());
}

auto parse_feature(const runir::kr::uns::dl::ast::BooleanFeature& node,
                   tyr::formalism::planning::DomainView domain,
                   Repository& repository,
                   DlBuilder& dl_builder,
                   UnsBuilder& builder,
                   const DiagnosticContext& diagnostics)
{
    const auto feature = parse_constructor(node.feature, domain, repository.get_dl_repository(), dl_builder, diagnostics);
    auto concrete_data = runir::kr::uns::checkout<runir::kr::uns::dl::Feature>(builder);
    concrete_data->feature = feature.get_index();
    concrete_data->symbol = node.symbol.text;
    const auto concrete = intern(repository, *concrete_data);

    auto data = runir::kr::uns::checkout<runir::kr::uns::Feature>(builder);
    data->value = concrete.get_index();
    return intern(repository, *data);
}

}  // namespace

ClassifierView parse_classifier(const std::string& description, tyr::formalism::planning::DomainView domain, Repository& repository)
{
    auto dl_builder = DlBuilder {};
    auto builder = UnsBuilder {};
    auto first = description.cbegin();
    const auto last = description.cend();
    auto ast = runir::kr::uns::dl::ast::Classifier {};
    auto errors = std::ostringstream {};
    auto error_handler = runir::kr::parser::ErrorHandlerType(first, last, errors);
    if (!runir::kr::parser::parse_full(first, last, parser::classifier_root_parser(), ast, error_handler))
        throw runir::kr::parser::DiagnosticContext::parse_error(error_handler, "Failed to parse DL classifier description.", first);
    auto diagnostics = DiagnosticContext {};
    const auto scope = DiagnosticContext::Scope(diagnostics, error_handler);

    auto data = runir::kr::uns::checkout<runir::kr::uns::Classifier>(builder);
    data->symbol = ast.symbol.text;
    data->features.reserve(ast.features.size());
    data->clauses.reserve(ast.expression.clauses.size());
    auto symbol_to_feature = std::unordered_map<std::string, ygg::Index<runir::kr::uns::Feature>> {};
    symbol_to_feature.reserve(ast.features.size());

    for (const auto& feature : ast.features)
    {
        const auto wrapper = parse_feature(feature, domain, repository, dl_builder, builder, diagnostics);
        if (!symbol_to_feature.emplace(feature.symbol.text, wrapper.get_index()).second)
            diagnostics.throw_at(feature.symbol, runir::kr::DuplicateDefinitionError("feature", feature.symbol.text));
        data->features.push_back(wrapper.get_index());
    }

    const auto parse_literal = [&](const auto& literal)
    {
        using Literal = std::remove_cvref_t<decltype(literal)>;
        const auto it = symbol_to_feature.find(literal.symbol.text);
        if (it == symbol_to_feature.end())
            diagnostics.throw_at(literal.symbol, runir::kr::UndefinedSymbolError("feature", literal.symbol.text));

        constexpr auto polarity = std::same_as<Literal, runir::kr::uns::dl::ast::PositiveLiteral>;
        auto literal_data = runir::kr::uns::checkout<runir::kr::uns::ClassifierLiteral>(builder);
        literal_data->value = it->second;
        literal_data->polarity = polarity;
        return intern(repository, *literal_data).get_index();
    };

    for (const auto& ast_clause : ast.expression.clauses)
    {
        auto clause_data = runir::kr::uns::checkout<runir::kr::uns::ClassifierClause>(builder);
        clause_data->literals.reserve(ast_clause.literals.size());
        for (const auto& ast_literal : ast_clause.literals)
            clause_data->literals.push_back(boost::apply_visitor(parse_literal, ast_literal.get()));
        data->clauses.push_back(intern(repository, *clause_data).get_index());
    }

    return intern(repository, *data);
}

}  // namespace runir::kr::uns::dl
