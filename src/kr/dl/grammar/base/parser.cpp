#include "runir/kr/dl/grammar/parser.hpp"

#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/dl/grammar/canonicalization.hpp"
#include "runir/kr/dl/grammar/constructor_repository.hpp"
#include "runir/kr/dl/grammar/parser/parser.hpp"
#include "runir/kr/parser/diagnostics.hpp"

#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <cista/containers/variant.h>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <tyr/formalism/planning/repository.hpp>
#include <utility>

namespace runir::kr::dl::grammar
{
namespace
{

using DiagnosticContext = runir::kr::parser::DiagnosticContext;
using Repository = ConstructorRepositoryFor<runir::kr::BaseFamilyTag>;

struct ParserState
{
    Repository& repository;
    Builder<runir::kr::BaseFamilyTag> builder;
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
}  // namespace runir::kr::dl::grammar

template<typename T, typename Initialize>
auto intern(ParserState& state, Initialize&& initialize)
{
    auto data = runir::kr::dl::grammar::checkout<T>(state.builder);
    std::forward<Initialize>(initialize)(*data);
    return runir::kr::dl::grammar::get_or_create(state.repository, *data).first;
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename T>
auto intern_constructor(ParserState& state, ygg::Index<T> index)
{
    return intern<Constructor<Family, Category>>(state, [&](auto& data) { data.variant = index; });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename T, typename Initialize>
auto intern_concrete(ParserState& state, Initialize&& initialize)
{
    return intern_constructor<Family, Category>(state, intern<T>(state, std::forward<Initialize>(initialize)).get_index());
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
auto parse(const ast::Constructor<Family, Category>& node,
           tyr::formalism::planning::DomainView domain,
           ParserState& repository,
           const DiagnosticContext& diagnostics);

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
auto parse(const ast::NonTerminal<Family, Category>& node, tyr::formalism::planning::DomainView, ParserState& repository, const DiagnosticContext&)
{
    return intern<NonTerminal<Family, Category>>(repository, [&](auto& data) { data.name = node.name.text; });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
auto parse(const ast::ConstructorOrNonTerminal<Family, Category>& node,
           tyr::formalism::planning::DomainView domain,
           ParserState& repository,
           const DiagnosticContext& diagnostics)
{
    const auto index = boost::apply_visitor([&](const auto& arg) -> ygg::Data<ConstructorOrNonTerminal<Family, Category>>::Variant
                                            { return parse(unwrap(arg), domain, repository, diagnostics).get_index(); },
                                            node.get());

    return intern<ConstructorOrNonTerminal<Family, Category>>(repository, [&](auto& data) { data.variant = index; });
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

void append_objects(tyr::formalism::planning::DomainView domain,
                    const std::vector<ast::Identifier>& names,
                    const DiagnosticContext& diagnostics,
                    ygg::IndexList<tyr::formalism::Object>& objects)
{
    for (const auto& name : names)
        objects.push_back(require_object(domain, name, diagnostics));
}

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptBot<Family>&, tyr::formalism::planning::DomainView, ParserState& repository, const DiagnosticContext&)
{
    const auto concrete = intern<Concept<Family, BotTag>>(repository, [](auto&) {});
    return intern_constructor<Family, ConceptTag>(repository, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptTop<Family>&, tyr::formalism::planning::DomainView, ParserState& repository, const DiagnosticContext&)
{
    const auto concrete = intern<Concept<Family, TopTag>>(repository, [](auto&) {});
    return intern_constructor<Family, ConceptTag>(repository, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptAtomicState<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ParserState& repository,
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
                                 const auto concrete = intern<Concept<Family, AtomicStateTag<T>>>(repository,
                                                                                                  [&](auto& data)
                                                                                                  {
                                                                                                      data.predicate = predicate;
                                                                                                      data.polarity = true;
                                                                                                  });
                                 return intern_constructor<Family, ConceptTag>(repository, concrete.get_index());
                             });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptAtomicGoal<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ParserState& repository,
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
                                 const auto concrete = intern<Concept<Family, AtomicGoalTag<T>>>(repository,
                                                                                                 [&](auto& data)
                                                                                                 {
                                                                                                     data.predicate = predicate;
                                                                                                     data.polarity = node.polarity;
                                                                                                 });
                                 return intern_constructor<Family, ConceptTag>(repository, concrete.get_index());
                             });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptIntersection<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ParserState& repository,
           const DiagnosticContext& diagnostics)
{
    const auto lhs = parse(node.lhs, domain, repository, diagnostics).get_index();
    const auto rhs = parse(node.rhs, domain, repository, diagnostics).get_index();
    const auto concrete = intern<Concept<Family, IntersectionTag>>(repository,
                                                                   [&](auto& data)
                                                                   {
                                                                       data.lhs = lhs;
                                                                       data.rhs = rhs;
                                                                   });
    return intern_constructor<Family, ConceptTag>(repository, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptUnion<Family>& node, tyr::formalism::planning::DomainView domain, ParserState& repository, const DiagnosticContext& diagnostics)
{
    const auto lhs = parse(node.lhs, domain, repository, diagnostics).get_index();
    const auto rhs = parse(node.rhs, domain, repository, diagnostics).get_index();
    const auto concrete = intern<Concept<Family, UnionTag>>(repository,
                                                            [&](auto& data)
                                                            {
                                                                data.lhs = lhs;
                                                                data.rhs = rhs;
                                                            });
    return intern_constructor<Family, ConceptTag>(repository, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptNegation<Family>& node, tyr::formalism::planning::DomainView domain, ParserState& repository, const DiagnosticContext& diagnostics)
{
    const auto arg = parse(node.arg, domain, repository, diagnostics).get_index();
    const auto concrete = intern<Concept<Family, NegationTag>>(repository, [&](auto& data) { data.arg = arg; });
    return intern_constructor<Family, ConceptTag>(repository, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptValueRestriction<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ParserState& repository,
           const DiagnosticContext& diagnostics)
{
    const auto lhs = parse(node.lhs, domain, repository, diagnostics).get_index();
    const auto rhs = parse(node.rhs, domain, repository, diagnostics).get_index();
    const auto concrete = intern<Concept<Family, ValueRestrictionTag>>(repository,
                                                                       [&](auto& data)
                                                                       {
                                                                           data.lhs = lhs;
                                                                           data.rhs = rhs;
                                                                       });
    return intern_constructor<Family, ConceptTag>(repository, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptExistentialQuantification<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ParserState& repository,
           const DiagnosticContext& diagnostics)
{
    const auto lhs = parse(node.lhs, domain, repository, diagnostics).get_index();
    const auto rhs = parse(node.rhs, domain, repository, diagnostics).get_index();
    const auto concrete = intern<Concept<Family, ExistentialQuantificationTag>>(repository,
                                                                                [&](auto& data)
                                                                                {
                                                                                    data.lhs = lhs;
                                                                                    data.rhs = rhs;
                                                                                });
    return intern_constructor<Family, ConceptTag>(repository, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptAtLeastNumberRestriction<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ParserState& repository,
           const DiagnosticContext& diagnostics)
{
    const auto role = parse(node.role, domain, repository, diagnostics).get_index();
    const auto concrete = intern<Concept<Family, AtLeastNumberRestrictionTag>>(repository,
                                                                               [&](auto& data)
                                                                               {
                                                                                   data.n = node.n;
                                                                                   data.role = role;
                                                                               });
    return intern_constructor<Family, ConceptTag>(repository, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptAtMostNumberRestriction<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ParserState& repository,
           const DiagnosticContext& diagnostics)
{
    const auto role = parse(node.role, domain, repository, diagnostics).get_index();
    const auto concrete = intern<Concept<Family, AtMostNumberRestrictionTag>>(repository,
                                                                              [&](auto& data)
                                                                              {
                                                                                  data.n = node.n;
                                                                                  data.role = role;
                                                                              });
    return intern_constructor<Family, ConceptTag>(repository, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptExactNumberRestriction<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ParserState& repository,
           const DiagnosticContext& diagnostics)
{
    const auto role = parse(node.role, domain, repository, diagnostics).get_index();
    const auto concrete = intern<Concept<Family, ExactNumberRestrictionTag>>(repository,
                                                                             [&](auto& data)
                                                                             {
                                                                                 data.n = node.n;
                                                                                 data.role = role;
                                                                             });
    return intern_constructor<Family, ConceptTag>(repository, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptQualifiedAtLeastNumberRestriction<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ParserState& repository,
           const DiagnosticContext& diagnostics)
{
    const auto role = parse(node.role, domain, repository, diagnostics).get_index();
    const auto concept_index = parse(node.concept_, domain, repository, diagnostics).get_index();
    const auto concrete = intern<Concept<Family, QualifiedAtLeastNumberRestrictionTag>>(repository,
                                                                                        [&](auto& data)
                                                                                        {
                                                                                            data.n = node.n;
                                                                                            data.role = role;
                                                                                            data.concept_ = concept_index;
                                                                                        });
    return intern_constructor<Family, ConceptTag>(repository, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptQualifiedAtMostNumberRestriction<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ParserState& repository,
           const DiagnosticContext& diagnostics)
{
    const auto role = parse(node.role, domain, repository, diagnostics).get_index();
    const auto concept_index = parse(node.concept_, domain, repository, diagnostics).get_index();
    const auto concrete = intern<Concept<Family, QualifiedAtMostNumberRestrictionTag>>(repository,
                                                                                       [&](auto& data)
                                                                                       {
                                                                                           data.n = node.n;
                                                                                           data.role = role;
                                                                                           data.concept_ = concept_index;
                                                                                       });
    return intern_constructor<Family, ConceptTag>(repository, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptQualifiedExactNumberRestriction<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ParserState& repository,
           const DiagnosticContext& diagnostics)
{
    const auto role = parse(node.role, domain, repository, diagnostics).get_index();
    const auto concept_index = parse(node.concept_, domain, repository, diagnostics).get_index();
    const auto concrete = intern<Concept<Family, QualifiedExactNumberRestrictionTag>>(repository,
                                                                                      [&](auto& data)
                                                                                      {
                                                                                          data.n = node.n;
                                                                                          data.role = role;
                                                                                          data.concept_ = concept_index;
                                                                                      });
    return intern_constructor<Family, ConceptTag>(repository, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptRoleValueMap<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ParserState& repository,
           const DiagnosticContext& diagnostics)
{
    const auto lhs = parse(node.lhs, domain, repository, diagnostics).get_index();
    const auto rhs = parse(node.rhs, domain, repository, diagnostics).get_index();
    const auto concrete = intern<Concept<Family, RoleValueMapTag>>(repository,
                                                                   [&](auto& data)
                                                                   {
                                                                       data.lhs = lhs;
                                                                       data.rhs = rhs;
                                                                   });
    return intern_constructor<Family, ConceptTag>(repository, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptAgreement<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ParserState& repository,
           const DiagnosticContext& diagnostics)
{
    const auto lhs = parse(node.lhs, domain, repository, diagnostics).get_index();
    const auto rhs = parse(node.rhs, domain, repository, diagnostics).get_index();
    const auto concrete = intern<Concept<Family, AgreementTag>>(repository,
                                                                [&](auto& data)
                                                                {
                                                                    data.lhs = lhs;
                                                                    data.rhs = rhs;
                                                                });
    return intern_constructor<Family, ConceptTag>(repository, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptRoleFillers<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ParserState& repository,
           const DiagnosticContext& diagnostics)
{
    const auto role = parse(node.role, domain, repository, diagnostics).get_index();
    const auto concrete = intern<Concept<Family, RoleFillersTag>>(repository,
                                                                  [&](auto& data)
                                                                  {
                                                                      data.role = role;
                                                                      append_objects(domain, node.object_names, diagnostics, data.objects);
                                                                  });
    return intern_constructor<Family, ConceptTag>(repository, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptOneOf<Family>& node, tyr::formalism::planning::DomainView domain, ParserState& repository, const DiagnosticContext& diagnostics)
{
    const auto concrete =
        intern<Concept<Family, OneOfTag>>(repository, [&](auto& data) { append_objects(domain, node.object_names, diagnostics, data.objects); });
    return intern_constructor<Family, ConceptTag>(repository, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::ConceptNominal<Family>& node, tyr::formalism::planning::DomainView domain, ParserState& repository, const DiagnosticContext& diagnostics)
{
    const auto object = require_object(domain, node.object_name, diagnostics);
    const auto concrete = intern<Concept<Family, NominalTag>>(repository, [&](auto& data) { data.object = object; });
    return intern_constructor<Family, ConceptTag>(repository, concrete.get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleUniversal<Family>&, tyr::formalism::planning::DomainView, ParserState& repository, const DiagnosticContext&)
{
    return intern_concrete<Family, RoleTag, Role<Family, UniversalTag>>(repository, [](auto&) {});
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleAtomicState<Family>& node, tyr::formalism::planning::DomainView domain, ParserState& repository, const DiagnosticContext& diagnostics)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             2,
                             "RoleAtomicState",
                             diagnostics,
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 return intern_concrete<Family, RoleTag, Role<Family, AtomicStateTag<T>>>(repository,
                                                                                                          [&](auto& data)
                                                                                                          {
                                                                                                              data.predicate = predicate;
                                                                                                              data.polarity = true;
                                                                                                          });
                             });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleAtomicGoal<Family>& node, tyr::formalism::planning::DomainView domain, ParserState& repository, const DiagnosticContext& diagnostics)
{
    return resolve_predicate(domain,
                             node.predicate_name,
                             2,
                             "RoleAtomicGoal",
                             diagnostics,
                             [&](auto tag, auto predicate)
                             {
                                 using T = decltype(tag);
                                 return intern_concrete<Family, RoleTag, Role<Family, AtomicGoalTag<T>>>(repository,
                                                                                                         [&](auto& data)
                                                                                                         {
                                                                                                             data.predicate = predicate;
                                                                                                             data.polarity = node.polarity;
                                                                                                         });
                             });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleIntersection<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ParserState& repository,
           const DiagnosticContext& diagnostics)
{
    const auto lhs = parse(node.lhs, domain, repository, diagnostics).get_index();
    const auto rhs = parse(node.rhs, domain, repository, diagnostics).get_index();
    return intern_concrete<Family, RoleTag, Role<Family, IntersectionTag>>(repository,
                                                                           [&](auto& data)
                                                                           {
                                                                               data.lhs = lhs;
                                                                               data.rhs = rhs;
                                                                           });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleUnion<Family>& node, tyr::formalism::planning::DomainView domain, ParserState& repository, const DiagnosticContext& diagnostics)
{
    const auto lhs = parse(node.lhs, domain, repository, diagnostics).get_index();
    const auto rhs = parse(node.rhs, domain, repository, diagnostics).get_index();
    return intern_concrete<Family, RoleTag, Role<Family, UnionTag>>(repository,
                                                                    [&](auto& data)
                                                                    {
                                                                        data.lhs = lhs;
                                                                        data.rhs = rhs;
                                                                    });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleComplement<Family>& node, tyr::formalism::planning::DomainView domain, ParserState& repository, const DiagnosticContext& diagnostics)
{
    const auto arg = parse(node.arg, domain, repository, diagnostics).get_index();
    return intern_concrete<Family, RoleTag, Role<Family, ComplementTag>>(repository, [&](auto& data) { data.arg = arg; });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleInverse<Family>& node, tyr::formalism::planning::DomainView domain, ParserState& repository, const DiagnosticContext& diagnostics)
{
    const auto arg = parse(node.arg, domain, repository, diagnostics).get_index();
    return intern_concrete<Family, RoleTag, Role<Family, InverseTag>>(repository, [&](auto& data) { data.arg = arg; });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleComposition<Family>& node, tyr::formalism::planning::DomainView domain, ParserState& repository, const DiagnosticContext& diagnostics)
{
    const auto lhs = parse(node.lhs, domain, repository, diagnostics).get_index();
    const auto rhs = parse(node.rhs, domain, repository, diagnostics).get_index();
    return intern_concrete<Family, RoleTag, Role<Family, CompositionTag>>(repository,
                                                                          [&](auto& data)
                                                                          {
                                                                              data.lhs = lhs;
                                                                              data.rhs = rhs;
                                                                          });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleTransitiveClosure<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ParserState& repository,
           const DiagnosticContext& diagnostics)
{
    const auto arg = parse(node.arg, domain, repository, diagnostics).get_index();
    return intern_concrete<Family, RoleTag, Role<Family, TransitiveClosureTag>>(repository, [&](auto& data) { data.arg = arg; });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleReflexiveTransitiveClosure<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ParserState& repository,
           const DiagnosticContext& diagnostics)
{
    const auto arg = parse(node.arg, domain, repository, diagnostics).get_index();
    return intern_concrete<Family, RoleTag, Role<Family, ReflexiveTransitiveClosureTag>>(repository, [&](auto& data) { data.arg = arg; });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleRestriction<Family>& node, tyr::formalism::planning::DomainView domain, ParserState& repository, const DiagnosticContext& diagnostics)
{
    const auto lhs = parse(node.lhs, domain, repository, diagnostics).get_index();
    const auto rhs = parse(node.rhs, domain, repository, diagnostics).get_index();
    return intern_concrete<Family, RoleTag, Role<Family, RestrictionTag>>(repository,
                                                                          [&](auto& data)
                                                                          {
                                                                              data.lhs = lhs;
                                                                              data.rhs = rhs;
                                                                          });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::RoleIdentity<Family>& node, tyr::formalism::planning::DomainView domain, ParserState& repository, const DiagnosticContext& diagnostics)
{
    const auto arg = parse(node.arg, domain, repository, diagnostics).get_index();
    return intern_concrete<Family, RoleTag, Role<Family, IdentityTag>>(repository, [&](auto& data) { data.arg = arg; });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::BooleanAtomicState<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ParserState& repository,
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
                                 return intern_concrete<Family, BooleanTag, Boolean<Family, AtomicStateTag<T>>>(repository,
                                                                                                                [&](auto& data)
                                                                                                                {
                                                                                                                    data.predicate = predicate;
                                                                                                                    data.polarity = node.polarity;
                                                                                                                });
                             });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::BooleanAtomicGoal<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ParserState& repository,
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
                                 return intern_concrete<Family, BooleanTag, Boolean<Family, AtomicGoalTag<T>>>(repository,
                                                                                                               [&](auto& data)
                                                                                                               {
                                                                                                                   data.predicate = predicate;
                                                                                                                   data.polarity = node.polarity;
                                                                                                               });
                             });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::BooleanNonempty<Family>& node, tyr::formalism::planning::DomainView domain, ParserState& repository, const DiagnosticContext& diagnostics)
{
    const auto arg = boost::apply_visitor([&](const auto& value) -> ygg::Data<Boolean<Family, NonemptyTag>>::Arg
                                          { return parse(unwrap(value), domain, repository, diagnostics).get_index(); },
                                          node.arg.get());

    return intern_concrete<Family, BooleanTag, Boolean<Family, NonemptyTag>>(repository, [&](auto& data) { data.arg = arg; });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::NumericalCount<Family>& node, tyr::formalism::planning::DomainView domain, ParserState& repository, const DiagnosticContext& diagnostics)
{
    const auto arg = boost::apply_visitor([&](const auto& value) -> ygg::Data<Numerical<Family, CountTag>>::Arg
                                          { return parse(unwrap(value), domain, repository, diagnostics).get_index(); },
                                          node.arg.get());

    return intern_concrete<Family, NumericalTag, Numerical<Family, CountTag>>(repository, [&](auto& data) { data.arg = arg; });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
auto parse(const ast::NumericalDistance<Family>& node,
           tyr::formalism::planning::DomainView domain,
           ParserState& repository,
           const DiagnosticContext& diagnostics)
{
    const auto lhs = parse(node.lhs, domain, repository, diagnostics).get_index();
    const auto mid = parse(node.mid, domain, repository, diagnostics).get_index();
    const auto rhs = parse(node.rhs, domain, repository, diagnostics).get_index();
    return intern_concrete<Family, NumericalTag, Numerical<Family, DistanceTag>>(repository,
                                                                                 [&](auto& data)
                                                                                 {
                                                                                     data.lhs = lhs;
                                                                                     data.mid = mid;
                                                                                     data.rhs = rhs;
                                                                                 });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
auto parse(const ast::Constructor<Family, Category>& node,
           tyr::formalism::planning::DomainView domain,
           ParserState& repository,
           const DiagnosticContext& diagnostics)
{
    return boost::apply_visitor([&](const auto& arg) { return parse(unwrap(arg), domain, repository, diagnostics); }, node.get());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
auto parse(const ast::DerivationRule<Family, Category>& node,
           tyr::formalism::planning::DomainView domain,
           ParserState& repository,
           const DiagnosticContext& diagnostics)
{
    return intern<DerivationRule<Family, Category>>(repository,
                                                    [&](auto& data)
                                                    {
                                                        for (const auto& symbol : node.rhs)
                                                            data.rhs.push_back(parse(symbol, domain, repository, diagnostics).get_index());
                                                        data.lhs = parse(node.lhs, domain, repository, diagnostics).get_index();
                                                    });
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
void append_derivation_rule(ygg::IndexList<DerivationRule<Family, Category>>& rules,
                            const ast::DerivationRule<Family, Category>& node,
                            tyr::formalism::planning::DomainView domain,
                            ParserState& repository,
                            const DiagnosticContext& diagnostics)
{
    rules.push_back(parse(node, domain, repository, diagnostics).get_index());
}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::FamilyTag Family>
void append_derivation_rule(ygg::Data<GrammarTag<Family>>& data,
                            const ast::DerivationRuleVariant<Family>& node,
                            tyr::formalism::planning::DomainView domain,
                            ParserState& repository,
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
auto parse_fixed_start(tyr::formalism::planning::DomainView domain, ParserState& repository, const DiagnosticContext& diagnostics)
{
    ast::NonTerminal<runir::kr::BaseFamilyTag, Category> start;
    start.name.text = fixed_start_name<Category>();
    return parse(start, domain, repository, diagnostics).get_index();
}

auto parse_grammar_impl(const std::string& description, tyr::formalism::planning::DomainView domain, Repository& repository)
{
    auto errors = std::ostringstream {};
    auto error_handler = runir::kr::parser::ErrorHandlerType(description.cbegin(), description.cend(), errors);
    auto ast = runir::kr::dl::grammar::ast::Grammar<runir::kr::BaseFamilyTag> {};
    parser::parse_grammar_ast(description, ast, error_handler);
    auto diagnostics = DiagnosticContext {};
    const auto scope = DiagnosticContext::Scope(diagnostics, error_handler);
    auto state = ParserState { repository, {} };

    auto data = runir::kr::dl::grammar::checkout<GrammarTag<runir::kr::BaseFamilyTag>>(state.builder);

    data->concept_start = parse_fixed_start<ConceptTag>(domain, state, diagnostics);
    data->role_start = parse_fixed_start<RoleTag>(domain, state, diagnostics);
    data->boolean_start = parse_fixed_start<BooleanTag>(domain, state, diagnostics);
    data->numerical_start = parse_fixed_start<NumericalTag>(domain, state, diagnostics);

    for (const auto& rule : ast.rules)
        append_derivation_rule(*data, rule, domain, state, diagnostics);

    data->domain = domain.get_index();
    return runir::kr::dl::grammar::get_or_create(repository, *data).first;
}

}  // namespace

FamilyGrammarView<runir::kr::BaseFamilyTag> parse_grammar(const std::string& description, tyr::formalism::planning::DomainView domain, Repository& repository)
{
    return parse_grammar_impl(description, domain, repository);
}

}  // namespace runir::kr::dl::grammar
